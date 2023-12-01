#define SOURCE_FILE "MOTOR"

#include "motor.h"
#include "common.h"
#include <hardware/gpio.h>
#include <pico/time.h>

static dispenserState_t sleepState_t = (dispenserState_t){.function = &sleepState};
static dispenserState_t upState_t = (dispenserState_t){.function = &upState};
static dispenserState_t topState_t = (dispenserState_t){.function = &topState};
static dispenserState_t downState_t = (dispenserState_t){.function = &downState};
static dispenserState_t errorState_t = (dispenserState_t){.function = &errorState};

static uint8_t counterTorque = 0;
static uint16_t torque = 0;
void setUpEnablePin(Motor_t *motor, SerialAddress_t id) {
    switch (id) {
    case 0:
        motor->enablePin = MOTOR_ENABLE_PINT_0;
        break;
    case 1:
        motor->enablePin = MOTOR_ENABLE_PINT_1;
        break;
    case 2:
        motor->enablePin = MOTOR_ENABLE_PINT_2;
        break;
    case 3:
        motor->enablePin = MOTOR_ENABLE_PINT_3;
        break;
    default:
        motor->enablePin = 0;
    }
    gpio_init(motor->enablePin);
    gpio_set_dir(motor->enablePin, GPIO_OUT);
}

void setUpMotor(Motor_t *motor, SerialAddress_t address, serialUart_t uart) {
    setUpEnablePin(motor, address);
    disableMotorByPin(motor);

    TMC2209_setup(&motor->tmc2209, uart, SERIAL_BAUD_RATE, address);

    while (!TMC2209_isSetupAndCommunicating(&motor->tmc2209)) {
        if (TMC2209_disabledByInputPin(&motor->tmc2209)) {
            PRINT_DEBUG("Setup: Stepper driver with address %u DISABLED by input pin!", address)
        }
        PRINT_DEBUG("Setup: Stepper driver with address %u NOT communicating and setup!", address)
        TMC2209_setup(&motor->tmc2209, uart, SERIAL_BAUD_RATE, address);
        sleep_ms(500);
    }

    PRINT_DEBUG("Setup: Stepper driver with address %u communicating and setup!", address)
    TMC2209_setRunCurrent(&motor->tmc2209, 100);
    TMC2209_setHoldCurrent(&motor->tmc2209, 50);
    TMC2209_enable(&motor->tmc2209);

    motor->direction = DIRECTION_UP;
}

bool motorIsCommunicating(Motor_t *motor) {
    return TMC2209_isSetupAndCommunicating(&motor->tmc2209);
}

void motorDoStep(dispenser_t *dispenser) {
    if (!motorIsCommunicating(&(dispenser->motor))) {
        dispenser->state = errorState_t;
    }
    dispenser->state = dispenser->state.function(dispenser);
}

static dispenserState_t errorState(dispenser_t *dispenser) {
    setUpMotor(&dispenser->motor, dispenser->address, dispenser->uart);
    if (motorIsCommunicating(&dispenser->motor)) {
        disableMotorByPin(&dispenser->motor);
        dispenser->haltSteps = 0;
        return sleepState_t;
    }
    return errorState_t;
}

static dispenserState_t sleepState(dispenser_t *dispenser) {
    if (dispenser->haltSteps > 0) {
        enableMotorByPin(&dispenser->motor);
        moveMotorUp(&dispenser->motor);
        return upState_t;
    }
    return sleepState_t;
}

static dispenserState_t upState(dispenser_t *dispenser) {
    torque = TMC2209_getStallGuardResult(&dispenser->motor.tmc2209);
    PRINT_DEBUG("upState")
    PRINT_DEBUG("Torque: %i", torque)
    PRINT_DEBUG("%i", dispenser->stepsDone)

    // If the torque is below 10 twice in a row, stop
    if (torque < 10){
        counterTorque++;
        if (counterTorque == 2){
            PRINT_DEBUG("detect Top Position")
            stopMotor(&dispenser->motor);
            counterTorque = 0;
            dispenser->stepsUp = dispenser->stepsDone;
            return topState_t;
        }
    }
    else counterTorque = 0;

    if (!limitSwitchIsClosed(dispenser->limitSwitch)) {
        dispenser->stepsDone++;
    }
    return upState_t;
}

static dispenserState_t topState(dispenser_t *dispenser) {
    PRINT_DEBUG("topState")
    if (dispenser->stepsDone >
        dispenser->stepsUp + 2 * dispenser->othersTriggered + dispenser->haltSteps) {
        moveMotorDown(&dispenser->motor);
        return downState_t;
    }
    dispenser->stepsDone++;
    return topState_t;
}

static dispenserState_t downState(dispenser_t *dispenser) {
    PRINT_DEBUG("downState")
    if (limitSwitchIsClosed(dispenser->limitSwitch)) {
        stopMotor(&dispenser->motor);
        disableMotorByPin(&dispenser->motor);
        dispenser->haltSteps = 0;
        return sleepState_t;
    }
    dispenser->stepsDone++;
    return downState_t;
}

bool dispenserAllInSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser) {
    for (uint8_t i = 0; i < number_of_dispenser; ++i) {
        if (dispenser[i].state.function != sleepState_t.function) {
            return false;
        }
    }
    return true;
}
void enableMotorByPin(Motor_t *motor) {
    gpio_pull_down(motor->enablePin);
}

void disableMotorByPin(Motor_t *motor) {
    gpio_pull_up(motor->enablePin);
}
//enable and disable should be simplified

Motor_t createMotor(SerialAddress_t address, serialUart_t uart) {
    Motor_t motor = {.address = address};
    setUpMotor(&motor, address, uart);
    return motor;
}

void moveMotorUp(Motor_t *motor) {
    // TODO: Why double function calls?
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED);
}

void moveMotorDown(Motor_t *motor) {
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * -MOTOR_DOWN_SPEED);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * -MOTOR_DOWN_SPEED);
}

void stopMotor(Motor_t *motor) {
    TMC2209_moveAtVelocity(&motor->tmc2209, 0);
    TMC2209_moveAtVelocity(&motor->tmc2209, 0);
}
//maybe we should change the names of TMC2209_moveAtVelocity
