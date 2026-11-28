/**
 * @file pulsador.h
 *
 * @brief Public interface for debounced button input.
 *
 * @details Provides the button state type and functions to initialize a
 *          button and poll it for debounced press events.
 */

#ifndef PULSADOR_H
#define PULSADOR_H

/* Includes -----------------------------------------------------------------*/

#include <stdint.h>
#include "pulsador_config.h"

/* Defines ------------------------------------------------------------------*/

/** @brief Debounce interval applied to both button presses and releases, in ms. */
#define PULSADOR_MS_REBOTE  20U

/* Types --------------------------------------------------------------------*/

/**
 * @brief States used by the button debounce state machine.
 */
typedef enum
{
    PULSADOR_REPOSO = 0,             /**< Waiting for a possible press. */
    PULSADOR_POSIBLE_PULSACION,      /**< Verifying that a press is stable. */
    PULSADOR_PULSADO,                /**< Button press has been confirmed. */
    PULSADOR_POSIBLE_LIBERACION      /**< Verifying that a release is stable. */
} pulsador_estado_t;

/**
 * @brief Runtime data for one polled button.
 *
 * @details Stores the input register and bit to sample, the current debounce
 *          state, and the timestamp at which that state was entered.
 */
typedef struct
{
    const volatile uint32_t *entrada; /**< Pointer to the GPIO input register. */
    uint8_t bit;                      /**< Button bit index in the input register. */
    pulsador_estado_t estado;         /**< Current debounce state. */
    uint32_t t_inicio;                /**< Tick when the current state began. */
} pulsador_t;

/* Public functions ---------------------------------------------------------*/

void PULSADOR_Init(pulsador_t *p, const volatile uint32_t *entrada, uint8_t bit);
uint8_t PULSADOR_Actualizar(pulsador_t *p);

#endif

/*** end of file ***/
