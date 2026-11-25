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

extern volatile uint8_t movimiento_colision_flag;
extern volatile uint8_t movimiento_reset_flag;
extern volatile uint8_t movimiento_uartTimeout_flag;

uint8_t movimiento_joystickCambioModo_flag;

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

uint8_t movimiento_cambioModo_flag;

uint8_t movimiento_uartComando_flag;
uart_estado_t uart_estado_siguiente = UART_PARADO;
uint8_t uart_velocidad_movimiento = 0;

static movimiento_estado_t mov_estado = MOVIMIENTO_INIT;
static uart_estado_t uart_estado_actual = UART_PARADO;
static joystick_estado_t joystick_estado = JOYSTICK_INIT;

static motor_t motor_superior_izquierda;
static motor_t motor_inferior_izquierda;
static motor_t motor_inferior_derecha;
static motor_t motor_superior_derecha;

void UART_Actualizar();

movimiento_estado_t MOVIMIENTO_Get_movimiento_estado() { return mov_estado; }
uart_estado_t MOVIMIENTO_Get_uart_estado() { return uart_estado_actual; }
joystick_estado_t MOVIMIENTO_Get_joystick_estado() { return joystick_estado; }


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

    MOTOR_create(&htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, &motor_superior_izquierda);
    MOTOR_create(&htim3, TIM_CHANNEL_4, TIM_CHANNEL_3, &motor_inferior_izquierda);
    MOTOR_create(&htim4, TIM_CHANNEL_2, TIM_CHANNEL_1, &motor_inferior_derecha);
    MOTOR_create(&htim4, TIM_CHANNEL_4, TIM_CHANNEL_3, &motor_superior_derecha);

    break;

  case MOVIMIENTO_UART:
    if(movimiento_cambioModo_flag == 1)
    {
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
      movimiento_cambioModo_flag = 0;
      mov_estado = MOVIMIENTO_JOYSTICK;
    }
    else
    {
      UART_Actualizar();
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
  movimiento_union_t movimiento = {0};

  switch (uart_estado_actual)
  {
  case UART_PARADO:
    // COMPLETAR: desactivamos el timer del timeout
    // COMPLETAR: paramos el coche
    break;

  case UART_IZQUIERDA:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.left = 1;
    break;

  case UART_TANQUE_IZQUIERDA:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.tank_left = 1;
    break;

  case UART_DELANTE:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.forward = 1;
    break;

  case UART_ATRAS:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.backward = 1;
    break;

  case UART_DERECHA:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.right = 1;
    break;

  case UART_TANQUE_DERECHA:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.tank_right = 1;
    break;

  default:
    uart_estado_actual = UART_PARADO;
    break;
  }

  if(movimiento.raw == 0)
  {
    MOTOR_update(&motor_superior_izquierda, 0, MOTOR_FORWARD);
    MOTOR_update(&motor_superior_derecha, 0, MOTOR_FORWARD);
    MOTOR_update(&motor_inferior_izquierda, 0, MOTOR_FORWARD);
    MOTOR_update(&motor_inferior_derecha, 0, MOTOR_FORWARD);
  }
  else
  {
    //Motor superior izquierda
    if(movimiento.forward || movimiento.right || movimiento.tank_right)
      MOTOR_update(&motor_superior_izquierda, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_superior_izquierda, uart_velocidad_movimiento, MOTOR_BACKWARD);

    //Motor superior derecha
    if(movimiento.forward || movimiento.left || movimiento.tank_left)
      MOTOR_update(&motor_superior_derecha, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_superior_derecha, uart_velocidad_movimiento, MOTOR_BACKWARD);

    //Motor inferior izquierda
    if(movimiento.forward || movimiento.left || movimiento.tank_right)
      MOTOR_update(&motor_inferior_izquierda, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_inferior_izquierda, uart_velocidad_movimiento, MOTOR_BACKWARD);

    //Motor inferior derecha
    if(movimiento.forward || movimiento.right || movimiento.tank_left)
      MOTOR_update(&motor_inferior_derecha, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_inferior_derecha, uart_velocidad_movimiento, MOTOR_BACKWARD);
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

void MOVIMIENTO_UART_CambiarEstado(uart_estado_t nuevo, uint8_t speed)
{
  movimiento_uartComando_flag = 1;
  uart_estado_siguiente = nuevo;
  uart_velocidad_movimiento = speed;
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
