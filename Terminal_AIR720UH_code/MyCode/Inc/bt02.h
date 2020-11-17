#ifndef __BT02_H
#define __BT02_H

#include "main.h"

#define TRANS  0
#define CONFIG 1

#define MASTER 0
#define SLAVE  1

#define BT02_Sleep()  {LL_GPIO_SetOutputPin(BLE_WKP_GPIO_Port, BLE_WKP_Pin);}
#define BT02_WakeUp() {LL_GPIO_ResetOutputPin(BLE_WKP_GPIO_Port, BLE_WKP_Pin);}


u8 BT02_Init(void);
u8 BT02_Send_Command(u8 *cmd,u8 *ack,u16 waittime,u16 sendtime);
u8* BT02_Find_ack(u8 *str);
void BT02_StartADV(void);
void BT02_StopADV(void);
void BT02_StartSCAN(void);
void BT02_StopSCAN(void);
void BT02_Disconnect(void);

u8 BT02_Set_UUID(u16 uuid);

u8 BT02_BindMAC(void);
u16 Check_BT02_USART1_data_lench(void);
void BT02_Send_Data(u8 *data_buf, u16 lench);
void BT02_Set_ROLE(u8 role);
void BT02_Set_MOD(u8 mode);
void BT02_Reset(void);
void BT02_LINK_Pin_EXTI_Config(void);
u8 Is_BT02_Connected(void);




#endif

