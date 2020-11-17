#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////  

static u8  fac_us=0;							//us��ʱ������			   
//static u16 fac_ms=0;							//ms��ʱ������,��ucos��,����ÿ�����ĵ�ms��
	
			   
//��ʼ���ӳٺ���
//SYSTICK��ʱ������ΪHCLKʱ�ӵ�1/8
//SYSCLK:ϵͳʱ��
void delay_init()
{
	//LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK_DIV8);
	//HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);
	fac_us = SystemCoreClock/1000000;				
	
	//fac_ms = (u16)fac_us*1000;					//��OS��,����ÿ��ms��Ҫ��systickʱ����   
}								    

//��ʱnus
//nusΪҪ��ʱ��us��.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//ʱ�����	  		 
	SysTick->VAL=0x00;        					//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;      					 //��ռ�����	 
	
	LL_Init1msTick(40000000);
}

#if 0
//��ʱnms
//ע��nms�ķ�Χ
//SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK��λΪHz,nms��λΪms
//��80M������,nms<=1677 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL =0x00;							//��ռ�����
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//��ʼ����  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//�ȴ�ʱ�䵽��   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;       					//��ռ�����	  	    
} 
#endif

void delay_ms(u32 nms)
{
	LL_mDelay(nms);
}


void delay_s(u16 s)
{
	u16 i = 0;
	for(i=0; i<s; i++)
	{
		delay_ms(1000);
	}
}

//void HAL_Delay(uint32_t Delay)
//{
//	delay_us(Delay);
//}















