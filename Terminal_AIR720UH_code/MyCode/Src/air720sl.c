
#include "air720sl.h"
#include "myusart.h"
#include "delay.h"
#include "myiwdg.h"
#include "myrtc.h"
#include "low_power.h"
#include "commu.h"

u8 *server_addr_str = "AT+CIPSTART=\"TCP\",\"121.196.130.108\",\"8100\"";//FC正式服务器





u8 AIR720_Init(void)
{
	u8 csq_time = 0, cgatt_time = 0, creg_time = 0;
	//Stop2_delay_s(1);
	AIR720_Hardware_Power_reset();//无论当前状态是关机还是开机 都重新开机
	debug_print("AIR720 POWER ON\r\n");
	Stop2_delay_s(2);
	AIR720_Send_Command("AT", "OK", 100, 5);//发送AT直到返回OK进行自适应波特率
	AIR720_Send_Command("AT+IPR=115200","OK",300,5);//设置波特率为115200
	AIR720_Send_Command("ATE0","OK",500,5);//取消回显
	AIR720_Send_Command("AT+VER",0,0,0);//
	if(AIR720_Send_Command("AT+CPIN?","READY",300,5))//检查SIM卡是否准备好
	{
		debug_print("SIM card not ready!\r\n");
		return 1;
	}
	
	while(AIR720_Send_Command("AT+CSQ","99,99",200,1)==0)//检查信号值是否正常
	{
		Stop2_delay_ms(500);
		csq_time++;
		if(csq_time>150)
		{
			csq_time = 0;
			debug_print("CSQ is \"99,99\",it is wrong!\r\n");
			return 1;
		}
	}

	while(AIR720_Wait_CREG())//查询当前GPRS注册状态
	{
		Stop2_delay_ms(500);
		creg_time++;
		if(creg_time>150)
		{
			creg_time = 0;
			debug_print("AIR720 CREG failed!\r\n");
			return 1;
			
		}
	}

	AIR720_Send_Command("AT+CGATT=1", "OK", 50, 5);
	while(AIR720_Send_Command("AT+CGATT?", "1", 50, 5))//查看当前GPRS附着状态
	{
		AIR720_Send_Command("AT+CGATT=1", "OK", 50, 5);
		Stop2_delay_ms(500);
		cgatt_time++;
		if(cgatt_time>150)
		{
			cgatt_time = 0;
			debug_print("CGATT is not 1!\r\n");
			return 1;
		}
	}
	
	AIR720_Send_Command("AT+CIPMODE=1","OK",100,5);//设置TCPIP应用为透明传输模式
	AIR720_Send_Command("AT+CIPMUX=0", "OK", 100, 5);//设置为单链接模式
	AIR720_Send_Command("AT+CIPQSEND=0", "OK", 100, 5);//设置为快发模式
	//AIR720_Send_Command("AT+CIPSHUT", 0,0,0);
	//AIR720_Send_Command("AT+CGDCONT?", 0, 0, 0);
	//debug_print("Send AT+CSTT!\r\n");
	AIR720_Send_Command("AT+CSTT", "OK", 100, 5);//启动任务，设置APN
	//AIR720_Send_Command("AT+CSTT=cmnet", "OK", 100, 5);//启动任务，设置APN
	AIR720_Send_Command("AT+CIICR", "OK", 100, 5);//激活移动场景，获取IP地址
	AIR720_Send_Command("AT+CIFSR",0,0,0);//查询分配的IP地址
	AIR720_Send_Command("AT+CIPSTATUS", "IP STATUS", 100, 5);//查询当前链接状态
	//AIR720_Send_Command("AT+CIPSHOWTP=0","OK",100,5);//不在接收数据头中显示协议类型
	//AIR720_Send_Command("AT+CIPHEAD=0","OK",100,5);//不显示数据头
	//AIR720_Send_Command("AT+CIPSRIP=0","OK",100,5);//不显示数据发送放的IP地址和端口
	//AIR720_Send_Command("AT+CIPRX=1", "OK", 100, 5);//每笔收到的数据末尾不添加回车换行
	if(AIR720_Send_Command(server_addr_str,"CONNECT",500,5))
	{
		debug_print("Connect server failed!\r\n");
		return 1;
	}
	
	return 0;
}



u8 AIR720_Send_data(u8* data_buf, u16 len)
{
	#if 0
	//u8 qisend_status = 0;
	u16 wait_ack_time = 0;
	u8 at_cipsend_str[13] = {0};

	sprintf((char*)at_cipsend_str, "AT+CIPSEND=%d", len);
	//AIR720_Send_Command(server_addr_str,"CONNECT",500,5);
	if(AIR720_Send_Command(at_cipsend_str,">",50,5)==0)//向AIR720发送指定长度的发送数据命令，5次不成功报错
	{
		delay_ms(50);
		USART3_Send(data_buf, len);
		//while(AIR720_Find_ack("DATA ACCEPT")==NULL)
		while(AIR720_Find_ack("SEND OK")==NULL)
		{
			delay_ms(10);
			wait_ack_time++;
			if(wait_ack_time > 3000)return 1;
		}
	}
	else
	{
		debug_print("AT+CIPSEND failed!\r\n");
		return 1;
	}
	#endif
	
	USART3_Send(data_buf, len);
	return 0;
}


u8 AIR720_Wait_CREG(void)
{
	u8 wait_flag = 0, count = 0;
	u16 buf_lench = 0, i = 0;
	u8 *creg_str = "AT+CREG?\r\n";
	while(wait_flag==0)
	{
		//air720_usart3_ATack_data_buf[0] = 0;//长度清零,准备接收数据
		//air720_usart3_ATack_data_buf[1] = 0;//长度清零,准备接收数据
		memset((void*)air720_usart3_ATack_data_buf, 0, AIR720_ATACK_DATA_LENTH);//清零,准备接收数据
		USART3_Send(creg_str,strlen((char*)creg_str));
		//USART1_Send(creg_str,strlen((char*)creg_str));
		delay_ms(100);
		buf_lench = Check_AIR720_USART3_data_lench();
		if(buf_lench != 0)
		{
			for(i = 2; i < buf_lench+2; i++)
			{
				if(air720_usart3_ATack_data_buf[i]==':')//接收到了返回数据
				{
					if(air720_usart3_ATack_data_buf[i+4] == '5')
					{
						wait_flag = 1;
						return 0;
					}
					else if(air720_usart3_ATack_data_buf[i+4] == '1')
					{
						if((air720_usart3_ATack_data_buf[i+5] != '1')&&(air720_usart3_ATack_data_buf[i+5] != '0'))
						{
							wait_flag = 1;
							return 0;
						}
					}
				}
			}
			count++;
			if(count > 20)//查询15次还是没有注册成功
			{
				count = 0;
				return 1;
			}
		}
	}
	return 0;
}


//向AIR720SL发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//sendtime:失败最多重发次数
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 AIR720_Send_Command(u8 *cmd,u8 *ack,u16 waittime,u16 sendtime)
{
	u8 send_flag = 0, send_count = 0;
	u16 delay_time = waittime;
	while(send_flag==0)
	{
		//air720_usart3_ATack_data_buf[0] = 0;//长度清零,准备接收数据
		//air720_usart3_ATack_data_buf[1] = 0;//长度清零,准备接收数据
		memset((void*)air720_usart3_ATack_data_buf, 0, AIR720_ATACK_DATA_LENTH);//清零,准备接收数据
		if((u32)cmd <= 0xFF)
		{
			while (!(USART3->ISR & USART_ISR_TC));
			USART3->RDR = (u32)cmd;
		}
		else
		{
			if(strstr((const char*)cmd,(const char*)"+++") == NULL)
			{
				USART3_Send((u8*)cmd,strlen((char*)cmd));
				USART3_Send((u8*)"\r\n",2);
			}
			else
				USART3_Send((u8*)cmd,3);
			//LPUART1_Send((u8*)cmd,strlen((char*)cmd));
			//LPUART1_Send((u8*)"\r\n",2);
		}
		delay_time = waittime;		
		if(ack&&waittime)//如果需要应答
		{
			while(--delay_time)	//等待倒计时
			{
				delay_ms(10);
				if(AIR720_Find_ack(ack))
				{
					send_flag = 1;
					break;
				}
			}
			if(delay_time == 0)
			{
				if(send_count >= sendtime)
				{
					return 1;
				}
				send_count++;
			}
		}
		else
			break;
	}
	return 0;
}



//mc20e发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//其他,期待应答结果的位置(str的位置)
u8* AIR720_Find_ack(u8 *str)
{
	char *strx = NULL;
	u16 buf_lench = 0;

	buf_lench = Check_AIR720_USART3_data_lench();
	
	if(buf_lench != 0)
	{
		air720_usart3_ATack_data_buf[buf_lench+2] = 0;//添加结束符
		strx=strstr((const char*)&air720_usart3_ATack_data_buf[2],(const char*)str);
	}
	return (u8*)strx;
}

//检查并获取串口3接收的AT指令返回数据的长度
u16 Check_AIR720_USART3_data_lench(void)
{
	u16 lench = 0;
	if(((air720_usart3_ATack_data_buf[1]<<8) + air720_usart3_ATack_data_buf[0]) != 0)
	{
		lench = (air720_usart3_ATack_data_buf[1]<<8) + air720_usart3_ATack_data_buf[0] - 2;
		air720_usart3_ATack_data_buf[0] = 0;
		air720_usart3_ATack_data_buf[1] = 0;
	}
	return lench;
}


u8 AIR720_Disconnet_and_PowerOff(void)
{	
	if(LL_GPIO_IsOutputPinSet(LTE_PWR_CTL_GPIO_Port,LTE_PWR_CTL_Pin))
	{
		Stop2_delay_ms(1500);
		if(AIR720_Send_Command("+++", "OK", 100, 5))
		{
			AIR720_Hardware_Power_OFF();
		}
		else
		{
			Stop2_delay_ms(1000);
			debug_print("Close connect!\r\n");
			if(AIR720_Send_Command("AT+CIPCLOSE", "CLOSE OK", 100, 5))
			{
				AIR720_Hardware_Power_OFF();
			}
			else
			{
				Stop2_delay_s(2);
				AIR720_POWER_OFF();
			}
		}
	}
	return 0;
}


//硬件控制电源断开再通电的方法进行重启
void AIR720_Hardware_Power_reset(void)
{
	AIR720_Hardware_Power_OFF();
	AIR720_Hardware_Power_ON();
}


//硬件切断电源，切断之前先查询模块是否是开机状态，
//若是先通过POWERKEY关机，等待12s以上再切断电源
void AIR720_Hardware_Power_OFF(void)
{
	if(LL_GPIO_IsOutputPinSet(LTE_PWR_CTL_GPIO_Port,LTE_PWR_CTL_Pin))
	{
		if(AIR720_Send_Command("AT","OK",200,1))
		{
			AIR720_POWER_OFF();
		}
		else
		{
			AIR720_PWRKEY_PWOFF();
			Stop2_delay_s(13);
			AIR720_POWER_OFF();
			debug_print("AIR720 PWRKEY OFF.\r\n");
		}
		Stop2_delay_s(3);
	}
}


//硬件控制通电，通电600ms以内需要按下POWERKEY，否则需要再多等待2s
void AIR720_Hardware_Power_ON(void)
{
	AIR720_POWER_ON();
	Stop2_delay_ms(400);
	AIR720_PWRKEY_PWON();
}


//通过POWERKEY进行开机，按下1.2s以上开机
void AIR720_PWRKEY_PWON(void)
{
	AIR720_PWRKEY_PRESS();
	Stop2_delay_ms(1400);
	AIR720_PWRKEY_FREED();
}

//通过POWERKEY进行关机，按下1.5s以上关机
void AIR720_PWRKEY_PWOFF(void)
{
	AIR720_PWRKEY_PRESS();
	Stop2_delay_ms(1700);
	AIR720_PWRKEY_FREED();
}



