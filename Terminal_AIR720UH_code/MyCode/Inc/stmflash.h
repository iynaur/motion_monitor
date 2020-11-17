#ifndef __STMFLASH_H
#define __STMFLASH_H

#include "main.h"

//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 	256 	 			//��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN 	1              		//ʹ��FLASHд��(0��������;1��ʹ��)
//////////////////////////////////////////////////////////////////////////////////////////////////////

//FLASH��ʼ��ַ
#define STM32_FLASH_BASE 0x08000000 			//STM32 FLASH����ʼ��ַ


//bootloader��ʼ��ַ  ��С16K
#define STMFLASH_IAP_START_ADDR STM32_FLASH_BASE
#define STMFLASH_IAP_MAX_LENTH 16*1024 //0X4000

#if STM32_FLASH_SIZE==128
//������ʼ��ַ   ��С48K
#define STMFLASH_APP_START_ADDR STMFLASH_IAP_START_ADDR+STMFLASH_IAP_MAX_LENTH  //0X08004000
#define STMFLASH_APP_MAX_LENTH 48*1024 //0xC000

//�������򻺴���ʼ��ַ   ��С48K
#define STMFLASH_UPDATE_DATA_ADDR STMFLASH_APP_START_ADDR+STMFLASH_APP_MAX_LENTH //0X08010000
#define STMFLASH_UPDATE_DATA_MAX_LENTH 48*1024 //0xC000


//δ�������ݰ������ʼ��ַ
#define STMFLASH_UNSEND_DATA_ADDR STMFLASH_UPDATE_DATA_ADDR+STMFLASH_UPDATE_DATA_MAX_LENTH  //0X0801C000
#define UNSEND_DATA_MAX_NUM 224 //ÿ64���ֽڴ��һ��δ�ɹ����͵����ݰ�,���224


//id�洢��ַ ռ4���ֽ�
#define STMFLASH_ID_ADDR 0x0801F800 //63ҳ 0X0801F800
#define STMFLASH_ID_LENTH 4

#define STMFLASH_BLE_INIT_FLAG_ADDR 0x0801F808
#define STMFLASH_BLE_INIT_FLAG_LENTH 2

#define STMFLASH_MACBAND_FLAG_ADDR 0x0801F810
#define STMFLASH_MACBAND_FLAG_LENTH 2

#define STMFLASH_UNSEND_DATA_PHEAD_ADDR 0x0801F818 
#define STMFLASH_UNSEND_DATA_PTAIL_ADDR 0x0801F820
#define STMFLASH_UNSEND_DATA_INIT_FLAG_ADDR 0x0801F828

//δ�������ݰ�������ʼ��ַ 2K ֻ��2���ֽ�
#define STMFLASH_UNSEND_DATA_NUM_ADDR 0x0801F830

//�洢������������ȵ���ʼ��ַ 2K ֻ��4���ֽ�
#define STMFLASH_UPDATE_DATA_LENTH_ADDR 0x0801F838

//�洢������־λ����ʼ��ַ  2K ֻ��4���ֽ�
#define STMFLASH_UPDATE_FLAG_ADDR 0x0801F840

#elif STM32_FLASH_SIZE==256
//������ʼ��ַ   ��С96K
#define STMFLASH_APP_START_ADDR STMFLASH_IAP_START_ADDR+STMFLASH_IAP_MAX_LENTH  //0X08004000
#define STMFLASH_APP_MAX_LENTH 96*1024 //0x18000

//�������򻺴���ʼ��ַ   ��С96K
#define STMFLASH_UPDATE_DATA_ADDR STMFLASH_APP_START_ADDR+STMFLASH_APP_MAX_LENTH //0X0801C000
#define STMFLASH_UPDATE_DATA_MAX_LENTH 96*1024 //0x18000


//δ�������ݰ������ʼ��ַ
#define STMFLASH_UNSEND_DATA_ADDR STMFLASH_UPDATE_DATA_ADDR+STMFLASH_UPDATE_DATA_MAX_LENTH  //0X08034000
#define UNSEND_DATA_MAX_NUM 736 //ÿ64���ֽڴ��һ��δ�ɹ����͵����ݰ�,���736


//id�洢��ַ ռ4���ֽ�
#define STMFLASH_ID_ADDR 0x0803F800 //127ҳ 0X0803F800
#define STMFLASH_ID_LENTH 4

#define STMFLASH_BLE_INIT_FLAG_ADDR 0x0803F808
#define STMFLASH_BLE_INIT_FLAG_LENTH 2

#define STMFLASH_MACBAND_FLAG_ADDR 0x0803F810
#define STMFLASH_MACBAND_FLAG_LENTH 2

#define STMFLASH_UNSEND_DATA_PHEAD_ADDR 0x0803F818 
#define STMFLASH_UNSEND_DATA_PTAIL_ADDR 0x0803F820
#define STMFLASH_UNSEND_DATA_INIT_FLAG_ADDR 0x0803F828

//δ�������ݰ�������ʼ��ַ 2K ֻ��2���ֽ�
#define STMFLASH_UNSEND_DATA_NUM_ADDR 0x0803F830

//�洢������������ȵ���ʼ��ַ 2K ֻ��4���ֽ�
#define STMFLASH_UPDATE_DATA_LENTH_ADDR 0x0803F838

//�洢������־λ����ʼ��ַ  2K ֻ��4���ֽ�
#define STMFLASH_UPDATE_FLAG_ADDR 0x0803F840

#endif

//FLASH������ֵ
#define FLASH_KEY1               0x45670123U
#define FLASH_KEY2               0xCDEF89ABU

#define FLASH_OPTKEY1             0x08192A3BU                          /*!< Flash option byte key1 */
#define FLASH_OPTKEY2             0x4C5D6E7FU                          /*!< Flash option byte key2: used with FLASH_OPTKEY1
                                                                            to allow option bytes operations */

u8 STMFLASH_Unlock(void);					  	//FLASH����
u8 STMFLASH_Lock(void);					  	//FLASH����
u8 STMFLASH_OB_Unlock(void);
u8 STMFLASH_OB_Lock(void);
u8 STMFLASH_OB_Launch(void);
u8 STMFLASH_GetStatus(void);				  	//���״̬
u8 STMFLASH_WaitDone(u16 time);				  	//�ȴ���������
u8 STMFLASH_ErasePage(u32 addr);			  	//����ҳ
u8 STMFLASH_ReadByte(u32 faddr);		  	//��������  
void STMFLASH_Write(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite);		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read(u32 ReadAddr,u8 *pBuffer,u16 NumToRead);   		//��ָ����ַ��ʼ����ָ�����ȵ�����
void STMFLASH_Write_NoCheck(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite);   

//����д��
void Test_Write(u32 WriteAddr,u8 WriteData);								   
#endif



