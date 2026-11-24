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
#define PULSADOR_B2_REGISTRO    (&(GPIOA->IDR))     /* B2 = PA8 */
#define PULSADOR_B2_BIT         8U
#define PULSADOR_B3_REGISTRO    (&(GPIOB->IDR))     /* B3 = PB10 */
#define PULSADOR_B3_BIT         10U

#endif
