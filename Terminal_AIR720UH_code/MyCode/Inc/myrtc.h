
#ifndef __MYRTC_H
#define __MYRTC_H	    

#include "main.h"


#define RTC_WKP_DISABLE 0xFFFFFFFF

#define RTC_WAKEUPCLOCK_DIV_16          0x00000000U                           /*!< RTC/16 clock is selected */
#define RTC_WAKEUPCLOCK_DIV_8           RTC_CR_WUCKSEL_0                      /*!< RTC/8 clock is selected */
#define RTC_WAKEUPCLOCK_DIV_4           RTC_CR_WUCKSEL_1                      /*!< RTC/4 clock is selected */
#define RTC_WAKEUPCLOCK_DIV_2           (RTC_CR_WUCKSEL_1 | RTC_CR_WUCKSEL_0) /*!< RTC/2 clock is selected */
#define RTC_WAKEUPCLOCK_CKSPRE          RTC_CR_WUCKSEL_2                      /*!< ck_spre (usually 1 Hz) clock is selected */
#define RTC_WAKEUPCLOCK_CKSPRE_WUT      (RTC_CR_WUCKSEL_2 | RTC_CR_WUCKSEL_1) /*!< ck_spre (usually 1 Hz) clock is selected and 2exp16 is added to the WUT counter value*/


void RTC_Init(void);


void RTC_WakeUp_Config(uint32_t WakeupClock, u16 RTC_WUT_TIME);

#ifndef RTC_USE_HAL_LIB
ErrorStatus RTC_Set_Time(u8 hour, u8 min, u8 sec);
ErrorStatus RTC_Set_Date(u8 year, u8 month, u8 date, u8 week);
void RTC_Time_Get(LL_RTC_TimeTypeDef *RTC_TimeStruct);
void RTC_Date_Get(LL_RTC_DateTypeDef *RTC_DateStruct);
#else
HAL_StatusTypeDef RTC_Set_Time(u8 hour, u8 min, u8 sec);
HAL_StatusTypeDef RTC_Set_Date(u8 year, u8 month, u8 date, u8 week);
void RTC_Time_Get(RTC_TimeTypeDef *RTC_TimeStruct);
void RTC_Date_Get(RTC_DateTypeDef *RTC_DateStruct);
#endif

u8 RTC_Get_Week(u16 year,u8 month,u8 day);





#endif



