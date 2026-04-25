/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM_FREQ 84000000
#define PWM_ARR 100
#define REST 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
volatile uint8_t jumpRequest = 0;
volatile uint8_t jumpEvent   = 0;
volatile uint8_t moveCmd     = 0;
volatile uint16_t currentNoteIndex = 0;
volatile uint8_t melodyInterrupted = 0;

uint8_t buffer[10];
uint8_t dato = 80;
uint8_t datoArray[3];
uint8_t datoFrec[20];

uint8_t rx6;
volatile uint8_t datoRecibido = 0;

const uint16_t jump_notes[] = {
    1062, // G5
    946,  // A5
    795,  // C6
    REST
};

const uint16_t jump_durations[] = {
    60,
    60,
    120,
    20
};

const uint16_t notes[] = {
    3210, 2859, 2547, 2142, 1908, 2142, 2547, 2859,
    3210, 2547, 2142, 1908, 2142, 2547, 2859, 3210,

    1908, 2142, 2547, 2859, 3210, 2859, 2547, 2142,
    1908, 2142, 2547, 1908, 2142, 2547, 2859, 3210,

    2142, 1908, 1700, 1908, 2142, 2547, 2142, 1908,
    1700, 1908, 2142, 2547, 2859, 3210, 2859, 2547,

    2142, 2547, 2859, 3210, 2859, 2547, 2142, 1908,
    2142, 2547, 2859, 2142, 1908, 1700, 1908, 2142,

    3210, 2859, 2547, 2142, 1908, 2142, 2547, 2859,
    3210, 2547, 2142, 1908, 2142, 2547, 2859, 3210,

    1908, 2142, 2547, 2859, 3210, 2859, 2547, 2142,
    1908, 2142, 2547, 1908, 2142, 2547, 2859, 3210,

    2142, 1908, 1700, 1908, 2142, 2547, 2859, 3210,
    2859, 2547, 2142, 1908, 2142, 2547, 2859, 3210
};

const uint16_t durations[] = {
    250,250,250,250,250,250,250,250,
    250,250,250,250,250,250,250,250,

    250,250,250,250,250,250,250,250,
    250,250,250,250,250,250,250,250,

    250,250,250,250,250,250,250,250,
    250,250,250,250,250,250,250,250,

    250,250,250,250,250,250,250,250,
    250,250,250,250,250,250,250,250,

    250,250,250,250,250,250,250,250,
    250,250,250,250,250,250,250,250,

    250,250,250,250,250,250,250,250,
    250,250,250,250,250,250,250,250,

    250,250,250,250,250,250,250,250,
    250,250,250,250,250,250,250,250,

    250,250,250,250,250,250,250,500,
    250,250,250,250,250,250,250,1000
};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
uint8_t playTone(uint16_t prescaler, uint16_t timeMs);
void playMelody(void);
void playJumpSound(void);
void processRxCommands(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void processRxCommands(void)
{
    if (jumpEvent)
    {
        jumpEvent = 0;
        HAL_UART_Transmit(&huart2, (uint8_t*)"Recibi: Salto\r\n", 16, 1000);
    }

    if (moveCmd != 0)
    {
        char msgRx[40];

        switch (moveCmd)
        {
            case 'F': snprintf(msgRx, sizeof(msgRx), "Recibi: Adelante\r\n"); break;
            case 'B': snprintf(msgRx, sizeof(msgRx), "Recibi: Atras\r\n"); break;
            case 'L': snprintf(msgRx, sizeof(msgRx), "Recibi: Izquierda\r\n"); break;
            case 'R': snprintf(msgRx, sizeof(msgRx), "Recibi: Derecha\r\n"); break;
            case 'S': snprintf(msgRx, sizeof(msgRx), "Recibi: Reposo\r\n"); break;
            default:  msgRx[0] = '\0'; break;
        }

        if (msgRx[0] != '\0')
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)msgRx, strlen(msgRx), 1000);
        }

        moveCmd = 0;
    }
}

uint8_t playTone(uint16_t prescaler, uint16_t timeMs)
{
    if (prescaler == REST)
    {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);

        for (uint16_t t = 0; t < timeMs; t++)
        {
            processRxCommands();

            if (jumpRequest)
            {
                return 1;
            }
            HAL_Delay(1);
        }
        return 0;
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
    __HAL_TIM_SET_PRESCALER(&htim1, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim1, PWM_ARR - 1);
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    HAL_TIM_GenerateEvent(&htim1, TIM_EVENTSOURCE_UPDATE);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, PWM_ARR / 2);

    for (uint16_t t = 0; t < timeMs; t++)
    {
        processRxCommands();

        if (jumpRequest)
        {
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
            return 1;
        }
        HAL_Delay(1);
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);

    for (uint16_t t = 0; t < 30; t++)
    {
        processRxCommands();

        if (jumpRequest)
        {
            return 1;
        }
        HAL_Delay(1);
    }

    return 0;
}

void playJumpSound(void)
{
    uint16_t len = sizeof(jump_notes) / sizeof(jump_notes[0]);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    for (uint16_t i = 0; i < len; i++)
    {
        if (jump_notes[i] == REST)
        {
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
            HAL_Delay(jump_durations[i]);
        }
        else
        {
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
            __HAL_TIM_SET_PRESCALER(&htim1, jump_notes[i]);
            __HAL_TIM_SET_AUTORELOAD(&htim1, PWM_ARR - 1);
            __HAL_TIM_SET_COUNTER(&htim1, 0);
            HAL_TIM_GenerateEvent(&htim1, TIM_EVENTSOURCE_UPDATE);
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, PWM_ARR / 2);
            HAL_Delay(jump_durations[i]);
        }
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
}

void playMelody(void)
{
    uint16_t len = sizeof(notes) / sizeof(notes[0]);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    for (; currentNoteIndex < len; currentNoteIndex++)
    {
    	if (playTone(notes[currentNoteIndex], durations[currentNoteIndex]))
    	{
    	    jumpRequest = 0;
    	    melodyInterrupted = 1;

    	    if (currentNoteIndex < len - 1)
    	    {
    	        currentNoteIndex++;
    	    }

    	    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);
    	    playJumpSound();
    	    return;
    	}
    }

    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_4);

    // Si llegó al final, reinicia la canción
    currentNoteIndex = 0;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart6, &rx6, 1);
    uint8_t Test[] = "Hola mundo!\r\n";
      HAL_UART_Transmit(&huart2, Test, sizeof(Test)-1, 1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  playMelody();
	  processRxCommands();


  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 100-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 50;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART6)
    {
        if (rx6 != '\r' && rx6 != '\n')
        {
            if (rx6 == 'J')
            {
                jumpRequest = 1;
                jumpEvent = 1;
            }
            else if (rx6 == 'F' || rx6 == 'B' || rx6 == 'L' ||
                     rx6 == 'R' || rx6 == 'S')
            {
                moveCmd = rx6;
            }
        }

        HAL_UART_Receive_IT(&huart6, &rx6, 1);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
