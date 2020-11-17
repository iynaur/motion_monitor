#ifndef __COMMU_H
#define __COMMU_H

#include "main.h"


#define BLE_TYPE_ID 100
#define BLE_COMMU_DATA_LENTH 100
#define BLE_CMDACK_DATA_LENTH 100
extern vu8 bt02_usart1_commu_data_buf[BLE_COMMU_DATA_LENTH];
extern vu8 bt02_usart1_CMDack_data_buf[BLE_CMDACK_DATA_LENTH];

#define GPS_DATA_LENTH 1000
extern vu8 gps_usart2_data_buf[GPS_DATA_LENTH];

#define DEVICE_TYPE_ID 0x01
#define AIR720_COMMU_DATA_LENTH 500
#define AIR720_ATACK_DATA_LENTH 500
extern vu8 air720_usart3_commu_data_buf[AIR720_COMMU_DATA_LENTH];
extern vu8 air720_usart3_ATack_data_buf[AIR720_ATACK_DATA_LENTH];

extern u32 device_id;
extern char Version[10];
extern float pitch,roll,yaw; 		//欧拉角
extern short temp;


//协议包中的升级数据,主机和从机升级协议数据相同
typedef struct
{
	u8 master_or_slave;
	u8 update_ver1;
	u8 update_ver2;
	u8 update_ver3;
	u8 pack_count;
	u8 pack_num;
	u8 update_data[256];
	u8 xor_check;
}update_msg_t, *update_msg_p;


/******BLE相关宏定义以及结构体 begin******/
#define BLE_DATA_HEAD1 0x55
#define BLE_DATA_HEAD2 0xAA

#define BLE_CHECK_TIME_ACK_LENTH 18
#define BLE_DATA_ACK_LENTH 12

typedef enum
{
	BLE_CMD_DATA = 0x09,
	BLE_CMD_DATA_ACK = 0x89,
	BLE_CMD_UPDATE = 0x82,
	BLE_CMD_UPDATE_ACK = 0x02,
	BLE_CMD_CHECK_TIME = 0x0A,
	BLE_CMD_CHECK_TIME_ACK = 0x8A
}BLE_CMD;

//油箱盖协议包中的油箱数据
typedef struct
{
	u8 height_h;
	u8 height_l;
	u8 temp_h;
	u8 temp_l;
	u8 oil_status;
	u8 oil_BAT_value;
	u8 oil_ver1;
	u8 oil_ver2;
	u8 oil_ver3;
	u8 xor_check;
}ble_oil_data_t, *ble_oil_data_p;

//油箱盖协议包中时间数据
typedef struct
{
	u8 year;
	u8 month;
	u8 date;
	u8 week;
	u8 hour;
	u8 min;
	u8 sec;
	u8 xor_check;
}ble_time_t, *ble_time_p;


typedef struct
{
	u8 data_ack;
	u8 xor_check;
}ble_ack_t;

//不同的指令数据包会有不同，但起始地址一样，共享内存，使用联合体定义
typedef union
{
	ble_oil_data_t ble_oil_data;
	update_msg_t ble_update_msg;
	ble_time_t ble_time;
	ble_ack_t ble_ack;
	u8 xor_check;
}ble_data_u;

//整个协议包数据
typedef struct
{
	u8 head1;
	u8 head2;
	u8 type_id;
	u8 id1;
	u8 id2;
	u8 id3;
	u8 data_num;
	u8 lenth_h;
	u8 lenth_l;
	u8 cmd;
	ble_data_u ble_data;
}ble_data_pack_t, *ble_data_pack_p;

extern ble_oil_data_t oil_data;
extern vu8 ble_connect_flag;
extern vu8 ble_updating_flag;
extern vu8 ble_update_flag;
extern vu32 ble_update_len;
extern vu8 ble_pack_count;
extern vu8 ble_pack_num;
extern vu32 wait_ble_update_time;
extern vu32 wait_ble_msg_time;

/******BLE相关宏定义以及结构体 end******/


/******socket网络通信相关宏定义以及结构体 begin******/
#define SOCKET_DATA_HEAD1 0x55
#define SOCKET_DATA_HEAD2 0xAA

#define SOCKET_CHECK_TIME_LENTH 11
#define SOCKET_UPDATE_ACK_LENTH 18
#define SOCKET_DATA_LENTH 69


#define MACAO_ONE_MINUTE 0
#if MACAO_ONE_MINUTE
#define CHANGE_STATE_TIME    2    //用于确定最终状态的结果次数，三次状态相同才判断为最终状态
#define SLEEP_STATE_COUNT    27
#define SLEEP_SEND_TIME      60    //休眠情况下发送数据次数间隔
#else
#define CHANGE_STATE_TIME    6    //用于确定最终状态的结果次数，状态相同才判断为最终状态
#define SLEEP_STATE_COUNT    9
#define SLEEP_SEND_TIME      20    //休眠情况下发送数据次数间隔
#endif


#define APP_UPDATE_FLAG 0xF7E755AA

typedef enum
{
	SOCKET_CMD_DATA = 0x01,
 	SOCKET_CMD_UPDATE_ACK = 0x02,
	SOCKET_CMD_CHECK_TIME = 0x03,
	SOCKET_CMD_DATA_ACK = 0x81,
	SOCKET_CMD_UPDATE_SERVER = 0x82,
	SOCKET_CMD_CHECK_TIME_ACK = 0x83
}SOCKET_CMD;

typedef enum
{
	IN_CHARGING = 0x01,
	FINISHED_CHARGING = 0x02,
	NO_CHARGING = 0x03
}CHARGE_STATUS;

//机械状态包
typedef struct
{
	u8 status;
	u8 s_year;
	u8 s_month;
	u8 s_day;
	u8 s_hour;
	u8 s_min;
	u8 s_sec;
}status_data_t, *status_data_p;

//协议中的正常上报的机械数据
typedef struct
{
	status_data_t status_data[3];
	u8 charge_status;
	u8 BAT_value;
	u8 temp_h;
	u8 temp_l;
	u8 pitch_h;
	u8 pitch_l;
	u8 yaw_h;
	u8 yaw_l;
	u8 roll_h;
	u8 roll_l;
	u8 ew_ns_flag;
	u8 longitude1;
	u8 longitude2;
	u8 longitude3;
	u8 longitude4;
	u8 latitude1;
	u8 latitude2;
	u8 latitude3;
	u8 latitude4;

	u8 oil_height_h;
	u8 oil_height_l;
	u8 oil_temp_h;
	u8 oil_temp_l;
	u8 oil_status;
	u8 oil_BAT_value;
	
	u8 master_ver1;
	u8 master_ver2;
	u8 master_ver3;
	u8 slave_ver1;
	u8 slave_ver2;
	u8 slave_ver3;
	u8 speed_h;
	u8 speed_l;
	u8 altitude_24b;
	u8 altitude_16b;
	u8 altitude_8b;
	u8 altitude_b;
	u8 xor_check;
}machine_data_t, *machine_data_p;


//协议包中时间数据
typedef struct
{
	u8 year;
	u8 month;
	u8 date;
	u8 week;
	u8 hour;
	u8 min;
	u8 sec;
	u8 xor_check;
}socket_time_t, *socket_time_p;

extern machine_data_t machine_data;
extern update_msg_t update_msg;

//不同的指令数据包会有不同，但起始地址一样，共享内存，使用联合体定义
typedef union
{
	machine_data_t socket_machine_data;
	socket_time_t socket_time;
	update_msg_t socket_update_msg;
	u8 xor_check;
}socket_data_u;

//整个协议包数据
typedef struct
{
	u8 head1;
	u8 head2;
	u8 type_id;
	u8 id1;
	u8 id2;
	u8 id3;
	u8 data_num;
	u8 lenth_h;
	u8 lenth_l;
	u8 cmd;
	socket_data_u socket_data;
}socket_data_pack_t, *socket_data_pack_p;
/******socket网络通信相关宏定义以及结构体 end******/

/******机械运动状态相关宏定义和变量 start*****/
typedef enum
{
	STATE_NONE = 0x00,
	STATE_MOTIONLESS,
	STATE_STANDBY,
	STATE_MOTION,
	STATE_DORMANT,
	STATE_DISMOUNT,
	STATE_ERROR
}MACHINE_STATUS;

extern vs32 feed_iwdg_time;
extern vu32 check_rtc_time;
extern vu32 gps_power_on_time;
extern vu32 wait_update_msg_time;
extern vu32 wait_next_update_msg_time;
extern vu8 update_flag;

/******机械运动状态相关宏定义和变量 end*****/


void USART1_data_record(u8 data, u8 flag);
void USART2_data_record(u8 data, u8 flag);
void USART3_data_record(u8 data, u8 flag);
u8 Check_XOR_sum(u8 *pMsg, u16 len);
u16 CRC16(u8 * pMsg, u16 Len);
u32 Get_Device_ID(void);
u8 hextochar(u8 data);


#endif

