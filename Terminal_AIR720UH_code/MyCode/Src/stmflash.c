#include "stmflash.h"
#include "usart.h"
#include "delay.h"
#include "myusart.h"
//解锁STM32的FLASH
u8 STMFLASH_Unlock(void)
{
	FLASH->KEYR=FLASH_KEY1;//写入解锁序列.
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

//flash上锁
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


//得到FLASH状态
u8 STMFLASH_GetStatus(void)
{	
	if(READ_BIT(FLASH->SR, FLASH_SR_BSY))return 1;		    //忙
	else if(READ_BIT(FLASH->SR, FLASH_SR_PROGERR))return 2;//编程错误
	else if(READ_BIT(FLASH->SR, FLASH_SR_WRPERR))return 3;//写保护错误
	return 0;						//操作完成
}

//等待操作完成
//time:要延时的长短
//返回值:状态.
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;//非忙,无需等待了,直接退出.
		//delay_us(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}


//擦除页
//addr:地址
//返回值:执行情况
u8 STMFLASH_ErasePage(u32 addr)
{
	u8 res=0;
	uint32_t paddr = 0;                       //页码
	paddr = (addr - 0x8000000) / 0x800;     //提取页码
	
	res=STMFLASH_WaitDone(0XFFFF);//等待上次操作结束,>20ms    
	if(res==0)
	{ 
		MODIFY_REG(FLASH->CR, FLASH_CR_PNB, (paddr << FLASH_CR_PNB_Pos));
		SET_BIT(FLASH->CR, FLASH_CR_PER);
		SET_BIT(FLASH->CR, FLASH_CR_STRT);		  
		res=STMFLASH_WaitDone(0XFFFF);//等待操作结束,>20ms  
		if(res!=1)//非忙
		{
			CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));//清除页擦除标志.
		}
	}
	return res;
}


//读取指定地址的8位数据
//faddr:读地址 
//返回值:对应数据.
u8 STMFLASH_ReadByte(u32 faddr)
{
	return *(vu8*)faddr; 
}


//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:字节(8位)数   
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
	if(j != 8)//长度不是8的整倍数，也按8的整倍数写入，多余的填0xFF
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
		SET_BIT(FLASH->CR, FLASH_CR_PG);//编程使能
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
	res=STMFLASH_WaitDone(0XFFFF);//等待操作完成
	//debug_print("flash wait done res id %d.\r\n", res);
	if(res!=1)//操作成功
	{
		CLEAR_BIT(FLASH->CR, FLASH_CR_PG);//清除PG位.
	}
} 


//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:字节(8位)数(就是要写入的8位数据的个数.)
//#if STM32_FLASH_SIZE<256
//#define STM_SECTOR_SIZE 1024 //字节
//#else 
#define STM_SECTOR_SIZE	2048
//#endif		 
u8 STMFLASH_BUF[STM_SECTOR_SIZE];//最多是2K字节
void STMFLASH_Write(u32 WriteAddr,u8 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(8位字计算)
	u16 secremain; //扇区内剩余地址(8位字计算)	   
 	u16 i;    
	u32 offaddr;   //去掉0X08000000后的地址
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//非法地址
	
	if(STMFLASH_Unlock()==0)				//解锁
		printf("Unlock success!\r\n");
	else
		printf("Unlock failed!\r\n");

	CLEAR_BIT(FLASH->SR,FLASH_SR_OPTVERR);
	
	offaddr=WriteAddr-STM32_FLASH_BASE;		//实际偏移地址.
	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址 0~63
	secoff=offaddr%STM_SECTOR_SIZE;		//在扇区内的偏移
	secremain=STM_SECTOR_SIZE-secoff;		//扇区剩余空间大小   
	if(NumToWrite<=secremain)secremain=NumToWrite;//不大于该扇区范围
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE);//读出整个扇区的内容

		STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
		for(i=0;i<secremain;i++)//复制
		{
			STMFLASH_BUF[i+secoff]=pBuffer[i];
			//printf("The NO.%d data is %2x.\r\n",i+secoff, pBuffer[i]);
			//printf("pBuffer addr is 0x%x.\r\n",pBuffer);
		}
		
		STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE);//写入整个扇区  
		
#if 0
		for(i=0;i<secremain;i++)//校验数据
		{
			//printf("The NO.%d data is %2x.\r\n",i+secoff, STMFLASH_BUF[i+secoff]);
			if(STMFLASH_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			//printf("Need to ErasePage!\r\n");
			STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
			for(i=0;i<secremain;i++)//复制
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];
				//printf("The NO.%d data is %2x.\r\n",i+secoff, pBuffer[i]);
				//printf("pBuffer addr is 0x%x.\r\n",pBuffer);
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE);//写入整个扇区  
		}
		else
		{
			//printf("Don't need erase.\r\n");
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		}
#endif

		if(NumToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
		  pBuffer+=secremain;  	//指针偏移
			WriteAddr+=secremain;	//写地址偏移	   
		  NumToWrite-=secremain;	//字节数递减
			if(NumToWrite>(STM_SECTOR_SIZE))secremain=STM_SECTOR_SIZE;//下一个扇区还是写不完
			else secremain=NumToWrite;//下一个扇区可以写完了
		}	 
	};	
	STMFLASH_Lock();//上锁
}


//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:字节(8位)数
void STMFLASH_Read(u32 ReadAddr,u8 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadByte(ReadAddr);//读取1个字节.
		ReadAddr+=1;//偏移1个字节.	
	}
}



//////////////////////////////////////////测试用///////////////////////////////////////////
//WriteAddr:起始地址
//WriteData:要写入的数据
void Test_Write(u32 WriteAddr,u8 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//写入一个字 
}

