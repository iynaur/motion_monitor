#include "commu.h"
#include "stmflash.h"
#include "myusart.h"

vu8 bt02_usart1_commu_data_buf[BLE_COMMU_DATA_LENTH] = {0};
vu8 bt02_usart1_CMDack_data_buf[BLE_CMDACK_DATA_LENTH] = {0};

vu8 gps_usart2_data_buf[GPS_DATA_LENTH] = {0};

vu8 air720_usart3_commu_data_buf[AIR720_COMMU_DATA_LENTH] = {0};
vu8 air720_usart3_ATack_data_buf[AIR720_ATACK_DATA_LENTH] = {0};

u32 device_id = 0;

#if MACAO_ONE_MINUTE
char Version[10] = "4.03.09";
#else
char Version[10] = "3.03.09";
#endif

vs32 feed_iwdg_time = 600000;
vu32 check_rtc_time = 0;
vu32 gps_power_on_time = 0;
vu32 wait_update_msg_time = 0;
vu32 wait_next_update_msg_time = 0;
vu8 update_flag = 0;
	
float pitch = 0,roll = 0,yaw = 0; 		//欧拉角
short temp = 0;
machine_data_t machine_data = {0};
update_msg_t update_msg = {0};

ble_oil_data_t oil_data = {0};
vu8 ble_connect_flag = 0;
vu8 ble_updating_flag = 0;		
vu8 ble_update_flag = 0;		//蓝牙连接从机（油箱盖）升级标志位
vu32 ble_update_len = 0;		//蓝牙连接从机（油箱盖）升级数据长度
vu8 ble_pack_count = 0;			//蓝牙连接从机（油箱盖）升级数据总包数
vu8 ble_pack_num = 0;
vu32 wait_ble_update_time = 0;
vu32 wait_ble_msg_time = 0;


void USART1_data_record(u8 data, u8 flag)
{
	static u8 frameFlag = 0, cmd_frameFlag = 0, xor_check = 0;
	static u16 recvcnt = 0, lench = 0, cmd_recvcnt = 0;

	if(cmd_recvcnt >= BLE_CMDACK_DATA_LENTH)
	{
		cmd_recvcnt = 0;
	}
	
	if(flag != 0)
	{
		//frameFlag = 0;
		//recvcnt = 0;
		//lench = 0;
		//xor_check = 0;
		
		cmd_frameFlag = 0;
		bt02_usart1_CMDack_data_buf[0] = cmd_recvcnt;
		cmd_recvcnt = 0;
		return;
	}
	else
		LL_USART_TransmitData8(LPUART1, data);//将BT01数据通过串口2打印


	
#if 1
	if(cmd_frameFlag == 0)
	{
		cmd_recvcnt = 1;
		bt02_usart1_CMDack_data_buf[0] = 0;
		bt02_usart1_CMDack_data_buf[cmd_recvcnt ++] = data;
		cmd_frameFlag = 1;
	}
	else
	{
		bt02_usart1_CMDack_data_buf[cmd_recvcnt ++] = data;
	}
#endif

	if(frameFlag == 0 && data == BLE_DATA_HEAD1)//头码1
	{
		recvcnt = 2;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		frameFlag = 1;
	}
	else if(frameFlag == 1 && data == BLE_DATA_HEAD2)//头码2
	{
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		frameFlag = 2;
	}
	else if(frameFlag == 2 && data == BLE_TYPE_ID)//设备类型
	{
		xor_check ^= data;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		frameFlag = 3;
	}
	else if(data==((device_id>>16)&0xFF)&&frameFlag==3)//设备ID1
	{
		xor_check ^= data;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		frameFlag = 4;
	}
	else if(data==((device_id>> 8)&0xFF)&&frameFlag==4)//设备ID2
	{
		xor_check ^= data;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		frameFlag = 5;
	}
	else if(data==((device_id>> 0)&0xFF)&&frameFlag==5)//设备ID3
	{
		xor_check ^= data;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		frameFlag = 6;
	}
	else if(frameFlag == 6)//数据包序号
	{
		xor_check ^= data;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		frameFlag = 7;
	}
	else if(frameFlag == 7)//数据包长度高8位
	{
		xor_check ^= data;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		lench = data;
		frameFlag = 8;
	}
	else if(frameFlag == 8)//数据包长度低8位
	{
		xor_check ^= data;
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		lench = (lench << 8) + data + 2;
		frameFlag = 9;
	}
	else if(frameFlag == 9)//开始接收数据
	{
		bt02_usart1_commu_data_buf[recvcnt++] = data;
		if(recvcnt == lench)//recvcnt从1开始接收数据，接收完成自加1，所以要和lench + 1比较
		{
			if(xor_check == data)//最后一个数据为校验码
			{
				bt02_usart1_commu_data_buf[0] = recvcnt&0xFF;				
				bt02_usart1_commu_data_buf[1] = (recvcnt >> 8)&0xFF;
				frameFlag = 0;
				recvcnt = 0;
				lench = 0;
				xor_check = 0;
				wait_ble_msg_time = 0;
				return;
			}
			else
			{
				bt02_usart1_commu_data_buf[0] = 0;
				bt02_usart1_commu_data_buf[1] = 0;
				frameFlag = 0;
				recvcnt = 0;
				lench = 0;
				xor_check = 0;
				return;
			}
		}
		else if(recvcnt < lench)//数据还没结束
		{
			xor_check ^= data;
		}
	}
	else
	{
		bt02_usart1_commu_data_buf[0] = 0;
		bt02_usart1_commu_data_buf[1] = 0;
		frameFlag = 0;
		recvcnt = 0;
		lench = 0;
		xor_check = 0;
		return;
	}
	

	
	return;
}



//串口2接收GPS数据
void USART2_data_record(u8 data, u8 flag)
{
	static u16 recvcnt = 0, frameFlag = 0;

	if(recvcnt >= GPS_DATA_LENTH)
	{
		recvcnt = 0;
	}
	
	if(flag != 0)
	{
		frameFlag = 0;
		gps_usart2_data_buf[0] = recvcnt&0xFF;
		gps_usart2_data_buf[1] = (recvcnt>>8)&0xFF;
		recvcnt = 0;
		return;
	}
//	else
//		LL_USART_TransmitData8(LPUART1, data);
	
	if(frameFlag == 0)
	{
		recvcnt = 2;
		gps_usart2_data_buf[0] = 0;
		gps_usart2_data_buf[1] = 0;
		gps_usart2_data_buf[recvcnt++] = data;
		frameFlag = 1;
	}
	else
	{
		gps_usart2_data_buf[recvcnt++] = data;
	}
	return;
}


//串口3接收Air720返回的数据
void USART3_data_record(u8 data, u8 flag)
{
	static u16 commu_recvcnt = 0, commu_frameFlag = 0;
	static u16 ATack_recvcnt = 0, ATack_frameFlag = 0;
	static u16 ack_lench = 0;
	static u8 xor_check = 0;
	//static u8 server_data_flag = 0;

	if(ATack_recvcnt >= AIR720_ATACK_DATA_LENTH)
	{
		ATack_recvcnt = 0;
	}
	
	if(flag != 0)
	{
		//commu_frameFlag = 0;
		//commu_recvcnt = 0;
		//ack_lench = 0;
		//xor_check = 0;
	
		ATack_frameFlag = 0;
		air720_usart3_ATack_data_buf[0] = ATack_recvcnt&0xFF;
		air720_usart3_ATack_data_buf[1] = (ATack_recvcnt>>8)&0xFF;
		ATack_recvcnt = 0;
		return;
	}
	else
		LL_USART_TransmitData8(LPUART1, data);
	
	if(ATack_frameFlag == 0)
	{
		ATack_recvcnt = 2;
		air720_usart3_ATack_data_buf[0] = 0;
		air720_usart3_ATack_data_buf[1] = 0;
		air720_usart3_ATack_data_buf[ATack_recvcnt++] = data;
		ATack_frameFlag = 1;
	}
	else
	{
		air720_usart3_ATack_data_buf[ATack_recvcnt++] = data;
	}

	if(commu_frameFlag==0&&data==SOCKET_DATA_HEAD1)
	{
		commu_recvcnt = 2;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		commu_frameFlag = 1;
	}
	else if(commu_frameFlag==1&&data==SOCKET_DATA_HEAD2)
	{
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		commu_frameFlag = 2;
	}
	else if(data==DEVICE_TYPE_ID&&commu_frameFlag==2)
	//else if(commu_frameFlag==2)
	{
		xor_check = xor_check^data;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		commu_frameFlag = 3;
	}
	else if(data==((device_id>>16)&0xFF)&&commu_frameFlag==3)
	{
		xor_check = xor_check^data;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		commu_frameFlag = 4;
	}
	else if(data==((device_id>>8)&0xFF)&&commu_frameFlag==4)
	{
		xor_check = xor_check^data;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		commu_frameFlag = 5;
	}
	else if(data==((device_id>>0)&0xFF)&&commu_frameFlag==5)
	{
		xor_check = xor_check^data;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		commu_frameFlag = 6;
	}
	else if(commu_frameFlag==6)
	{
		xor_check = xor_check^data;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		commu_frameFlag = 7;
	}
	else if(commu_frameFlag==7)
	{
		xor_check = xor_check^data;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		ack_lench = data;
		commu_frameFlag = 8;
	}
	else if(commu_frameFlag==8)
	{
		xor_check = xor_check^data;
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		ack_lench = (ack_lench<<8) + data + 2;
		commu_frameFlag = 9;
	}
	else if(commu_frameFlag==9)
	{
		air720_usart3_commu_data_buf[commu_recvcnt++] = data;
		if(commu_recvcnt == ack_lench)
		{
			if(xor_check==data)
			{
				//debug_print("XOR right is 0x%02x.\r\n", xor_check);
				//debug_print("ack lench is %d.\r\n", ack_lench);
				air720_usart3_commu_data_buf[0] = ack_lench&0xFF;
				air720_usart3_commu_data_buf[1] = (ack_lench>>8)&0xFF;
				commu_frameFlag = 0;
				commu_recvcnt = 0;
				ack_lench = 0;
				xor_check = 0;
				return;
			}
			else
			{
				debug_print("XOR erro is 0x%02x.\r\n", xor_check);
				debug_print("ack lench is %d.\r\n", ack_lench);
				air720_usart3_commu_data_buf[0] = 0;
				air720_usart3_commu_data_buf[1] = 0;
				commu_frameFlag = 0;
				commu_recvcnt = 0;
				ack_lench = 0;
				xor_check = 0;
				return;
			}
		}
		else if(commu_recvcnt < ack_lench)
		{
			xor_check = xor_check^data;
		}
	}
	else
	{
		air720_usart3_commu_data_buf[0] = 0;
		air720_usart3_commu_data_buf[1] = 0;
		commu_frameFlag = 0;
		commu_recvcnt = 0;
		ack_lench = 0;
		xor_check = 0;
		return;
	}
	return;
	
}




u8 Check_XOR_sum(u8 *pMsg, u16 len)
{
	u8 checksum = 0;
	u16 i = 0;
	for(i = 0; i < len; i++)
	{
		checksum ^= pMsg[i];
	}
	return checksum;
}


static u8 auchCRCHi[] = { /* CRC 高位字节值表*/
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};
static u8 auchCRCLo[] = { /* CRC 低位字节值表*/
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

u16 CRC16(u8 * pMsg, u16 Len)
{ 
	u8 i;
	u8 uchCRCHi; //CRC 高字节
	u8 uchCRCLo; //CRC 低字节
	
	uchCRCHi = 0xFF;
	uchCRCLo = 0xFF;
	while (Len--)
	{
		i = uchCRCHi ^ (*pMsg++);
		uchCRCHi = uchCRCLo ^ auchCRCHi[i];
		uchCRCLo = auchCRCLo[i];
	}
	//return (uchCRCHi << 8 | uchCRCLo);
	return (uchCRCLo << 8 | uchCRCHi);
}

u32 Get_Device_ID(void)
{
	u32 id = 0;
	STMFLASH_Read(STMFLASH_ID_ADDR, (u8*)&id, STMFLASH_ID_LENTH);

	return id;
}

u8 hextochar(u8 data)
{
	u8 i=0;
    if(data >=0x0A && data <= 0x0F)
	{
		i= 'A' + (data - 0x0A);
		return i;
	}
	else if(data <= 9)
	{
		i= '0' + data;
		return i;
	}
	else return 0;
}

