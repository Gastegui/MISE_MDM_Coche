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
extern volatile uint8_t movimiento_uartComando_flag;
extern volatile uint8_t movimiento_uartTimeout_flag;

uint8_t movimiento_joystickCambioModo_flag;

static movimiento_estado_t mov_estado = MOVIMIENTO_INIT;
static uart_estado_t uart_estado_actual = UART_PARADO;
static uart_estado_t uart_estado_siguiente = UART_PARADO;
static joystick_estado_t joystick_estado = JOYSTICK_GIRO;

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

/* Acción al entrar:  */
static void UART_Entrar()
{
  uart_estado_actual = uart_estado_siguiente;
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  switch (uart_estado_actual)
  {
  case UART_PARADO:
    // COMPLETAR: desactivamos el timer del timeout
    // COMPLETAR: paramos el coche
    break;

  case UART_IZQUIERDA:
    // COMPLETAR: activamos/actualizamos el timeout
    // COMPLETAR: movemos el coche hacia la izquierda
    break;

  case UART_TANQUE_IZQUIERDA:
    // COMPLETAR: activamos/actualizamos el timeout
    // COMPLETAR: giramos el coche sobre si mismo a la izquierda
    break;

  case UART_DELANTE:
    // COMPLETAR: activamos/actualizamos el timeout
    // COMPLETAR: movemos el coche hacia delante
    break;

  case UART_ATRAS:
    // COMPLETAR: activamos/actualizamos el timeout
    // COMPLETAR: movemos el coche hacia atras
    break;

  case UART_DERECHA:
    // COMPLETAR: activamos/actualizamos el timeout
    // COMPLETAR: movemos el coche hacia la derecha
    break;

  case UART_TANQUE_DERECHA:
    // COMPLETAR: activamos/actualizamos el timeout
    // COMPLETAR: giramos el coche sobre si mismo a la derecha
    break;

  default:
    uart_estado_actual = UART_PARADO;
    break;
  }
}

void UART_Actualizar()
{
  if(movimiento_uartComando_flag == 1)
  {
    UART_Entrar();
    movimiento_uartComando_flag = 0;
    movimiento_uartTimeout_flag = 0;
  }
  else if(movimiento_uartTimeout_flag == 1)
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    uart_estado_actual = UART_PARADO;
    movimiento_uartTimeout_flag = 0;
  }
}

void JOYSTICK_Actualizar()
{
  if(movimiento_joystickCambioModo_flag == 1)
  {
    movimiento_joystickCambioModo_flag = 0;

    if(joystick_estado == JOYSTICK_GIRO)
    {
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
      joystick_estado = JOYSTICK_DIAGONAL;
    }
    else
    {
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
      joystick_estado = JOYSTICK_GIRO;
    }
  }

  switch(joystick_estado)
  {
  case JOYSTICK_GIRO:
    // COMPLETAR: mover el coche en modo giro en funcion de la entrada del joystick
    break;
  case JOYSTICK_DIAGONAL:
    // COMPLETAR: mover el coche en modo diagonal en funcion de la entrada del joystick
    break;
  default:
    joystick_estado = JOYSTICK_GIRO;
    break;
  }
}

/*** end of file ***/
