/** @file movimiento.c
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#include "movimiento.h"
#include "main.h"
#include <stdint.h>

extern volatile uint8_t movimiento_cambioModo_flag;
extern volatile uint8_t movimiento_colision_flag;
extern volatile uint8_t movimiento_reset_flag;

static movimiento_estado_t mov_estado = MOVIMIENTO_INIT;

/*!
* @brief Actualiza la maquina de estados del movimiento del coche.
*/
void MOVIMIENTO_Actualizar()
{
  switch(mov_estado)
  {
  case MOVIMIENTO_INIT:
    HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    mov_estado = MOVIMIENTO_UART;
    break;
  case MOVIMIENTO_UART:
    if(movimiento_cambioModo_flag == 1)
    {
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
      movimiento_cambioModo_flag = 0;
      mov_estado = MOVIMIENTO_JOYSTICK;
    }
    if(movimiento_colision_flag)
    {
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
      movimiento_colision_flag = 0;
      mov_estado = MOVIMIENTO_COLISION;
    }
    break;

  case MOVIMIENTO_JOYSTICK:
    if(movimiento_cambioModo_flag == 1)
    {
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
      movimiento_cambioModo_flag = 0;
      mov_estado = MOVIMIENTO_UART;
    }
    if(movimiento_colision_flag)
    {
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
      movimiento_colision_flag = 0;
      mov_estado = MOVIMIENTO_COLISION;
    }
    break;

  case MOVIMIENTO_COLISION:
    if(movimiento_reset_flag == 1)
    {
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
      movimiento_reset_flag = 0;
      mov_estado = MOVIMIENTO_UART;
    }
    break;

  default:
    mov_estado = MOVIMIENTO_UART;
    break;
  }
}

/*** end of file ***/
