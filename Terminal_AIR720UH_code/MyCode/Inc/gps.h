#ifndef __GPS_H
#define __GPS_H

#include "main.h"


#define GPS_POWER_ON()  {LL_GPIO_SetOutputPin(GPS_PWR_CTL_GPIO_Port, GPS_PWR_CTL_Pin);}
#define GPS_POWER_OFF() {LL_GPIO_ResetOutputPin(GPS_PWR_CTL_GPIO_Port, GPS_PWR_CTL_Pin);}

#pragma pack(1)

//GPS NMEA-0183协议重要参数结构体定义 
//卫星信息
typedef struct  
{										    
 	u8 num;		//卫星编号
	u8 eledeg;	//卫星仰角
	u16 azideg;	//卫星方位角
	u8 sn;		//信噪比		   
}nmea_slmsg;  

//UTC时间信息
typedef struct  
{										    
 	u16 year;	//年份
	u8 month;	//月份
	u8 date;	//日期
	u8 hour; 	//小时
	u8 min; 	//分钟
	u8 sec; 	//秒钟
}nmea_utc_time;   	   

//NMEA 0183 协议解析后数据存放结构体
typedef struct  
{										    
 	u8 svnum;					//可见卫星数
	nmea_slmsg slmsg[12];		//最多12颗卫星
	nmea_utc_time utc;			//UTC时间
	u32 latitude;				//纬度 分扩大100000倍,实际要除以100000
	u8 nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	u32 longitude;			    //经度 分扩大100000倍,实际要除以100000
	u8 ewhemi;					//东经/西经,E:东经;W:西经
	u8 gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
 	u8 posslnum;				//用于定位的卫星数,0~12.
 	u8 possl[12];				//用于定位的卫星编号
	u8 fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
	u16 pdop;					//位置精度因子 0~500,对应实际值0~50.0
	u16 hdop;					//水平精度因子 0~500,对应实际值0~50.0
	u16 vdop;					//垂直精度因子 0~500,对应实际值0~50.0 

	int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	u32 speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时	 
}nmea_msg; 

//本地时间结构体
typedef struct  
{										    
 	vu16 year;	//年份
	vu8 month;	//月份
	vu8 date;	//日期
	vu8 week; //星期
	vu8 hour; 	//小时
	vu8 min; 	//分钟
	vu8 sec; 	//秒钟
}local_time_t;   

#pragma pack( )

extern nmea_msg gps_msg;
extern local_time_t BJTime;


int NMEA_Str2num(u8 *buf,u8*dx);
void GPS_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,u8 *buf);

void UTCTime_to_LocalTime(nmea_msg *gpsx, local_time_t *localtime, char timezone);

#endif


