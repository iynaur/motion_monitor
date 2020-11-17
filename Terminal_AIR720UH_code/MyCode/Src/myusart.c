

#include "myusart.h"
#include "usart.h"

//加入以下代码直接使用printf函数
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((PRINT_USART->ISR&USART_ISR_TC)==0);//循环发送,直到发送完毕   
	PRINT_USART->TDR = (u8) ch;      
	return ch;
}
#endif 



//LPUART1初始化，打开相关中断，设置波特率
void LPUART1_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_LPUART1_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_LPUART1_CLKSOURCE);
	LL_USART_SetBaudRate(LPUART1,periphclk,LL_USART_OVERSAMPLING_16,bound);
	//LL_USART_EnableIT_RXNE(LPUART1);//打开接收中断
	//LL_USART_EnableIT_IDLE(LPUART1);//打开空闲中断
}



//USART1初始化，打开相关中断，设置波特率
void USART1_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_USART1_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART1_CLKSOURCE);
	LL_USART_SetBaudRate(USART1,periphclk,LL_USART_OVERSAMPLING_16,bound);
	LL_USART_EnableIT_RXNE(USART1);//打开接收中断
	LL_USART_EnableIT_IDLE(USART1);//打开空闲中断
}

//USART2初始化，打开相关中断，设置波特率
void USART2_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_USART2_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART2_CLKSOURCE);
	LL_USART_SetBaudRate(USART2,periphclk,LL_USART_OVERSAMPLING_16,bound);
	LL_USART_EnableIT_RXNE(USART2);//打开接收中断
	LL_USART_EnableIT_IDLE(USART2);//打开空闲中断
}

//USART3初始化，打开相关中断，设置波特率
void USART3_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_USART3_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART3_CLKSOURCE);
	LL_USART_SetBaudRate(USART3,periphclk,LL_USART_OVERSAMPLING_16,bound);
	LL_USART_EnableIT_RXNE(USART3);//打开接收中断
	LL_USART_EnableIT_IDLE(USART3);//打开空闲中断
}



//LPUART1发送函数
void LPUART1_Send(uint8_t* data, uint16_t lench)
{
	u16 i;
	
	for (i = 0; i < lench; i++)
	{	
		//LL_USART_IsActiveFlag_TC(USART1);
		LL_USART_TransmitData8(LPUART1, data[i]);
		while (!(LPUART1->ISR & USART_ISR_TC));
	}
	
} 


//串口1发送函数
void USART1_Send(uint8_t* data, uint16_t lench)
{
	u16 i;
	
	for (i = 0; i < lench; i++)
	{	
		//LL_USART_IsActiveFlag_TC(USART1);
		LL_USART_TransmitData8(USART1, data[i]);
		while (!(USART1->ISR & USART_ISR_TC));
	}
	
} 

//串口2发送函数
void USART2_Send(uint8_t* data,uint16_t lench)
{
	u16 i;
	
	for (i = 0; i < lench; i++)
	{	
		//LL_USART_IsActiveFlag_TC(USART2);
		LL_USART_TransmitData8(USART2, data[i]);
		while (!(USART2->ISR & USART_ISR_TC));
	}
	
} 

//串口3发送函数
void USART3_Send(uint8_t* data,uint16_t lench)
{
	u16 i;
	//USART3_RS485_TX();
	for (i = 0; i < lench; i++)
	{	
		//LL_USART_IsActiveFlag_TC(USART2);
		LL_USART_TransmitData8(USART3, data[i]);
		while (!(USART3->ISR & USART_ISR_TC));
	}
	//USART3_RS485_RX();
} 



