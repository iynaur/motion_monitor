#ifndef __AIR720SL_H
#define __AIR720SL_H

#include "main.h"



//extern u16 send_time;


#define AIR720_POWER_ON() {LL_GPIO_SetOutputPin(LTE_PWR_CTL_GPIO_Port,LTE_PWR_CTL_Pin);}
#define AIR720_POWER_OFF() {LL_GPIO_ResetOutputPin(LTE_PWR_CTL_GPIO_Port,LTE_PWR_CTL_Pin);}

#define AIR720_PWRKEY_PRESS() {LL_GPIO_SetOutputPin(LTE_PWRKEY_GPIO_Port,LTE_PWRKEY_Pin);}
#define AIR720_PWRKEY_FREED() {LL_GPIO_ResetOutputPin(LTE_PWRKEY_GPIO_Port,LTE_PWRKEY_Pin);}


u8 AIR720_Init(void);
u8 AIR720_Send_data(u8* data_buf, u16 len);

u8 AIR720_Wait_CREG(void);
u8 AIR720_Send_Command(u8 *cmd,u8 *ack,u16 waittime,u16 sendtime);
u8* AIR720_Find_ack(u8 *str);
u16 Check_AIR720_USART3_data_lench(void);
u8 AIR720_Disconnet_and_PowerOff(void);

void AIR720_Hardware_Power_reset(void);
void AIR720_Hardware_Power_OFF(void);
void AIR720_Hardware_Power_ON(void);
void AIR720_PWRKEY_PWON(void);
void AIR720_PWRKEY_PWOFF(void);
void AIR720_PWRKEY_Reset(void);


#endif


