#include "myadc.h"
#include "delay.h"

void ADC_Init(void)
{
	MX_ADC1_Init();
  LL_ADC_Enable(ADC1);
}

//���ADCֵ
//ch: @ref ADC_channels 
//ͨ��ֵ 0~16ȡֵ��ΧΪ��ADC_Channel_0~ADC_Channel_16
//����ֵ:ת�����
u16 Get_Adc(u32 ch)   
{
	 //����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, ch);
  LL_ADC_SetChannelSamplingTime(ADC1, ch, LL_ADC_SAMPLINGTIME_640CYCLES_5);
  LL_ADC_SetChannelSingleDiff(ADC1, ch, LL_ADC_SINGLE_ENDED);
	LL_ADC_REG_StartConversion(ADC1);		//ʹ��ָ����ADC1�����ת����������	
	while(LL_ADC_REG_IsConversionOngoing(ADC1));//�ȴ�ת������
	LL_ADC_REG_StopConversion(ADC1);
	return LL_ADC_REG_ReadConversionData12(ADC1);	//�������һ��ADC1�������ת�����
}

//��ȡͨ��ch��ת��ֵ��ȡtimes��,Ȼ��ƽ�� 
//ch:ͨ�����
//times:��ȡ����
//����ֵ:ͨ��ch��times��ת�����ƽ��ֵ
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





