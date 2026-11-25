/** @file motor.c
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#include "main.h"
#include "motor.h"

static inline void stop_pwm(TIM_HandleTypeDef* timer, uint8_t channel)
{
  __HAL_TIM_SetCompare(timer, channel, 0);
  HAL_TIM_PWM_Stop(timer, channel);
}

static inline void set_pwm(TIM_HandleTypeDef* timer, uint8_t channel, uint16_t pulse)
{
    __HAL_TIM_SetCompare(timer, channel, pulse);
    __HAL_TIM_SetCounter(timer, 0);
    HAL_TIM_PWM_Start(timer, channel);
}

void MOTOR_create(TIM_HandleTypeDef* timer, uint8_t channel_forwards, uint8_t channel_backwards, motor_t* output)
{
  output->timer = timer;
  output->channel_forwards = channel_forwards;
  output->channel_backwards = channel_backwards;
  output->speed = 0;
  output->direction = 0;

  stop_pwm(timer, channel_forwards);
  stop_pwm(timer, channel_backwards);
}

void MOTOR_update(motor_t* motor, uint8_t speed, motor_direction direction)
{
  if(motor->speed == speed && motor->direction == direction)
    return;

  if(direction == MOTOR_FORWARD)
  {
    stop_pwm(motor->timer, motor->channel_backwards);
    set_pwm(motor->timer, motor->channel_forwards, speed * PWM_CONVERSION);
  }
  else
  {
    stop_pwm(motor->timer, motor->channel_forwards);
    set_pwm(motor->timer, motor->channel_backwards, speed * PWM_CONVERSION);
  }

  motor->speed = speed;
  motor->direction = direction;
  return;
}
