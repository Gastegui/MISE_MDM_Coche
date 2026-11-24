/** @file uart.c
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#include <stdint.h>
#include "uart.h"

/*!
* @brief Envia un texto por la USART2 y espera a que termine.
*
* @param[in]  txt La string a enviar.
*
* @return 0 si la transmision fue bien, 1 si no.
*/
uint8_t UART_Send(const char * txt)
{
  uint8_t return_value = 0;

  if(HAL_UART_Transmit(&huart2, (uint8_t *)txt, strlen(txt), HAL_MAX_DELAY) != HAL_OK)
  {
    return_value = 1;
  }

	return return_value;
}

/*** end of file ***/
