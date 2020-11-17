
#ifndef __LOW_POWER_H
#define __LOW_POWER_H

#include "main.h"

typedef enum
{
	HIGH = 0,
	LOW  = 1
}POLARITY;


void SYSCLKConfig_STOP(void);
void Entry_Low_Power(uint32_t LowPowerMode);
void LowPower_EnableWakeUpPin(uint32_t WakeUpPin, POLARITY polarity);
void Stop2_delay_ms(u16 nms);
void Stop2_delay_s(u16 ns);
void Shutdown_delay_ms(u16 nms);
void Shutdown_delay_s(u16 ns);


#endif


