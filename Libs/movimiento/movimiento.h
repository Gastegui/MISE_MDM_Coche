/** @file movimiento.h
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

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

typedef enum
{
  JOYSTICK_INIT = 0,
  JOYSTICK_GIRO,
  JOYSTICK_DIAGONAL
} joystick_estado_t;

void MOVIMIENTO_Actualizar();
void UART_Actualizar();
void JOYSTICK_Actualizar();

#endif /* MOVIMIENTO_H */

/*** end of file ***/
