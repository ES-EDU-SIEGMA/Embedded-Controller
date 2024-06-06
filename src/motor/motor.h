#ifndef SIEGMA_MOTOR_H
#define SIEGMA_MOTOR_H

#include "tmc2209.h"
#include <stdint.h>


/// struct containing information (pins, stepper driver, direction, Serial address) for a motor
typedef struct Motor {
    TMC2209_t tmc2209;
    SerialAddress_t address;
    int direction;
} Motor_t;

typedef enum motorAddress {
    MOTOR_ADDRESS_0 = 0,
    MOTOR_ADDRESS_1 = 1,
    MOTOR_ADDRESS_2 = 2,
    MOTOR_ADDRESS_3 = 3,
} motorAddress_t;

void initializeAndActivateMotorsEnablePin();

/*! initializes a new motor
 *
 * @param motor   the new initialized motor
 * @param address serial address of the motor (for more information:
 * https://github.com/janelia-arduino/TMC2209)
 * @param uart    uart pins to be used
 */
void setUpMotor(Motor_t *motor, SerialAddress_t address); // serialUart_t uart

/*! Move the motor up
 *
 * @param motor motor to be moved
 */
/*void moveMotorUp(Motor_t *motor);*/
void moveMotorUp(motorAddress_t address);

/*! Move the motor up to Stop under the Dispenser
 *
 * @param motor motor to be moved
 */
void moveMotorUpSlowSpeed(motorAddress_t address);

/*! Move the motor down
 *
 * @param motor motor to be moved
 */
/*void moveMotorDown(Motor_t *motor);*/
void moveMotorDown(motorAddress_t address);

/*! Stop the motor
 *
 * @param motor motor to be stopped
 */
/*void stopMotor(Motor_t *motor);*/
void stopMotor(motorAddress_t address);

/*! Create a new motor and initialize its pins
 *
 * @param address serial address of the stepper driver (for more information:
 * https://github.com/janelia-arduino/TMC2209)
 * @param uart    uart pins to be used
 * @return        motor struct of the new motor
 */
Motor_t createMotor(motorAddress_t address);

/*! Check if the motor is capable of communication over uart
 *
 * @param motor motor to be checked
 * @return      true if the motor communicates, otherwise false
 */
bool motorIsCommunicating(motorAddress_t address);

Motor_t *getMotor(motorAddress_t address);

uint16_t motorGetTorque(motorAddress_t address);

#endif // SIEGMA_MOTOR_H
