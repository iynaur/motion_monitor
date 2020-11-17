
#include "low_power.h"
#include "myrtc.h"


/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE��LSE, PLL
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
  * ���ܣ�����͹���ģʽ
  * LowPowerMode��ѡ�����͹��ĵ�ģʽ�������ǣ�
  *               LL_PWR_MODE_STOP0
  *               LL_PWR_MODE_STOP1
  *               LL_PWR_MODE_STOP2
  *               LL_PWR_MODE_STANDBY
  *               LL_PWR_MODE_SHUTDOWN
  *
  * ���أ���
*/
void Entry_Low_Power(uint32_t LowPowerMode)
{
  LL_ADC_Disable(ADC1);//����͹���֮ǰ�ر�ADC
	LL_PWR_SetPowerMode(LowPowerMode);//���õ͹���ģʽ                              
	LL_LPM_EnableDeepSleep(); //�����������
	__WFI(); //����͹���
	
	/* Reset SLEEPDEEP bit of Cortex System Control Register */
	LL_LPM_EnableSleep();
	
	SYSCLKConfig_STOP();//�ָ��͹���֮ǰ��ʱ��
  LL_ADC_Enable(ADC1);//����֮����ADC
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

//RTC 16��Ƶ WakeUp time base = 16/32768 = 0.48828125ms ~= 0.5ms
//WakeUpCounter���Ϊ0xFFFF ��ʱ���Ϊ 65535 * 0.5 = 32s
//����nms���Ϊ32768
//Stop2ģʽ�µ���ʱ�����������������ͨ��ʱ�Խ��͹���
void Stop2_delay_ms(u16 nms)
{
	RTC_WakeUp_Config(RTC_WAKEUPCLOCK_DIV_16, nms * 2 - 1);
	Entry_Low_Power(LL_PWR_MODE_STOP2);
}

//ns���Ϊ65536
void Stop2_delay_s(u16 ns)
{
	RTC_WakeUp_Config(RTC_WAKEUPCLOCK_CKSPRE, ns - 1);//1Hz
	Entry_Low_Power(LL_PWR_MODE_STOP2);
}


//Shutdownģʽ�µ���ʱ��Ҫ���ڳ�ʱ�����߶�ʱ���ѣ�����֮�����λ
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


