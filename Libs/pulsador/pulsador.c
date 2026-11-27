/** @file pulsador.c
*
* @brief Pulsador control module.
*
*/

#include "pulsador.h"

void PULSADOR_Init(pulsador_t *p, const volatile uint32_t *entrada, uint8_t bit)
{
    p->entrada = entrada;
    p->bit = bit;
    p->estado = PULSADOR_REPOSO;
    p->t_inicio = 0U;
}

/* Devuelve 1 una sola vez por pulsación, cuando ya no hay rebote */
uint8_t PULSADOR_Actualizar(pulsador_t *p)
{
    uint8_t pulsacion = 0U;
    uint8_t apretado = 0U;
    uint32_t ahora;

    ahora = PULSADOR_TIEMPO_MS();

    /* Botón a GND con pull-up: apretado = bit a 0 */
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
                pulsacion = 1U;                     /* evento: pulsación confirmada */
                p->estado = PULSADOR_PULSADO;
            }
            else
            {
                p->estado = PULSADOR_REPOSO;        /* era ruido */
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
                p->estado = PULSADOR_PULSADO;       /* era rebote al soltar */
            }
        }
        break;

    default:
        p->estado = PULSADOR_REPOSO;        /* estado no previsto: al seguro */
        break;
    }
    return pulsacion;
}

/*** end of file ***/
