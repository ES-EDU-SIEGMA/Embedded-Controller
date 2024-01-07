#define SOURCE_FILE "MOTOR"

#include "motor.h"
#include "common.h"
#include <hardware/gpio.h>
#include <pico/time.h>
#define SERIAL_UART SERIAL2 /// The uart Pins to be used

static uint16_t torque = 0;
Motor_t motors[4];

void initializeMotorsEnablePin(Motor_t *motor, SerialAddress_t id) {
    switch (id) {
    case 0:
        motor[id].enablePin = MOTOR_ENABLE_PINT_0;
        break;
    case 1:
        motor[id].enablePin = MOTOR_ENABLE_PINT_1;
        break;
    case 2:
        motor[id].enablePin = MOTOR_ENABLE_PINT_2;
        break;
    case 3:
        motor[id].enablePin = MOTOR_ENABLE_PINT_3;
        break;
    default:
        motor[id].enablePin = 0;
    }
    gpio_init(motor[id].enablePin);
    gpio_set_dir(motor[id].enablePin, GPIO_OUT);
}

void setUpMotor(Motor_t *motor, SerialAddress_t address) {
    //setUpEnablePin(motor, address);
    initializeMotorsEnablePin(motor, address);
    disableMotorByPin((int)address);

    TMC2209_setupByMotor(&motor->tmc2209,address);

    while (!TMC2209_isSetupAndCommunicating(&motor->tmc2209)) {
        if (TMC2209_disabledByInputPin(&motor->tmc2209)) {
            PRINT_DEBUG("Setup: Stepper driver with address %u DISABLED by input pin!", address)
        }
        PRINT_DEBUG("Setup: Stepper driver with address %u NOT communicating and setup!", address)
        TMC2209_setupByMotor(&motor->tmc2209,address);
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

void enableMotorByPin(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    gpio_pull_down(motor->enablePin);
}

void disableMotorByPin(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    gpio_pull_up(motor->enablePin);
}

Motor_t* getMotor(motorAddress_t address) {
    if (address >= 0 && address < 5) {
        return &motors[address];
    } else {
        PRINT_DEBUG("Didnt find motor")
        // Handle invalid motorNum
        return NULL;
    }
}

Motor_t createMotor(motorAddress_t address) {
    Motor_t motor = {.address = (int)address};
    setUpMotor(&motor, (int)address);
    return motor;
}

/*void moveMotorUp(Motor_t *motor) {
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
}*/

uint16_t motorGetTorque(motorAddress_t address){
    Motor_t *motor = getMotor(address);
    torque = TMC2209_getStallGuardResult(&motor->tmc2209);
    return torque;
}

void moveMotorUp(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED);
}

void moveMotorUpSlowSpeed(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED_SLOW);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * MOTOR_UP_SPEED_SLOW);
}

void moveMotorDown(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * -MOTOR_DOWN_SPEED);
    TMC2209_moveAtVelocity(&motor->tmc2209, motor->direction * -MOTOR_DOWN_SPEED);
}

void stopMotor(motorAddress_t address) {
    Motor_t *motor = getMotor(address);
    TMC2209_moveAtVelocity(&motor->tmc2209, 0);
    TMC2209_moveAtVelocity(&motor->tmc2209, 0);
}
//maybe we should change the names of TMC2209_moveAtVelocity
