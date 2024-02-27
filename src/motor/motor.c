#define SOURCE_FILE "MOTOR"

#include "motor.h"
#include "common.h"
#include <hardware/gpio.h>
#include <pico/time.h>

#define SERIAL_UART SERIAL2 /// The uart Pins to be used

static uint16_t torque = 0;
static uint8_t enablePin;
Motor_t motors[4];

void initializeAndActivateMotorsEnablePin() {
    enablePin = MOTOR_ENABLE_PINT;
    gpio_init(enablePin);
    gpio_set_dir(enablePin, GPIO_OUT);
    gpio_pull_down(enablePin);
}

void setUpMotor(Motor_t *motor, SerialAddress_t address) {

    TMC2209_setupByMotor(&motor->tmc2209, address);

    while (!TMC2209_isSetupAndCommunicating(&motor->tmc2209)) {
        if (TMC2209_disabledByInputPin(&motor->tmc2209)) {
            PRINT_DEBUG("Setup: Stepper driver with address %u DISABLED by input pin!", address)
        }
        PRINT_DEBUG("Setup: Stepper driver with address %u NOT communicating and setup!", address)
        TMC2209_setupByMotor(&motor->tmc2209, address);
        sleep_ms(500);
    }

    PRINT_DEBUG("Setup: Stepper driver with address %u communicating and setup!", address)
    TMC2209_setRunCurrent(&motor->tmc2209, 100);
    TMC2209_setHoldCurrent(&motor->tmc2209, 50);
    TMC2209_enable(&motor->tmc2209);

    motor->direction = DIRECTION_UP;
}

bool motorIsCommunicating(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    return TMC2209_isSetupAndCommunicating(&motor->tmc2209);
}

Motor_t *getMotor(motorAddress_t address) {
    if (address >= 0 && address < 5) {
        return &motors[address];
    } else {
        PRINT_DEBUG("Didnt find motor")
        // Handle invalid motorNum
        return NULL;
    }
}

Motor_t createMotor(motorAddress_t address) {
    motors[address].address = (int)address;
    setUpMotor(&motors[address], (int)address);
    return motors[address];
}

uint16_t motorGetTorque(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    torque = TMC2209_getStallGuardResult(&motor->tmc2209);
    return torque;
}

void moveMotorUp(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED);
}

void moveMotorUpSlowSpeed(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED_SLOW);
}

void moveMotorDown(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * -MOTOR_DOWN_SPEED);
}

void stopMotor(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, 0);
}
