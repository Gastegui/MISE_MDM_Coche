/**
 * @file tasks.h
 *
 * @brief Declarations for the application task module.
 *
 * @details Provides initialization and task functions for button polling,
 *          ADC joystick input, UART communication, and movement control.
 *
 * @author Julen Gallastegui de la Cruz
 * @author Andrea Nieto Gil
 */

#ifndef TASKS_TASKS_H
#define TASKS_TASKS_H

#include "main.h"

void TASK_Polling_Buttons_Init(void);
task_retval_t TASK_Polling_Buttons(void);

void TASK_ADC_Init(void);
task_retval_t TASK_ADC(void);

void TASK_UART_Rx_Init(void);
task_retval_t TASK_UART_Rx(void);
task_retval_t TASK_UART_Tx(void);

task_retval_t TASK_Movement(void);

#endif /* TASKS_TASKS_H */

/*** end of file ***/