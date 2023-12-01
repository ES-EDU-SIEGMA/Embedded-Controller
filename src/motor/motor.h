#ifndef SIEGMA_MOTOR_H
#define SIEGMA_MOTOR_H

#include "serialUART.h"
#include "tmc2209.h"
#include <stdint.h>
#include <dispenser.h>

#define MOTOR_ENABLE_PINT_0 2
#define MOTOR_ENABLE_PINT_1 3
#define MOTOR_ENABLE_PINT_2 6
#define MOTOR_ENABLE_PINT_3 7

#ifndef MOTOR_UP_SPEED
//! current hardware only allows '>=120000' for 4 cl Dispenser, without deformation
//! for 3 cl Dispenser '>=100000' is allowed, without deformation
#define MOTOR_UP_SPEED 120000
#endif
#ifndef MOTOR_DOWN_SPEED
//! current dispenser only allows '>=80000', without problems with refilling
#define MOTOR_DOWN_SPEED 80000
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

/*! Dispenser cycles to the next state
 *
 * @param dispenser the Dispenser to take action on
 */
void motorDoStep(dispenser_t *dispenser);
/*! Change dispenser state to the sleep state (wait for new command)
 *
 * @param dispenser dispenser to be set
 * @return          new state (sleep state) of the dispenser
 */
static dispenserState_t sleepState(dispenser_t *dispenser);

/*! Change dispenser state to the up state (drive upwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (up state) of the dispenser
 */
static dispenserState_t upState(dispenser_t *dispenser);

/*! Change dispenser state to the top state (stay in the up position)
 *
 * @param dispenser dispenser to be set
 * @return          new state (top state) of the dispenser
 */
static dispenserState_t topState(dispenser_t *dispenser);

/*! Change dispenser state to the down state (drive downwards)
 *
 * @param dispenser dispenser to be set
 * @return          new state (down state) of the dispenser
 */
static dispenserState_t downState(dispenser_t *dispenser);

/*! Change dispenser state to the error state (no connection to the tmc -> try again)
 *
 * @param dispenser dispenser to be set
 * @return          new state (error state) of the dispenser
 */
static dispenserState_t errorState(dispenser_t *dispenser);

/*! Calculate the time a dispenser needs to reach TopState
 *
 * @param dispenserCL 2Cl or 4Cl Dispenser
 * @return        Time that a dispenser needs to reach TopState
 */

/*! Check if all Dispenser are sleeping
 *
 * @param dispenser           an array holding a reference to all dispenser
 * @param number_of_dispenser the amount of initialized dispenser
 * @return true if all Dispenser are in a sleeping state
 */
bool dispenserAllInSleepState(dispenser_t *dispenser, uint8_t number_of_dispenser);
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
