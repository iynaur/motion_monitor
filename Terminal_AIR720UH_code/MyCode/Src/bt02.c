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
	if(init_flag != 0xEFE7)//����־λ
	{
		debug_print("BTO2 config mode!\r\n");
		//device_id = Get_Device_ID();
#ifdef BLE_MASTER //����ģʽ
		sprintf((char*)ble_name_set_str, "<NAMEFCBTM%04d>", device_id);
		BT02_Send_Command("<STOPSNIF>", "OK", 100, 5);//�ر���̽����
		BT02_Send_Command("<SCANGAP20>", "OK", 100, 5);//����ɨ���϶<SCANGAPxxx>,xxxΪ20��10000,��Ӧ20ms��10s
#else //�ӻ�ģʽ
		sprintf((char*)ble_name_set_str, "<NAMEFCBTS%04d>", device_id);
		BT02_Send_Command("<ADVGAP320>", "OK", 100, 5);//���ù㲥��϶<ADVGAPxxx>,xxxΪ32��16000,��Ӧ20ms��10s
#endif
		BT02_Send_Command("<MNAME>", 0, 0, 0);
		if(BT02_Send_Command(ble_name_set_str, "OK", 100, 5))
		{
			debug_print("BT02 set name failed!\r\n");
			return 1;
		}//������������<NAMExxxx>
		BT02_Set_UUID((u16)device_id);//��ֵ����UUID,����UUIDһ���Ż�������������
		BT02_Send_Command("<WSMON>", "OK", 100, 5);//����ģʽ����
		//BT02_Send_Command("<WSMOFF>", "OK", 100, 5);//�ر�ģʽ����
		BT02_Send_Command("<COMWKPOFF>", "OK", 100, 5);//�رմ��ڻ���
		//BT02_Send_Command("<APPON>", "OK", 100, 5);//���������ָʾ���������ݻ���10ms�ӳ٣����ڻ���MCU
		BT02_Send_Command("<APPOFF>", "OK", 100, 5);//�ر��������ָʾ
		BT02_Send_Command("<DISBEFSLPOFF>", "OK", 100, 5);//�������ߺ󱣳���������
		BT02_Send_Command("<DISBOND>", "OK", 100, 5);//��ʼ��ʱ�ر�MAC��
		BT02_Set_MOD(TRANS);//������ɽ���͸��ģʽ
		BT02_Sleep();//������ɽ�������
#ifndef BLE_MASTER
		BT02_Reset();//����BT02
#endif
		init_flag = 0xEFE7;
		STMFLASH_Write(STMFLASH_BLE_INIT_FLAG_ADDR, (u8*)&init_flag, STMFLASH_BLE_INIT_FLAG_LENTH);//���±�־λ
		//debug_print("init flag is 0x%x.\r\n", (u8*)&init_flag);
	}
	else
	{
		debug_print("BT02 have be inited!\r\n");
	}
#ifdef BLE_MASTER
	BT02_Set_ROLE(MASTER);
	BT02_Reset();//����BT02
	BT02_StartSCAN();
#else
	BT02_StartADV();
#endif
	debug_print("BT02 have Start ADV!\r\n");
	return 0;
}


//��BT02��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//sendtime:ʧ������ط�����
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
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
		bt02_usart1_CMDack_data_buf[0] = 0;//��������,׼����������
		
		if((u32)cmd <= 0xFF)
		{
			while (!(USART1->ISR & USART_ISR_TC));
			USART1->TDR = (u32)cmd;
		}
		else
		{
			if(strstr((const char*)cmd,(const char*)"<BONDMAC") != NULL)
			{
				USART1_Send((u8*)cmd,15);//��MAC��ַ
			}
			else if(strstr((const char*)cmd,(const char*)"<SVRUUID") != NULL)
			{
				USART1_Send((u8*)cmd,11);//����UUID
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
		if(ack&&waittime)//�����ҪӦ��
		{
			while(--delay_time)	//�ȴ�����ʱ
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




//BT02���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* BT02_Find_ack(u8 *str)
{
	char *strx = NULL;
	u16 buf_lench = 0;

	buf_lench = Check_BT02_USART1_data_lench();
	
	if(buf_lench != 0)
	{
		bt02_usart1_CMDack_data_buf[buf_lench+1] = 0;//��ӽ�����
		strx=strstr((const char*)&bt02_usart1_CMDack_data_buf[1],(const char*)str);
	}
	return (u8*)strx;
}

//BT02�����㲥
void BT02_StartADV(void)
{
	//BT02_WakeUp();//BT02Ĭ��Ϊ����ģʽ��������Ҫ�Ȼ���
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//����ģʽ
	BT02_Send_Command("<STARTADV>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}


//BT02ֹͣ�㲥
void BT02_StopADV(void)
{
	//BT02_WakeUp();//BT02Ĭ��Ϊ����ģʽ��������Ҫ�Ȼ���
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//����ģʽ
	BT02_Send_Command("<STOPADV>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}


//BT02����ɨ��
void BT02_StartSCAN(void)
{
	//BT02_WakeUp();//BT02Ĭ��Ϊ����ģʽ��������Ҫ�Ȼ���
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//����ģʽ
	BT02_Send_Command("<STARTSCAN>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}

//BT02ֹͣɨ��
void BT02_StopSCAN(void)
{
	//BT02_WakeUp();//BT02Ĭ��Ϊ����ģʽ��������Ҫ�Ȼ���
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//����ģʽ
	BT02_Send_Command("<STOPSCAN>","OK",200,5);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}

void BT02_Disconnect(void)
{
	//BT02_WakeUp();//BT02Ĭ��Ϊ����ģʽ��������Ҫ�Ȼ���
	//Stop2_delay_ms(60);
	//BT02_Set_MOD(CONFIG);//����ģʽ
	BT02_Send_Command("<DISCONNECT>","OK",100,3);
	//BT02_Set_MOD(TRANS);
	//BT02_Sleep();
}

//����BT02��UUID
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


//BT02���������豸��MAC��ַ
u8 BT02_BindMAC(void)
{
	u8 bind_cmd_str[15] = {0};
	u8 get_mac_conut = 0;
	u8 *bind_mac_str = "<BONDMAC";
	while(bt02_usart1_CMDack_data_buf[0] == 0)
	{
		BT02_Send_Command("<PEERMAC>",0,0,0);//��ѯ��ǰ�������豸��MAC��ַ
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


//��鲢��ȡBT02���ڽ��յ������ݳ���
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

//BT02��������
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

//BT02�Ƿ����豸����
u8 Is_BT02_Connected(void)
{
	if(LL_GPIO_IsInputPinSet(BLE_STATE_GPIO_Port,BLE_STATE_Pin)) return 0; 
	else return 1;
}

//����BT02�Ľ�ɫ���������Ǵӻ�
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

//����BT02ģʽ������ģʽ����͸��ģʽ
void BT02_Set_MOD(u8 mode)
{
	switch(mode)
	{
		case TRANS:
			LL_GPIO_SetOutputPin(BLE_MOD_GPIO_Port, BLE_MOD_Pin);//͸��ģʽ
			break;
		case CONFIG:
			LL_GPIO_ResetOutputPin(BLE_MOD_GPIO_Port, BLE_MOD_Pin);//����ģʽ
			break;
		default:
			break;
	}
}

//ͨ��RESET��������BTO2
void BT02_Reset(void)
{
	LL_GPIO_SetOutputPin(BLE_RST_GPIO_Port, BLE_RST_Pin);
	Stop2_delay_ms(1500);
	LL_GPIO_ResetOutputPin(BLE_RST_GPIO_Port, BLE_RST_Pin);
	Stop2_delay_ms(1500);
}


