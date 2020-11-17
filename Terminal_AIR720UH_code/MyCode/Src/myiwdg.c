#include "myiwdg.h"
#include "stmflash.h"
#include "myusart.h"

//用户看门狗初始化函数
//time = (prer/32) * rlr 单位ms
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


//喂狗函数，定时调用
void IWDG_Feed(void)
{   
 	LL_IWDG_ReloadCounter(IWDG);										   
}


/*
函数功能：在standby模式下看门狗停止计数，防止其触发复位
将FLASH_OPTR寄存器中IWDG_STDBY位置0即可
*/
void Freeze_IWDG_Instdby(void)
{
	uint8_t flag_IWDG;
	flag_IWDG = (READ_BIT(FLASH->OPTR, FLASH_OPTR_IWDG_STDBY) != 0);//设置旗标，避免反复加载flash导致重启
	if(flag_IWDG != 0)
	{
		STMFLASH_Unlock();
		STMFLASH_OB_Unlock();//解锁FLASH

		CLEAR_BIT(FLASH->OPTR, FLASH_OPTR_IWDG_STDBY);//修改看门狗参数为：休眠时看门狗停止计数
		/* Set OPTSTRT Bit */
		SET_BIT(FLASH->CR, FLASH_CR_OPTSTRT);//以下3句不可缺少，否则无法修改FLASH->OPTR
		/* Wait for last operation to be completed */
		STMFLASH_WaitDone(0XFFFF);//等待上次操作结束,>20ms    
		/* If the option byte program operation is completed, disable the OPTSTRT Bit */
		CLEAR_BIT(FLASH->CR, FLASH_CR_OPTSTRT);
		STMFLASH_OB_Launch();//加载flash，会导致重启
	}
	else
	{
		debug_print("Have seted OPTR-IWDG_STDBY.\r\n");
	}
	STMFLASH_OB_Lock();
	STMFLASH_Lock();//修改完后关闭flash，已经上锁再执行一遍问题不大
}


