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


void MOVIMIENTO_Actualizar();

#endif /* MOVIMIENTO_H */

/*** end of file ***/
