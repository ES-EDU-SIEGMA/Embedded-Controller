/** ----------------------------------------------------------------------------
 * Adapted from:
 *  https://github.com/earlephilhower/arduino-pico
 *  https://github.com/earlephilhower/arduino-pico/blob/master/cores/rp2040/serialUartCreate.cpp
 *** ---------------------------------------------------------------------------- */

/*
    Serial-over-UART for the Raspberry Pi Pico RP2040

    Copyright (c) 2021 Earle F. Philhower, III <earlephilhower@yahoo.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "serialUART.h"
#include <hardware/gpio.h>
#include <hardware/irq.h>
#include <hardware/uart.h>
#include <stdlib.h>

static serialUart_t serialUart;

serialUart_t serialUartCreate(uart_inst_t *uart, uint8_t tx, uint8_t rx) {
    serialUart._uart = uart;
    serialUart._tx = tx;
    serialUart._rx = rx;
    serialUart._rts = rx;
    serialUart._cts = tx;
    serialUart._running = false;
    serialUart._polling = false;
    serialUart._fifoSize = 32;
    return serialUart;
}

static void serialUartUart0Irq();

static void serialUartUart1Irq();

void serialUartBegin(unsigned long baud, uint16_t config) {
    if (serialUart._running) {
        serialUartEnd();
    }
    serialUart._overflow = false;
    serialUart._queue = malloc(serialUart._fifoSize);
    serialUart._baud = baud;
    uart_init(serialUart._uart, baud);
    int bits, stop;
    uart_parity_t parity;
    switch (config & SERIAL_PARITY_MASK) {
    case SERIAL_PARITY_EVEN:
        parity = UART_PARITY_EVEN;
        break;
    case SERIAL_PARITY_ODD:
        parity = UART_PARITY_ODD;
        break;
    default:
        parity = UART_PARITY_NONE;
        break;
    }
    switch (config & SERIAL_STOP_BIT_MASK) {
    case SERIAL_STOP_BIT_1:
        stop = 1;
        break;
    default:
        stop = 2;
        break;
    }
    switch (config & SERIAL_DATA_MASK) {
    case SERIAL_DATA_5:
        bits = 5;
        break;
    case SERIAL_DATA_6:
        bits = 6;
        break;
    case SERIAL_DATA_7:
        bits = 7;
        break;
    default:
        bits = 8;
        break;
    }
    uart_set_format(serialUart._uart, bits, stop, parity);
    serialUart._fcnTx = gpio_get_function(serialUart._tx);
    serialUart._fcnRx = gpio_get_function(serialUart._rx);
    gpio_set_function(serialUart._tx, GPIO_FUNC_UART);
    gpio_set_function(serialUart._rx, GPIO_FUNC_UART);
    if (serialUart._rts != UART_PIN_NOT_DEFINED) {
        serialUart._fcnRts = gpio_get_function(serialUart._rts);
        gpio_set_function(serialUart._rts, GPIO_FUNC_UART);
    }
    if (serialUart._cts != UART_PIN_NOT_DEFINED) {
        serialUart._fcnCts = gpio_get_function(serialUart._cts);
        gpio_set_function(serialUart._cts, GPIO_FUNC_UART);
    }
    uart_set_hw_flow(serialUart._uart, serialUart._rts != UART_PIN_NOT_DEFINED,
                     serialUart._cts != UART_PIN_NOT_DEFINED);
    serialUart._writer = 0;
    serialUart._reader = 0;

    if (!serialUart._polling) {
        if (serialUart._uart == uart0) {
            irq_set_exclusive_handler(UART0_IRQ, serialUartUart0Irq);
            irq_set_enabled(UART0_IRQ, true);
        } else {
            irq_set_exclusive_handler(UART1_IRQ, serialUartUart1Irq);
            irq_set_enabled(UART1_IRQ, true);
        }
        // Set the IRQ enables and FIFO level to minimum
        uart_set_irq_enables(serialUart._uart, true, false);
    } else {
        // Polling mode has no IRQs used
    }
    serialUart._running = true;
}

void serialUartEnd() {
    if (!serialUart._running) {
        return;
    }
    serialUart._running = false;
    if (!serialUart._polling) {
        if (serialUart._uart == uart0) {
            irq_set_enabled(UART0_IRQ, false);
        } else {
            irq_set_enabled(UART1_IRQ, false);
        }
    }

    uart_deinit(serialUart._uart);
    free(serialUart._queue);

    // Restore pin functions
    gpio_set_function(serialUart._tx, serialUart._fcnTx);
    gpio_set_function(serialUart._rx, serialUart._fcnRx);
    if (serialUart._rts != UART_PIN_NOT_DEFINED) {
        gpio_set_function(serialUart._rts, serialUart._fcnRts);
    }
    if (serialUart._cts != UART_PIN_NOT_DEFINED) {
        gpio_set_function(serialUart._cts, serialUart._fcnCts);
    }
}

void __not_in_flash_func(serialUartHandleIrq)(bool inIRQ) {
    // ICR is write-to-clear
    uart_get_hw(serialUart._uart)->icr = UART_UARTICR_RTIC_BITS | UART_UARTICR_RXIC_BITS;
    while (uart_is_readable(serialUart._uart)) {
        int val = uart_getc(serialUart._uart);
        uint8_t next_writer = serialUart._writer + 1;
        if (next_writer == serialUart._fifoSize) {
            next_writer = 0;
        }
        if (next_writer != serialUart._reader) {
            serialUart._queue[serialUart._writer] = val;
            asm volatile(
                "" ::
                    : "memory"); // Ensure the queue is written before the written count advances
            // Avoid using division or mod because the HW divider could be in use
            serialUart._writer = next_writer;
        } else {
            serialUart._overflow = true;
        }
    }
}

void serialUartPumpFifo() {
    int irqno = (serialUart._uart == uart0) ? UART0_IRQ : UART1_IRQ;
    bool enabled = irq_is_enabled(irqno);
    irq_set_enabled(irqno, false);
    serialUartHandleIrq(false);
    irq_set_enabled(irqno, enabled);
}

int serialUartRead() {
    if (serialUart._polling) {
        serialUartHandleIrq(false);
    } else {
        serialUartPumpFifo();
    }
    if (serialUart._writer != serialUart._reader) {
        int ret = serialUart._queue[serialUart._reader];
        // asm volatile("":: : "memory"); // Ensure the value is read before advancing
        uint8_t next_reader = (serialUart._reader + 1) % serialUart._fifoSize;
        // asm volatile("":: : "memory"); // Ensure the reader value is only written once, correctly
        serialUart._reader = next_reader;
        return ret;
    }
    return -1;
}

bool serialUartOverflow() {
    bool hold = serialUart._overflow;
    serialUart._overflow = false;
    return hold;
}

uint8_t serialUartAvailable() {
    if (serialUart._polling) {
        serialUartHandleIrq(false);
    } else {
        serialUartPumpFifo();
    }
    return (serialUart._fifoSize + serialUart._writer - serialUart._reader) % serialUart._fifoSize;
}

size_t serialUartWrite(uint8_t c) {
    if (serialUart._polling) {
        serialUartHandleIrq(false);
    }
    uart_putc_raw(serialUart._uart, c);
    return 1;
}

static void __not_in_flash_func(serialUartUart0Irq)() {
    serialUartHandleIrq(true);
}

static void __not_in_flash_func(serialUartUart1Irq)() {
    serialUartHandleIrq(true);
}
