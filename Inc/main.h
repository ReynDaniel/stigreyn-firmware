/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LEAK_PC13_Pin GPIO_PIN_13
#define LEAK_PC13_GPIO_Port GPIOC
#define REED_SW_PC0_Pin GPIO_PIN_0
#define REED_SW_PC0_GPIO_Port GPIOC
#define BATT_ADC_PA0_Pin GPIO_PIN_0
#define BATT_ADC_PA0_GPIO_Port GPIOA
#define MOT_STBD_PA1_Pin GPIO_PIN_1
#define MOT_STBD_PA1_GPIO_Port GPIOA
#define UART_TX_PA2_Pin GPIO_PIN_2
#define UART_TX_PA2_GPIO_Port GPIOA
#define UART_TX_PA2A3_Pin GPIO_PIN_3
#define UART_TX_PA2A3_GPIO_Port GPIOA
#define MOT_PORT_PA5_Pin GPIO_PIN_5
#define MOT_PORT_PA5_GPIO_Port GPIOA
#define LED_STATUS_PB0_Pin GPIO_PIN_0
#define LED_STATUS_PB0_GPIO_Port GPIOB
#define LED_ARMED_PB1_Pin GPIO_PIN_1
#define LED_ARMED_PB1_GPIO_Port GPIOB
#define PI_SLEEP_PB4_Pin GPIO_PIN_4
#define PI_SLEEP_PB4_GPIO_Port GPIOB
#define IMU_SCL_PB8_Pin GPIO_PIN_8
#define IMU_SCL_PB8_GPIO_Port GPIOB
#define IMU_SDA_PB9_Pin GPIO_PIN_9
#define IMU_SDA_PB9_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
