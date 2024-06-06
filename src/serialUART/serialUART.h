#ifndef SIEGMA_SERIAL_UART_H
#define SIEGMA_SERIAL_UART_H

#include <hardware/gpio.h>
#include <hardware/uart.h>

#include <stdbool.h>
#include <stdint.h>

#define SERIAL_PARITY_EVEN (0x1ul)
#define SERIAL_PARITY_ODD (0x2ul)
#define SERIAL_PARITY_NONE (0x3ul)
#define SERIAL_PARITY_MARK (0x4ul)
#define SERIAL_PARITY_SPACE (0x5ul)
#define SERIAL_PARITY_MASK (0xFul)

#define SERIAL_STOP_BIT_1 (0x10ul)
#define SERIAL_STOP_BIT_1_5 (0x20ul)
#define SERIAL_STOP_BIT_2 (0x30ul)
#define SERIAL_STOP_BIT_MASK (0xF0ul)

#define SERIAL_DATA_5 (0x100ul)
#define SERIAL_DATA_6 (0x200ul)
#define SERIAL_DATA_7 (0x300ul)
#define SERIAL_DATA_8 (0x400ul)
#define SERIAL_DATA_MASK (0xF00ul)

#define SERIAL_8N1 (SERIAL_STOP_BIT_1 | SERIAL_PARITY_NONE | SERIAL_DATA_8)

#define UART_PIN_NOT_DEFINED (255u)

#define SERIAL_BAUD_RATE 115200

/* region Serial 0 */
#define PIN_SERIAL0_TX (0u)
#define PIN_SERIAL0_RX (1u)

#define SERIAL0 serialUartCreate(uart0, PIN_SERIAL0_TX, PIN_SERIAL0_RX)
/* endregion Serial 0 */
/* region Serial 1 */
#define PIN_SERIAL1_TX (4u)
#define PIN_SERIAL1_RX (5u)

#define SERIAL1 serialUartCreate(uart1, PIN_SERIAL1_TX, PIN_SERIAL1_RX)
/* endregion Serial 1 */

struct serialUart {
    bool _running; // set to false by init
    uart_inst_t *_uart;
    uint8_t _tx, _rx;
    uint8_t _rts, _cts;
    enum gpio_function _fcnTx, _fcnRx, _fcnRts, _fcnCts;
    unsigned long _baud;
    bool _polling; // set to false by init
    bool _overflow;

    // Lockless, IRQ-handled circular queue
    uint32_t _writer;
    uint32_t _reader;
    uint8_t _fifoSize; // set to 32 by init
    uint8_t *_queue;
};
typedef struct serialUart serialUart_t;

void serialUartBegin(unsigned long baud, uint16_t config);

void serialUartEnd();

int serialUartRead();

uint8_t serialUartAvailable();

size_t serialUartWrite(uint8_t c);

serialUart_t serialUartCreate(uart_inst_t *uart, uint8_t tx, uint8_t rx);

#endif // SIEGMA_SERIAL_UART_H
