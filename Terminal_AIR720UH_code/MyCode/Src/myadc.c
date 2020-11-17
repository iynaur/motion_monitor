#include "myadc.h"
#include "delay.h"

void ADC_Init(void)
{
	MX_ADC1_Init();
  LL_ADC_Enable(ADC1);
}

//获得ADC值
//ch: @ref ADC_channels 
//通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
//返回值:转换结果
u16 Get_Adc(u32 ch)   
{
	 //设置指定ADC的规则组通道，一个序列，采样时间
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, ch);
  LL_ADC_SetChannelSamplingTime(ADC1, ch, LL_ADC_SAMPLINGTIME_640CYCLES_5);
  LL_ADC_SetChannelSingleDiff(ADC1, ch, LL_ADC_SINGLE_ENDED);
	LL_ADC_REG_StartConversion(ADC1);		//使能指定的ADC1的软件转换启动功能	
	while(LL_ADC_REG_IsConversionOngoing(ADC1));//等待转换结束
	LL_ADC_REG_StopConversion(ADC1);
	return LL_ADC_REG_ReadConversionData12(ADC1);	//返回最近一次ADC1规则组的转换结果
}

//获取通道ch的转换值，取times次,然后平均 
//ch:通道编号
//times:获取次数
//返回值:通道ch的times次转换结果平均值
u16 Get_Adc_Average(u32 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 





