/** @file motor.c
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#include "main.h"
#include "motor.h"

// Funciones privadas
static inline void MOTOR_Stop_Pwm(TIM_HandleTypeDef * timer, uint8_t channel);
static inline void MOTOR_Set_Pwm(TIM_HandleTypeDef * timer, uint8_t channel, uint16_t pulse);


void MOTOR_Create(TIM_HandleTypeDef * timer, uint8_t channel_forwards, uint8_t channel_backwards, motor_t * output)
{
  output->timer = timer;
  output->channel_forwards = channel_forwards;
  output->channel_backwards = channel_backwards;
  output->speed = 0;
  output->direction = 0;

  MOTOR_Stop_Pwm(timer, channel_forwards);
  MOTOR_Stop_Pwm(timer, channel_backwards);
}

void MOTOR_Update(motor_t * motor, uint16_t speed, motor_direction_t direction)
{
  if(motor->speed == speed && motor->direction == direction)
    return;

  if(direction == MOTOR_FORWARD)
  {
    MOTOR_Stop_Pwm(motor->timer, motor->channel_backwards);
    MOTOR_Set_Pwm(motor->timer, motor->channel_forwards, speed);
  }
  else
  {
    MOTOR_Stop_Pwm(motor->timer, motor->channel_forwards);
    MOTOR_Set_Pwm(motor->timer, motor->channel_backwards, speed);
  }

  motor->speed = speed;
  motor->direction = direction;
  return;
}

static inline void MOTOR_Stop_Pwm(TIM_HandleTypeDef * timer, uint8_t channel)
{
  __HAL_TIM_SetCompare(timer, channel, 0);
  HAL_TIM_PWM_Stop(timer, channel);
}

static inline void MOTOR_Set_Pwm(TIM_HandleTypeDef * timer, uint8_t channel, uint16_t pulse)
{
    __HAL_TIM_SetCompare(timer, channel, pulse);
    __HAL_TIM_SetCounter(timer, 0);
    HAL_TIM_PWM_Start(timer, channel);
}
