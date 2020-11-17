#ifndef __LOGIC_APP_H
#define __LOGIC_APP_H

#include "main.h"
#include "commu.h"


void APP_Run(void);
void Device_Check_LowPowerMode(u8 send2server);
void msg_check(void);
u8 Send_Machine_Data(void);
u8 Send_DismountMsg2Server(void);
void Fill_Machine_Data(void);
u8 UpdateMsg_Handle(u8 *data_buf, u16 lench);
u8 Get_Machine_Status(void);
void Check_Time_with_Server(void);
u8 Send_SocketData2Server(SOCKET_CMD cmd, u16 lench, u8 ack);
void Save_Unsend_Data2Flash(void);
void Check_Flash_Unsend_Data(void);
u8 BLE_Msg_Handle(u8 *data_buf, u16 lench);
u8 Send_BLE_Data2Master(BLE_CMD cmd, u16 lench);
u8 Send_BLE_UpdateData2Master(u8 pack_num);



#endif

