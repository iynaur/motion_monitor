#include "logic_app.h"
#include "delay.h"
#include "myadc.h"
#include "myiic.h"
#include "myiwdg.h"
#include "myrtc.h"
#include "mytim.h"
#include "myusart.h"
#include "stmflash.h"
#include "low_power.h"

#include "commu.h"

#include "air720sl.h"
#include "bt02.h"
#include "gps.h"
#include "icm20689.h"

#include "algorithm.h"
void APP_Run(void)
{	
	if((update_flag == 0) && (ble_updating_flag == 0))
	{
		Send_Machine_Data();//算法运算；发送状态数据
	}
	
	msg_check();//各个串口消息处理

	if(wait_ble_update_time > 30000)
	{
		ble_updating_flag = 0;
	}

#if MACAO_ONE_MINUTE
	if(gps_power_on_time > 90000)//打开GPS70秒，然后关闭GPS
#else
	if(gps_power_on_time > 70000)//打开GPS70秒，然后关闭GPS
#endif
	{
		gps_power_on_time = 0;
		GPS_POWER_OFF();
	}

#if MACAO_ONE_MINUTE
	if(wait_update_msg_time > 90000 && update_flag == 0)//如果180秒内没有升级消息则关闭4G模块
#else
	if(wait_update_msg_time > 30000 && update_flag == 0)//如果180秒内没有升级消息则关闭4G模块
#endif
	{
		wait_update_msg_time = 0;
		AIR720_Disconnet_and_PowerOff();
	}

	if(wait_next_update_msg_time > 30000)//等待下一条升级消息超过30秒，关闭4G模块，停止升级
	{
		update_flag = 0;
		wait_next_update_msg_time = 0;
		AIR720_Disconnet_and_PowerOff();
	}

	if(Is_BT02_Connected())//检测蓝牙是否连接成功以置位相应标志位
	{
		ble_connect_flag = 1;
	}
	else
	{
		ble_connect_flag = 0;
	}

	if(wait_ble_msg_time > 3600000)//一小时内没有油箱盖数据，则初始化为0
	{
		wait_ble_msg_time = 0;
		memset((void*)&oil_data, 0, sizeof(ble_oil_data_t));//油箱数据初始化为0
		//BT02_Disconnect();
	}

	if(check_rtc_time > 86400000)//每24小时进行一次对时，以校准RTC时间
	{
		check_rtc_time = 0;
		Check_Time_with_Server();
	}

	feed_iwdg_time = 600000;//看门狗重启时间为10分钟+32.768s
	
	Device_Check_LowPowerMode(1);
	
}


void Device_Check_LowPowerMode(u8 send2server)
{
	if(LL_GPIO_IsInputPinSet(WAKE_PIN2_GPIO_Port, WAKE_PIN2_Pin) != 1)
	{
		debug_print("Dismount!!!\r\n");
		Stop2_delay_s(10);
		if(LL_GPIO_IsInputPinSet(WAKE_PIN2_GPIO_Port, WAKE_PIN2_Pin) != 1)//如果底板抽出
		{
			if(send2server)
			{
				Send_DismountMsg2Server();//发送底板抽出消息到服务器
			}
			GPS_POWER_OFF();//关闭GPS
			BT02_Disconnect();//断开蓝牙连接
			BT02_StopADV();//关闭蓝牙广播
			
			if(LL_GPIO_IsInputPinSet(WAKE_PIN2_GPIO_Port, WAKE_PIN2_Pin) != 1)
			{
				Stop2_delay_s(10);
				if(LL_GPIO_IsInputPinSet(WAKE_PIN2_GPIO_Port, WAKE_PIN2_Pin) != 1)//如果底板抽出
				{
					debug_print("Entry Low Power Mode! \r\n");
					ICM20689_Low_Power_Mode();//六轴ICM20689进入低功耗模式
					RTC_WakeUp_Config(RTC_WKP_DISABLE,0);//禁止RTC唤醒
					LowPower_EnableWakeUpPin(LL_PWR_WAKEUP_PIN2, HIGH);//设置wakeup2脚上升沿唤醒
					Entry_Low_Power(LL_PWR_MODE_SHUTDOWN);//进入SHUTDOWN低功耗模式
				}
			}
			BT02_StartADV();
		}
	}
}

void msg_check(void)
{
	u8 *gps_data_p = NULL, *air720_data_p = NULL, *ble_data_p = NULL;
	u16 gps_len = 0, air720_recv_len = 0, ble_recv_len = 0;

	gps_len = (gps_usart2_data_buf[1]<<8)+gps_usart2_data_buf[0];
	
	if(gps_len != 0)
	{
		debug_print("gps_len is %d.\r\n",gps_len);
		gps_usart2_data_buf[0] = 0;
		gps_usart2_data_buf[1] = 0;
		gps_data_p = (u8*)&gps_usart2_data_buf[2];
		//debug_print("Begin GPS analysis!\r\n");
		GPS_Analysis(&gps_msg, gps_data_p);
		UTCTime_to_LocalTime(&gps_msg,&BJTime,8);
	}

	if((air720_usart3_commu_data_buf[1]<<8)+air720_usart3_commu_data_buf[0] != 0)
	{
		air720_recv_len = (air720_usart3_commu_data_buf[1]<<8)+air720_usart3_commu_data_buf[0] - 2;
		debug_print("air720_recv_lench is %d.\r\n",air720_recv_len);
		air720_usart3_commu_data_buf[0] = 0;
		air720_usart3_commu_data_buf[1] = 0;
		air720_data_p = (u8*)&air720_usart3_commu_data_buf[2];
		UpdateMsg_Handle(air720_data_p,air720_recv_len);
	}

	if((bt02_usart1_commu_data_buf[1] << 8) + bt02_usart1_commu_data_buf[0] != 0)
	{
		ble_recv_len = (bt02_usart1_commu_data_buf[1] << 8) + bt02_usart1_commu_data_buf[0] - 2;
		debug_print("ble_recv_lench is %d.\r\n",ble_recv_len);
		bt02_usart1_commu_data_buf[0] = 0;
		bt02_usart1_commu_data_buf[1] = 0;
		ble_data_p = (u8*)&bt02_usart1_commu_data_buf[2];
		BLE_Msg_Handle(ble_data_p,ble_recv_len);
	}
}


#if MACAO_ONE_MINUTE == 0
u8 Send_Machine_Data(void)
{	
	u8 machine_status = 0;//, i = 0;
	u8 status_new = STATE_MOTIONLESS;

	//static u8 status_arry[10] = {STATE_MOTIONLESS};

	static u8 status_count = 0;
	
	static u8 final_motionless_count = 0;
	static u8 final_standby_count = 0;
	static u8 final_motion_count = 0;

//	u8 motionless_count = 0;
//	u8 standby_count = 0;
//	u8 motion_count = 0;

	static u8 sleep_flag = 0;
	static u8 sleep_motionless_count = 0;
	static u8 sleep_count = 0;
	
	static u8 status_pack_num = 0;
	
	LL_RTC_TimeTypeDef rtc_time = {0};
	LL_RTC_DateTypeDef rtc_date = {0};
	
	machine_status = Get_Machine_Status();

	if(machine_status != STATE_NONE)//已经通过算法获得了状态
	{		
		status_count++;//得到状态次数加1
		status_new = machine_status;
		debug_print("machine_status[%d] is %d.\r\n", status_count, status_new);
#if 0
		for(i=0;i<3;i++)//复制最新的状态到前一个元素
		{
			status_arry[i] = status_arry[i+1];
		}
		status_arry[2] = machine_status;//将最新的状态保存在最后一个数组元素

		//status_old = status_new;//保存上一次的最终状态

		for(i = 0; i < 3; i++)
		{
			switch(status_arry[i])
			{
				case STATE_MOTIONLESS:
					motionless_count++;
					break;

				case STATE_STANDBY:
					standby_count++;
					break;

				case STATE_MOTION:
					motion_count++;
					break;

				default:
					break;
			}
		}

		if(motion_count)
		{
			status_new = STATE_MOTION;
		}
		else if(motionless_count >= 2)
		{
			status_new = STATE_MOTIONLESS;
		}
		else
		{
			status_new = STATE_STANDBY;
		}
#endif

		switch(status_new)
		{
			case STATE_MOTIONLESS:
				final_motionless_count++;
				break;
		
			case STATE_STANDBY:
				final_standby_count++;
				break;
		
			case STATE_MOTION:
				final_motion_count++;
				break;
		
			default:
				break;
		}

		if(status_count >= CHANGE_STATE_TIME)
		{
			status_count = 0;//计数清零，准备下一次计数
			if(final_standby_count >= 6)
			{
				status_new = STATE_STANDBY;
			}
			else if(final_motionless_count && (final_motion_count < 3))
			{
				status_new = STATE_MOTIONLESS;
			}
			else
			{
				status_new = STATE_MOTION;
			}

			//debug_print("status new is %d.\r\n", status_new);
			
			final_standby_count = 0;
			final_motionless_count = 0;
			final_motion_count = 0;

			if(status_new != STATE_MOTIONLESS)
			{
				sleep_motionless_count = 0;
				sleep_flag = 0;
				//change_count = CHANGE_STATE_TIME;
			}
			else if(status_new == STATE_MOTIONLESS && sleep_flag == 0)
			{
				sleep_motionless_count++;
				//debug_print("sleep_motionless_count is %d.\r\n", sleep_motionless_count);
			}

			//如果静止状态达到一定次数，则状态改为休眠
			//并将得到状态的次数改为60次返回最终状态
			if(sleep_motionless_count >= SLEEP_STATE_COUNT)
			{
				sleep_motionless_count = 0;
				sleep_flag = 1;
				//change_count = SLEEP_SEND_TIME;
			}

			if(sleep_flag)
			{
				sleep_count++;
				//debug_print("sleep_count is %d.\r\n", sleep_count);
				if(sleep_count == SLEEP_SEND_TIME)
				{
					sleep_count = 0;
					
					machine_data.status_data[status_pack_num].status	= STATE_DORMANT;
				}
				else
				{
					return 1;
				}
			}
			else
			{
				sleep_count = 0;
				//debug_print("sleep_flag is %d.\r\n", sleep_flag);
				machine_data.status_data[status_pack_num].status	= status_new;
			}
			
			RTC_Time_Get(&rtc_time);
			RTC_Date_Get(&rtc_date);
			machine_data.status_data[status_pack_num].s_year	= rtc_date.Year;
			machine_data.status_data[status_pack_num].s_month = rtc_date.Month;
			machine_data.status_data[status_pack_num].s_day 	= rtc_date.Day;
			machine_data.status_data[status_pack_num].s_hour	= rtc_time.Hours;
			machine_data.status_data[status_pack_num].s_min 	= rtc_time.Minutes;
			machine_data.status_data[status_pack_num].s_sec 	= rtc_time.Seconds;
			
			status_pack_num++;
			debug_print("status num is %d.\r\n", status_pack_num);
			debug_print("status_data[%d] is %d.\r\n", status_pack_num - 1, machine_data.status_data[status_pack_num-1].status);
			if(status_pack_num < 3) 
			{
				if(status_pack_num == 2)
				{
					debug_print("GPS power on!\r\n");
					GPS_POWER_ON();
					gps_power_on_time = 0;
				}
				return 1;
			}
			else status_pack_num = 0;
			
			Fill_Machine_Data();
			
			if(Send_SocketData2Server(SOCKET_CMD_DATA, SOCKET_DATA_LENTH,1) != SOCKET_CMD_DATA_ACK)
			{
				Save_Unsend_Data2Flash();
				AIR720_Disconnet_and_PowerOff();
			}
			else
			{
				Check_Flash_Unsend_Data();
			}
		}
	}
	return 0;
}
#else
u8 Send_Machine_Data(void)
{	
	u8 machine_status = 0, i = 0;

	static u8 status_arry[10] = {STATE_MOTIONLESS};

	static u8 status_count = 0;
	
	static u8 status_old = STATE_MOTIONLESS;
	static u8 status_new = STATE_MOTIONLESS;
	
	static u8 sleep_flag = 0;
	static u8 sleep_motionless_count = 0;
	//static u8 sleep_count = 0;
	
	static u8 status_pack_num = 0;

	static u8 change_count = CHANGE_STATE_TIME;
	
	LL_RTC_TimeTypeDef rtc_time = {0};
	LL_RTC_DateTypeDef rtc_date = {0};
	
	machine_status = Get_Machine_Status();

	if(machine_status != STATE_NONE)//已经通过算法获得了状态
	{		
		status_count++;//得到状态次数加1
		debug_print("machine_status[%d] is %d.\r\n", status_count, machine_status);
		
		for(i=0;i<CHANGE_STATE_TIME;i++)//复制最新的状态到前一个元素
		{
			status_arry[i] = status_arry[i+1];
		}
		status_arry[CHANGE_STATE_TIME-1] = machine_status;//将最新的状态保存在最后一个数组元素

		status_old = status_new;//保存上一次的最终状态

		for(i=0;i<CHANGE_STATE_TIME-1;i++)
		{
			if((status_arry[i] == STATE_MOTIONLESS) || (status_arry[CHANGE_STATE_TIME-1] == STATE_MOTIONLESS))
			{
				status_new = STATE_MOTIONLESS;
				break;
			}
			if(status_arry[i] != status_arry[CHANGE_STATE_TIME-1])//比较是否三次状态都相同
			{
				status_new = status_old;//若有不同，新的最终状态保持为上一次的最终状态
				break;
			}
		}

		if(i == (CHANGE_STATE_TIME-1))//三次状态都相同
		{
			status_new = status_arry[CHANGE_STATE_TIME-1];//更新最终状态为最新的一次状态
		}

		
		//如果最终状态不是静止，则清零休眠相关标志位和计数
		//并将得到状态的次数改为三次就确定最终状态
		if(status_new != STATE_MOTIONLESS)
		{
			sleep_motionless_count = 0;
			sleep_flag = 0;
			change_count = CHANGE_STATE_TIME;
		}
		
		if(status_count >= change_count)//如果得到状态的次数已经达到指定次数，说明更新了最终状态
		{
			status_count = 0;//计数清零，准备下一次计数
			//如果最终状态为静止且休眠标志为0，则静止次数加1
			if(status_new==STATE_MOTIONLESS && sleep_flag == 0)
			{
				sleep_motionless_count++;
			}

			//如果静止状态达到一定次数，则状态改为休眠
			//并将得到状态的次数改为60次返回最终状态
			if(sleep_motionless_count >= SLEEP_STATE_COUNT)
			{
				sleep_motionless_count = 0;
				change_count = SLEEP_SEND_TIME;
				sleep_flag = 1;
			}
			
			RTC_Time_Get(&rtc_time);
			RTC_Date_Get(&rtc_date);
			
			if(sleep_flag)
				machine_data.status_data[status_pack_num].status  = STATE_DORMANT;
			else
				machine_data.status_data[status_pack_num].status  = status_new;
			machine_data.status_data[status_pack_num].s_year	= rtc_date.Year;
			machine_data.status_data[status_pack_num].s_month = rtc_date.Month;
			machine_data.status_data[status_pack_num].s_day 	= rtc_date.Day;
			machine_data.status_data[status_pack_num].s_hour	= rtc_time.Hours;
			machine_data.status_data[status_pack_num].s_min 	= rtc_time.Minutes;
			machine_data.status_data[status_pack_num].s_sec 	= rtc_time.Seconds;
			
			status_pack_num++;
			debug_print("status num is %d.\r\n", status_pack_num);
			debug_print("status_data[%d] is %d.\r\n", status_pack_num - 1, machine_data.status_data[status_pack_num-1].status);
			if(status_pack_num < 3) 
			{
				if(status_pack_num == 2)
				{
					debug_print("GPS power on!\r\n");
					GPS_POWER_ON();
					gps_power_on_time = 0;
				}
				return 1;
			}
			else status_pack_num = 0;
			
			Fill_Machine_Data();
			
			if(Send_SocketData2Server(SOCKET_CMD_DATA, SOCKET_DATA_LENTH,1) != SOCKET_CMD_DATA_ACK)
			{
				Save_Unsend_Data2Flash();
				AIR720_Disconnet_and_PowerOff();
			}
			else
			{
				Check_Flash_Unsend_Data();
			}
		}
	}
	return 0;
}
#endif

u8 Get_Machine_Status(void)
{
	static u16 i = 0;
	
  short aacx,aacy,aacz;		//加速度传感器原始数据
  short gyrox,gyroy,gyroz;	//陀螺仪原始数据

	float value_stand, value_motion;
	u8 int_state = 0;
	u8 status = STATE_NONE;
	
	
	ICM20689_Get_INT_Status(&int_state);
	if(int_state == 1)
	{
		ICM20689_Get_Accelerometer(&aacx,&aacy,&aacz); //得到加速度传感器数据
		ICM20689_Get_Gyroscope(&gyrox,&gyroy,&gyroz);	//得到陀螺仪数据
		ICM20689_Get_Temperature(&temp);
		//debug_print("ax is %d,ay is %d,az is %d\r\n",aacx,aacy,aacz);
		//debug_print("gx is %d,gy is %d,gz is %d\r\n",gyrox,gyroy,gyroz);
		if(i<1000)
		{
			gx_in[i] = (float)(gyrox-0.00f)*500/32768;
			gy_in[i] = (float)(gyroy-0.00f)*500/32768;
			gz_in[i] = (float)(gyroz-0.00f)*500/32768;

			if(i%2==0)
			{
				ax_in[i/2] = (float)(aacx-0.00f)*4/32768;
				ay_in[i/2] = (float)(aacy-0.00f)*4/32768;
				az_in[i/2] = (float)(aacz-0.00f)*4/32768; 
			}
			i++;
		}
		else
		{
			i = 0;
			
			#if 1
			pitch = (atan(ax_in[499]/sqrt(ay_in[499]*ay_in[499]+az_in[499]*az_in[499])));
			pitch = pitch * 57.2957795f; //后面的数字是180/PI 目的是弧度转角度
			roll = (atan(ay_in[499]/sqrt(ax_in[499]*ax_in[499]+az_in[499]*az_in[499])));
			roll = roll * 57.2957795f; //后面的数字是180/PI 目的是弧度转角度

			if(az_in[499] < 0)
			{
				if(pitch > 0)pitch = 180 - pitch;
				else pitch = -180 - pitch;

				if(roll > 0)roll = 180 - roll;
				else roll = -180 - roll;
			}
			#endif
			
			value_stand = algor(ax_in, ay_in,az_in);//得到判断静止状态的值
			value_motion = optimi(gx_in,gy_in,gz_in);//得到新算法的值

			//新算法判断状态
			if(value_motion>0.42f)
			{
				status = STATE_MOTION;//运动状态
			}
			else 
			{
				//debug_print("value_stand is %f.\r\n",value_stand);
				if(value_stand>0.00376f)//开始是0.00194
				{ 																								
					status = STATE_STANDBY;//待机状态
				}
				else
				{
					status = STATE_MOTIONLESS;
				}
			}
			//debug_print("Have get status!\r\n");
		}
	}
	return status;
}


u8 Send_DismountMsg2Server(void)
{
	LL_RTC_TimeTypeDef rtc_time = {0};
	LL_RTC_DateTypeDef rtc_date = {0};

	u8 i = 0;
	
	RTC_Time_Get(&rtc_time);
	RTC_Date_Get(&rtc_date);

	for(i=0; i<3; i++)
	{
		machine_data.status_data[i].status  = STATE_DISMOUNT;
		machine_data.status_data[i].s_year  = rtc_date.Year;
		machine_data.status_data[i].s_month = rtc_date.Month;
		machine_data.status_data[i].s_day	  = rtc_date.Day;
		machine_data.status_data[i].s_hour  = rtc_time.Hours;
		machine_data.status_data[i].s_min	  = rtc_time.Minutes;
		machine_data.status_data[i].s_sec	  = rtc_time.Seconds;
	}
	
	Fill_Machine_Data();

	if(Send_SocketData2Server(SOCKET_CMD_DATA, SOCKET_DATA_LENTH,1) != SOCKET_CMD_DATA_ACK)
	{
		Save_Unsend_Data2Flash();
	}
	AIR720_Disconnet_and_PowerOff();
	return 0;
}


void Fill_Machine_Data(void)
{
	u16 adc = 0;
	u16 speed = 0;

	
	//STAT1  STAT2
	//	0 		 1			正在充电
	//	1 		 0			充电完成
	//	1 		 1			其他
	if(LL_GPIO_IsInputPinSet(CHARGE_STAT1_GPIO_Port, CHARGE_STAT1_Pin))
	{
		debug_print("STAT1 set.\r\n");
		if(LL_GPIO_IsInputPinSet(CHARGE_STAT2_GPIO_Port, CHARGE_STAT2_Pin))
			machine_data.charge_status = NO_CHARGING;
		else
			machine_data.charge_status = FINISHED_CHARGING;
	}
	else
	{
		debug_print("STAT1 reset.\r\n");
		if(LL_GPIO_IsInputPinSet(CHARGE_STAT2_GPIO_Port, CHARGE_STAT2_Pin))
			machine_data.charge_status = IN_CHARGING;
		else
			machine_data.charge_status = NO_CHARGING;
	}
	
#if 1
	debug_print("charge is %d.\r\n",machine_data.charge_status);
	adc = Get_Adc_Average(ADC_BAT_CHANNEL, 5);
	adc = (adc*3340*2/4095 - 3350)*100/(4150-3350);
	if(adc > 100) adc = 100;
	machine_data.BAT_value = adc;
	
	machine_data.temp_h = (temp >> 8)&0xFF;
	machine_data.temp_l = temp & 0xFF;
	debug_print("temp is %d.\r\n", temp);
	machine_data.pitch_h = (((int)pitch*10 + 1800) >> 8)&0xFF;
	machine_data.pitch_l = ((int)pitch*10 + 1800)&0xFF;
	
	machine_data.yaw_h = (((int)yaw*10 + 1800) >> 8)&0xFF;
	machine_data.yaw_l = ((int)yaw*10 + 1800)&0xFF;
	
	machine_data.roll_h = (((int)roll*10 + 1800) >> 8)&0xFF;
	machine_data.roll_l = ((int)roll*10 + 1800)&0xFF;
	
	machine_data.ew_ns_flag = 0x00;
	if(gps_msg.nshemi == 'S') 								//经纬度标志位
	{
		machine_data.ew_ns_flag += 0xF0;
	} 	
	if(gps_msg.ewhemi == 'E') 
	{
		machine_data.ew_ns_flag += 0x0F;
	} 
	debug_print("ew_ns is 0x%02x.\r\n",machine_data.ew_ns_flag);
	if(gps_msg.gpssta)
	{
		machine_data.longitude1 = gps_msg.longitude / 100000;
		machine_data.longitude2 = gps_msg.longitude % 100000 * 60 / 65536;
		machine_data.longitude3 = gps_msg.longitude % 100000 * 60 % 65536 / 256;
		machine_data.longitude4 = gps_msg.longitude % 100000 * 60 % 256;
		
		machine_data.latitude1 = gps_msg.latitude / 100000;
		machine_data.latitude2 = gps_msg.latitude % 100000 * 60 / 65536;
		machine_data.latitude3 = gps_msg.latitude % 100000 * 60 % 65536 / 256;
		machine_data.latitude4 = gps_msg.latitude % 100000 * 60 % 256;
	
		speed = gps_msg.speed / 10;
		machine_data.speed_h = (speed >> 8) & 0xFF;
		machine_data.speed_l = speed & 0xFF;
		
		machine_data.altitude_24b = (gps_msg.altitude >> 24) & 0xFF;
		machine_data.altitude_16b = (gps_msg.altitude >> 16) & 0xFF;
		machine_data.altitude_8b	= (gps_msg.altitude >>	8) & 0xFF;
		machine_data.altitude_b 	= (gps_msg.altitude >>	0) & 0xFF;
	}
	else
	{
		memset((void*)&machine_data.longitude1, 0, 8);//未定到位填0
		memset((void*)&machine_data.speed_h, 0, 6);//未定到位填0
	}
	
	memcpy((void*)&machine_data.oil_height_h, (void*)&oil_data.height_h, 6);
	debug_print("oil_height_h is %d.\r\n", oil_data.height_h);
	debug_print("oil_height_l is %d.\r\n", oil_data.height_l);
	
	machine_data.master_ver1 = Version[0] - '0';
	machine_data.master_ver2 = 10 * (Version[2] - '0') + (Version[3] - '0');
	machine_data.master_ver3 = 10 * (Version[5] - '0') + (Version[6] - '0');
	
	// machine_data.slave_ver1 = Version[0] - '0';
	// machine_data.slave_ver2 = 10 * (Version[2] - '0') + (Version[3] - '0');
	// machine_data.slave_ver3 = 10 * (Version[5] - '0') + (Version[6] - '0');
	machine_data.slave_ver1 = oil_data.oil_ver1;
	machine_data.slave_ver2 = oil_data.oil_ver2;
	machine_data.slave_ver3 = oil_data.oil_ver3;
#endif
}

u8 UpdateMsg_Handle(u8 *data_buf, u16 lench)
{
	static u8 image_count = 0;//包数量
	static u32 image_lench = 0;//程序总长度
	u16 pack_lench = 0;//每一包的长度
	u32 flash_update_flag = APP_UPDATE_FLAG;
	u16 recv_lench = 0;
	u16 i = 0;
	u8 image_data[256] = {0};
	u16 re_write = 0;
	socket_data_pack_p socket_msg = NULL;

	socket_msg = (void*)data_buf;
	
	if(socket_msg->cmd == SOCKET_CMD_UPDATE_SERVER)
	{
		debug_print("Start update!!!\r\n");

		recv_lench = (socket_msg->lenth_h << 8) + socket_msg->lenth_l;

		memcpy((void*)&update_msg, &socket_msg->socket_data.socket_update_msg, recv_lench-11);

		if(update_msg.pack_num == 0)//服务器下发的升级确认指令
		{
			update_msg.update_data[0] = 0x01;
			Send_SocketData2Server(SOCKET_CMD_UPDATE_ACK, SOCKET_UPDATE_ACK_LENTH, 0);
			update_flag = 1;
		}
		else
		{
			if(update_flag==0)
			{
				image_count  = 0;
				image_lench = 0;
				debug_print("update_flag is seted to zero!\r\n");
				return 1;
			}
			
			if(update_msg.pack_count*256 > STMFLASH_UPDATE_DATA_MAX_LENTH)
			{
				image_count  = 0;
				image_lench = 0;
				update_flag = 0;
				debug_print("Update data is too large!\r\n");
				return 1;
			}
			wait_next_update_msg_time = 0;
			image_count++;
			if(update_msg.pack_num == image_count)//包序号是否与已接收的包数量相等
			{
				pack_lench = recv_lench - 17;
				
				FLASH_REWRITE:
				re_write++;
				if(re_write > 50)
				{
					image_count  = 0;
					image_lench = 0;
					update_flag = 0;
					debug_print("FLASH re_write time out!\r\n");
					return 1;
				}

				STMFLASH_Write(STMFLASH_UPDATE_DATA_ADDR+image_lench, (u8*)update_msg.update_data, pack_lench);
				delay_ms(100);
				STMFLASH_Read(STMFLASH_UPDATE_DATA_ADDR+image_lench, image_data, pack_lench);
				
				debug_print("pack_lench is %d.\r\n",pack_lench);
				
				for(i = 0; i < pack_lench; i++)
				{
					if(image_data[i] != update_msg.update_data[i])
					{
						debug_print("update_data[%d] is 0x%02x, image_data[%d] is 0x%02x.\r\n",i,update_msg.update_data[i],i,image_data[i]);
						goto FLASH_REWRITE;
					}
				}
				
				image_lench +=  pack_lench;
				
				update_msg.update_data[0] = 0x01;
				//if(image_count != update_msg.pack_count)
				Send_SocketData2Server(SOCKET_CMD_UPDATE_ACK, SOCKET_UPDATE_ACK_LENTH, 0);
				//debug_print("image_count is 0x%02x, pack_count  is 0x%02x.\r\n",image_count,pack_counts);
				delay_ms(1000);
			}
			else
			{
				update_msg.update_data[0] = 0x00;//包序号出错
				Send_SocketData2Server(SOCKET_CMD_UPDATE_ACK, SOCKET_UPDATE_ACK_LENTH, 0);

				image_count  = 0;
				image_lench = 0;
				update_flag = 0;
				return 1;
			}

			if(image_count == update_msg.pack_count)//是否接收完毕
			{
				//主从机区分处理
				if(update_msg.master_or_slave == 0x01)
				{
					STMFLASH_Write(STMFLASH_UPDATE_DATA_LENTH_ADDR, (u8*)&image_lench,4);
					STMFLASH_Write(STMFLASH_UPDATE_FLAG_ADDR, (u8*)&flash_update_flag,4);

					debug_print("image_count is 0x%02x, pack_count  is 0x%02x.\r\n",image_count,update_msg.pack_count);

					AIR720_Disconnet_and_PowerOff();

					NVIC_SystemReset();
				}
				else
				{
					debug_print("TankCover image_count is 0x%02x, pack_count  is 0x%02x.\r\n",image_count,update_msg.pack_count);
					ble_update_flag = 1;
					ble_update_len = image_lench;
					ble_pack_count = update_msg.pack_count;
				}
				image_count  = 0;
				image_lench = 0;
				update_flag = 0;
			}
		}
	}
	return 0;
}



void Check_Time_with_Server(void)
{
	u16 check_time_count = 0;
	while(Send_SocketData2Server(SOCKET_CMD_CHECK_TIME, SOCKET_CHECK_TIME_LENTH, 1) != SOCKET_CMD_CHECK_TIME_ACK)
	{
		debug_print("Set RTC time from server failed!Try again......\r\n");
		delay_ms(500);
		check_time_count++;
		if(check_time_count>=5)
		{
			debug_print("Set RTC_time from server time out!\r\n");
			break;
		}
	}
	//AIR720_Disconnet_and_PowerOff();
	if(check_time_count>=5)
	{
		check_time_count = 0;
		debug_print("Set RTC_time from GPS's UTC_time\r\n");
		GPS_POWER_ON();
		delay_s(3);
		msg_check();
		while(gps_msg.utc.year < 2020 || gps_msg.utc.year > 2030)//GPS时间小于2020年说明没有成功获取卫星时间
		{
			debug_print("UTC year is %d.\r\n",gps_msg.utc.year);
			delay_ms(100);
			msg_check();
			delay_ms(100);
			check_time_count++;
			if(check_time_count > 1000)
			{
				GPS_POWER_OFF();
				return;
			}
		}
		GPS_POWER_OFF();
		debug_print("GPS power off!\r\n");
		
		debug_print("UTC year is %d.\r\n",gps_msg.utc.year);
		RTC_Set_Time(BJTime.hour,
								 BJTime.min,
								 BJTime.sec);
		RTC_Set_Date(BJTime.year-2000,
								 BJTime.month,
								 BJTime.date,
								 BJTime.week);
	}
	debug_print("RTC set right\r\n");

}


u8 Send_SocketData2Server(SOCKET_CMD cmd, u16 lench, u8 ack)
{	
	
	u8 data_str[210] = {0};
	u16 i = 0;
	
	u8 data_buf[100] = {0};
	static u8 data_num = 0;
	socket_data_pack_p socket_send = (void*)data_buf;

	u16 wait_time = 0;
	u16 send_time = 0;
	u16 init_time = 0;
	u16 recv_lench = 0;
	socket_data_pack_p socket_recv = NULL;

	socket_send->head1 = SOCKET_DATA_HEAD1;
	socket_send->head2 = SOCKET_DATA_HEAD2;

	socket_send->type_id = DEVICE_TYPE_ID;

	socket_send->id1 = (device_id>> 16) & 0xFF;
	socket_send->id2 = (device_id >>  8) & 0xFF;
	socket_send->id3 = (device_id) & 0xFF;

	socket_send->data_num = data_num++;
	
	socket_send->lenth_h = (lench >> 8) & 0xFF;
	socket_send->lenth_l = lench & 0xFF;

	socket_send->cmd = cmd;
	
	if(socket_send->cmd==SOCKET_CMD_CHECK_TIME)
	{
		socket_send->socket_data.xor_check = Check_XOR_sum(&socket_send->type_id, lench - 3);//不算头码和校验本身
	}
	else if(socket_send->cmd == SOCKET_CMD_DATA)
	{
		memcpy(&socket_send->socket_data.socket_machine_data, (void*)&machine_data, lench - 11);
		socket_send->socket_data.socket_machine_data.xor_check = Check_XOR_sum(&socket_send->type_id, lench - 3);
	}
	else if(socket_send->cmd == SOCKET_CMD_UPDATE_ACK)
	{
		memcpy(&socket_send->socket_data.socket_update_msg, (void*)&update_msg, lench - 11);
		socket_send->socket_data.socket_update_msg.update_data[1] = Check_XOR_sum(&socket_send->type_id, lench - 3);
	}

	for(i=0; i<lench; i++)
	{
		data_str[i*3] = hextochar((data_buf[i]>>4)&0x0F);
		data_str[i*3+1] = hextochar(data_buf[i]&0x0F);
		data_str[i*3+2] = 0x20;//空格
	}
	LPUART1_Send(data_str,lench*3);									//以字符串方式打印数据包
	debug_print("\r\n");

	if(LL_GPIO_IsOutputPinSet(LTE_PWR_CTL_GPIO_Port, LTE_PWR_CTL_Pin) != 1)
	{
		while(AIR720_Init())
		{
			delay_ms(200);
			init_time++;
			if(init_time>3) 
			{
				AIR720_Disconnet_and_PowerOff();
				return 1;
			}
		}
	}
	while(AIR720_Send_data(data_buf, lench))//发送数据，3次不成功返回错误1
	{
		delay_ms(200);
		send_time++;
		if(send_time>3) 
		{
			AIR720_Disconnet_and_PowerOff();
			return 1;
		}
	}
	
	if(ack)
	{
		while(((air720_usart3_commu_data_buf[1]<<8) + air720_usart3_commu_data_buf[0])==0)
		{
			delay_ms(10);	
			wait_time++;
			//debug_print("No ack!\r\n");
			if(wait_time>3000)
			{
				AIR720_Disconnet_and_PowerOff();
				return 1;
			}	
		}
		recv_lench = (air720_usart3_commu_data_buf[1]<<8) + air720_usart3_commu_data_buf[0] - 2;
		debug_print("recv_lench is %d.\r\n",recv_lench);
		air720_usart3_commu_data_buf[0] = 0;
		air720_usart3_commu_data_buf[1] = 0;
		socket_recv = (void*)&air720_usart3_commu_data_buf[2];
		if(socket_recv->cmd == (cmd|0x80))
		{
			if(socket_recv->cmd==SOCKET_CMD_CHECK_TIME_ACK)
			{
				RTC_Set_Time(socket_recv->socket_data.socket_time.hour,
					           socket_recv->socket_data.socket_time.min,
					           socket_recv->socket_data.socket_time.sec);
				RTC_Set_Date(socket_recv->socket_data.socket_time.year,
										 socket_recv->socket_data.socket_time.month,
										 socket_recv->socket_data.socket_time.date,
										 socket_recv->socket_data.socket_time.week);
			}
			debug_print("cmd is 0x%02x.\r\n", socket_recv->cmd);
			//AIR720_Disconnet_and_PowerOff();
			wait_update_msg_time = 0;
			return socket_recv->cmd;
		}
		else 
		{
			AIR720_Disconnet_and_PowerOff();
			return 1;
		}
	}
	//AIR720_Disconnet_and_PowerOff();
	wait_update_msg_time = 0;
	return 0;
}


void Save_Unsend_Data2Flash(void)
{
	u16 unsend_data_num = 0;
	u32 Phead = 0;
	u32 Ptail = 0;
	
	STMFLASH_Read(STMFLASH_UNSEND_DATA_NUM_ADDR,(u8*)&unsend_data_num,2);
	STMFLASH_Read(STMFLASH_UNSEND_DATA_PTAIL_ADDR,(u8*)&Ptail,4);
	
	unsend_data_num++;

	if(Ptail > STMFLASH_UNSEND_DATA_ADDR + 64*(640-1))
	{
		Ptail = STMFLASH_UNSEND_DATA_ADDR;
	}
	
	if(unsend_data_num > UNSEND_DATA_MAX_NUM)
	{
		unsend_data_num = UNSEND_DATA_MAX_NUM;
		Phead = Ptail + 64;
		STMFLASH_Write(STMFLASH_UNSEND_DATA_PHEAD_ADDR,(u8*)&Phead,4);
	}

	STMFLASH_Write(Ptail, (u8*)&machine_data, sizeof(machine_data));
	Ptail += 64;
	
	//debug_print("Write unsend data num is %d.\r\n",unsend_data_num);
	STMFLASH_Write(STMFLASH_UNSEND_DATA_NUM_ADDR,(u8*)&unsend_data_num,2);
	STMFLASH_Write(STMFLASH_UNSEND_DATA_PTAIL_ADDR,(u8*)&Ptail,4);
}


void Check_Flash_Unsend_Data(void)
{
	u16 unsend_data_num = 0, i = 0;
	u16 data_num = 0;
	u32 Phead = 0;
	machine_data_t machine_data_buf = {0};

	STMFLASH_Read(STMFLASH_UNSEND_DATA_NUM_ADDR,(u8*)&unsend_data_num,2);
	STMFLASH_Read(STMFLASH_UNSEND_DATA_PHEAD_ADDR,(u8*)&Phead,4);
	debug_print("Unsend data num is %d.\r\n",unsend_data_num);
	
	if(unsend_data_num != 0)
	{
		for(i=0;i<unsend_data_num;i++)
		{
			if(Phead > STMFLASH_UNSEND_DATA_ADDR + 64*(640-1))
			{
				Phead = STMFLASH_UNSEND_DATA_ADDR;
			}
			
			STMFLASH_Read(Phead, (u8*)&machine_data_buf, sizeof(machine_data_t));
			memcpy((void*)&machine_data, &machine_data_buf, sizeof(machine_data_t));
			
			if(Send_SocketData2Server(SOCKET_CMD_DATA,SOCKET_DATA_LENTH,1) == SOCKET_CMD_DATA_ACK)
			{
				data_num = unsend_data_num - i - 1;
				Phead += 64;
			}
			else
			{
				break;
			}
		}
		debug_print("Rewrite unsend data num is %d.\r\n", data_num);
		STMFLASH_Write(STMFLASH_UNSEND_DATA_NUM_ADDR, (u8*)&data_num, 2);
		STMFLASH_Write(STMFLASH_UNSEND_DATA_PHEAD_ADDR,(u8*)&Phead,4);
	}
}


u8 BLE_Msg_Handle(u8 *data_buf, u16 lench)
{
	u8 i = 0;
	ble_data_pack_p ble_msg = NULL;

	LL_RTC_TimeTypeDef rtc_time = {0};
	LL_RTC_DateTypeDef rtc_date = {0};
	
	ble_msg = (void*)data_buf;

	wait_ble_msg_time = 0;

	
	//debug_print("ble cmd is 0x%02x.\r\n", ble_msg->cmd);
	
	if(ble_msg->cmd == BLE_CMD_CHECK_TIME)
	{
		Send_BLE_Data2Master(BLE_CMD_CHECK_TIME_ACK, BLE_CHECK_TIME_ACK_LENTH);
	}
	else if(ble_msg->cmd == BLE_CMD_DATA)
	{
		memcpy((void*)&oil_data, &ble_msg->ble_data.ble_oil_data, sizeof(ble_oil_data_t));
		Send_BLE_Data2Master(BLE_CMD_DATA_ACK, BLE_DATA_ACK_LENTH);
		if(oil_data.oil_status==0x02 || oil_data.oil_status==0x03)
		{
			//memcpy((void*)&machine_data.oil_height_h, (void*)&oil_data.height_h, 6);
			
			RTC_Time_Get(&rtc_time);
			RTC_Date_Get(&rtc_date);

			for(i=0;i<3;i++)
			{
				machine_data.status_data[i].s_year  = rtc_date.Year;
				machine_data.status_data[i].s_month = rtc_date.Month;
				machine_data.status_data[i].s_day   = rtc_date.Day;
				machine_data.status_data[i].s_hour  = rtc_time.Hours;
				machine_data.status_data[i].s_min	  = rtc_time.Minutes;
				machine_data.status_data[i].s_sec	  = rtc_time.Seconds;
			}
			Fill_Machine_Data();
			
			if(Send_SocketData2Server(SOCKET_CMD_DATA, SOCKET_DATA_LENTH, 1) != SOCKET_CMD_DATA_ACK)
			{
				Save_Unsend_Data2Flash();
			}
		}
		
		//发送升级请求
		if(ble_update_flag == 1)
		{
			wait_ble_update_time = 0;	//升级等待时间清零
			ble_updating_flag = 1;	//正在升级标准位
			ble_pack_num = 0;
			Send_BLE_UpdateData2Master(ble_pack_num);
			ble_pack_num++;
		}
	}
	else if ((ble_msg->cmd == BLE_CMD_UPDATE_ACK) && (ble_updating_flag == 1))
	{
		wait_ble_update_time = 0;  //升级等待时间清零
		if(ble_pack_num <= ble_pack_count)
		{
			Send_BLE_UpdateData2Master(ble_pack_num);
			ble_pack_num++;
		}
		else
		{
			//升级成功
			wait_ble_update_time = 0;
			ble_updating_flag = 0;
			ble_update_flag = 0;
			ble_pack_num = 0;
		}
	}

	return 0;
}

u8 Send_BLE_Data2Master(BLE_CMD cmd, u16 lench)
{
	
	u8 data_buf[30] = {0};
	static u8 data_num = 0;

	LL_RTC_TimeTypeDef rtc_time = {0};
	LL_RTC_DateTypeDef rtc_date = {0};
	
	ble_data_pack_p ble_data_send = (void*)data_buf;
	
	ble_data_send->head1 = BLE_DATA_HEAD1;
	ble_data_send->head2 = BLE_DATA_HEAD2;

	ble_data_send->type_id = BLE_TYPE_ID;

	ble_data_send->id1 = (device_id >> 16) & 0xFF;
	ble_data_send->id2 = (device_id >>  8) & 0xFF;
	ble_data_send->id3 = (device_id >>  0) & 0xFF;

	ble_data_send->data_num = data_num++;

	ble_data_send->lenth_h = (lench >> 8) & 0xFF ;
	ble_data_send->lenth_l = lench & 0xFF;

	ble_data_send->cmd = cmd;
	if(ble_data_send->cmd == BLE_CMD_CHECK_TIME_ACK)
	{
		RTC_Time_Get(&rtc_time);
		RTC_Date_Get(&rtc_date);
		ble_data_send->ble_data.ble_time.year  = rtc_date.Year;
		ble_data_send->ble_data.ble_time.month = rtc_date.Month;
		ble_data_send->ble_data.ble_time.date  = rtc_date.Day;
		ble_data_send->ble_data.ble_time.week  = rtc_date.WeekDay;
		ble_data_send->ble_data.ble_time.hour  = rtc_time.Hours;
		ble_data_send->ble_data.ble_time.min= rtc_time.Minutes;
		ble_data_send->ble_data.ble_time.sec= rtc_time.Seconds;
		ble_data_send->ble_data.ble_time.xor_check = Check_XOR_sum(&ble_data_send->type_id, lench - 3);//不算两个头码和校验本身
	}
	else if(ble_data_send->cmd == BLE_CMD_DATA_ACK)
	{
		ble_data_send->ble_data.ble_ack.data_ack = 0x01;
		ble_data_send->ble_data.ble_ack.xor_check = Check_XOR_sum(&ble_data_send->type_id, lench - 3);
	}

#if 1
	u8 data_str[90] = {0};
	u16 i = 0;
	for(i=0; i<lench; i++)
	{
		data_str[i*3] = hextochar((data_buf[i]>>4)&0x0F);
		data_str[i*3+1] = hextochar(data_buf[i]&0x0F);
		data_str[i*3+2] = 0x20;//空格
	}
	LPUART1_Send(data_str,lench*3);									//以字符串方式打印数据包
	debug_print("\r\n");
#endif

	BT02_Send_Data(data_buf, lench);

	
	return 0;
}


u8 Send_BLE_UpdateData2Master(u8 pack_num)
{
	u8 data_buf[300] = {0};
	u16 lench = 0;
	u16 pack_lench = 0;//每一包的长度
	static u32 image_lench = 0;//程序数据偏移
	static u8 data_num = 0;
	
	ble_data_pack_p ble_data_send = (void*)data_buf;
	
	ble_data_send->head1 = BLE_DATA_HEAD1;
	ble_data_send->head2 = BLE_DATA_HEAD2;

	ble_data_send->type_id = BLE_TYPE_ID;

	ble_data_send->id1 = (device_id >> 16) & 0xFF;
	ble_data_send->id2 = (device_id >>  8) & 0xFF;
	ble_data_send->id3 = (device_id >>  0) & 0xFF;

	ble_data_send->data_num = data_num++;

	ble_data_send->cmd = BLE_CMD_UPDATE;
	
	ble_data_send->ble_data.ble_update_msg.master_or_slave = 0x00;
	ble_data_send->ble_data.ble_update_msg.update_ver3 = 0;
	ble_data_send->ble_data.ble_update_msg.update_ver2 = 0;
	ble_data_send->ble_data.ble_update_msg.update_ver1 = 0;
	ble_data_send->ble_data.ble_update_msg.pack_count = ble_pack_count;
	ble_data_send->ble_data.ble_update_msg.pack_num = pack_num;

	if(pack_num == 0)
	{
		image_lench = 0;
		ble_data_send->lenth_h = (17 >> 8) & 0xFF ;
		ble_data_send->lenth_l = 17 & 0xFF;
		lench = 17;
		ble_data_send->ble_data.ble_update_msg.update_data[0] = Check_XOR_sum(&ble_data_send->type_id, 17 - 3);
	}
	else
	{
		if((ble_update_len - image_lench) > 256) pack_lench = 256;
		else pack_lench = (ble_update_len - image_lench);
		
		STMFLASH_Read(STMFLASH_UPDATE_DATA_ADDR+image_lench, ble_data_send->ble_data.ble_update_msg.update_data, pack_lench);
		ble_data_send->lenth_h = ((pack_lench+17) >> 8) & 0xFF ;
		ble_data_send->lenth_l = (pack_lench+17) & 0xFF;
		lench = (pack_lench+17);
		if(pack_lench == 256)
		{
			ble_data_send->ble_data.ble_update_msg.xor_check = Check_XOR_sum(&ble_data_send->type_id, lench-3);
		}
		else
		{
			ble_data_send->ble_data.ble_update_msg.update_data[pack_lench] = Check_XOR_sum(&ble_data_send->type_id, lench-3);
		}
		image_lench = image_lench+pack_lench;
		if(image_lench >= ble_update_len) image_lench = 0;
	}
	
// #if 1
// 	u8 data_str[90] = {0};
// 	u16 i = 0;
// 	// for(i=0; i<lench; i++)
// 	// {
// 	// 	data_str[i*3] = hextochar((data_buf[i]>>4)&0x0F);
// 	// 	data_str[i*3+1] = hextochar(data_buf[i]&0x0F);
// 	// 	data_str[i*3+2] = 0x20;//空格
// 	// }
// 	// LPUART1_Send(data_str,lench*3);									//以字符串方式打印数据包
// 	debug_print("\r\n");
// #endif

	BT02_Send_Data(data_buf, lench);

	return 0;
}


