/** @file movimiento.c
*
* @brief A description of the module’s purpose.
*
* @author Julen Gallastegui de la Cruz
* @author Andrea Nieto Gil
*/

#include "movimiento.h"
#include "main.h"
#include <stdint.h>

//Flags desde interrupciones
volatile uint8_t movimiento_colision_flag;
volatile uint8_t movimiento_uartTimeout_flag;

//Flags desde comandos de UART o botones sin interrupción
uint8_t movimiento_reset_flag;
uint8_t movimiento_joystickCambioModo_flag = 0U;
uint8_t movimiento_cambioModo_flag = 0U;
uint8_t movimiento_uartComando_flag = 0U;

//Comandos del UART
uart_estado_t uart_estado_siguiente = UART_PARADO;
uint16_t uart_velocidad_movimiento = 0U;

//Estados de las máquinas de estados
static movimiento_estado_t mov_estado = MOVIMIENTO_INIT;
static uart_estado_t uart_estado_actual = UART_PARADO;
static joystick_estado_t joystick_estado = JOYSTICK_GIRO;

//Valores de los joysticks
static int16_t joystick_x;
static int16_t joystick_y;
#define JOYSTICK_DEADZONE 100
#define JOYSTICK_CENTER 2100

//Motores y PWM
static motor_t motor_superior_izquierda;
static motor_t motor_inferior_izquierda;
static motor_t motor_inferior_derecha;
static motor_t motor_superior_derecha;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

static void UART_Entrar();
static void UART_Actualizar();
static void JOYSTICK_Actualizar();
static void MOVIMIENTO_InitMotores();
static void MOVIMIENTO_PararMotores();
static inline void MOVIMIENTO_EntryUart();
static void MOVIMIENTO_EntryColision();

movimiento_estado_t MOVIMIENTO_Get_movimiento_estado() { return mov_estado; }
uart_estado_t MOVIMIENTO_Get_uart_estado() { return uart_estado_actual; }
joystick_estado_t MOVIMIENTO_Get_joystick_estado() { return joystick_estado; }

int16_t MOVIMIENTO_Get_Joystick_X() { return joystick_x; }
int16_t MOVIMIENTO_Get_Joystick_Y() { return joystick_y; }

static int16_t joystick_conversion(uint16_t original)
{
  int16_t ret = JOYSTICK_CENTER - original;

  if(ret > JOYSTICK_DEADZONE || -ret > JOYSTICK_DEADZONE)
    return ret;

  return 0;
}

void MOVIMIENTO_Set_Joystick_X(uint16_t valor)
{
  joystick_x = joystick_conversion(valor);
}
void MOVIMIENTO_Set_Joystick_Y(uint16_t valor)
{
  joystick_y = joystick_conversion(valor);
}

/*!
* @brief Actualiza la maquina de estados del movimiento del coche.
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
      // Acciones al salir del estado UART
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
      // Acciones al salir del estado JOYSTICK
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
      HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
      MOVIMIENTO_EntryUart();
      movimiento_reset_flag = 0;
      mov_estado = MOVIMIENTO_UART;
    }
    else
    {
      // COMPLETAR: Activar el buzzer de alarma
    }
    break;

  default:
    mov_estado = MOVIMIENTO_UART;
    break;
  }
}

/* Acción al entrar:  */
static void UART_Entrar()
{
  uart_estado_actual = uart_estado_siguiente;
  movimiento_union_t movimiento = {0};

  switch (uart_estado_actual)
  {
  case UART_PARADO:
    // COMPLETAR: desactivamos el timer del timeout
    break;

  case UART_IZQUIERDA:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.left = 1;
    break;

  case UART_TANQUE_IZQUIERDA:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.tank_left = 1;
    break;

  case UART_DELANTE:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.forward = 1;
    break;

  case UART_ATRAS:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.backward = 1;
    break;

  case UART_DERECHA:
    // COMPLETAR: activamos/actualizamos el timeout
    movimiento.right = 1;
    break;

  case UART_TANQUE_DERECHA:
    // COMPLETAR: activamos/actualizamos el timeout
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
    //Motor superior izquierda
    if(movimiento.forward || movimiento.right || movimiento.tank_right)
      MOTOR_update(&motor_superior_izquierda, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_superior_izquierda, uart_velocidad_movimiento, MOTOR_BACKWARD);

    //Motor superior derecha
    if(movimiento.forward || movimiento.left || movimiento.tank_left)
      MOTOR_update(&motor_superior_derecha, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_superior_derecha, uart_velocidad_movimiento, MOTOR_BACKWARD);

    //Motor inferior izquierda
    if(movimiento.forward || movimiento.left || movimiento.tank_right)
      MOTOR_update(&motor_inferior_izquierda, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_inferior_izquierda, uart_velocidad_movimiento, MOTOR_BACKWARD);

    //Motor inferior derecha
    if(movimiento.forward || movimiento.right || movimiento.tank_left)
      MOTOR_update(&motor_inferior_derecha, uart_velocidad_movimiento, MOTOR_FORWARD);
    else
      MOTOR_update(&motor_inferior_derecha, uart_velocidad_movimiento, MOTOR_BACKWARD);
  }
}

static void UART_Actualizar()
{
  if(movimiento_uartComando_flag == 1)
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    UART_Entrar();
    movimiento_uartComando_flag = 0;
    movimiento_uartTimeout_flag = 0;
  }
  else if(movimiento_uartTimeout_flag == 1)
  {
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
    uart_estado_siguiente = UART_PARADO;
    UART_Entrar();
    movimiento_uartTimeout_flag = 0;

  }
}

void MOVIMIENTO_UART_CambiarEstado(uart_estado_t nuevo, uint16_t speed)
{
  movimiento_uartComando_flag = 1;
  uart_estado_siguiente = nuevo;
  uart_velocidad_movimiento = speed;
}

void convertir_ADC_a_PWM(int16_t vy, int16_t vx, int16_t omega)
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

  MOTOR_update(&motor_superior_izquierda, fl > 0 ? fl : -fl, fl > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
  MOTOR_update(&motor_superior_derecha,   fr > 0 ? fr : -fr, fr > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
  MOTOR_update(&motor_inferior_izquierda, rl > 0 ? rl : -rl, rl > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
  MOTOR_update(&motor_inferior_derecha,   rr > 0 ? rr : -rr, rr > 0 ? MOTOR_FORWARD : MOTOR_BACKWARD);
}

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
    // COMPLETAR: mover el coche en modo giro en funcion de la entrada del joystick
    convertir_ADC_a_PWM(joystick_y*2, 0, joystick_x*2);
    break;
  case JOYSTICK_DIAGONAL:
    // COMPLETAR: mover el coche en modo diagonal en funcion de la entrada del joystick
    convertir_ADC_a_PWM(joystick_y*2, joystick_x*2, 0); //MOVER ESTO AL DIAGONAL !!!!
    break;
  default:
    joystick_estado = JOYSTICK_GIRO;
    break;
  }
}

static void MOVIMIENTO_InitMotores()
{
  MOTOR_create(&htim3, TIM_CHANNEL_1, TIM_CHANNEL_2, &motor_superior_izquierda);
  MOTOR_create(&htim3, TIM_CHANNEL_4, TIM_CHANNEL_3, &motor_inferior_izquierda);
  MOTOR_create(&htim4, TIM_CHANNEL_2, TIM_CHANNEL_1, &motor_inferior_derecha);
  MOTOR_create(&htim4, TIM_CHANNEL_4, TIM_CHANNEL_3, &motor_superior_derecha);
}

static void MOVIMIENTO_PararMotores()
{
  MOTOR_update(&motor_superior_izquierda, 0, MOTOR_FORWARD);
  MOTOR_update(&motor_superior_derecha, 0, MOTOR_FORWARD);
  MOTOR_update(&motor_inferior_izquierda, 0, MOTOR_FORWARD);
  MOTOR_update(&motor_inferior_derecha, 0, MOTOR_FORWARD);
}

static void MOVIMIENTO_EntryColision()
{
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
  movimiento_colision_flag = 0;
  mov_estado = MOVIMIENTO_COLISION;
}

static inline void MOVIMIENTO_EntryUart()
{
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
}

/*** end of file ***/
