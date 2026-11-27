/** @file pulsador.h
*
* @brief A description of the module’s purpose.
*
*/

#ifndef PULSADOR_H
#define PULSADOR_H

#include <stdint.h>
#include "pulsador_config.h"

/* Tiempo que se deja pasar para que acabe el rebote */
#define PULSADOR_MS_REBOTE  20U

/* Los cuatro estados de la máquina */
typedef enum
{
    PULSADOR_REPOSO = 0,
    PULSADOR_POSIBLE_PULSACION,
    PULSADOR_PULSADO,
    PULSADOR_POSIBLE_LIBERACION
} pulsador_estado_t;

/* Cada pulsador guarda dónde está, su estado y cuándo entró en él */
typedef struct
{
    const volatile uint32_t *entrada;   /* registro de entrada del puerto */
    uint8_t bit;                    /* número de bit del pulsador */
    pulsador_estado_t estado;
    uint32_t t_inicio;
} pulsador_t;

void PULSADOR_Init(pulsador_t *p, const volatile uint32_t *entrada, uint8_t bit);
uint8_t PULSADOR_Actualizar(pulsador_t *p);

#endif

/*** end of file ***/
