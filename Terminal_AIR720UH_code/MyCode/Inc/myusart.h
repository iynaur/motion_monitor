#ifndef __MYUSART_H
#define __MYUSART_H

#include "main.h"

#define debug_print printf

#define USE_RS485 0

#define PRINT_USART LPUART1

#if USE_RS485
 #define USART3_RS485_TX() {LL_GPIO_SetOutputPin(RS485_DE_GPIO_Port, RS485_DE_Pin);}
 #define USART3_RS485_RX() {LL_GPIO_ResetOutputPin(RS485_DE_GPIO_Port, RS485_DE_Pin);}
#endif

void LPUART1_Init(u32 bound);
void USART1_Init(u32 bound);
void USART2_Init(u32 bound);
void USART3_Init(u32 bound);
//void UART4_Init(u32 bound);
//void UART5_Init(u32 bound);

void LPUART1_Send(uint8_t* data, uint16_t lench);
void USART1_Send(uint8_t* data, uint16_t lench);
void USART2_Send(uint8_t* data, uint16_t lench);
void USART3_Send(uint8_t* data, uint16_t lench);
//void UART4_Send(uint8_t* data, uint16_t lench);
//void UART5_Send(uint8_t* data, uint16_t lench);


#endif


