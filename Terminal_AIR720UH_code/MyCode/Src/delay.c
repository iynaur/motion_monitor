#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////  

static u8  fac_us=0;							//us延时倍乘数			   
//static u16 fac_ms=0;							//ms延时倍乘数,在ucos下,代表每个节拍的ms数
	
			   
//初始化延迟函数
//SYSTICK的时钟配置为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init()
{
	//LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK_DIV8);
	//HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8);
	fac_us = SystemCoreClock/1000000;				
	
	//fac_ms = (u16)fac_us*1000;					//非OS下,代表每个ms需要的systick时钟数   
}								    

//延时nus
//nus为要延时的us数.		    								   
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD=nus*fac_us; 					//时间加载	  		 
	SysTick->VAL=0x00;        					//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数	  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;      					 //清空计数器	 
	
	LL_Init1msTick(40000000);
}

#if 0
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对80M条件下,nms<=1677 
void delay_ms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD=(u32)nms*fac_ms;				//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL =0x00;							//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;	//开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));		//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//关闭计数器
	SysTick->VAL =0X00;       					//清空计数器	  	    
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















