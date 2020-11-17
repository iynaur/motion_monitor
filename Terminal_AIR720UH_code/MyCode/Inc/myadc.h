#ifndef __MYADC_H
#define __MYADC_H

#include "main.h"
#include "adc.h"


#define ADC_BAT_CHANNEL LL_ADC_CHANNEL_6

void ADC_Init(void);
u16 Get_Adc(u32 ch);   
u16 Get_Adc_Average(u32 ch,u8 times);


#endif

