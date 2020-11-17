#include "spi_flash.h" 
#include "spi.h"
#include "delay.h"   
//Mini STM32������
//W25X16 �������� 
//����ԭ��@ALIENTEK
//2010/6/13
//V1.0

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
*/



u16 SPI_FLASH_TYPE=W25Q128;//Ĭ�Ͼ���25Q128
//4KbytesΪһ��Sector
//16������Ϊ1��Block
//W25X16
//����Ϊ2M�ֽ�,����32��Block,512��Sector 

//��ʼ��SPI FLASH��IO��
void SPI_Flash_Init(void)
{

	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  	/* Peripheral clock enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);

	GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 	LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_12);
	MX_SPI2_Init();//��ʼ��SPI
	LL_SPI_SetBaudRatePrescaler(SPI1,LL_SPI_BAUDRATEPRESCALER_DIV8);
	LL_SPI_SetRxFIFOThreshold(SPI1, LL_SPI_RX_FIFO_TH_QUARTER);
	LL_SPI_Enable(SPI2);//ʹ��SPI1
	LL_SPI_EnableIT_RXNE(SPI2);
	LL_SPI_EnableIT_TXE(SPI2);
	SPI_ReadWriteByte(0xff);//��������
	SPI_FLASH_TYPE=SPI_Flash_ReadID();//��ȡFLASH ID.
}  

//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;   
	//SPI_FLASH_CS=0;                            //ʹ������
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_ReadStatusReg);    //���Ͷ�ȡ״̬�Ĵ�������    
	byte=SPI_ReadWriteByte(0Xff);             //��ȡһ���ֽ�  
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ
	SET_FLASH_CS();
	return byte;   
} 
//дSPI_FLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void SPI_FLASH_Write_SR(u8 sr)   
{   
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_WriteStatusReg);   //����дȡ״̬�Ĵ�������    
	SPI_ReadWriteByte(sr);               //д��һ���ֽ�  
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
}   
//SPI_FLASHдʹ��	
//��WEL��λ   
void SPI_FLASH_Write_Enable(void)   
{
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
  SPI_ReadWriteByte(W25X_WriteEnable);      //����дʹ��  
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
} 
//SPI_FLASHд��ֹ	
//��WEL����  
void SPI_FLASH_Write_Disable(void)   
{  
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
  SPI_ReadWriteByte(W25X_WriteDisable);     //����д��ָֹ��    
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
} 			    
//��ȡоƬID W25X16��ID:0XEF14
u16 SPI_Flash_ReadID(void)
{
	u16 Temp = 0;	  
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(0x90);//���Ͷ�ȡID����	    
	SPI_ReadWriteByte(0x00); 	    
	SPI_ReadWriteByte(0x00); 	    
	SPI_ReadWriteByte(0x00); 	 			   
	Temp|=SPI_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI_ReadWriteByte(0xFF);	 
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
	return Temp;
}   		    
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//pBuffer:���ݴ洢��
//ReadAddr:��ʼ��ȡ�ĵ�ַ(24bit)
//NumByteToRead:Ҫ��ȡ���ֽ���(���65535)
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_ReadData);         //���Ͷ�ȡ����   
	SPI_ReadWriteByte((u8)((ReadAddr)>>16));  //����24bit��ַ    
	SPI_ReadWriteByte((u8)((ReadAddr)>>8));   
	SPI_ReadWriteByte((u8)ReadAddr);   
	for(i=0;i<NumByteToRead;i++)
	{ 
		pBuffer[i]=SPI_ReadWriteByte(0XFF);   //ѭ������  
	}
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
}  
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
  SPI_FLASH_Write_Enable();                  //SET WEL 
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_PageProgram);      //����дҳ����   
	SPI_ReadWriteByte((u8)((WriteAddr)>>16)); //����24bit��ַ    
	SPI_ReadWriteByte((u8)((WriteAddr)>>8));   
	SPI_ReadWriteByte((u8)WriteAddr);   
	for(i=0;i<NumByteToWrite;i++)
	{
		SPI_ReadWriteByte(pBuffer[i]);//ѭ��д��  
	}
//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
	SPI_Flash_Wait_Busy();					   //�ȴ�д�����
} 
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //��ҳʣ����ֽ���		 	    
	if(NumByteToWrite<=pageremain)
	{
		pageremain=NumByteToWrite;//������256���ֽ�
	}
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)
		{
			break;//д�������
		}
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(NumByteToWrite>256)
			{
				pageremain=256; //һ�ο���д��256���ֽ�
			}
			else
			{
				pageremain=NumByteToWrite; 	  //����256���ֽ���
			}
		}
	}    
} 
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)  		   
u8 SPI_FLASH_BUF[4096];
void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    

	secpos=WriteAddr/4096;//������ַ 0~511 for w25x16
	secoff=WriteAddr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   

	if(NumByteToWrite<=secremain)
	{
		secremain=NumByteToWrite;//������4096���ֽ�
	}
	while(1) 
	{	
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//������������������
		
		for(i=0;i<secremain;i++)//У������
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)
			{
				break;//��Ҫ����  	  
			}
		}
		
		if(i<secremain)//��Ҫ����
		{
			SPI_Flash_Erase_Sector(secpos);//�����������
			for(i=0;i<secremain;i++)	   //����
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//д����������  

		}
		else
		{
			SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 			
		}	 
		
		if(NumByteToWrite==secremain)
		{
			break;//д�������
		}
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		  pBuffer+=secremain;  //ָ��ƫ��
			WriteAddr+=secremain;//д��ַƫ��	   
		  NumByteToWrite-=secremain;				//�ֽ����ݼ�
			if(NumByteToWrite>4096)
			{
				secremain=4096;	//��һ����������д����
			}
			else
			{
				secremain=NumByteToWrite;			//��һ����������д����
			}
		}	 
	} 	 
}
//��������оƬ
//��Ƭ����ʱ��:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//�ȴ�ʱ�䳬��...
void SPI_Flash_Erase_Chip(void)   
{                                             
	SPI_FLASH_Write_Enable();                  //SET WEL 
	SPI_Flash_Wait_Busy();   
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_ChipErase);        //����Ƭ��������  
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
	SPI_Flash_Wait_Busy();   				   //�ȴ�оƬ��������
}   
//����һ������
//Dst_Addr:������ַ 0~511 for w25x16
//����һ��ɽ��������ʱ��:150ms
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{   
	Dst_Addr*=4096;
	SPI_FLASH_Write_Enable();                  //SET WEL 	 
	SPI_Flash_Wait_Busy();   
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_SectorErase);      //������������ָ�� 
	SPI_ReadWriteByte((u8)((Dst_Addr)>>16));  //����24bit��ַ    
	SPI_ReadWriteByte((u8)((Dst_Addr)>>8));   
	SPI_ReadWriteByte((u8)Dst_Addr);  
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
	SPI_Flash_Wait_Busy();   				   //�ȴ��������
} 

//дһ������
//���������д�뻺���� PBuffer,��С4096
//���������������ַ����Χ��0-511
void SPI_Flash_Write_Sector(u8* pBuffer,u32 Sector_num)
{ 
  u32 WriteAddr = 0;
  WriteAddr = Sector_num*4096;
	SPI_Flash_Erase_Sector(Sector_num);
	SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,4096);//д���������� 
}

//дһ������
//���������д�뻺���� PBuffer,��С4096
//���������������ַ����Χ��0-511
void SPI_Flash_Read_Sector(u8* pBuffer,u32 Sector_num)   
{ 
 	u16 i;   
  u32 ReadAddr = 0;	
	ReadAddr = Sector_num * 4096;   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_ReadData);         //���Ͷ�ȡ����   
	SPI_ReadWriteByte((u8)((ReadAddr)>>16));  //����24bit��ַ    
	SPI_ReadWriteByte((u8)((ReadAddr)>>8));   
	SPI_ReadWriteByte((u8)ReadAddr);   
  for(i = 0;i < 4096;i++)
	{ 
		pBuffer[i]=SPI_ReadWriteByte(0XFF);   //ѭ������  
  }                       
	SET_FLASH_CS();                              //ȡ��Ƭѡ 
}






//�ȴ�����
void SPI_Flash_Wait_Busy(void)   
{   
	while ((SPI_Flash_ReadSR()&0x01)==0x01);   // �ȴ�BUSYλ���
}  
//�������ģʽ
void SPI_Flash_PowerDown(void)   
{ 
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_PowerDown);        //���͵�������  
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
	delay_us(3);                               //�ȴ�TPD  
}   
//����
void SPI_Flash_WAKEUP(void)   
{  
	//SPI_FLASH_CS=0;                            //ʹ������   
	RESET_FLASH_CS();
	SPI_ReadWriteByte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB    
	//SPI_FLASH_CS=1;                            //ȡ��Ƭѡ 
	SET_FLASH_CS();
	delay_us(3);                               //�ȴ�TRES1
}   


//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while(LL_SPI_IsActiveFlag_TXE(SPI2) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}			  
	LL_SPI_TransmitData8(SPI2, TxData); //ͨ������SPIx����һ������
	retry=0;

	while(LL_SPI_IsActiveFlag_RXNE(SPI2) == RESET)//���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		retry++;
		if(retry>200)return 0;
	}	  						    
	return LL_SPI_ReceiveData8(SPI2); //����ͨ��SPIx������յ�����					    
}





