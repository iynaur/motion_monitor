

#include "myusart.h"
#include "usart.h"

//�������´���ֱ��ʹ��printf����
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((PRINT_USART->ISR&USART_ISR_TC)==0);//ѭ������,ֱ���������   
	PRINT_USART->TDR = (u8) ch;      
	return ch;
}
#endif 



//LPUART1��ʼ����������жϣ����ò�����
void LPUART1_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_LPUART1_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_LPUART1_CLKSOURCE);
	LL_USART_SetBaudRate(LPUART1,periphclk,LL_USART_OVERSAMPLING_16,bound);
	//LL_USART_EnableIT_RXNE(LPUART1);//�򿪽����ж�
	//LL_USART_EnableIT_IDLE(LPUART1);//�򿪿����ж�
}



//USART1��ʼ����������жϣ����ò�����
void USART1_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_USART1_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART1_CLKSOURCE);
	LL_USART_SetBaudRate(USART1,periphclk,LL_USART_OVERSAMPLING_16,bound);
	LL_USART_EnableIT_RXNE(USART1);//�򿪽����ж�
	LL_USART_EnableIT_IDLE(USART1);//�򿪿����ж�
}

//USART2��ʼ����������жϣ����ò�����
void USART2_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_USART2_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART2_CLKSOURCE);
	LL_USART_SetBaudRate(USART2,periphclk,LL_USART_OVERSAMPLING_16,bound);
	LL_USART_EnableIT_RXNE(USART2);//�򿪽����ж�
	LL_USART_EnableIT_IDLE(USART2);//�򿪿����ж�
}

//USART3��ʼ����������жϣ����ò�����
void USART3_Init(u32 bound)
{
	uint32_t periphclk = LL_RCC_PERIPH_FREQUENCY_NO;
	MX_USART3_UART_Init();
	periphclk = LL_RCC_GetUSARTClockFreq(LL_RCC_USART3_CLKSOURCE);
	LL_USART_SetBaudRate(USART3,periphclk,LL_USART_OVERSAMPLING_16,bound);
	LL_USART_EnableIT_RXNE(USART3);//�򿪽����ж�
	LL_USART_EnableIT_IDLE(USART3);//�򿪿����ж�
}



//LPUART1���ͺ���
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


//����1���ͺ���
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

//����2���ͺ���
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

//����3���ͺ���
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



