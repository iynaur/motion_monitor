
#include "mytim.h"
#include "tim.h"



//arr
//10:1ms
//100:10ms
//1000:100ms
//10000:1s
//200000:20s
void TIM2_Init(u32 arr,u16 psc)
{
	MX_TIM2_Init();
	LL_TIM_SetPrescaler(TIM2, psc);
	LL_TIM_SetAutoReload(TIM2,arr);
	LL_TIM_EnableIT_UPDATE(TIM2);
	LL_TIM_EnableCounter(TIM2);
}


