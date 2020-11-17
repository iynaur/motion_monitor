#include "stmflash.h"
#include "usart.h"
#include "delay.h"
#include "myusart.h"
//����STM32��FLASH
u8 STMFLASH_Unlock(void)
{
	FLASH->KEYR=FLASH_KEY1;//д���������.
	FLASH->KEYR=FLASH_KEY2;
	u8 status = 0;
	if(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
  {
    /* Authorize the FLASH Registers access */
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);

    /* Verify Flash is unlocked */
    if(READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U)
    {
      status = 1;
    }
  }
  return status;

}

//flash����
u8 STMFLASH_Lock(void)
{
	SET_BIT(FLASH->CR, FLASH_CR_LOCK);
	return 0;
}


/**
  * @brief  Unlock the FLASH Option Bytes Registers access.
  * @retval Status
  */
u8 STMFLASH_OB_Unlock(void)
{
	FLASH->OPTKEYR = FLASH_OPTKEY1;
	FLASH->OPTKEYR = FLASH_OPTKEY2;
	u8 status = 0;
  if(READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) != 0U)
  {
    /* Authorizes the Option Byte register programming */
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY1);
    WRITE_REG(FLASH->OPTKEYR, FLASH_OPTKEY2);
  }
  else
  {
    status = 1;
  }

  return status;
}

/**
  * @brief  Lock the FLASH Option Bytes Registers access.
  * @retval Status
  */
u8 STMFLASH_OB_Lock(void)
{
  /* Set the OPTLOCK Bit to lock the FLASH Option Byte Registers access */
  SET_BIT(FLASH->CR, FLASH_CR_OPTLOCK);

  return 0;
}


u8 STMFLASH_OB_Launch(void)
{
  /* Set the bit to force the option byte reloading */
  SET_BIT(FLASH->CR, FLASH_CR_OBL_LAUNCH);

  /* Wait for last operation to be completed */
  return(STMFLASH_WaitDone(0XFFFF));
}


//�õ�FLASH״̬
u8 STMFLASH_GetStatus(void)
{	
	if(READ_BIT(FLASH->SR, FLASH_SR_BSY))return 1;		    //æ
	else if(READ_BIT(FLASH->SR, FLASH_SR_PROGERR))return 2;//��̴���
	else if(READ_BIT(FLASH->SR, FLASH_SR_WRPERR))return 3;//д��������
	return 0;						//�������
}

//�ȴ��������
//time:Ҫ��ʱ�ĳ���
//����ֵ:״̬.
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;//��æ,����ȴ���,ֱ���˳�.
		//delay_us(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}


//����ҳ
//addr:��ַ
//����ֵ:ִ�����
u8 STMFLASH_ErasePage(u32 addr)
{
	u8 res=0;
	uint32_t paddr = 0;                       //ҳ��
	paddr = (addr - 0x8000000) / 0x800;     //��ȡҳ��
	
	res=STMFLASH_WaitDone(0XFFFF);//�ȴ��ϴβ�������,>20ms    
	if(res==0)
	{ 
		MODIFY_REG(FLASH->CR, FLASH_CR_PNB, (paddr << FLASH_CR_PNB_Pos));
		SET_BIT(FLASH->CR, FLASH_CR_PER);
		SET_BIT(FLASH->CR, FLASH_CR_STRT);		  
		res=STMFLASH_WaitDone(0XFFFF);//�ȴ���������,>20ms  
		if(res!=1)//��æ
		{
			CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));//���ҳ������־.
		}
	}
	return res;
}


//��ȡָ����ַ��8λ����
//faddr:����ַ 
//����ֵ:��Ӧ����.
u8 STMFLASH_ReadByte(u32 faddr)
{
	return *(vu8*)faddr; 
}


//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:�ֽ�(8λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite)   
{
	u16 res, i, j, double_world_num;
	u16 write_len = NumToWrite;
	u8 *data_p = NULL;
	//u8 k = 0;
	j = 8 - NumToWrite%8;
	data_p = pBuffer;
	//printf("pBuffer is 0x%x.\r\n", data_p);
	//for(i=0;i<NumToWrite;i++)
	//{
	//	printf("pBuffer[%d] is 0x%x.\r\n", i,pBuffer[i]);
	//}
	if(j != 8)//���Ȳ���8����������Ҳ��8��������д�룬�������0xFF
	{
		write_len = NumToWrite + j;
		for(i=0; i<j; i++)
		{
			data_p[NumToWrite+i] = 0xFF;
		}
	}
	double_world_num = write_len / 8;
	res=STMFLASH_WaitDone(0XFFFF);
	//debug_print("flash wait done res id %d.\r\n", res);
	if(res==0)//OK
	{
		printf("Begin programing!\r\n");
		SET_BIT(FLASH->CR, FLASH_CR_PG);//���ʹ��
		for(i=0;i<double_world_num;i++)
		{ 
				*(vu32*)(WriteAddr + 8*i) = (data_p[i*8+3]  << 24)
																	+ (data_p[i*8+2]  << 16)
																	+ (data_p[i*8+1]  << 8)
																	+ (data_p[i*8]);
				__ISB();
				
				*(vu32*)(WriteAddr + 8*i + 4U) = (data_p[i*8+7]  << 24)
																			 + (data_p[i*8+6]  << 16)
																			 + (data_p[i*8+5]  << 8)
																			 + (data_p[i*8+4]);
				//printf("0x%x is 0x%8x.\r\n",WriteAddr + 8*i,*(u32*)(WriteAddr + 8*i));
		}
	}
	res=STMFLASH_WaitDone(0XFFFF);//�ȴ��������
	//debug_print("flash wait done res id %d.\r\n", res);
	if(res!=1)//�����ɹ�
	{
		CLEAR_BIT(FLASH->CR, FLASH_CR_PG);//���PGλ.
	}
} 


//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:�ֽ�(8λ)��(����Ҫд���8λ���ݵĸ���.)
//#if STM32_FLASH_SIZE<256
//#define STM_SECTOR_SIZE 1024 //�ֽ�
//#else 
#define STM_SECTOR_SIZE	2048
//#endif		 
u8 STMFLASH_BUF[STM_SECTOR_SIZE];//�����2K�ֽ�
void STMFLASH_Write(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(8λ�ּ���)
	u16 secremain; //������ʣ���ַ(8λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ
	
	if(STMFLASH_Unlock()==0)				//����
		printf("Unlock success!\r\n");
	else
		printf("Unlock failed!\r\n");

	CLEAR_BIT(FLASH->SR,FLASH_SR_OPTVERR);
	
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ 0~63
	secoff=offaddr%STM_SECTOR_SIZE;		//�������ڵ�ƫ��
	secremain=STM_SECTOR_SIZE-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE);//������������������

		STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
		for(i=0;i<secremain;i++)//����
		{
			STMFLASH_BUF[i+secoff]=pBuffer[i];
			//printf("The NO.%d data is %2x.\r\n",i+secoff, pBuffer[i]);
			//printf("pBuffer addr is 0x%x.\r\n",pBuffer);
		}
		
		STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE);//д����������  
		
#if 0
		for(i=0;i<secremain;i++)//У������
		{
			//printf("The NO.%d data is %2x.\r\n",i+secoff, STMFLASH_BUF[i+secoff]);
			if(STMFLASH_BUF[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			//printf("Need to ErasePage!\r\n");
			STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];
				//printf("The NO.%d data is %2x.\r\n",i+secoff, pBuffer[i]);
				//printf("pBuffer addr is 0x%x.\r\n",pBuffer);
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE);//д����������  
		}
		else
		{
			//printf("Don't need erase.\r\n");
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		}
#endif

		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		  pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain;	//д��ַƫ��	   
		  NumToWrite-=secremain;	//�ֽ����ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE))secremain=STM_SECTOR_SIZE;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	STMFLASH_Lock();//����
}


//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:�ֽ�(8λ)��
void STMFLASH_Read(u32 ReadAddr,u8 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadByte(ReadAddr);//��ȡ1���ֽ�.
		ReadAddr+=1;//ƫ��1���ֽ�.	
	}
}



//////////////////////////////////////////������///////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(u32 WriteAddr,u8 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}

