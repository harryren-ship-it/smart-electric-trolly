/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_dwt.h"
#include "bsp_oled.h"
#include "bsp_siic.h"
#include "MPU6050.h"
#include "k210.h"
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
#define OLED_SCL_Pin GPIO_PIN_14
#define OLED_SCL_GPIO_Port GPIOC
#define ranger_tri_Pin GPIO_PIN_15
#define ranger_tri_GPIO_Port GPIOC
#define IIC_SCL_Pin GPIO_PIN_14
#define IIC_SCL_GPIO_Port GPIOB
#define IIC_SDA_Pin GPIO_PIN_15
#define IIC_SDA_GPIO_Port GPIOB
#define OLED_DC_Pin GPIO_PIN_3
#define OLED_DC_GPIO_Port GPIOB
#define OLED_RES_Pin GPIO_PIN_4
#define OLED_RES_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_5
#define OLED_SDA_GPIO_Port GPIOB
#define MPU6050_INT_Pin GPIO_PIN_9
#define MPU6050_INT_GPIO_Port GPIOB
#define MPU6050_INT_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */
#define C10B_HEAD1 0xCC
#define C10B_HEAD2 0xDD
#pragma pack(1) 
typedef struct {
	uint8_t Head1; 
    uint8_t Head2;
	uint8_t k210_alive;
	uint16_t k210_cx; 
	uint16_t k210_size;
	uint16_t color;
	short temperature;
	uint8_t BCCcheck;          
}C10B_Sendmsg;
#pragma pack() 
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
