/**
 * @file movimiento.h
 *
 * @brief Public interface for vehicle movement control.
 *
 * @details Declares the movement, UART and joystick states, as well as the
 *          functions used to update movement control and exchange joystick
 *          data with other modules.
 *
 * @author Julen Gallastegui de la Cruz
 * @author Andrea Nieto Gil
 */

#ifndef MOVIMIENTO_H
#define MOVIMIENTO_H

/* Includes -----------------------------------------------------------------*/

#include <stdint.h>

/* Types --------------------------------------------------------------------*/

/**
 * @brief Vehicle movement control states.
 */
typedef enum
{
  MOVIMIENTO_INIT = 0, /**< Hardware initialization state. */
  MOVIMIENTO_UART,     /**< Movement controlled through UART commands. */
  MOVIMIENTO_JOYSTICK, /**< Movement controlled by the joystick. */
  MOVIMIENTO_COLISION  /**< Movement halted while a collision is handled. */
} movimiento_estado_t;

/**
 * @brief Movement commands accepted from the UART interface.
 */
typedef enum
{
  UART_PARADO = 0,       /**< Stop all motors. */
  UART_IZQUIERDA,        /**< Strafe left. */
  UART_TANQUE_IZQUIERDA, /**< Turn left in place. */
  UART_DELANTE,          /**< Move forward. */
  UART_ATRAS,            /**< Move backward. */
  UART_DERECHA,          /**< Strafe right. */
  UART_TANQUE_DERECHA    /**< Turn right in place. */
} uart_estado_t;

/**
 * @brief Bit-field representation of a movement direction command.
 *
 * @details The raw member provides access to all direction flags as a single
 *          byte; the named members provide access to individual directions.
 */
typedef union
{
  uint8_t raw; /**< Combined direction flags. */
  struct
  {
    uint8_t forward : 1;    /**< Forward direction flag. */
    uint8_t backward : 1;   /**< Backward direction flag. */
    uint8_t left : 1;       /**< Left turn flag. */
    uint8_t right : 1;      /**< Right turn flag. */
    uint8_t tank_left : 1;  /**< Turn-in-place left flag. */
    uint8_t tank_right : 1; /**< Turn-in-place right flag. */
    uint8_t unused : 2;     /**< Reserved bits. */
  };
} movimiento_union_t;

/**
 * @brief Joystick control operating modes.
 */
typedef enum
{
  JOYSTICK_GIRO = 0, /**< Separate rotation and forward/backward control. */
  JOYSTICK_DIAGONAL  /**< Combined lateral and forward/backward control. */
} joystick_estado_t;

/* Public functions ---------------------------------------------------------*/

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