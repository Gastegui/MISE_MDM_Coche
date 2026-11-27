/** @file pulsador_config.h
*
* @brief A description of the module’s purpose.
*
*/

#ifndef PULSADOR_CONFIG_H
#define PULSADOR_CONFIG_H

/* =====================================================================
   Configuración del pulsador para ESTE micro y ESTE montaje.
   Para llevar la biblioteca a otro micro solo se cambia este fichero.
   ===================================================================== */

#include "main.h"               /* registros del STM32 y HAL_GetTick() */

/* Reloj: milisegundos desde el arranque */
#define PULSADOR_TIEMPO_MS()    HAL_GetTick()

/* Dónde está cada botón: registro de entrada (IDR) y número de bit.
   Pulsador a GND con pull-up: apretado = bit a 0 */
#define PULSADOR_BJ_REGISTRO    (&(GPIOA->IDR))     /* BJ = PA8 */
#define PULSADOR_BJ_BIT         8U
#define PULSADOR_B1_REGISTRO    (&(GPIOC->IDR))     /* B1 = PC13 */
#define PULSADOR_B1_BIT         13U

#endif

/*** end of file ***/
