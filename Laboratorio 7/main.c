/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Laboratorio 7 - DAC y PWM (PA0 y PA4)
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"
#include <math.h>
#include <string.h>

/* Private define ------------------------------------------------------------*/
#define SAMPLES 256

// Notas PWM (Frecuencias para Timer 2 con Prescaler 83)
#define PWM_C4  3822
#define PWM_D4  3405
#define PWM_E4  3033
#define PWM_F4  2863
#define PWM_G4  2551
#define PWM_A4  2272
#define PWM_B4  2024
#define PWM_PAUSA 0

// Notas DAC (Valores de ARR para Timer 6 con Prescaler 41)
#define DAC_C4  150
#define DAC_D4  134
#define DAC_E4  119
#define DAC_F4  112
#define DAC_G4  100
#define DAC_A4  89
#define DAC_PAUSA 0

/* Peripherals handlers */
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
UART_HandleTypeDef huart2;

/* Variables */
uint16_t audio_buffer[SAMPLES];
uint8_t uart_rx[1];
volatile uint8_t opcion_seleccionada = 0;

uint16_t melodia_dac[] = {DAC_C4, DAC_C4, DAC_G4, DAC_G4, DAC_A4, DAC_A4, DAC_G4, DAC_PAUSA};
uint16_t melodia_pwm[] = {PWM_E4, PWM_E4, PWM_F4, PWM_G4, PWM_G4, PWM_F4, PWM_E4, PWM_PAUSA};

/* Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART2_UART_Init(void);
void Mostrar_Menu(void);

int main(void) {
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_USART2_UART_Init();

  // Llenar buffer DAC (Senoide)
  for (int i = 0; i < SAMPLES; i++) {
    audio_buffer[i] = (uint16_t)((sin(i * 2 * M_PI / SAMPLES) + 1.0) * 2047.0);
  }

  Mostrar_Menu();
  HAL_UART_Receive_IT(&huart2, uart_rx, 1);

  while (1) {
    if (opcion_seleccionada == '1') {
      char m1[] = "\r\nTocando DAC en PA4...\r\n";
      HAL_UART_Transmit(&huart2, (uint8_t*)m1, strlen(m1), 100);
      for(int i=0; i<(sizeof(melodia_dac)/2); i++) {
        if(melodia_dac[i]==0) HAL_Delay(300);
        else {
          __HAL_TIM_SET_AUTORELOAD(&htim6, melodia_dac[i]);
          HAL_TIM_Base_Start(&htim6);
          HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)audio_buffer, SAMPLES, DAC_ALIGN_12B_R);
          HAL_Delay(400);
          HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
          HAL_TIM_Base_Stop(&htim6);
          HAL_Delay(50);
        }
      }
      opcion_seleccionada = 0; Mostrar_Menu();
    }
    else if (opcion_seleccionada == '2') {
      char m2[] = "\r\nTocando PWM en PA0...\r\n";
      HAL_UART_Transmit(&huart2, (uint8_t*)m2, strlen(m2), 100);
      for(int i=0; i<(sizeof(melodia_pwm)/2); i++) {
        if(melodia_pwm[i]==0) HAL_Delay(300);
        else {
          __HAL_TIM_SET_AUTORELOAD(&htim2, melodia_pwm[i]);
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, melodia_pwm[i]/2); // 50% Duty Cycle
          HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
          HAL_Delay(400);
          HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
          HAL_Delay(50);
        }
      }
      opcion_seleccionada = 0; Mostrar_Menu();
    }
  }
}

/* --- CONFIGURACIÓN CRÍTICA DE PINES --- */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // PA0 -> TIM2_CH1 (PWM)
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; // Modo Función Alternativa
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2; // ¡ESTO ES LO QUE CONECTA EL TIMER AL PIN!
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // PA4 -> DAC_OUT1
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG; // Modo Analógico para el DAC
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* --- CONFIGURACIÓN DE PERIFÉRICOS --- */
static void MX_TIM2_Init(void) {
  TIM_OC_InitTypeDef sConfigOC = {0};
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83; // Para clock de 84MHz -> 1MHz
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_PWM_Init(&htim2);
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);
}

static void MX_DAC_Init(void) {
  DAC_ChannelConfTypeDef sConfig = {0};
  hdac.Instance = DAC;
  HAL_DAC_Init(&hdac);
  sConfig.DAC_Trigger = DAC_TRIGGER_T6_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
}

static void MX_TIM6_Init(void) {
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 41;
  htim6.Init.Period = 100;
  HAL_TIM_Base_Init(&htim6);
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

static void MX_USART2_UART_Init(void) {
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.Mode = UART_MODE_TX_RX;
  HAL_UART_Init(&huart2);
}

static void MX_DMA_Init(void) {
  __HAL_RCC_DMA1_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

void Mostrar_Menu(void) {
  char menu[] = "\r\n1. DAC (PA4)\r\n2. PWM (PA0)\r\nSeleccione: ";
  HAL_UART_Transmit(&huart2, (uint8_t*)menu, strlen(menu), 100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART2) {
    opcion_seleccionada = uart_rx[0];
    HAL_UART_Receive_IT(&huart2, uart_rx, 1);
  }
}

void Error_Handler(void) { while(1); }
