/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "string.h"
#include <stdio.h>
#include "hx711.h"
#include <stdbool.h>
#include "stm32f4xx_hal.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
#define MAX_LEN 8
uint8_t nRxData[MAX_LEN];
uint8_t nTxData[MAX_LEN];
uint8_t STX[1] = {0xFF}; 
uint8_t ETX[1] = {0x40};
uint8_t SRX[1] = {0xFE}; 
uint8_t ERX[1] = {0x40};

bool bDataAvailable;
uint8_t strCommand[2]; 
uint8_t strOpt[2];    
uint8_t strData[2];   

uint8_t sampleArray[2] = {0x00, 0x04};
uint8_t sampleArray1[2] = {0x00, 0x00};
uint8_t stroffset[4];
uint8_t strtile[2];
uint8_t strload[2];
uint8_t strweight[2];

hx711_t loadcell;
float weight;
float tilechuyendoi;
int32_t offset=0;
int32_t load_value;
float scale;
int calibb1=0;
int calibb2=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
/* USER CODE BEGIN PFP */

uint8_t *subString(uint8_t *s, int pos, int index)
{
	uint8_t *t = &s[pos];
	s[pos-1] = '\0';
	for (int i= index; i< (strlen((char *)t)+1);i++)
	{
		t[i]='\0';
	}
	return t;
}

bool StrCompare(uint8_t *pBuff, uint8_t *Sample, uint8_t nSize)
{
	for(int i=0;i<nSize;i++)
	{
		if (pBuff[i] != Sample[i])
		{
			return false;
		}
	}
	return true;
}

bool WriteComm(uint8_t *pBuff, uint8_t nSize)
{
	return HAL_UART_Transmit(&huart4,pBuff,nSize,1000);
}

bool ReadComm(uint8_t *pBuff, uint8_t nSize)
{
 
	if ((pBuff[0] == SRX[0]) && (pBuff[7] == ERX[0]))
	{
		memcpy(strCommand, subString(pBuff,1,2),2);
		memcpy(strOpt, subString(pBuff,3,2),2);
		memcpy(strData, subString(pBuff,5,2),2);
		bDataAvailable = true;
		
	}
	else
	{
		bDataAvailable = false;
	}
	return bDataAvailable;
}
int a=0;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // X? lý các s? ki?n UART nh?n du?c d? li?u
		if (huart->Instance== huart4.Instance)
		{
			ReadComm(nRxData, MAX_LEN);
			HAL_UART_Receive_IT(&huart4, (uint8_t *)nRxData, MAX_LEN);
			a+=1;
		}
    // Th?c hi?n các công vi?c c?n thi?t
}

bool serialProcesscalib(void)
{
	uint8_t nIndex =0;
	if (bDataAvailable == true)
	{
		if (StrCompare(strCommand,(uint8_t *)sampleArray,2))
		{
			if (StrCompare(strData,(uint8_t *)sampleArray1,2)) // doc offset khi ko load
			{
				memcpy(nTxData +nIndex,STX,1);
				nIndex+=1;
				uint8_t soffset[2] = {0x01, 0x04};
				memcpy(nTxData + nIndex,soffset,2);
				nIndex+=2;
				//memcpy(nTxData + nIndex,sampleArray1,2);
				//nIndex+=2;
				offset = hx711_value_ave(&loadcell,10);
				hx711_tare(&loadcell, 10);
				memcpy(stroffset, &offset, sizeof(int32_t));
				memcpy(nTxData + nIndex,stroffset,4);
				nIndex+=4;
				memcpy(nTxData +nIndex,ETX,1);
						
				WriteComm(nTxData,MAX_LEN);
				calibb1 +=1;
			}
			else
			{
				memcpy(nTxData +nIndex,STX,1);
				nIndex+=1;
				uint8_t stile[2] = {0x02, 0x04};
				memcpy(nTxData + nIndex,stile,2);
				nIndex+=2;
				
				uint16_t combinedValue = (strData[1] << 8) | strData[0];
				scale = (float)combinedValue;
				load_value = hx711_value_ave(&loadcell,10);
				hx711_calibration(&loadcell,offset,load_value,scale);
				tilechuyendoi= hx711_coef_get(&loadcell);
				int tile= (int)tilechuyendoi;
				memcpy(strtile, &tile, sizeof(int));
				memcpy(nTxData + nIndex,strtile,2);
				nIndex+=2;
				memcpy(strload, &load_value, sizeof(int32_t));
				memcpy(nTxData + nIndex,strload,2);
				nIndex+=2;
				memcpy(nTxData +nIndex,ETX,1);

				WriteComm(nTxData,MAX_LEN);
				calibb1 +=1;
				calibb2 =1;
			}
		}
		bDataAvailable = false;
		
	}
	return true;
}



void UART_SendData(float data)
{
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%.2f\r\n", data); // Format d? li?u thành chu?i
    HAL_UART_Transmit(&huart4, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY); // G?i chu?i qua UART
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
	HAL_UART_Receive_IT(&huart4, (uint8_t *)nRxData, MAX_LEN);
	// C?u hình chân GPIO cho PC1 và PC2
  GPIO_TypeDef* HX711_CLK_GPIO_Port = GPIOC;
  uint16_t HX711_CLK_Pin = GPIO_PIN_1;
  GPIO_TypeDef* HX711_DATA_GPIO_Port = GPIOC;
  uint16_t HX711_DATA_Pin = GPIO_PIN_2;
	

	hx711_init(&loadcell, HX711_CLK_GPIO_Port, HX711_CLK_Pin, HX711_DATA_GPIO_Port, HX711_DATA_Pin);
  //hx711_coef_set(&loadcell, 354.5); // read afer calibration  354.5
  //hx711_tare(&loadcell, 10);
  while (1)
  {
    //HAL_Delay(500);
    //weight = hx711_weight(&loadcell, 10);
		//UART_SendData(weight);
		if (calibb1 <2)
		{
			serialProcesscalib();
		}
		if (calibb1 ==2)
		{
			if (calibb2 ==0)
			{
				serialProcesscalib();
				calibb1=1;
			}
			else 
			{
				HAL_Delay(500);
				uint8_t nIndex =0;
				memcpy(nTxData +nIndex,STX,1);
				nIndex+=1;
				memcpy(nTxData + nIndex,strCommand,2);
				nIndex+=2;
				memcpy(nTxData + nIndex,strtile,2);
				nIndex+=2;
				
				weight = hx711_weight(&loadcell, 10);
				memcpy(strweight, &weight, sizeof(float));
				memcpy(nTxData + nIndex,strweight,2);
				nIndex+=2;
				memcpy(nTxData +nIndex,ETX,1);
						
				WriteComm(nTxData,MAX_LEN);
			}
		}
		if (a>2)
		{
			calibb1=0;
			a=1;
		}
		
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
 
  
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  
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
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SCK_GPIO_Port, SCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SCK_Pin */
  GPIO_InitStruct.Pin = SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DI_Pin */
  GPIO_InitStruct.Pin = DI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DI_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
