#include "logic_app.h"



#include "algorithm.h"
#define debug_print printf
void main(void)
{	

		Send_Machine_Data();//算法运算；发送状态数据

	
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

        if(status_count >= 233)
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
            if(sleep_motionless_count >= 233)
			{
				sleep_motionless_count = 0;
				sleep_flag = 1;
				//change_count = SLEEP_SEND_TIME;
			}

			if(sleep_flag)
			{
				sleep_count++;
				//debug_print("sleep_count is %d.\r\n", sleep_count);
                if(sleep_count == 233)
				{
					sleep_count = 0;
					
                    STATE_DORMANT;
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
                status_new;
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
	
	
//	ICM20689_Get_INT_Status(&int_state);
	if(int_state == 1)
	{

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


