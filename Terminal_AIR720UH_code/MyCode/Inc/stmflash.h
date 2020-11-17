#ifndef __STMFLASH_H
#define __STMFLASH_H

#include "main.h"

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 	256 	 			//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 	1              		//使能FLASH写入(0，不是能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 			//STM32 FLASH的起始地址


//bootloader起始地址  大小16K
#define STMFLASH_IAP_START_ADDR STM32_FLASH_BASE
#define STMFLASH_IAP_MAX_LENTH 16*1024 //0X4000

#if STM32_FLASH_SIZE==128
//程序起始地址   大小48K
#define STMFLASH_APP_START_ADDR STMFLASH_IAP_START_ADDR+STMFLASH_IAP_MAX_LENTH  //0X08004000
#define STMFLASH_APP_MAX_LENTH 48*1024 //0xC000

//升级程序缓存起始地址   大小48K
#define STMFLASH_UPDATE_DATA_ADDR STMFLASH_APP_START_ADDR+STMFLASH_APP_MAX_LENTH //0X08010000
#define STMFLASH_UPDATE_DATA_MAX_LENTH 48*1024 //0xC000


//未发送数据包存放起始地址
#define STMFLASH_UNSEND_DATA_ADDR STMFLASH_UPDATE_DATA_ADDR+STMFLASH_UPDATE_DATA_MAX_LENTH  //0X0801C000
#define UNSEND_DATA_MAX_NUM 224 //每64个字节存放一个未成功发送的数据包,最多224


//id存储地址 占4个字节
#define STMFLASH_ID_ADDR 0x0801F800 //63页 0X0801F800
#define STMFLASH_ID_LENTH 4

#define STMFLASH_BLE_INIT_FLAG_ADDR 0x0801F808
#define STMFLASH_BLE_INIT_FLAG_LENTH 2

#define STMFLASH_MACBAND_FLAG_ADDR 0x0801F810
#define STMFLASH_MACBAND_FLAG_LENTH 2

#define STMFLASH_UNSEND_DATA_PHEAD_ADDR 0x0801F818 
#define STMFLASH_UNSEND_DATA_PTAIL_ADDR 0x0801F820
#define STMFLASH_UNSEND_DATA_INIT_FLAG_ADDR 0x0801F828

//未发送数据包数量起始地址 2K 只用2个字节
#define STMFLASH_UNSEND_DATA_NUM_ADDR 0x0801F830

//存储升级程序包长度的起始地址 2K 只用4个字节
#define STMFLASH_UPDATE_DATA_LENTH_ADDR 0x0801F838

//存储升级标志位的起始地址  2K 只用4个字节
#define STMFLASH_UPDATE_FLAG_ADDR 0x0801F840

#elif STM32_FLASH_SIZE==256
//程序起始地址   大小96K
#define STMFLASH_APP_START_ADDR STMFLASH_IAP_START_ADDR+STMFLASH_IAP_MAX_LENTH  //0X08004000
#define STMFLASH_APP_MAX_LENTH 96*1024 //0x18000

//升级程序缓存起始地址   大小96K
#define STMFLASH_UPDATE_DATA_ADDR STMFLASH_APP_START_ADDR+STMFLASH_APP_MAX_LENTH //0X0801C000
#define STMFLASH_UPDATE_DATA_MAX_LENTH 96*1024 //0x18000


//未发送数据包存放起始地址
#define STMFLASH_UNSEND_DATA_ADDR STMFLASH_UPDATE_DATA_ADDR+STMFLASH_UPDATE_DATA_MAX_LENTH  //0X08034000
#define UNSEND_DATA_MAX_NUM 736 //每64个字节存放一个未成功发送的数据包,最多736


//id存储地址 占4个字节
#define STMFLASH_ID_ADDR 0x0803F800 //127页 0X0803F800
#define STMFLASH_ID_LENTH 4

#define STMFLASH_BLE_INIT_FLAG_ADDR 0x0803F808
#define STMFLASH_BLE_INIT_FLAG_LENTH 2

#define STMFLASH_MACBAND_FLAG_ADDR 0x0803F810
#define STMFLASH_MACBAND_FLAG_LENTH 2

#define STMFLASH_UNSEND_DATA_PHEAD_ADDR 0x0803F818 
#define STMFLASH_UNSEND_DATA_PTAIL_ADDR 0x0803F820
#define STMFLASH_UNSEND_DATA_INIT_FLAG_ADDR 0x0803F828

//未发送数据包数量起始地址 2K 只用2个字节
#define STMFLASH_UNSEND_DATA_NUM_ADDR 0x0803F830

//存储升级程序包长度的起始地址 2K 只用4个字节
#define STMFLASH_UPDATE_DATA_LENTH_ADDR 0x0803F838

//存储升级标志位的起始地址  2K 只用4个字节
#define STMFLASH_UPDATE_FLAG_ADDR 0x0803F840

#endif

//FLASH解锁键值
#define FLASH_KEY1               0x45670123U
#define FLASH_KEY2               0xCDEF89ABU

#define FLASH_OPTKEY1             0x08192A3BU                          /*!< Flash option byte key1 */
#define FLASH_OPTKEY2             0x4C5D6E7FU                          /*!< Flash option byte key2: used with FLASH_OPTKEY1
                                                                            to allow option bytes operations */

u8 STMFLASH_Unlock(void);					  	//FLASH解锁
u8 STMFLASH_Lock(void);					  	//FLASH上锁
u8 STMFLASH_OB_Unlock(void);
u8 STMFLASH_OB_Lock(void);
u8 STMFLASH_OB_Launch(void);
u8 STMFLASH_GetStatus(void);				  	//获得状态
u8 STMFLASH_WaitDone(u16 time);				  	//等待操作结束
u8 STMFLASH_ErasePage(u32 addr);			  	//擦除页
u8 STMFLASH_ReadByte(u32 faddr);		  	//读出半字  
void STMFLASH_Write(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u8 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据
void STMFLASH_Write_NoCheck(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite);   

//测试写入
void Test_Write(u32 WriteAddr,u8 WriteData);								   
#endif



