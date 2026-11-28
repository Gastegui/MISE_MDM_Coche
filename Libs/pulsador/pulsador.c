/**
 * @file pulsador.c
 *
 * @brief Debounced button input implementation.
 *
 * @details Implements a polling state machine that filters unstable press and
 *          release transitions using the configured debounce interval.
 */

#include "pulsador.h"

/**
 * @brief Initializes a button instance.
 *
 * @param[out] p       Button instance to initialize.
 * @param[in]  entrada Pointer to the GPIO input register.
 * @param[in]  bit     Button bit index in the input register.
 */
void PULSADOR_Init(pulsador_t *p, const volatile uint32_t *entrada, uint8_t bit)
{
    p->entrada = entrada;
    p->bit = bit;
    p->estado = PULSADOR_REPOSO;
    p->t_inicio = 0U;
}

/**
 * @brief Polls a button and reports a debounced press event.
 *
 * @details Samples the active-low GPIO input and advances the debounce state
 *          machine. A confirmed press produces a one-call event; button
 *          releases are also debounced before a subsequent press is accepted.
 *
 * @param[in,out] p  Button instance to sample and update.
 *
 * @return 1U once when a press is confirmed; otherwise 0U.
 */
uint8_t PULSADOR_Actualizar(pulsador_t *p)
{
    uint8_t pulsacion = 0U;
    uint8_t apretado = 0U;
    uint32_t ahora;

    ahora = PULSADOR_TIEMPO_MS();

    /** A button connected to ground with a pull-up is active-low. */
    if ((*(p->entrada) & (1U << p->bit)) == 0U)
    {
        apretado = 1U;
    }

    switch (p->estado)
    {
    case PULSADOR_REPOSO:
        if (apretado == 1U)
        {
            p->t_inicio = ahora;
            p->estado = PULSADOR_POSIBLE_PULSACION;
        }
        break;

    case PULSADOR_POSIBLE_PULSACION:
        if ((ahora - p->t_inicio) >= PULSADOR_MS_REBOTE)
        {
            if (apretado == 1U)
            {
                pulsacion = 1U;
                p->estado = PULSADOR_PULSADO;
            }
            else
            {
                p->estado = PULSADOR_REPOSO;
            }
        }
        break;

    case PULSADOR_PULSADO:
        if (apretado == 0U)
        {
            p->t_inicio = ahora;
            p->estado = PULSADOR_POSIBLE_LIBERACION;
        }
        break;

    case PULSADOR_POSIBLE_LIBERACION:
        if ((ahora - p->t_inicio) >= PULSADOR_MS_REBOTE)
        {
            if (apretado == 0U)
            {
                p->estado = PULSADOR_REPOSO;
            }
            else
            {
                p->estado = PULSADOR_PULSADO;
            }
        }
        break;

    default:
        p->estado = PULSADOR_REPOSO;
        break;
    }
    return pulsacion;
}

/*** end of file ***/
