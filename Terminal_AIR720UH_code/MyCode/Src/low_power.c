
#include "low_power.h"
#include "myrtc.h"


/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE、LSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
void SYSCLKConfig_STOP(void)
{
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
	
  LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
  LL_RCC_LSE_Enable();
   /* Wait till LSE is ready */
  while(LL_RCC_LSE_IsReady() != 1)
  {
    
  }
  //LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
  LL_RCC_EnableRTC();
	
  //LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 20, LL_RCC_PLLR_DIV_4);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_Enable();
   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
    
  }
	
  //LL_RCC_PLLSAI1_ConfigDomain_ADC(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 8, LL_RCC_PLLSAI1R_DIV_2);
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

	#if 0
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  LL_Init1msTick(80000000);

  LL_SetSystemCoreClock(80000000);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
  LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
  LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);
  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_SYSCLK);
  LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_PLLSAI1);
	#endif
}



/**
  * 功能：进入低功耗模式
  * LowPowerMode：选择进入低功耗的模式，可以是：
  *               LL_PWR_MODE_STOP0
  *               LL_PWR_MODE_STOP1
  *               LL_PWR_MODE_STOP2
  *               LL_PWR_MODE_STANDBY
  *               LL_PWR_MODE_SHUTDOWN
  *
  * 返回：无
*/
void Entry_Low_Power(uint32_t LowPowerMode)
{
  LL_ADC_Disable(ADC1);//进入低功耗之前关闭ADC
	LL_PWR_SetPowerMode(LowPowerMode);//设置低功耗模式                              
	LL_LPM_EnableDeepSleep(); //允许深度休眠
	__WFI(); //进入低功耗
	
	/* Reset SLEEPDEEP bit of Cortex System Control Register */
	LL_LPM_EnableSleep();
	
	SYSCLKConfig_STOP();//恢复低功耗之前的时钟
  LL_ADC_Enable(ADC1);//唤醒之后开启ADC
}

/**
  * @brief  Enable the WakeUp PINx functionality
  * @param  WakeUpPin This parameter can be one of the following values:
  *         @arg @ref LL_PWR_WAKEUP_PIN1
  *         @arg @ref LL_PWR_WAKEUP_PIN2
  *         @arg @ref LL_PWR_WAKEUP_PIN3
  *         @arg @ref LL_PWR_WAKEUP_PIN4
  *         @arg @ref LL_PWR_WAKEUP_PIN5
  *	@param	WakeUpPin polarity value
  *					@arg @ref HIGH
  *         @arg @ref LOW
  * @retval None
  */
void LowPower_EnableWakeUpPin(uint32_t WakeUpPin, POLARITY polarity)
{
	LL_PWR_EnableWakeUpPin(WakeUpPin);
	
	if(polarity == HIGH)
		LL_PWR_SetWakeUpPinPolarityHigh(WakeUpPin);
	else if(polarity == LOW)
		LL_PWR_SetWakeUpPinPolarityLow(WakeUpPin);
	
	switch(WakeUpPin)
	{
		case LL_PWR_WAKEUP_PIN1:
			LL_PWR_ClearFlag_WU1();
			break;
		case LL_PWR_WAKEUP_PIN2:
			LL_PWR_ClearFlag_WU2();
			break;
		case LL_PWR_WAKEUP_PIN3:
			LL_PWR_ClearFlag_WU3();
			break;
		case LL_PWR_WAKEUP_PIN4:
			LL_PWR_ClearFlag_WU4();
			break;
		case LL_PWR_WAKEUP_PIN5:
			LL_PWR_ClearFlag_WU5();
			break;
		default:
			break;
	}
}

//RTC 16分频 WakeUp time base = 16/32768 = 0.48828125ms ~= 0.5ms
//WakeUpCounter最大为0xFFFF 计时最大为 65535 * 0.5 = 32s
//所以nms最大为32768
//Stop2模式下的延时可用于替代程序中普通延时以降低功耗
void Stop2_delay_ms(u16 nms)
{
	RTC_WakeUp_Config(RTC_WAKEUPCLOCK_DIV_16, nms * 2 - 1);
	Entry_Low_Power(LL_PWR_MODE_STOP2);
}

//ns最大为65536
void Stop2_delay_s(u16 ns)
{
	RTC_WakeUp_Config(RTC_WAKEUPCLOCK_CKSPRE, ns - 1);//1Hz
	Entry_Low_Power(LL_PWR_MODE_STOP2);
}


//Shutdown模式下的延时主要用于长时间休眠定时唤醒，唤醒之后程序复位
void Shutdown_delay_ms(u16 nms)
{
	RTC_WakeUp_Config(RTC_WAKEUPCLOCK_DIV_16, nms * 2 - 1);
	Entry_Low_Power(LL_PWR_MODE_SHUTDOWN);
}

void Shutdown_delay_s(u16 ns)
{
	RTC_WakeUp_Config(RTC_WAKEUPCLOCK_CKSPRE, ns - 1);//1Hz
	Entry_Low_Power(LL_PWR_MODE_SHUTDOWN);
}


