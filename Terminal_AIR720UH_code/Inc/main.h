/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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


#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef int  s32;
typedef short  s16;
typedef char  s8;

typedef const s32 sc32;  /*!< Read Only */
typedef const s16 sc16;  /*!< Read Only */
typedef const s8 sc8;   /*!< Read Only */





typedef unsigned long long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef const u32 uc32;  /*!< Read Only */
typedef const u16 uc16;  /*!< Read Only */
typedef const u8 uc8;   /*!< Read Only */



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
#define WAKE_PIN2_Pin LL_GPIO_PIN_13
#define WAKE_PIN2_GPIO_Port GPIOC
#define CHARGE_STAT2_Pin LL_GPIO_PIN_2
#define CHARGE_STAT2_GPIO_Port GPIOB
#define CHARGE_STAT1_Pin LL_GPIO_PIN_10
#define CHARGE_STAT1_GPIO_Port GPIOB
#define GPS_PWR_CTL_Pin LL_GPIO_PIN_11
#define GPS_PWR_CTL_GPIO_Port GPIOB
#define LED_Pin LL_GPIO_PIN_8
#define LED_GPIO_Port GPIOC
#define MPU_INT_Pin LL_GPIO_PIN_11
#define MPU_INT_GPIO_Port GPIOA
#define LTE_PWRKEY_Pin LL_GPIO_PIN_12
#define LTE_PWRKEY_GPIO_Port GPIOA
#define LTE_WKP_MOD_Pin LL_GPIO_PIN_15
#define LTE_WKP_MOD_GPIO_Port GPIOA
#define LTE_PWR_CTL_Pin LL_GPIO_PIN_12
#define LTE_PWR_CTL_GPIO_Port GPIOC
#define BLE_ROLE_Pin LL_GPIO_PIN_2
#define BLE_ROLE_GPIO_Port GPIOD
#define BLE_STATE_Pin LL_GPIO_PIN_3
#define BLE_STATE_GPIO_Port GPIOB
#define BLE_RST_Pin LL_GPIO_PIN_4
#define BLE_RST_GPIO_Port GPIOB
#define BLE_WKP_Pin LL_GPIO_PIN_5
#define BLE_WKP_GPIO_Port GPIOB
#define BLE_DATA_Pin LL_GPIO_PIN_8
#define BLE_DATA_GPIO_Port GPIOB
#define BLE_MOD_Pin LL_GPIO_PIN_9
#define BLE_MOD_GPIO_Port GPIOB
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif
/* USER CODE BEGIN Private defines */
//#define RTC_USE_HAL_LIB
//#define IIC_USE_HAL_LIB
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
