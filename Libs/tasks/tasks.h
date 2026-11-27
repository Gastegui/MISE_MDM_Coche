/** @file tasks.h
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#ifndef TASKS_TASKS_H
#define TASKS_TASKS_H

#include "main.h"

void task_pollig_buttons_init(void);
task_retval_t task_polling_buttons(void);

void task_adc_init(void);
task_retval_t task_adc(void);

task_retval_t task_uart_tx(void);

void task_uart_rx_init(void);
task_retval_t task_uart_rx(void);


task_retval_t task_movement(void);

#endif /* TASKS_TASKS_H */
