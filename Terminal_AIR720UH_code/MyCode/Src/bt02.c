#include "bt02.h"
#include "delay.h"
#include "stmflash.h"
#include "myusart.h"
#include "commu.h"
#include "myiwdg.h"
#include "low_power.h"


u8 BT02_Init(void)
{
	u16 init_flag = 0;
	//u32 device_id = 0;
	u8 ble_name_set_str[16] = {0};
	
	STMFLASH_Read(STMFLASH_BLE_INIT_FLAG_ADDR, (u8*)&init_flag, STMFLASH_BLE_INIT_FLAG_LENTH);
	//debug_print("000init flag is 0x%x.\r\n",init_flag);
	if(init_flag != 0xEFE7)//检查标志位
	{
		debug_print("BTO2 config mode!\r\n");
		//device_id = Get_Device_ID();
#ifdef BLE_MASTER //主机模式
		sprintf((char*)ble_name_set_str, "<NAMEFCBTM%04d>", device_id);
		BT02_Send_Command("<STOPSNIF>", "OK", 100, 5);//关闭嗅探功能
		BT02_Send_Command("<SCANGAP20>", "OK", 100, 5);//设置扫描间隙<SCANGAPxxx>,xxx为20～10000,对应20ms到10s
#else //从机模式
		sprintf((char*)ble_name_set_str, "<NAMEFCBTS%04d>", device_id);
		BT02_Send_Command("<ADVGAP320>", "OK", 100, 5);//设置广播间隙<ADVGAPxxx>,xxx为32～16000,对应20ms到10s
#endif
		BT02_Send_Command("<MNAME>", 0, 0, 0);
		if(BT02_Send_Command(ble_name_set_str, "OK", 100, 5))
		{
			debug_print("BT02 set name failed!\r\n");
			return 1;
		}//设置蓝牙名称<NAMExxxx>
		BT02_Set_UUID((u16)device_id);//设值服务UUID,必须UUID一样才会主动发起连接
		BT02_Send_Command("<WSMON>", "OK", 100, 5);//开启模式反馈
		//BT02_Send_Command("<WSMOFF>", "OK", 100, 5);//关闭模式反馈
		BT02_Send_Command("<COMWKPOFF>", "OK", 100, 5);//关闭串口唤醒
		//BT02_Send_Command("<APPON>", "OK", 100, 5);//打开数据输出指示，串口数据会有10ms延迟，用于唤醒MCU
		BT02_Send_Command("<APPOFF>", "OK", 100, 5);//关闭数据输出指示
		BT02_Send_Command("<DISBEFSLPOFF>", "OK", 100, 5);//进入休眠后保持蓝牙连接
		BT02_Send_Command("<DISBOND>", "OK", 100, 5);//初始化时关闭MAC绑定
		BT02_Set_MOD(TRANS);//配置完成进入透传模式
		BT02_Sleep();//配置完成进入休眠
#ifndef BLE_MASTER
		BT02_Reset();//重启BT02
#endif
		init_flag = 0xEFE7;
		STMFLASH_Write(STMFLASH_BLE_INIT_FLAG_ADDR, (u8*)&init_flag, STMFLASH_BLE_INIT_FLAG_LENTH);//更新标志位
		//debug_print("init flag is 0x%x.\r\n", (u8*)&init_flag);
	}
	else
	{
		debug_print("BT02 have be inited!\r\n");
	}
#ifdef BLE_MASTER
	BT02_Set_ROLE(MASTER);
	BT02_Reset();//重启BT02
	BT02_StartSCAN();
#else
	BT02_StartADV();
#endif
	debug_print("BT02 have Start ADV!\r\n");
	return 0;
}


//向BT02发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//sendtime:失败最多重发次数
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 BT02_Send_Command(u8 *cmd,u8 *ack,u16 waittime,u16 sendtime)
{
	u8 send_flag = 0, send_count = 0;
	u16 delay_time = waittime;	
	Stop2_delay_ms(200);
	BT02_WakeUp();
	Stop2_delay_ms(200);
	BT02_Set_MOD(CONFIG);
	Stop2_delay_ms(200);
	while(send_flag==0)
	{
		bt02_usart1_CMDack_data_buf[0] = 0;//长度清零,准备接收数据
		
		if((u32)cmd <= 0xFF)
		{
			while (!(USART1->ISR & USART_ISR_TC));
			USART1->TDR = (u32)cmd;
		}
		else
		{
			if(strstr((const char*)cmd,(const char*)"<BONDMAC") != NULL)
			{
				USART1_Send((u8*)cmd,15);//绑定MAC地址
			}
			else if(strstr((const char*)cmd,(const char*)"<SVRUUID") != NULL)
			{
				USART1_Send((u8*)cmd,11);//设置UUID
			}
			else
			{
				USART1_Send((u8*)cmd,strlen((char*)cmd));
				
				LPUART1_Send((u8*)cmd,strlen((char*)cmd));
			}
			//USART2_Send((u8*)cmd,strlen((char*)cmd));
			//USART1_Send((u8*)"\r\n",2);
			//USART1_Send((u8*)cmd,strlen((char*)cmd));
		}
		delay_time = waittime;		
		if(ack&&waittime)//如果需要应答
		{
			while(--delay_time)	//等待倒计时
			{
				delay_ms(10);
				if(BT02_Find_ack(ack))
				{
					send_flag = 1;
					break;
				}
			}
			if(delay_time == 0)
			{
				if(send_count >= sendtime)
				{
					Stop2_delay_ms(200);
					BT02_Set_MOD(TRANS);
					Stop2_delay_ms(200);
					BT02_Sleep();
					Stop2_delay_ms(200);
					return 1;
				}
				send_count++;
			}
		}
		else
			break;
	}
	Stop2_delay_ms(200);
	BT02_Set_MOD(TRANS);
	Stop2_delay_ms(200);
	BT02_Sleep();
	Stop2_delay_ms(200);
	return 0;
}




//BT02发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* BT02_Find_ack(u8 *str)
{
	char *strx = NULL;
	u16 buf_lench = 0;

	buf_lench = Check_BT02_USART1_data_lench();
	
	if(buf_lench != 0)
	{
		bt02_usart1_CMDack_data_buf[buf_lench+1] = 0;//添加结束符
		strx=strstr((const char*)&bt02_usart1_CMDack_data_buf[1],(const char*)str);
	}
	return (u8*)strx;
}

//BT02开启广播
void BT02_StartADV(void)
{
	//BT02_WakeUp();//BT02默认为休眠模式，所以需要先唤醒
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//配置模式
	BT02_Send_Command("<STARTADV>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}


//BT02停止广播
void BT02_StopADV(void)
{
	//BT02_WakeUp();//BT02默认为休眠模式，所以需要先唤醒
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//配置模式
	BT02_Send_Command("<STOPADV>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}


//BT02开启扫描
void BT02_StartSCAN(void)
{
	//BT02_WakeUp();//BT02默认为休眠模式，所以需要先唤醒
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//配置模式
	BT02_Send_Command("<STARTSCAN>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}

//BT02停止扫描
void BT02_StopSCAN(void)
{
	//BT02_WakeUp();//BT02默认为休眠模式，所以需要先唤醒
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//配置模式
	BT02_Send_Command("<STOPSCAN>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}

void BT02_Disconnect(void)
{
	//BT02_WakeUp();//BT02默认为休眠模式，所以需要先唤醒
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//配置模式
	BT02_Send_Command("<DISCONNECT>","OK",100,3);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}

//设置BT02的UUID
u8 BT02_Set_UUID(u16 uuid)
{
  u8 uuid_cmd_str[12] = {0};
  u8 *set_uuid_str = "<SVRUUID";
  memcpy((void*)uuid_cmd_str, (void*)set_uuid_str, 8);
  memcpy((void*)(uuid_cmd_str+8), (void*)&uuid, 2);
  uuid_cmd_str[10] = '>';
  if(BT02_Send_Command(uuid_cmd_str, "OK", 100, 5)) return 1;
  return 0;  
}


//BT02绑定已连接设备的MAC地址
u8 BT02_BindMAC(void)
{
	u8 bind_cmd_str[15] = {0};
	u8 get_mac_conut = 0;
	u8 *bind_mac_str = "<BONDMAC";
	while(bt02_usart1_CMDack_data_buf[0] == 0)
	{
		BT02_Send_Command("<PEERMAC>",0,0,0);//查询当前已连接设备的MAC地址
		delay_ms(100);
		get_mac_conut++;
		if(get_mac_conut > 5) return 1;
	}

	if(BT02_Find_ack("DISCONN")==NULL)
	{
		memcpy((void*)bind_cmd_str, (void*)bind_mac_str, 8);
		memcpy((void*)(bind_cmd_str+8), (void*)&bt02_usart1_CMDack_data_buf[2], 6);
		bind_cmd_str[14] = '>';
		if(BT02_Send_Command(bind_cmd_str, "OK", 100, 5)) return 1;
	}
	else return 1;

	return 0;
	
}


//检查并获取BT02串口接收到的数据长度
u16 Check_BT02_USART1_data_lench(void)
{
	u16 lench = 0;
	if(bt02_usart1_CMDack_data_buf[0] != 0)
	{
		lench = bt02_usart1_CMDack_data_buf[0] - 1;
		bt02_usart1_CMDack_data_buf[0] = 0;
	}
	return lench;
}

//BT02发送数据
void BT02_Send_Data(u8 *data_buf, u16 lench)
{
	BT02_WakeUp();
	Stop2_delay_ms(100);
	USART1_Send(data_buf, lench);
	//Stop2_delay_ms(100);
	BT02_Sleep();
}


void BT02_LINK_Pin_EXTI_Config(void)
{
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

	
  /**/
  LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE3);

  /**/
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_3;
  EXTI_InitStruct.Line_32_63 = LL_EXTI_LINE_NONE;
  EXTI_InitStruct.LineCommand = ENABLE;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
  LL_EXTI_Init(&EXTI_InitStruct);

  /**/
  //LL_GPIO_SetPinPull(BLE_LINK_STA_GPIO_Port, BLE_LINK_STA_Pin, LL_GPIO_PULL_NO);

  /**/
  //LL_GPIO_SetPinMode(BLE_LINK_STA_GPIO_Port, BLE_LINK_STA_Pin, LL_GPIO_MODE_INPUT);

  /* EXTI interrupt init*/
  NVIC_SetPriority(EXTI3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
  NVIC_EnableIRQ(EXTI3_IRQn);
}

//BT02是否有设备连接
u8 Is_BT02_Connected(void)
{
	if(LL_GPIO_IsInputPinSet(BLE_STATE_GPIO_Port,BLE_STATE_Pin)) return 0; 
	else return 1;
}

//设置BT02的角色，主机还是从机
void BT02_Set_ROLE(u8 role)
{
	switch(role)
	{
		case MASTER:
			LL_GPIO_ResetOutputPin(BLE_ROLE_GPIO_Port, BLE_ROLE_Pin);
			break;
		case SLAVE:
			LL_GPIO_SetOutputPin(BLE_ROLE_GPIO_Port, BLE_ROLE_Pin);
			break;
		default:
			break;
	}
}

//设置BT02模式，配置模式还是透传模式
void BT02_Set_MOD(u8 mode)
{
	switch(mode)
	{
		case TRANS:
			LL_GPIO_SetOutputPin(BLE_MOD_GPIO_Port, BLE_MOD_Pin);//透传模式
			break;
		case CONFIG:
			LL_GPIO_ResetOutputPin(BLE_MOD_GPIO_Port, BLE_MOD_Pin);//配置模式
			break;
		default:
			break;
	}
}

//通过RESET引脚重启BTO2
void BT02_Reset(void)
{
	LL_GPIO_SetOutputPin(BLE_RST_GPIO_Port, BLE_RST_Pin);
	Stop2_delay_ms(1500);
	LL_GPIO_ResetOutputPin(BLE_RST_GPIO_Port, BLE_RST_Pin);
	Stop2_delay_ms(1500);
}


