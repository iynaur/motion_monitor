#include "myiwdg.h"
#include "stmflash.h"
#include "myusart.h"

//�û����Ź���ʼ������
//time = (prer/32) * rlr ��λms
void IWDG_Init(uint32_t prer, uint32_t rlr)
{
	Freeze_IWDG_Instdby();
  LL_IWDG_Enable(IWDG);
  LL_IWDG_EnableWriteAccess(IWDG);
  LL_IWDG_SetPrescaler(IWDG, prer);
  LL_IWDG_SetReloadCounter(IWDG, rlr);
  while (LL_IWDG_IsReady(IWDG) != 1)
  {
  }
	LL_IWDG_SetWindow(IWDG, 4095);
  LL_IWDG_ReloadCounter(IWDG);
}


//ι����������ʱ����
void IWDG_Feed(void)
{   
 	LL_IWDG_ReloadCounter(IWDG);										   
}


/*
�������ܣ���standbyģʽ�¿��Ź�ֹͣ��������ֹ�䴥����λ
��FLASH_OPTR�Ĵ�����IWDG_STDBYλ��0����
*/
void Freeze_IWDG_Instdby(void)
{
	uint8_t flag_IWDG;
	flag_IWDG = (READ_BIT(FLASH->OPTR, FLASH_OPTR_IWDG_STDBY) != 0);//������꣬���ⷴ������flash��������
	if(flag_IWDG != 0)
	{
		STMFLASH_Unlock();
		STMFLASH_OB_Unlock();//����FLASH

		CLEAR_BIT(FLASH->OPTR, FLASH_OPTR_IWDG_STDBY);//�޸Ŀ��Ź�����Ϊ������ʱ���Ź�ֹͣ����
		/* Set OPTSTRT Bit */
		SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);//����3�䲻��ȱ�٣������޷��޸�FLASH->OPTR
		/* Wait for last operation to be completed */
		STMFLASH_WaitDone(0XFFFF);//�ȴ��ϴβ�������,>20ms    
		/* If the option byte program operation is completed, disable the OPTSTRT Bit */
		CLEAR_BIT(FLASH->CR, FLASH_CR_OPTSTRT);
		STMFLASH_OB_Launch();//����flash���ᵼ������
	}
	else
	{
		debug_print("Have seted OPTR-IWDG_STDBY.\r\n");
	}
	STMFLASH_OB_Lock();
	STMFLASH_Lock();//�޸����ر�flash���Ѿ�������ִ��һ�����ⲻ��
}


