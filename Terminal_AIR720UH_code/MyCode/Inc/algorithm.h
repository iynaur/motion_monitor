#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include "main.h"
//#include "stm32_dsp.h"
//#include "table_fft.h"


#define algor_N	500
#define algor_M 25
#define algor_Q 20

#define optimi_N 1000
#define optimi_M 25
#define optimi_Q 40

#define PACKET_LENGTH   23
#define PACKET_QUAT     2

#define FFT_N 256
#define MATH_M 25
#define MATH_Q 40
#define MATH_N 1000


//extern long ay_fft_in[FFT_N];
//extern long ay_fft_out[FFT_N];
//extern long mag_arry[FFT_N/2]; 

extern float gx_in[MATH_N];
extern float gy_in[MATH_N];
extern float gz_in[MATH_N];
extern float ax_in[algor_N];
extern float ay_in[algor_N];
extern float az_in[algor_N];

float find_min(float a,float b);
float FFT(long* arry_out, long* arry_in, u16 Nbin);
float MATH(float* grrx_in, float* grry_in,float* grrz_in);
float optimi(float* grrx_in, float* grry_in,float* grrz_in);
float algor(float* arrx_in, float* arry_in,float* arrz_in);
float add_algor(float* add_arrx_in, float* add_arry_in,float* add_arrz_in);
float find_motion(float* Pitch,float* Yaw);

typedef enum
{
    STATE_NONE = 0x00,
    STATE_MOTIONLESS,
    STATE_STANDBY,
    STATE_MOTION,
    STATE_DORMANT,
    STATE_DISMOUNT,
    STATE_ERROR
}MACHINE_STATUS;
#endif
