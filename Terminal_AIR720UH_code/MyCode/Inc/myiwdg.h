#ifndef __MYIWDG_H
#define __MYIWDG_H

#include "main.h"

void IWDG_Init(uint32_t prer, uint32_t rlr);
void IWDG_Feed(void);
void Freeze_IWDG_Instdby(void);



#endif

