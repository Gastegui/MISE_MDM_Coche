/*
 * motor.h
 *
 *  Created on: 25 nov 2026
 *      Author: ehu
 */

#ifndef MOTOR_MOTOR_H_
#define MOTOR_MOTOR_H_

typedef enum
{
  MOTOR_FORWARD,
  MOTOR_BACKWARD
}motor_direction;

typedef struct
{
  TIM_HandleTypeDef* timer;
  uint8_t channel_forwards;
  uint8_t channel_backwards;
  uint8_t speed;
  motor_direction direction;
}motor_t;

void MOTOR_create(TIM_HandleTypeDef* timer, uint8_t channel_forwards, uint8_t channel_backwards, motor_t* output);

uint8_t MOTOR_update(motor_t* motor, uint8_t speed, motor_direction direction);

#endif /* MOTOR_MOTOR_H_ */
