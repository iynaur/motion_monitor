/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "iwdg.h"
#include "usart.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay.h"
#include "myadc.h"
#include "myiic.h"
#include "myiwdg.h"
#include "myrtc.h"
#include "mytim.h"
#include "myusart.h"
#include "stmflash.h"
#include "low_power.h"
#include "spi_flash.h"

#include "commu.h"

#include "air720sl.h"
#include "bt02.h"
#include "gps.h"
#include "icm20689.h"

#include "logic_app.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
	u16 unsend_num = 0;
	u32 unsend_init_flag = 0;
	u32 Phead = NULL, Ptail = NULL;
//#ifndef RTC_USE_HAL_LIB	
//	LL_RTC_TimeTypeDef rtc_time = {0};
//	LL_RTC_DateTypeDef rtc_date = {0};
//#else
//  RTC_TimeTypeDef rtc_time = {0};
//	RTC_DateTypeDef rtc_date = {0};
//#endif
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	delay_init();//延时初始化
	delay_ms(200);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
	LPUART1_Init(115200);//LPUART1初始化，用于调试输出打印信息
	USART1_Init(19200);//串口1初始化，用于蓝牙模块BT02通讯
	USART2_Init(9600);//串口2初始化，用于GPS模块通讯
	USART3_Init(115200);//串口3初始化，用于4G模块AIR720SL通讯
	I2C1_Init();
	RTC_Init();
	ADC_Init();
	TIM2_Init(10-1, 4000-1);//定时器初始化，1ms
	IWDG_Init(LL_IWDG_PRESCALER_256, 4095); // 256 / 32 * 4096 = 32768ms;
	debug_print("APP Start!\r\n");
	device_id = Get_Device_ID();
	debug_print("APP----ID is %d.\r\n", device_id);
	debug_print("Software Version is %s.\r\n", Version);
	
	while(ICM20689_Init())//六轴ICM20689初始化
	{
  	LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		debug_print("ICM20689 init failed!\r\n");
		delay_s(2);
	}
	debug_print("Init OK!\r\n");
	
	BT02_Init();//蓝牙模块BT02初始化
	
	//memset((void*)&oil_data, 0, sizeof(ble_oil_data_t));//油箱数据初始化为0

	/*检查FLASH中未发送数据条数，如果超过最大值或者味初始化，则清零*/
	STMFLASH_Read(STMFLASH_UNSEND_DATA_INIT_FLAG_ADDR, (u8*)&unsend_init_flag, 4);
	if(unsend_init_flag != 0x98765432)
	{
		unsend_num = 0;
		Phead = STMFLASH_UNSEND_DATA_ADDR;
		Ptail = STMFLASH_UNSEND_DATA_ADDR;

		STMFLASH_Write(STMFLASH_UNSEND_DATA_NUM_ADDR, (u8*)&unsend_num, 2);
		STMFLASH_Write(STMFLASH_UNSEND_DATA_PHEAD_ADDR, (u8*)&Phead, 4);
		STMFLASH_Write(STMFLASH_UNSEND_DATA_PTAIL_ADDR, (u8*)&Ptail, 4);

		unsend_init_flag = 0x98765432;
		STMFLASH_Write(STMFLASH_UNSEND_DATA_INIT_FLAG_ADDR, (u8*)&unsend_init_flag, 4);
	}
	
	Device_Check_LowPowerMode(0);

	Check_Time_with_Server();//与服务器校时，设置RTC时间
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  	//LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		//debug_print("Hello Macao!\r\n");
		//delay_s(1);
		APP_Run();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);

   if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2)
  {
  Error_Handler();  
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
    
  }
  LL_RCC_LSI_Enable();

   /* Wait till LSI is ready */
  while(LL_RCC_LSI_IsReady() != 1)
  {
    
  }
  LL_PWR_EnableBkUpAccess();
  LL_RCC_ForceBackupDomainReset();
  LL_RCC_ReleaseBackupDomainReset();
  LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
  LL_RCC_LSE_Enable();

   /* Wait till LSE is ready */
  while(LL_RCC_LSE_IsReady() != 1)
  {
    
  }
  LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
  LL_RCC_EnableRTC();
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 20, LL_RCC_PLLR_DIV_4);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
    
  }
  LL_RCC_PLLSAI1_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLSAI1R_DIV_2);
  LL_RCC_PLLSAI1_EnableDomain_ADC();
  LL_RCC_PLLSAI1_Enable();

   /* Wait till PLLSAI1 is ready */
  while(LL_RCC_PLLSAI1_IsReady() != 1)
  {
    
  }
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(40000000);

  LL_SetSystemCoreClock(40000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
  LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_SYSCLK);
  LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_PLLSAI1);
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
