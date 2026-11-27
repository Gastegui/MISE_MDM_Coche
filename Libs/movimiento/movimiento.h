/** @file movimiento.h
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

#include <stdint.h>

typedef enum
{
  MOVIMIENTO_INIT = 0,
  MOVIMIENTO_UART,
  MOVIMIENTO_JOYSTICK,
  MOVIMIENTO_COLISION
} movimiento_estado_t;

typedef enum
{
  UART_PARADO = 0,
  UART_IZQUIERDA,
  UART_TANQUE_IZQUIERDA,
  UART_DELANTE,
  UART_ATRAS,
  UART_DERECHA,
  UART_TANQUE_DERECHA
} uart_estado_t;

typedef union
{
  uint8_t raw;
  struct
  {
    uint8_t forward : 1;
    uint8_t backward : 1;
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t tank_left : 1;
    uint8_t tank_right : 1;
    uint8_t unused : 2;
  };
} movimiento_union_t;

typedef enum
{
  JOYSTICK_GIRO = 0,
  JOYSTICK_DIAGONAL
} joystick_estado_t;

void MOVIMIENTO_Actualizar();
void MOVIMIENTO_UART_CambiarEstado(uart_estado_t nuevo, uint16_t speed);

movimiento_estado_t MOVIMIENTO_Get_MovimientoEstado();
uart_estado_t MOVIMIENTO_Get_UartEstado();
joystick_estado_t MOVIMIENTO_Get_JoystickEstado();


int16_t MOVIMIENTO_Get_Joystick_X();
int16_t MOVIMIENTO_Get_Joystick_Y();
void MOVIMIENTO_Set_Joystick_X(uint16_t valor);
void MOVIMIENTO_Set_Joystick_Y(uint16_t valor);

#endif /* MOVIMIENTO_H */

/*** end of file ***/
