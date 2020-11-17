

#include "myrtc.h"
#include "rtc.h"
#include "commu.h"

void RTC_Init(void)
{
	MX_RTC_Init();
}
/*
* WakeupClock：RTC唤醒时钟分频，可以是
* 						 RTC_WAKEUPCLOCK_DIV_16
* 						 RTC_WAKEUPCLOCK_DIV_8
* 						 RTC_WAKEUPCLOCK_DIV_4
* 						 RTC_WAKEUPCLOCK_DIV_2
* 						 RTC_WAKEUPCLOCK_CKSPRE
* 						 RTC_WAKEUPCLOCK_CKSPRE_WUT
*              RTC_WKP_DISABLE
* 	
* sleep_time：设置RTC唤醒的时间，time = sleep_time * (WakeupClock / 32.768KHz)
*/
void RTC_WakeUp_Config(uint32_t WakeupClock, u16 RTC_WUT_Time)
{
	/* Disable RTC registers write protection */
  //LL_RTC_DisableWriteProtection(RTC);
  WRITE_REG(RTC->WPR, 0xCAU);
  WRITE_REG(RTC->WPR, 0x53U);

	
	/* Disable wake up timer to modify it */
  //LL_RTC_WAKEUP_Disable(RTC);
	CLEAR_BIT(RTC->CR, RTC_CR_WUTE);
	
	/* In case of interrupt mode is used, the interrupt source must disabled */
	//LL_RTC_DisableIT_WUT(RTC);
	CLEAR_BIT(RTC->CR, RTC_CR_WUTIE);
	
	/* Reset Internal Wake up flag */
  //LL_RTC_ClearFlag_WUT(RTC); 
	WRITE_REG(RTC->ISR, (~((RTC_ISR_WUTF | RTC_ISR_INIT) & 0x0000FFFFU) | (RTC->ISR & RTC_ISR_INIT)));
	
  /* Wait until it is allow to modify wake up reload value */
  //while (LL_RTC_IsActiveFlag_WUTW(RTC) != 1)
	while((READ_BIT(RTC->ISR, RTC_ISR_WUTWF) == (RTC_ISR_WUTWF)) != 1)
  {
  
  }

	if(WakeupClock != RTC_WKP_DISABLE)
	{
	  //LL_RTC_WAKEUP_SetAutoReload(RTC, RTC_WUT_Time);
	  //LL_RTC_WAKEUP_SetClock(RTC, WakeupClock);
		MODIFY_REG(RTC->WUTR, RTC_WUTR_WUT, RTC_WUT_Time);
		MODIFY_REG(RTC->CR, RTC_CR_WUCKSEL, WakeupClock);

		//LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_20);
		//LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_20);
		SET_BIT(EXTI->IMR1, EXTI_IMR1_IM20);
		SET_BIT(EXTI->RTSR1, EXTI_IMR1_IM20);
		
	  /* Enable wake up counter and wake up interrupt */
	  /* Note: Periodic wakeup interrupt should be enabled to exit the device 
	     from low-power modes.*/
	  //LL_RTC_EnableIT_WUT(RTC);
	  //LL_RTC_WAKEUP_Enable(RTC);
		SET_BIT(RTC->CR, RTC_CR_WUTIE);
		SET_BIT(RTC->CR, RTC_CR_WUTE);
		
	  /* Check that PWR Internal Wake-up is enabled */
	  //if (LL_PWR_IsEnabledInternWU() == 0)
		if (((READ_BIT(PWR->CR3, PWR_CR3_EIWF) == (PWR_CR3_EIWF)) ? 1UL : 0UL) == 0)
	  {
	    /* Need to enable the Internal Wake-up line */
	    //LL_PWR_EnableInternWU();
			SET_BIT(PWR->CR3, PWR_CR3_EIWF);
	  }
	}
	
  /* Enable RTC registers write protection */
  //LL_RTC_EnableWriteProtection(RTC);
	WRITE_REG(RTC->WPR, 0xFFU);
}

#ifndef RTC_USE_HAL_LIB

ErrorStatus RTC_Set_Time(u8 hour, u8 min, u8 sec)
{
	LL_RTC_TimeTypeDef RTC_TimeStructure = {0};

	RTC_TimeStructure.Hours   = hour;
	RTC_TimeStructure.Minutes = min;
	RTC_TimeStructure.Seconds = sec;
	RTC_TimeStructure.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;

	return LL_RTC_TIME_Init(RTC,LL_RTC_FORMAT_BIN,&RTC_TimeStructure);
	
}

ErrorStatus RTC_Set_Date(u8 year, u8 month, u8 date, u8 week)
{
	LL_RTC_DateTypeDef RTC_DateStructure = {0};

	RTC_DateStructure.Year = year;
	RTC_DateStructure.Month = month;
	RTC_DateStructure.Day = date;
	RTC_DateStructure.WeekDay = week;

	return LL_RTC_DATE_Init(RTC,LL_RTC_FORMAT_BIN,&RTC_DateStructure);
}

void RTC_Time_Get(LL_RTC_TimeTypeDef *RTC_TimeStruct)
{
	u32 time = 0;

	time = LL_RTC_TIME_Get(RTC);

	RTC_TimeStruct->Hours   = __LL_RTC_CONVERT_BCD2BIN((time >> 16)&0xFF);
	RTC_TimeStruct->Minutes = __LL_RTC_CONVERT_BCD2BIN((time >>  8)&0xFF);
	RTC_TimeStruct->Seconds = __LL_RTC_CONVERT_BCD2BIN((time >>  0)&0xFF);
}

void RTC_Date_Get(LL_RTC_DateTypeDef *RTC_DateStruct)
{
	u32 date = 0;

	date = LL_RTC_DATE_Get(RTC);

	RTC_DateStruct->WeekDay = __LL_RTC_CONVERT_BCD2BIN((date>> 24)&0xFF);
	RTC_DateStruct->Day     = __LL_RTC_CONVERT_BCD2BIN((date>> 16)&0xFF);
	RTC_DateStruct->Month   = __LL_RTC_CONVERT_BCD2BIN((date>>  8)&0xFF);
	RTC_DateStruct->Year    = __LL_RTC_CONVERT_BCD2BIN((date>>  0)&0xFF);
}
#else
//RTC时间设置
//hour,min,sec:小时,分钟,秒钟
//ampm:@RTC_AM_PM_Definitions:RTC_HOURFORMAT12_AM/RTC_HOURFORMAT12_PM
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
HAL_StatusTypeDef RTC_Set_Time(u8 hour, u8 min, u8 sec)
{
	RTC_TimeTypeDef RTC_TimeStructure;
	
	RTC_TimeStructure.Hours=hour;
	RTC_TimeStructure.Minutes=min;
	RTC_TimeStructure.Seconds=sec;
	//RTC_TimeStructure.TimeFormat=RTC_HOURFORMAT12_AM;
	RTC_TimeStructure.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
  RTC_TimeStructure.StoreOperation=RTC_STOREOPERATION_RESET;
	return HAL_RTC_SetTime(&hrtc,&RTC_TimeStructure,RTC_FORMAT_BIN);		
}

//RTC日期设置
//year,month,date:年(0~99),月(1~12),日(0~31)
//week:星期(1~7,0,非法!)
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
HAL_StatusTypeDef RTC_Set_Date(u8 year, u8 month, u8 date, u8 week)
{
	RTC_DateTypeDef RTC_DateStructure;
    
	RTC_DateStructure.Date=date;
	RTC_DateStructure.Month=month;
	RTC_DateStructure.WeekDay=week;
	RTC_DateStructure.Year=year;
	return HAL_RTC_SetDate(&hrtc,&RTC_DateStructure,RTC_FORMAT_BIN);
}

void RTC_Time_Get(RTC_TimeTypeDef *RTC_TimeStruct)
{
	HAL_RTC_GetTime(&hrtc, RTC_TimeStruct, RTC_FORMAT_BIN);
}

void RTC_Date_Get(RTC_DateTypeDef *RTC_DateStruct)
{
	HAL_RTC_GetDate(&hrtc, RTC_DateStruct, RTC_FORMAT_BIN);
}

#endif

//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号																						 
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	u8 table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}			  


