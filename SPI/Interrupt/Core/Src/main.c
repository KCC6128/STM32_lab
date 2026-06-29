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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define WHO_AM_I_MPU6500    0x75
#define PWR_MGMT_1          0x6B
#define ACCEL_XOUT_H        0x3B

#define MPU6500_CS_PORT     GPIOE
#define MPU6500_CS_PIN      GPIO_PIN_3

#define ACCEL_DATA_LEN      6
#define ACCEL_SPI_LEN       7   // 1 byte address + 6 bytes accel data
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */
static uint8_t accelTx_IT[ACCEL_SPI_LEN];
static uint8_t accelRx_IT[ACCEL_SPI_LEN];
static uint8_t accelData[ACCEL_DATA_LEN];

static volatile uint8_t accelBusy = 0;
static volatile uint8_t accelReady = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_UART5_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void UART_Print(char *str)
{
    HAL_UART_Transmit(&huart5, (uint8_t *)str, strlen(str), 100);
}

static void MPU6500_CS_LOW(void)
{
    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_RESET);
}

static void MPU6500_CS_HIGH(void)
{
    HAL_GPIO_WritePin(MPU6500_CS_PORT, MPU6500_CS_PIN, GPIO_PIN_SET);
}

static HAL_StatusTypeDef MPU6500_ReadReg(uint8_t reg, uint8_t *data)
{
    HAL_StatusTypeDef ret;
    uint8_t tx[2];
    uint8_t rx[2];

    tx[0] = reg | 0x80;   // read command
    tx[1] = 0x00;         // dummy byte

    MPU6500_CS_LOW();
    ret = HAL_SPI_TransmitReceive(&hspi2, tx, rx, 2, 100);
    MPU6500_CS_HIGH();

    if (ret == HAL_OK)
    {
        *data = rx[1];
    }

    return ret;
}

static HAL_StatusTypeDef MPU6500_WriteReg(uint8_t reg, uint8_t data)
{
    HAL_StatusTypeDef ret;
    uint8_t tx[2];

    tx[0] = reg & 0x7F;   // write command
    tx[1] = data;

    MPU6500_CS_LOW();
    ret = HAL_SPI_Transmit(&hspi2, tx, 2, 100);
    MPU6500_CS_HIGH();

    return ret;
}

static HAL_StatusTypeDef MPU6500_ReadAccel_IT(void)
{
    HAL_StatusTypeDef ret;

    if (accelBusy)
    {
        return HAL_BUSY;
    }

    accelTx_IT[0] = ACCEL_XOUT_H | 0x80;

    for (uint8_t i = 1; i < ACCEL_SPI_LEN; i++)
    {
        accelTx_IT[i] = 0x00;   // dummy bytes
    }

    accelBusy = 1;
    accelReady = 0;

    MPU6500_CS_LOW();

    ret = HAL_SPI_TransmitReceive_IT(&hspi2,
                                      accelTx_IT,
                                      accelRx_IT,
                                      ACCEL_SPI_LEN);

    if (ret != HAL_OK)
    {
        MPU6500_CS_HIGH();
        accelBusy = 0;
    }

    return ret;
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
  MX_SPI2_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */
	uint8_t readData = 0;
	HAL_StatusTypeDef ret;

	int16_t accelX = 0;
	int16_t accelY = 0;
	int16_t accelZ = 0;

	char msg[128];
	uint32_t lastAccelTick = 0;

	MPU6500_CS_HIGH();

	HAL_Delay(100);

	/* 1. Read WHO_AM_I */
	ret = MPU6500_ReadReg(WHO_AM_I_MPU6500, &readData);

	if (ret == HAL_OK)
	{
			snprintf(msg, sizeof(msg), "WHO_AM_I = 0x%02X\r\n", readData);
			UART_Print(msg);

			if (readData == 0x70)
			{
					UART_Print("MPU6500 detected.\r\n");
			}
			else
			{
					UART_Print("WHO_AM_I value is not 0x70.\r\n");
			}
	}
	else
	{
			UART_Print("Read WHO_AM_I failed.\r\n");
	}

	/* 2. Wake up MPU6500 */
	ret = MPU6500_WriteReg(PWR_MGMT_1, 0x00);

	if (ret == HAL_OK)
	{
			UART_Print("Write PWR_MGMT_1 success.\r\n");
	}
	else
	{
			UART_Print("Write PWR_MGMT_1 failed.\r\n");
	}

	HAL_Delay(100);

	/* 3. Read back PWR_MGMT_1 */
	ret = MPU6500_ReadReg(PWR_MGMT_1, &readData);

	if (ret == HAL_OK)
	{
			snprintf(msg, sizeof(msg), "PWR_MGMT_1 = 0x%02X\r\n\r\n", readData);
			UART_Print(msg);
	}
	else
	{
			UART_Print("Read PWR_MGMT_1 failed.\r\n\r\n");
	}

	UART_Print("Start reading ACCEL data by SPI interrupt...\r\n");
	
	ret = MPU6500_ReadAccel_IT();

	if (ret == HAL_OK)
	{
			UART_Print("First ACCEL SPI interrupt read started.\r\n");
	}
	else
	{
			UART_Print("First ACCEL SPI interrupt read failed.\r\n");
	}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if (accelReady)
		{
				accelReady = 0;

				accelX = (int16_t)((accelData[0] << 8) | accelData[1]);
				accelY = (int16_t)((accelData[2] << 8) | accelData[3]);
				accelZ = (int16_t)((accelData[4] << 8) | accelData[5]);

				float accelX_g, accelY_g, accelZ_g;

				accelX_g = accelX / 16384.0f;
				accelY_g = accelY / 16384.0f;
				accelZ_g = accelZ / 16384.0f;

				snprintf(msg, sizeof(msg),
								 "AX = %.3f g, AY = %.3f g, AZ = %.3f g\r\n",
								 accelX_g, accelY_g, accelZ_g);

				UART_Print(msg);

				HAL_Delay(500);

				ret = MPU6500_ReadAccel_IT();

				if ((ret != HAL_OK) && (ret != HAL_BUSY))
				{
						UART_Print("Restart ACCEL SPI interrupt read failed.\r\n");
				}
		}
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2)
    {
        MPU6500_CS_HIGH();

        accelData[0] = accelRx_IT[1];
        accelData[1] = accelRx_IT[2];
        accelData[2] = accelRx_IT[3];
        accelData[3] = accelRx_IT[4];
        accelData[4] = accelRx_IT[5];
        accelData[5] = accelRx_IT[6];

        accelBusy = 0;
        accelReady = 1;
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2)
    {
        MPU6500_CS_HIGH();

        accelBusy = 0;
        accelReady = 0;
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
