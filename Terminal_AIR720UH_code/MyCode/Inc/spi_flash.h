#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H			    
#include "main.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//Mini STM32������
//W25X16 ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/6/13 
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	 
#define W25Q80 	 0XEF13 	
#define W25Q16 	 0XEF14
#define W25Q32 	 0XEF15
#define W25Q64 	 0XEF16
#define W25Q128  0XEF17
//�������ܴ�С��192kB
//ռ�ÿռ䣺Block0~Block3 �� 256KB
#define W25_ADDR1(BLOCK,SECTOR) (0x010000 * BLOCK + 0x001000 * SECTOR)       //������Χ��0-15
#define W25_ADDR2(SECTOR)       (0x001000 * SECTOR)                          //������Χ��0-255
 

extern u16 SPI_FLASH_TYPE;		//��������ʹ�õ�flashоƬ�ͺ�		

#define SET_FLASH_CS() LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12)
#define RESET_FLASH_CS() LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12)
////////////////////////////////////////////////////////////////////////////

extern u8 SPI_FLASH_BUF[4096];
//W25X16��д
#define FLASH_ID 0XEF17
//ָ���
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

//flash������Ϣ
/*
1.W25Q128 �ܴ�С��16Mb
2.��Ϊ256��block��ÿ��block��С64kb
3.ÿ��block��Ϊ16��Sector��ÿ��Sector��С4kb,��4096��Sector
4.flashִ��д����֮ǰ��Ҫ�Ȳ�����������SectorΪ��λ��ÿ�β���һ��Sector
flash�������ã�
      0ҳ~  59ҳ(��240kb)Ϊ�������������ɷ��������͹���������������bootloader��
	 ���˶γ��򿽱���STM32L431����APP��������
            60ҳ(��4kb)Ϊ����һ���Ƿ���������ı�־λ����APP�������յ���������������
	 �󣬽�����������дΪ0x55��bootloader��ͨ���жϴ�ҳ������ֵ�����Ƿ�������
	 ���ڳ��������ɺ󣬽���ҳ���в�����
	          61ҳ��ǰ2���ֽ�(���4000��)����δ���͵����ݰ��ĸ�������Ϊ0������δ���͵�
	 ���ݰ�
	   62ҳ~ 171ҳ(440kb)����δ���͵����ݰ���
	 �������ݴ洢ʱ��ÿ100���ֽڱ���һ֡����(56byte)��ÿҳ�ɱ���40֡���ݣ�����100ҳ��Ԥ��10ҳ
	         172ҳ(4kb)���ֽڴ洢����������֡�ܳ��ȣ���λΪ512�ֽ�
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
u16  SPI_Flash_ReadID(void);  	    //��ȡFLASH ID
u8	 SPI_Flash_ReadSR(void);        //��ȡ״̬�Ĵ��� 
void SPI_FLASH_Write_SR(u8 sr);  	//д״̬�Ĵ���
void SPI_FLASH_Write_Enable(void);  //дʹ�� 
void SPI_FLASH_Write_Disable(void);	//д����
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //��ȡflash
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//д��flash
void SPI_Flash_Erase_Chip(void);    	  //��Ƭ����
void SPI_Flash_Erase_Sector(u32 Dst_Addr);//��������
void SPI_Flash_Wait_Busy(void);           //�ȴ�����
void SPI_Flash_PowerDown(void);           //�������ģʽ
void SPI_Flash_WAKEUP(void);			  //����

void SPI_Flash_Write_Sector(u8* pBuffer,u32 Sector_num);
void SPI_Flash_Read_Sector(u8* pBuffer,u32 Sector_num);

u8 SPI_ReadWriteByte(u8 TxData);


#endif

