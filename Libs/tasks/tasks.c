/**
 * @file tasks.c
 *
 * @brief Application task implementations.
 *
 * @details Implements periodic tasks for buttons, ADC joystick input, UART
 *          communication, and movement control, along with their DMA
 *          callbacks.
 *
 * @author Julen Gallastegui de la Cruz
 * @author Andrea Nieto Gil
 */

 
#include "tasks.h"
#include <stdio.h>
#include <string.h>

// UART
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
volatile uint8_t uart_rx_buff[20];
volatile uint8_t uart_rx_pending = 0;
char uart_tx_buff[100];

// Flags
extern uint8_t movimiento_reset_flag;
extern uint8_t movimiento_joystickCambioModo_flag;
extern uint8_t movimiento_cambioModo_flag;;

// BUTTONS
static pulsador_t b1, bj;
static uint8_t peticion = 0U;

// ADC
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
volatile uint16_t adc_dma_buffer[ADC_BUFFER_SIZE];
volatile uint8_t adc_half_flag = 0;
volatile uint8_t adc_full_flag = 0;


/**
 * @brief Handles completed UART receive-to-idle DMA transfers.
 *
 * @details Marks the received data as pending and restarts the DMA reception
 *          when the callback belongs to the configured UART.
 *
 * @param[in] huart  UART handle that completed the reception.
 * @param[in] Size   Number of bytes received.
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  if(huart->Instance == USART_DMA)
  {
    uart_rx_pending = 1;
    uart_rx_buff[Size] = '\0';
    HAL_UARTEx_ReceiveToIdle_DMA(USART_TO_USE, (uint8_t*)uart_rx_buff, sizeof(uart_rx_buff));
  }
}

/**
 * @brief Handles completion of the first half of the ADC DMA buffer.
 *
 * @param[in] hadc  ADC handle that completed the conversion.
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  adc_half_flag = 1;
}

/**
 * @brief Handles completion of the full ADC DMA buffer.
 *
 * @param[in] hadc  ADC handle that completed the conversion.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  adc_full_flag = 1;
}


/**
 * @brief Initializes the buttons used by the polling task.
 */
void TASK_Polling_Buttons_Init(void)
{
  PULSADOR_Init(&b1, PULSADOR_B1_REGISTRO, PULSADOR_B1_BIT);
  PULSADOR_Init(&bj, PULSADOR_BJ_REGISTRO, PULSADOR_BJ_BIT);
}

/**
 * @brief Polls the movement-mode and joystick-mode buttons.
 *
 * @details Updates the movement control flags when a button press is valid
 *          for the current movement state.
 *
 * @return TASK_OK after processing both buttons.
 */
task_retval_t TASK_Polling_Buttons(void)
{
  // Pulsamos B1, cambio de modo de movimiento en maquina de estados de MOVIMIENTO
  peticion = PULSADOR_Actualizar(&b1);
  if(peticion == 1)
  {
    if(MOVIMIENTO_Get_MovimientoEstado() != MOVIMIENTO_COLISION)
    {
      movimiento_cambioModo_flag = 1U;
    }
  }

  // Pulsamos BJ, cambio de modo de movimiento en maquina de estados de JOYSTICK
  peticion = PULSADOR_Actualizar(&bj);
  if(peticion == 1)
  {
    if(MOVIMIENTO_Get_MovimientoEstado() == MOVIMIENTO_JOYSTICK)
    {
      movimiento_joystickCambioModo_flag = 1U;
    }
  }

  return TASK_OK;
}

/**
 * @brief Initializes ADC DMA sampling for the joystick.
 *
 * @details Clears the DMA buffer, starts ADC DMA conversions, and starts the
 *          timer that triggers the conversions.
 */
void TASK_ADC_Init(void)
{
  memset((uint8_t*)adc_dma_buffer, 0, ADC_BUFFER_SIZE*sizeof(adc_dma_buffer[0]));
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, ADC_BUFFER_SIZE);
  HAL_TIM_Base_Start(&htim2);
}

/**
 * @brief Processes the completed half of the ADC DMA buffer.
 *
 * @details Averages the joystick X and Y samples and updates the movement
 *          module. Returns without processing when no half-buffer is ready.
 *
 * @return TASK_OK after processing or when no new buffer half is available.
 */
task_retval_t TASK_ADC(void)
{
  uint8_t start;
  if(adc_half_flag)
  {
    start = 0;
    adc_half_flag = 0;
  }
  else if(adc_full_flag)
  {
    start = ADC_BUFFER_SIZE/2;
    adc_full_flag = 0;
  }
  else
    return TASK_OK;

  uint32_t x = 0;
  uint32_t y = 0;

  for(uint8_t i = start; i < start + ADC_BUFFER_SIZE/2; i++)
  {
    x += adc_dma_buffer[i];
    y += adc_dma_buffer[++i];
  }

  x /= (ADC_BUFFER_SIZE/4);
  y /= (ADC_BUFFER_SIZE/4);

  MOVIMIENTO_Set_Joystick_X(x);
  MOVIMIENTO_Set_Joystick_Y(y);

  return TASK_OK;
}

/**
 * @brief Initializes UART receive-to-idle DMA reception.
 */
void TASK_UART_Rx_Init(void)
{
  HAL_UARTEx_ReceiveToIdle_DMA(USART_TO_USE, (uint8_t*)uart_rx_buff, sizeof(uart_rx_buff));
}

/**
 * @brief Sends movement and joystick status over UART using DMA.
 *
 * @return TASK_OK when transmission starts, otherwise TASK_FAILED.
 */
task_retval_t TASK_UART_Tx(void)
{
  snprintf(uart_tx_buff, 100, "Tick: %lu, Mov_estado: %hu, Mov_uart_estado: %hu, Mov_Joy_estado: %hu, Joy_X: %i, Joy_Y: %i\r\n",
      HAL_GetTick(), (uint8_t)MOVIMIENTO_Get_MovimientoEstado(), (uint8_t)MOVIMIENTO_Get_UartEstado(), (uint8_t)MOVIMIENTO_Get_JoystickEstado(),
      MOVIMIENTO_Get_Joystick_X(), MOVIMIENTO_Get_Joystick_Y());

  if(HAL_UART_Transmit_DMA(USART_TO_USE, (uint8_t*)uart_tx_buff, strlen(uart_tx_buff)) == HAL_OK)
    return TASK_OK;

  return TASK_FAILED;
}

/**
 * @brief Processes a pending UART command.
 *
 * @details Handles movement commands, mode changes, and collision reset
 *          requests received by the UART DMA callback.
 *
 * @return TASK_FAILED when the received command is RETURN_FAILED; otherwise
 *         TASK_OK.
 */
task_retval_t TASK_UART_Rx(void)
{
  if(uart_rx_pending)
  {
    if(strncmp((char*)uart_rx_buff, "RETURN_FAILED", 13) == 0)
      return TASK_FAILED;
    else if(strncmp((char*)uart_rx_buff, "MOVE ", 5) == 0)
    {
      uart_estado_t estado = UART_PARADO;
      uint16_t move_speed = 0;
      switch(uart_rx_buff[5])
      {
      case 'W':
        estado = UART_DELANTE;
        break;
      case 'S':
        estado = UART_ATRAS;
        break;
      case 'A':
        estado = UART_IZQUIERDA;
        break;
      case 'D':
        estado = UART_DERECHA;
        break;
      case 'Q':
        estado = UART_TANQUE_IZQUIERDA;
        break;
      case 'E':
        estado = UART_TANQUE_DERECHA;
        break;
      case 'X':
      default:
        estado = UART_PARADO;
        break;
      }

      sscanf((char*)uart_rx_buff + 7, "%hu", &move_speed);

      if(move_speed > 100)
        move_speed = 100;

      if(MOVIMIENTO_Get_MovimientoEstado() == MOVIMIENTO_UART)
        MOVIMIENTO_UART_CambiarEstado(estado, move_speed * PWM_CONVERSION);
    }
    else if(strncmp((char*)uart_rx_buff, "CHANGE_MOVE_MODE", 16) == 0)
    {
      if(MOVIMIENTO_Get_MovimientoEstado() != MOVIMIENTO_COLISION)
        movimiento_cambioModo_flag = 1;
    }
    else if(strncmp((char*)uart_rx_buff, "CHANGE_JOYSTICK", 15) == 0)
    {
      if(MOVIMIENTO_Get_MovimientoEstado() == MOVIMIENTO_JOYSTICK)
        movimiento_joystickCambioModo_flag = 1;
    }
    else if(strncmp((char*)uart_rx_buff, "COLISION_RESET", 14) == 0)
    {
      if(MOVIMIENTO_Get_MovimientoEstado() == MOVIMIENTO_COLISION)
        movimiento_reset_flag = 1;
    }
    uart_rx_pending = 0;
  }
  return TASK_OK;
}

/**
 * @brief Updates the movement state machine.
 *
 * @return TASK_OK after updating the movement module.
 */
task_retval_t TASK_Movement(void)
{
  MOVIMIENTO_Actualizar();
  return TASK_OK;
}

/*** end of file ***/