/** @file motor.h
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#ifndef MOTOR_MOTOR_H
#define MOTOR_MOTOR_H

typedef enum
{
  MOTOR_FORWARD,
  MOTOR_BACKWARD
} motor_direction_t;

typedef struct
{
  TIM_HandleTypeDef * timer;
  uint8_t channel_forwards;
  uint8_t channel_backwards;
  uint16_t speed;
  motor_direction_t direction;
} motor_t;

void MOTOR_Create(TIM_HandleTypeDef * timer, uint8_t channel_forwards, uint8_t channel_backwards, motor_t * output);
void MOTOR_Update(motor_t * motor, uint16_t speed, motor_direction_t direction);

#endif /* MOTOR_MOTOR_H */

/*** end of file ***/
