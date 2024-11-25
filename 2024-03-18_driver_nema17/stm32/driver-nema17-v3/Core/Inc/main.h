/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32c0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#define VERSION "NEMA17 DRV 20241125\r\n by Scuola di Robotica\r\n «impara giocando»\r\n https://youtube.com/live/cfAlLIG2amo\r\n"

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
#define AN0_DEG_Pin GPIO_PIN_0
#define AN0_DEG_GPIO_Port GPIOA
#define U2_TX_Pin GPIO_PIN_2
#define U2_TX_GPIO_Port GPIOA
#define U2_RX_Pin GPIO_PIN_3
#define U2_RX_GPIO_Port GPIOA
#define A4988_nEN_Pin GPIO_PIN_4
#define A4988_nEN_GPIO_Port GPIOA
#define A4988_MS1_Pin GPIO_PIN_5
#define A4988_MS1_GPIO_Port GPIOA
#define A4988_MS2_Pin GPIO_PIN_6
#define A4988_MS2_GPIO_Port GPIOA
#define A4988_MS3_Pin GPIO_PIN_7
#define A4988_MS3_GPIO_Port GPIOA
#define A4988_nRST_Pin GPIO_PIN_0
#define A4988_nRST_GPIO_Port GPIOB
#define A4988_nSLEEP_Pin GPIO_PIN_1
#define A4988_nSLEEP_GPIO_Port GPIOB
#define A4988_STEP_Pin GPIO_PIN_2
#define A4988_STEP_GPIO_Port GPIOB
#define A4988_DIR_Pin GPIO_PIN_10
#define A4988_DIR_GPIO_Port GPIOB
#define ADD0_Pin GPIO_PIN_13
#define ADD0_GPIO_Port GPIOB
#define ADD1_Pin GPIO_PIN_14
#define ADD1_GPIO_Port GPIOB
#define ADD2_Pin GPIO_PIN_15
#define ADD2_GPIO_Port GPIOB
#define ADD3_Pin GPIO_PIN_8
#define ADD3_GPIO_Port GPIOA
#define ADD4_Pin GPIO_PIN_9
#define ADD4_GPIO_Port GPIOA
#define ADD5_Pin GPIO_PIN_6
#define ADD5_GPIO_Port GPIOC
#define ADD6_Pin GPIO_PIN_7
#define ADD6_GPIO_Port GPIOC
#define ADD7_Pin GPIO_PIN_10
#define ADD7_GPIO_Port GPIOA
#define U1_TXEN_485_Pin GPIO_PIN_3
#define U1_TXEN_485_GPIO_Port GPIOB
#define U1_TX_485_Pin GPIO_PIN_6
#define U1_TX_485_GPIO_Port GPIOB
#define U1_RX_485_Pin GPIO_PIN_7
#define U1_RX_485_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
