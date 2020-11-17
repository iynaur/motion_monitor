
#include "air720sl.h"
#include "myusart.h"
#include "delay.h"
#include "myiwdg.h"
#include "myrtc.h"
#include "low_power.h"
#include "commu.h"

u8 *server_addr_str = "AT+CIPSTART=\"TCP\",\"121.196.130.108\",\"8100\"";//FC��ʽ������





u8 AIR720_Init(void)
{
	u8 csq_time = 0, cgatt_time = 0, creg_time = 0;
	//Stop2_delay_s(1);
	AIR720_Hardware_Power_reset();//���۵�ǰ״̬�ǹػ����ǿ��� �����¿���
	debug_print("AIR720 POWER ON\r\n");
	Stop2_delay_s(2);
	AIR720_Send_Command("AT", "OK", 100, 5);//����ATֱ������OK��������Ӧ������
	AIR720_Send_Command("AT+IPR=115200","OK",300,5);//���ò�����Ϊ115200
	AIR720_Send_Command("ATE0","OK",500,5);//ȡ������
	AIR720_Send_Command("AT+VER",0,0,0);//
	if(AIR720_Send_Command("AT+CPIN?","READY",300,5))//���SIM���Ƿ�׼����
	{
		debug_print("SIM card not ready!\r\n");
		return 1;
	}
	
	while(AIR720_Send_Command("AT+CSQ","99,99",200,1)==0)//����ź�ֵ�Ƿ�����
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

	while(AIR720_Wait_CREG())//��ѯ��ǰGPRSע��״̬
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
	while(AIR720_Send_Command("AT+CGATT?", "1", 50, 5))//�鿴��ǰGPRS����״̬
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
	
	AIR720_Send_Command("AT+CIPMODE=1","OK",100,5);//����TCPIPӦ��Ϊ͸������ģʽ
	AIR720_Send_Command("AT+CIPMUX=0", "OK", 100, 5);//����Ϊ������ģʽ
	AIR720_Send_Command("AT+CIPQSEND=0", "OK", 100, 5);//����Ϊ�췢ģʽ
	//AIR720_Send_Command("AT+CIPSHUT", 0,0,0);
	//AIR720_Send_Command("AT+CGDCONT?", 0, 0, 0);
	//debug_print("Send AT+CSTT!\r\n");
	AIR720_Send_Command("AT+CSTT", "OK", 100, 5);//������������APN
	//AIR720_Send_Command("AT+CSTT=cmnet", "OK", 100, 5);//������������APN
	AIR720_Send_Command("AT+CIICR", "OK", 100, 5);//�����ƶ���������ȡIP��ַ
	AIR720_Send_Command("AT+CIFSR",0,0,0);//��ѯ�����IP��ַ
	AIR720_Send_Command("AT+CIPSTATUS", "IP STATUS", 100, 5);//��ѯ��ǰ����״̬
	//AIR720_Send_Command("AT+CIPSHOWTP=0","OK",100,5);//���ڽ�������ͷ����ʾЭ������
	//AIR720_Send_Command("AT+CIPHEAD=0","OK",100,5);//����ʾ����ͷ
	//AIR720_Send_Command("AT+CIPSRIP=0","OK",100,5);//����ʾ���ݷ��ͷŵ�IP��ַ�Ͷ˿�
	//AIR720_Send_Command("AT+CIPRX=1", "OK", 100, 5);//ÿ���յ�������ĩβ����ӻس�����
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
	if(AIR720_Send_Command(at_cipsend_str,">",50,5)==0)//��AIR720����ָ�����ȵķ����������5�β��ɹ�����
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
		//air720_usart3_ATack_data_buf[0] = 0;//��������,׼����������
		//air720_usart3_ATack_data_buf[1] = 0;//��������,׼����������
		memset((void*)air720_usart3_ATack_data_buf, 0, AIR720_ATACK_DATA_LENTH);//����,׼����������
		USART3_Send(creg_str,strlen((char*)creg_str));
		//USART1_Send(creg_str,strlen((char*)creg_str));
		delay_ms(100);
		buf_lench = Check_AIR720_USART3_data_lench();
		if(buf_lench != 0)
		{
			for(i = 2; i < buf_lench+2; i++)
			{
				if(air720_usart3_ATack_data_buf[i]==':')//���յ��˷�������
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
			if(count > 20)//��ѯ15�λ���û��ע��ɹ�
			{
				count = 0;
				return 1;
			}
		}
	}
	return 0;
}


//��AIR720SL��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//sendtime:ʧ������ط�����
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 AIR720_Send_Command(u8 *cmd,u8 *ack,u16 waittime,u16 sendtime)
{
	u8 send_flag = 0, send_count = 0;
	u16 delay_time = waittime;
	while(send_flag==0)
	{
		//air720_usart3_ATack_data_buf[0] = 0;//��������,׼����������
		//air720_usart3_ATack_data_buf[1] = 0;//��������,׼����������
		memset((void*)air720_usart3_ATack_data_buf, 0, AIR720_ATACK_DATA_LENTH);//����,׼����������
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
		if(ack&&waittime)//�����ҪӦ��
		{
			while(--delay_time)	//�ȴ�����ʱ
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



//mc20e���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//����,�ڴ�Ӧ������λ��(str��λ��)
u8* AIR720_Find_ack(u8 *str)
{
	char *strx = NULL;
	u16 buf_lench = 0;

	buf_lench = Check_AIR720_USART3_data_lench();
	
	if(buf_lench != 0)
	{
		air720_usart3_ATack_data_buf[buf_lench+2] = 0;//��ӽ�����
		strx=strstr((const char*)&air720_usart3_ATack_data_buf[2],(const char*)str);
	}
	return (u8*)strx;
}

//��鲢��ȡ����3���յ�ATָ������ݵĳ���
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


//Ӳ�����Ƶ�Դ�Ͽ���ͨ��ķ�����������
void AIR720_Hardware_Power_reset(void)
{
	AIR720_Hardware_Power_OFF();
	AIR720_Hardware_Power_ON();
}


//Ӳ���жϵ�Դ���ж�֮ǰ�Ȳ�ѯģ���Ƿ��ǿ���״̬��
//������ͨ��POWERKEY�ػ����ȴ�12s�������жϵ�Դ
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


//Ӳ������ͨ�磬ͨ��600ms������Ҫ����POWERKEY��������Ҫ�ٶ�ȴ�2s
void AIR720_Hardware_Power_ON(void)
{
	AIR720_POWER_ON();
	Stop2_delay_ms(400);
	AIR720_PWRKEY_PWON();
}


//ͨ��POWERKEY���п���������1.2s���Ͽ���
void AIR720_PWRKEY_PWON(void)
{
	AIR720_PWRKEY_PRESS();
	Stop2_delay_ms(1400);
	AIR720_PWRKEY_FREED();
}

//ͨ��POWERKEY���йػ�������1.5s���Ϲػ�
void AIR720_PWRKEY_PWOFF(void)
{
	AIR720_PWRKEY_PRESS();
	Stop2_delay_ms(1700);
	AIR720_PWRKEY_FREED();
}



