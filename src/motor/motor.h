#ifndef SIEGMA_MOTOR_H
#define SIEGMA_MOTOR_H

#include "serialUART.h"
#include "tmc2209.h"
#include <stdint.h>

#define MOTOR_ENABLE_PINT_0 2
#define MOTOR_ENABLE_PINT_1 3
#define MOTOR_ENABLE_PINT_2 6
#define MOTOR_ENABLE_PINT_3 7

#ifndef MOTOR_UP_SPEED
//! current hardware only allows '>=120000', without deformation
#define MOTOR_UP_SPEED 100000
#endif
#ifndef MOTOR_DOWN_SPEED
//! current dispenser only allows '>=100000', without problems with refilling
#define MOTOR_DOWN_SPEED 100000
#endif
#define DIRECTION_UP 1
#define DIRECTION_DOWN (-1)

/// struct containing information (pins, stepper driver, direction, Serial address) for a motor
typedef struct Motor {
    uint8_t enablePin;
    TMC2209_t tmc2209;
    SerialAddress_t address;
    int direction;
} Motor_t;

/*! initializes a new motor
 *
 * @param motor   the new initialized motor
 * @param address serial address of the motor (for more information: https://github.com/janelia-arduino/TMC2209)
 * @param uart    uart pins to be used
 */
void setUpMotor(Motor_t *motor, SerialAddress_t address, serialUart_t uart);

/*! Enable the motor
 *
 * @param motor motor to be enabled
 */
void enableMotorByPin(Motor_t *motor);

/*! Disable the motor
 *
 * @param motor motor to be disabled
 */
void disableMotorByPin(Motor_t *motor);

/*! Move the motor up
 *
 * @param motor motor to be moved
 */
void moveMotorUp(Motor_t *motor);

/*! Move the motor down
 *
 * @param motor motor to be moved
 */
void moveMotorDown(Motor_t *motor);

/*! Stop the motor
 *
 * @param motor motor to be stopped
 */
void stopMotor(Motor_t *motor);

/*! Create a new motor and initialize its pins
 *
 * @param address serial address of the stepper driver (for more information: https://github.com/janelia-arduino/TMC2209)
 * @param uart    uart pins to be used
 * @return        motor struct of the new motor
 */
Motor_t createMotor(SerialAddress_t address, serialUart_t uart);

/*! Check if the motor is capable of communication over uart
 *
 * @param motor motor to be checked
 * @return      true if the motor communicates, otherwise false
 */
bool motorIsCommunicating(Motor_t *motor);

#endif // SIEGMA_MOTOR_H
