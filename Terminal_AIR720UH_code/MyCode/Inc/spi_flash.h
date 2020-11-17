#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H			    
#include "main.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//Mini STM32开发板
//W25X16 驱动函数	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/6/13 
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	 
#define W25Q80 	 0XEF13 	
#define W25Q16 	 0XEF14
#define W25Q32 	 0XEF15
#define W25Q64 	 0XEF16
#define W25Q128  0XEF17
//程序区总大小：192kB
//占用空间：Block0~Block3 共 256KB
#define W25_ADDR1(BLOCK,SECTOR) (0x010000 * BLOCK + 0x001000 * SECTOR)       //扇区范围：0-15
#define W25_ADDR2(SECTOR)       (0x001000 * SECTOR)                          //扇区范围：0-255
 

extern u16 SPI_FLASH_TYPE;		//定义我们使用的flash芯片型号		

#define SET_FLASH_CS() LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12)
#define RESET_FLASH_CS() LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12)
////////////////////////////////////////////////////////////////////////////

extern u8 SPI_FLASH_BUF[4096];
//W25X16读写
#define FLASH_ID 0XEF17
//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

//flash分区信息
/*
1.W25Q128 总大小：16Mb
2.分为256个block，每个block大小64kb
3.每个block分为16个Sector，每个Sector大小4kb,共4096个Sector
4.flash执行写操作之前需要先擦除，擦除以Sector为单位，每次擦除一个Sector
flash分区设置：
      0页~  59页(共240kb)为程序区，保存由服务器发送过来的升级程序，在bootloader中
	 将此段程序拷贝到STM32L431内置APP程序区；
            60页(共4kb)为保存一个是否进行升级的标志位，在APP区程序收到完整的升级程序
	 后，将此区的数据写为0x55，bootloader区通过判断此页数据数值决定是否升级，
	 并在程序更新完成后，将此页进行擦除；
	          61页的前2个字节(最大4000个)保存未发送的数据包的个数，若为0，则无未发送的
	 数据包
	   62页~ 171页(440kb)保存未发送的数据包。
	 进行数据存储时，每100个字节保存一帧数据(56byte)，每页可保存40帧数据，共需100页，预留10页
	         172页(4kb)首字节存储升级用数据帧总长度，单位为512字节
*/

#define SECTOR_APP_START           0
#define SECTOR_APP_LENGTH          60
#define SECTOR_UPDATE_FLAG         60
#define SECTOR_UPDATE_LENGTH       1
#define SECTOR_UPDATE_DATANUM      172
#define SECTOR_UPDATE_DATALENGTH   1
#define SECTOR_UNSEND_FLAG         61
#define SECTOR_UNSEND_LENGTH       1
#define SECTOR_DATA_START          62
#define SECTOR_DATA_LENGTH         110



void SPI_Flash_Init(void);
u16  SPI_Flash_ReadID(void);  	    //读取FLASH ID
u8	 SPI_Flash_ReadSR(void);        //读取状态寄存器 
void SPI_FLASH_Write_SR(u8 sr);  	//写状态寄存器
void SPI_FLASH_Write_Enable(void);  //写使能 
void SPI_FLASH_Write_Disable(void);	//写保护
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //读取flash
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//写入flash
void SPI_Flash_Erase_Chip(void);    	  //整片擦除
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//扇区擦除
void SPI_Flash_Wait_Busy(void);           //等待空闲
void SPI_Flash_PowerDown(void);           //进入掉电模式
void SPI_Flash_WAKEUP(void);			  //唤醒

void SPI_Flash_Write_Sector(u8* pBuffer,u32 Sector_num);
void SPI_Flash_Read_Sector(u8* pBuffer,u32 Sector_num);

u8 SPI_ReadWriteByte(u8 TxData);


#endif

