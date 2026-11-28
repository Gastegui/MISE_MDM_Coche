/**
 * @file    movimiento.c
 *
 * @brief   Vehicle movement control module.
 *
 * @details Implements the movement state machine, UART command handling,
 *          joystick control and motor PWM coordination. It also manages
 *          collision indication and buzzer feedback.
 *
 * @author  Julen Gallastegui de la Cruz
 * @author  Andrea Nieto Gil
 */

/* Includes -----------------------------------------------------------------*/

#include "movimiento.h"
#include "main.h"
#include <stdint.h>

// Interrupt flags

volatile uint8_t movimiento_colision_flag;
volatile uint8_t movimiento_uartTimeout_flag;

// Flags from UART commands and polling buttons

uint8_t movimiento_reset_flag;
uint8_t movimiento_joystickCambioModo_flag = 0;
uint8_t movimiento_cambioModo_flag = 0;
uint8_t movimiento_uartComando_flag = 0;

// Pending UART commands

uart_estado_t uart_estado_siguiente = UART_PARADO;
uint16_t uart_velocidad_movimiento = 0;

// Current state of the state machine

static movimiento_estado_t mov_estado = MOVIMIENTO_INIT;
static uart_estado_t uart_estado_actual = UART_PARADO;
static joystick_estado_t joystick_estado = JOYSTICK_GIRO;

// Joysticks calibration

/** @brief ADC readings within this range of center are treated as neutral. */
#define JOYSTICK_DEADZONE 100

/** @brief ADC reading corresponding to the joystick center. */
#define JOYSTICK_CENTER 2100

static int16_t joystick_x;
static int16_t joystick_y;

// Motors and PWM

static motor_t motor_superior_izquierda;
static motor_t motor_inferior_izquierda;
static motor_t motor_inferior_derecha;
static motor_t motor_superior_derecha;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;

// Buzzer timing

/** @brief Buzzer toggle interval during collision mode, in milliseconds. */
#define BUZZER_COLISION_PERIOD 250U

/** @brief Buzzer toggle interval while moving backward, in milliseconds. */
#define BUZZER_BACKWARDS_PERIOD 400U

static uint32_t buzzer_last_time = 0;

/* Private function prototypes ---------------------------------------------*/

static void UART_Entrar();
static void UART_Actualizar();
static void JOYSTICK_Actualizar();
static int16_t JOYSTICK_Conversion(uint16_t original);
static void JOYSTICK_Convertir_ADCaPWM(int16_t vy, int16_t vx,
                                       int16_t omega);
static void MOVIMIENTO_InitMotores();
static void MOVIMIENTO_PararMotores();
static inline void MOVIMIENTO_EntryUart();
static void MOVIMIENTO_EntryColision();

/* Public functions ---------------------------------------------------------*/

/**
 * @brief  Returns the current movement state.
 *
 * @return Current movement state.
 */
movimiento_estado_t MOVIMIENTO_Get_MovimientoEstado()
{
    return mov_estado;
}

/**
 * @brief  Returns the current UART movement state.
 *
 * @return Current UART state.
 */
uart_estado_t MOVIMIENTO_Get_UartEstado()
{
    return uart_estado_actual;
}

/**
 * @brief  Returns the current joystick operating mode.
 *
 * @return Current joystick state.
 */
joystick_estado_t MOVIMIENTO_Get_JoystickEstado()
{
    return joystick_estado;
}

/**
 * @brief  Returns the converted horizontal joystick value.
 *
 * @return Horizontal joystick value.
 */
int16_t MOVIMIENTO_Get_Joystick_X()
{
    return joystick_x;
}

/**
 * @brief  Returns the converted vertical joystick value.
 *
 * @return Vertical joystick value.
 */
int16_t MOVIMIENTO_Get_Joystick_Y()
{
    return joystick_y;
}

/**
 * @brief  Updates the horizontal joystick value.
 *
 * @param[in] valor  Raw horizontal joystick ADC reading.
 */
void MOVIMIENTO_Set_Joystick_X(uint16_t valor)
{
    joystick_x = JOYSTICK_Conversion(valor);
}

/**
 * @brief  Updates the vertical joystick value.
 *
 * @param[in] valor  Raw vertical joystick ADC reading.
 */
void MOVIMIENTO_Set_Joystick_Y(uint16_t valor)
{
    joystick_y = JOYSTICK_Conversion(valor);
}

/**
 * @brief  Updates the vehicle movement state machine.
 *
 * @details Executes the actions associated with the current movement state
 *          and handles transitions between initialization, UART control,
 *          joystick control and collision states.
 */
void MOVIMIENTO_Actualizar()
{
  switch(mov_estado)
  {
  case MOVIMIENTO_INIT:
    MOVIMIENTO_InitMotores();
    MOVIMIENTO_EntryUart();
    mov_estado = MOVIMIENTO_UART;
    break;

  case MOVIMIENTO_UART:
    if(movimiento_cambioModo_flag == 1 || movimiento_colision_flag == 1)
    {
      /** Stop the buzzer and discard the active UART movement command. */
      HAL_GPIO_WritePin(BUZ_GPIO_Port, BUZ_Pin, GPIO_PIN_RESET);
      uart_estado_siguiente = UART_PARADO;
      UART_Entrar();
    }

    if(movimiento_cambioModo_flag == 1)
    {
      HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
      movimiento_cambioModo_flag = 0;
      mov_estado = MOVIMIENTO_JOYSTICK;
    }
    else
    {
      UART_Actualizar();
    }

    if(movimiento_colision_flag == 1)
    {
      MOVIMIENTO_EntryColision();
    }
    break;

  case MOVIMIENTO_JOYSTICK:
    if(movimiento_cambioModo_flag == 1 || movimiento_colision_flag == 1)
    {
      /** Reset joystick mode and stop the motors before leaving this state. */
      movimiento_joystickCambioModo_flag = 1;
      joystick_estado = JOYSTICK_GIRO;
      JOYSTICK_Actualizar();
      MOVIMIENTO_PararMotores();
    }

    if(movimiento_cambioModo_flag == 1)
    {
      MOVIMIENTO_EntryUart();
      movimiento_cambioModo_flag = 0;
      mov_estado = MOVIMIENTO_UART;
    }
    else
    {
      JOYSTICK_Actualizar();
    }

    if(movimiento_colision_flag == 1)
    {
      MOVIMIENTO_EntryColision();
    }
    break;

  case MOVIMIENTO_COLISION:
    if(movimiento_reset_flag == 1)
    {
      HAL_GPIO_WritePin(BUZ_GPIO_Port, BUZ_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
      MOVIMIENTO_EntryUart();
      movimiento_reset_flag = 0;
      mov_estado = MOVIMIENTO_UART;
    }
    else
    {
      if(HAL_GetTick() - buzzer_last_time >= BUZZER_COLISION_PERIOD)
      {
        HAL_GPIO_TogglePin(BUZ_GPIO_Port, BUZ_Pin);
        buzzer_last_time = HAL_GetTick();
      }
    }
    break;

  default:
    mov_estado = MOVIMIENTO_UART;
    break;
  }
}

/**
 * @brief  Requests a new movement state from the UART interface.
 *
 * @details Stores the requested movement state and speed. The request is
 *          processed by the UART update routine.
 *
 * @param[in] nuevo_estado  Requested UART movement state.
 * @param[in] speed         Requested motor PWM pulse width.
 */
void MOVIMIENTO_UART_CambiarEstado(uart_estado_t nuevo_estado, uint16_t speed)
{
  movimiento_uartComando_flag = 1;
  uart_estado_siguiente = nuevo_estado;
  uart_velocidad_movimiento = speed;
}

/**
 * @brief  Initializes the four vehicle motors.
 *
 * @details Associates each motor with its timer and forward/backward
 *          PWM channels.
 */
static void MOVIMIENTO_InitMotores()
{
  MOTOR_Create(&htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, &motor_superior_izquierda);
  MOTOR_Create(&htim3, TIM_CHANNEL_4, TIM_CHANNEL_3, &motor_inferior_izquierda);
  MOTOR_Create(&htim4, TIM_CHANNEL_2, TIM_CHANNEL_1, &motor_inferior_derecha);
  MOTOR_Create(&htim4, TIM_CHANNEL_4, TIM_CHANNEL_3, &motor_superior_derecha);
}

/**
 * @brief  Stops all vehicle motors.
 *
 * @details Sets the speed of each motor to zero.
 */
static void MOVIMIENTO_PararMotores()
{
  MOTOR_Update(&motor_superior_izquierda, 0, MOTOR_FORWARD);
  MOTOR_Update(&motor_superior_derecha, 0, MOTOR_FORWARD);
  MOTOR_Update(&motor_inferior_izquierda, 0, MOTOR_FORWARD);
  MOTOR_Update(&motor_inferior_derecha, 0, MOTOR_FORWARD);
}

/**
 * @brief  Performs the entry actions for the collision state.
 *
 * @details Updates the status LEDs, activates the collision indicator and
 *          clears the collision flag.
 */
static void MOVIMIENTO_EntryColision()
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  movimiento_colision_flag = 0;
  mov_estado = MOVIMIENTO_COLISION;
}

/**
 * @brief  Performs the entry actions for UART control mode.
 *
 * @details Turns on the UART mode LED and turns off the joystick mode LED.
 */
static inline void MOVIMIENTO_EntryUart()
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  Updates UART control and its associated indicators.
 *
 * @details Processes pending UART commands and timeout events. When the
 *          vehicle is moving backwards, periodically toggles the buzzer.
 */
static void UART_Actualizar()
{
  if(movimiento_uartComando_flag == 1)
  {
    HAL_GPIO_WritePin(BUZ_GPIO_Port, BUZ_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    UART_Entrar();
    movimiento_uartComando_flag = 0;
    movimiento_uartTimeout_flag = 0;
  }
  else if(movimiento_uartTimeout_flag == 1)
  {
    HAL_GPIO_WritePin(BUZ_GPIO_Port, BUZ_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    uart_estado_siguiente = UART_PARADO;
    UART_Entrar();
    movimiento_uartTimeout_flag = 0;
  }

  if (uart_estado_actual == UART_ATRAS)
  {
    if(HAL_GetTick() - buzzer_last_time >= BUZZER_BACKWARDS_PERIOD)
    {
      HAL_GPIO_TogglePin(BUZ_GPIO_Port, BUZ_Pin);
      buzzer_last_time = HAL_GetTick();
    }
  }
}

/**
 * @brief  Applies the pending UART movement command.
 *
 * @details Converts the requested UART state into motor directions and
 *          starts or stops the timeout timer as required.
 */
static void UART_Entrar()
{
  movimiento_union_t movimiento = {0};
  uart_estado_actual = uart_estado_siguiente;

  switch (uart_estado_actual)
  {
  case UART_PARADO:
    HAL_TIM_Base_Stop_IT(&htim5);
    break;

  case UART_IZQUIERDA:
    HAL_TIM_Base_Start_IT(&htim5);
    movimiento.left = 1;
    break;

  case UART_TANQUE_IZQUIERDA:
    HAL_TIM_Base_Start_IT(&htim5);
    movimiento.tank_left = 1;
    break;

  case UART_DELANTE:
    HAL_TIM_Base_Start_IT(&htim5);
    movimiento.forward = 1;
    break;

  case UART_ATRAS:
    HAL_TIM_Base_Start_IT(&htim5);
    movimiento.backward = 1;
    break;

  case UART_DERECHA:
    HAL_TIM_Base_Start_IT(&htim5);
    movimiento.right = 1;
    break;

  case UART_TANQUE_DERECHA:
    HAL_TIM_Base_Start_IT(&htim5);
    movimiento.tank_right = 1;
    break;

  default:
    uart_estado_actual = UART_PARADO;
    break;
  }

  if(movimiento.raw == 0)
  {
    MOVIMIENTO_PararMotores();
  }
  else
  {
    /** Apply the command to the upper-left motor. */
    if(movimiento.forward || movimiento.right || movimiento.tank_right)
      MOTOR_Update(&motor_superior_izquierda, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_Update(&motor_superior_izquierda, uart_velocidad_movimiento, MOTOR_BACKWARD);

    /** Apply the command to the upper-right motor. */
    if(movimiento.forward || movimiento.left || movimiento.tank_left)
      MOTOR_Update(&motor_superior_derecha, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_Update(&motor_superior_derecha, uart_velocidad_movimiento, MOTOR_BACKWARD);

    /** Apply the command to the lower-left motor. */
    if(movimiento.forward || movimiento.left || movimiento.tank_right)
      MOTOR_Update(&motor_inferior_izquierda, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_Update(&motor_inferior_izquierda, uart_velocidad_movimiento, MOTOR_BACKWARD);

    /** Apply the command to the lower-right motor. */
    if(movimiento.forward || movimiento.right || movimiento.tank_left)
      MOTOR_Update(&motor_inferior_derecha, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_Update(&motor_inferior_derecha, uart_velocidad_movimiento, MOTOR_BACKWARD);
  }
}

/**
 * @brief  Updates joystick control.
 *
 * @details Handles joystick mode changes and converts the joystick axes
 *          into the translational and rotational components used for
 *          motor control.
 */
static void JOYSTICK_Actualizar()
{
  if(movimiento_joystickCambioModo_flag == 1)
  {
    movimiento_joystickCambioModo_flag = 0;

    if(joystick_estado == JOYSTICK_GIRO)
    {
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
      joystick_estado = JOYSTICK_DIAGONAL;
    }
    else
    {
      HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
      joystick_estado = JOYSTICK_GIRO;
    }
  }

  switch(joystick_estado)
  {
  case JOYSTICK_GIRO:
    JOYSTICK_Convertir_ADCaPWM(joystick_y*2, 0, joystick_x*2);
    break;

  case JOYSTICK_DIAGONAL:
    JOYSTICK_Convertir_ADCaPWM(joystick_y*2, joystick_x*2, 0);
    break;

  default:
    joystick_estado = JOYSTICK_GIRO;
    break;
  }
}

/**
 * @brief  Converts a raw joystick ADC reading into a centered value.
 *
 * @details Applies the configured dead zone around the joystick center.
 *
 * @param[in] original  Raw ADC reading.
 *
 * @return Centered joystick value, or zero inside the dead zone.
 */
static int16_t JOYSTICK_Conversion(uint16_t original)
{
  int16_t ret = JOYSTICK_CENTER - original;

  if(ret > JOYSTICK_DEADZONE || -ret > JOYSTICK_DEADZONE)
    return ret;

  return 0;
}

/**
 * @brief  Converts joystick components into motor PWM commands.
 *
 * @details Calculates the four wheel commands for the vehicle, scales
 *          them when necessary to keep their magnitude within the
 *          configured maximum and updates each motor.
 *
 * @param[in] vy     Longitudinal movement component.
 * @param[in] vx     Lateral movement component.
 * @param[in] omega  Rotational movement component.
 */
static void JOYSTICK_Convertir_ADCaPWM(int16_t vy, int16_t vx, int16_t omega)
{
  //fl = front left
  //fr = front right
  //rl = rear left
  //rr = rear right

  int16_t fl = vy + vx + omega;
  int16_t fr = vy - vx - omega;
  int16_t rl = vy - vx + omega;
  int16_t rr = vy + vx - omega;

  int16_t max_val = 4900;
  int16_t abs_fl = fl < 0 ? -fl : fl;
  int16_t abs_fr = fr < 0 ? -fr : fr;
  int16_t abs_rl = rl < 0 ? -rl : rl;
  int16_t abs_rr = rr < 0 ? -rr : rr;

  int16_t peak = abs_fl;
  if(abs_fr > peak) peak = abs_fr;
  if(abs_rl > peak) peak = abs_rl;
  if(abs_rr > peak) peak = abs_rr;

  if(peak > max_val)
  {
    fl = fl * max_val / peak;
    fr = fr * max_val / peak;
    rl = rl * max_val / peak;
    rr = rr * max_val / peak;
  }

  //LOGS

  MOTOR_Update(&motor_superior_izquierda, fl > 0 ? fl : -fl, fl > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
  MOTOR_Update(&motor_superior_derecha,   fr > 0 ? fr : -fr, fr > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
  MOTOR_Update(&motor_inferior_izquierda, rl > 0 ? rl : -rl, rl > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
  MOTOR_Update(&motor_inferior_derecha,   rr > 0 ? rr : -rr, rr > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
}

/*** end of file ***/
