#include "algorithm.h"
//#include "mpu6050.h"


//long ay_fft_in[FFT_N] = {0};
//long ay_fft_out[FFT_N] = {0};
//long mag_arry[FFT_N/2] = {0}; 

float gx_in[MATH_N] = {0};
float gy_in[MATH_N] = {0};
float gz_in[MATH_N] = {0};
float ax_in[algor_N] = {0};
float ay_in[algor_N] = {0};
float az_in[algor_N] = {0};


float find_min(float a,float b)
{
	if(a<b)
		return a;
	else 
		return b;
}

#if 0
float  FFT(long* arry_out, long* arry_in, u16 Nbin)
{
	//u16 i = 0;
	float fhz = 0.00f;
	short lx, ly;
	float x, y;
	float mag;
	u32 max;
	short i = 0, k = 0;
	
	if(Nbin==1024)
	{
		cr4_fft_1024_stm32(arry_out, arry_in, Nbin);
	}
	else if(Nbin==256)
	{
		cr4_fft_256_stm32(arry_out, arry_in, Nbin);
	}

	for(i = 0; i<(FFT_N/2); i++)
	{
		lx = (ay_fft_out[i] << 16) >> 16;
		ly = (ay_fft_out[i] >> 16);

		x = FFT_N * ((float)lx) / 32768;
		y = FFT_N * ((float)ly) / 32768;
		
		mag = sqrt(x*x + y*y)/FFT_N;

		if(i==0)
			mag_arry[i] = (u32)(mag * 32768);
		else
			mag_arry[i] = (u32)(mag * 65536);
		if(i==1)
		{
			max = mag_arry[i];
		}
		else if(i>1)
		{
			if(mag_arry[i] > max)
			{
				max = mag_arry[i];
				k = i;
			}
		}
	}

	if(k==0) k = 1;
	fhz = (float)(k*MPU6050_HZ)/(float)(FFT_N);
	k = 0;
	return fhz;
} 
#endif


float  MATH(float* grrx_in, float* grry_in,float* grrz_in )
{  
	
	float gx[MATH_Q]={0};
	float gy[MATH_Q]={0};
	float gz[MATH_Q]={0};
	float gxmid[MATH_Q]={0};
	float gymid[MATH_Q]={0};
	float gzmid[MATH_Q]={0};
	float g[MATH_Q]={0};
	short i = 0,j = 0, k = 0;
	float tmp=0;
	
	//�����ݷ���
	for(k=0;k<MATH_Q;k++)
	{
		for(i=MATH_M*k;i<MATH_M*(k+1);i++) 		//��һ��0~25
		{
			gx[k]+= grrx_in[i];
			gy[k]+= grry_in[i];
			gz[k]+= grrz_in[i];
		}
	}

	
	//��ÿ���ƽ��,�����д�ŵ��Ǿ�ֵ
	for(i=0;i<MATH_Q;i++)
	{
	 	gx[i]/=MATH_M;
	 	gy[i]/=MATH_M;
	 	gz[i]/=MATH_M;
		//printf("%d gx:%0.2f\t,gy:%0.2f\t,gz:%0.2f\r\n",i,gx[i],gy[i],gz[i] );
	}


	//�����ÿһλ��ȥ��ֵ�������ŵ�ֵ�����ı�
	for(k=0;k<MATH_Q;k++)
	{
 		for(i=MATH_M*k;i<MATH_M*(k+1);i++)				//��һ��
 		{
		 	grrx_in[i]-=gx[k];
			grry_in[i]-=gy[k];
			grrz_in[i]-=gz[k];
 		}
	}

	//�����鰴��С�������������Ӧ��ֵ�Ѿ��ı�
	for(k=0;k<MATH_Q;k++)
	{
		for(i=0;i<MATH_M-1;i++)      //�Ƚ�MATH_M-1��  
		{
			for(j=MATH_M*k;j<MATH_M*(k+1)-i-1;j++)		//��һ��
			{
				if(grrx_in[j]>grrx_in[j+1])
				{
					tmp=grrx_in[j+1];
					grrx_in[j+1]=grrx_in[j];
					grrx_in[j]=tmp;
				}
				if(grry_in[j]>grry_in[j+1])
				{
					tmp=grry_in[j+1];
					grry_in[j+1]=grry_in[j];
					grry_in[j]=tmp;
				}
				if(grrz_in[j]>grrz_in[j+1])
				{
					tmp=grrz_in[j+1];
					grrz_in[j+1]=grrz_in[j];
					grrz_in[j]=tmp;
				}
			}
		}
	}
	
	//ȡ��λ��
	for(i=0;i<MATH_Q;i++)
	{ 
		gxmid[i]=fabs(grrx_in[12+i*25]);
		gymid[i]=fabs(grry_in[12+i*25]);
		gzmid[i]=fabs(grrz_in[12+i*25]);
		//printf("%d gxmid_1 is %f\t	gymid_1 is %f\t gzmid_1 is % f \r\n", i,gxmid[i],gymid[i],gzmid[i]);
	}
	//printf(" gxmid_1 is %0.2f\t gymid_1 is %0.2f\t gzmid_1 is %0.2f \r\n",gxmid_1,gymid_1,gzmid_1);
	//printf(" gxmid_1 is %0.2f\t gymid_1 is %0.2f \r\n",gxmid_1,gymid_1);
	
	for(i=0;i<MATH_Q;i++)
	{
		g[i]=gxmid[i]+gymid[i]+gzmid[i];
		//printf("%d	gis %f\t\r\n",i,g[i]);
	}
	//printf(" g1 is %0.2f\t	g2 is %0.2f\t g3 is %0.2f\t g4 is %0.2f\t g5 is %0.2f\r\n",g1,g2,g3,g4,g5);
	
	for(i=0;i<MATH_Q-1;i++) 		 
	{
		for(j=0;j<MATH_Q-i-1;j++)
		{
			if(g[j]>g[j+1])
			{
				tmp=g[j+1];
				g[j+1]=g[j];
				g[j]=tmp;
			}
		}
	}
	//for(i=0;i<MATH_Q;i++)
	//{
	//printf("no %d g is%f\r\n",i,g[i]);
	//}
	//printf("%f\r\n",g[29]*0.75+g[30]*0.25);
	return (g[29]*0.75f+g[30]*0.25f);
}



float algor(float* arrx_in, float* arry_in,float* arrz_in)
{
	
//	float ax[algor_Q]={0};
//	float ay[algor_Q]={0};
//	float az[algor_Q]={0};
	float axmid[algor_Q]={0};
	float aymid[algor_Q]={0};
	float azmid[algor_Q]={0};
	float a[algor_Q]={0};
	short i = 0,j = 0,k=0;
	float tmp=0;  

//�ú�һ����ȥǰһ��ȡ��ֵ�������ŵ�ֵ�Ѿ��ı�

	for(k=0;k<algor_Q;k++)									//��K�������ѭ��
	{
		for(i=algor_M*k;i<algor_M*(k+1)-1;i++)					
		{
			arrx_in[i]=arrx_in[i+1]-arrx_in[i];
			arry_in[i]=arry_in[i+1]-arry_in[i];
			arrz_in[i]=arrz_in[i+1]-arrz_in[i];
		}
		
	}


	
	//�����鰴��С�������������Ӧ��ֵ�Ѿ��ı�
	for(k=0;k<algor_Q;k++)
	{
		for(i=0;i<algor_M-2;i++) 	 //�Ƚ�MATH_M-2��
		{
			for(j=algor_M*k;j<algor_M*(k+1)-i-2;j++)
			{
			  if(arrx_in[j]>arrx_in[j+1])
				{
					tmp=arrx_in[j+1];
					arrx_in[j+1]=arrx_in[j];
					arrx_in[j]=tmp;
				}
			  if(arry_in[j]>arry_in[j+1])
				{
					tmp=arry_in[j+1];
					arry_in[j+1]=arry_in[j];
					arry_in[j]=tmp;
				}
			if(arrz_in[j]>arrz_in[j+1])
			{
				tmp=arrz_in[j+1];
				arrz_in[j+1]=arrz_in[j];
				arrz_in[j]=tmp;
			}
			}
		}
	}
	for(i=0;i<algor_Q;i++)
	{	
		axmid[i]=fabs(arrx_in[11+i*25]*0.5f+arrx_in[12+i*25]*0.5f);
		aymid[i]=fabs(arry_in[11+i*25]*0.5f+arry_in[12+i*25]*0.5f);
		azmid[i]=fabs(arrz_in[11+i*25]*0.5f+arrz_in[12+i*25]*0.5f);
	}

	for(i=0;i<algor_Q;i++)
	{
		a[i]=axmid[i]+aymid[i]+azmid[i];
	}
	
	for(i=0;i<algor_Q-1;i++) 	 //�Ƚ�49��
	{
		for(j=0;j<algor_Q-i-1;j++)
		{
			if(a[j]>a[j+1])
			{
				tmp=a[j+1];
				a[j+1]=a[j];
				a[j]=tmp;
			}
		}
	}
	return (a[14]*0.75f+a[15]*0.25f);
}




float add_algor(float* add_arrx_in, float* add_arry_in,float* add_arrz_in)
{

	float add_axmid[optimi_Q]={0};
	float add_aymid[optimi_Q]={0};
	float add_azmid[optimi_Q]={0};
	float add_a[optimi_Q]={0};
	short i = 0,j = 0,k=0;
	float tmp=0; 
	
	//����25��С���ڣ�����ʮ�����ݽ�������ÿ�����ݴ�С��������
	for(k=0;k<optimi_Q;k++)
	{
	  for(i=0;i<optimi_M-1;i++) 	 
		{
		  for(j=optimi_M*k;j<optimi_M*(k+1)-i-1;j++)
			{
			  if(add_arrx_in[j]>add_arrx_in[j+1])
				{
					tmp=add_arrx_in[j+1];
					add_arrx_in[j+1]=add_arrx_in[j];
					add_arrx_in[j]=tmp;
				}
			  if(add_arry_in[j]>add_arry_in[j+1])
				{
					tmp=add_arry_in[j+1];
					add_arry_in[j+1]=add_arry_in[j];
					add_arry_in[j]=tmp;
				}
			  if(add_arrz_in[j]>add_arrz_in[j+1])
				{
					tmp=add_arrz_in[j+1];
					add_arrz_in[j+1]=add_arrz_in[j];
					add_arrz_in[j]=tmp;
				}
			}		
		}
	}
		
//		for(i=0;i<optimi_M;i++)
//		{
//			
//		printf("%d ax_1 is %f\t	ay_1 is %f\t az_1 is % f \r\n", i,add_arrx_in[i],add_arry_in[i],add_arrz_in[i]);
//			
//		}

	for(i=0;i<optimi_Q;i++)
		{	
		
		add_axmid[i]=add_arrx_in[12+i*25];
		add_aymid[i]=add_arry_in[12+i*25];
		add_azmid[i]=add_arrz_in[12+i*25];
	//	printf("%d axmid_1 is %f\t	aymid_1 is %f\t azmid_1 is % f \r\n", i,add_axmid[i],add_aymid[i],add_azmid[i]);	
			
		}
		
		
//	for(i=0;i<optimi_Q;i++)
//	{
//		printf("%d\t%f\t%f\r\n",i,add_aymid[i],add_azmid[i]);
//	}
	for(i=0;i<optimi_Q-1;i++)
		{
		
		add_axmid[i]=fabs(add_axmid[i+1]-add_axmid[i]);
		add_aymid[i]=fabs(add_aymid[i+1]-add_aymid[i]);
		add_azmid[i]=fabs(add_azmid[i+1]-add_azmid[i]);
	//	printf("%d\t%f\t%f\r\n",i,add_aymid[i],add_azmid[i]);	

		}
		
	
	for(i=0;i<optimi_Q-1;i++)
		{

		add_a[i]=add_axmid[i]+add_aymid[i]+add_azmid[i];

		}
	
//	for(i=0;i<optimi_Q-1;i++)
//	{
//		printf("%d\t%f\r\n",i,add_a[i]);
//	}
	for(i=0;i<optimi_Q-2;i++)	 
		{
		for(j=0;j<optimi_Q-i-2;j++)
			{
			if(add_a[j]>add_a[j+1])
				{
					tmp=add_a[j+1];
					add_a[j+1]=add_a[j];
					add_a[j]=tmp;
	
				}
			}
		}
//	for(i=0;i<optimi_Q-1;i++)
//	{
//		printf("%d\t%f\r\n",i,add_axmid[i]);
//	}
//	printf("%f\r\n",add_a[19]);
	return (add_a[19]);
}


float optimi(float* grrx_in, float* grry_in,float* grrz_in )
{
	
	float gx[optimi_Q]={0};
	float gy[optimi_Q]={0};
	float gz[optimi_Q]={0};
	float gxmid[optimi_Q]={0};
	float gymid[optimi_Q]={0};
	float gzmid[optimi_Q]={0};
	float g[optimi_Q]={0};
	short i = 0,j = 0,k=0;
	float tmp=0;
	

//����25��С���ڣ�����ʮ�����ݽ�������ÿ�����ݴ�С��������
	for(k=0;k<optimi_Q;k++)
	{		
		for(i=0;i<optimi_M-1;i++)      //�Ƚ�optimi_M-1��  
		{
		  for(j=optimi_M*k;j<optimi_M*(k+1)-i-1;j++)		//��һ��
			{
			  if(grrx_in[j]>grrx_in[j+1])
				{
					tmp=grrx_in[j+1];
					grrx_in[j+1]=grrx_in[j];
					grrx_in[j]=tmp;
				}
			  if(grry_in[j]>grry_in[j+1])
				{
					tmp=grry_in[j+1];
					grry_in[j+1]=grry_in[j];
					grry_in[j]=tmp;
				}
			  if(grrz_in[j]>grrz_in[j+1])
				{
					tmp=grrz_in[j+1];
					grrz_in[j+1]=grrz_in[j];
					grrz_in[j]=tmp;
				}
		
			}
		}
		}
	
//ȡ25��С���ڵ���λ������40��
		for(i=0;i<optimi_Q;i++)
		{
			gxmid[i]=grrx_in[12+i*25];
			gymid[i]=grry_in[12+i*25];
			gzmid[i]=grrz_in[12+i*25];
		}

//��ÿ������ȥ��λ������ʮ�飬ԭ�����ֵ�Ѿ��ı�
	for(k=0;k<optimi_Q;k++)
	{
		for(i=optimi_M*k;i<optimi_M*(k+1);i++)			   //��һ��
		{
			grrx_in[i]-=gxmid[k];
			grry_in[i]-=gymid[k];
			grrz_in[i]-=gzmid[k];
		}
	}
			
	//ȡ����ֵ
	for(i=0;i<optimi_M*40;i++)
	{	
		grrx_in[i]=fabs(grrx_in[i]);
		grry_in[i]=fabs(grry_in[i]);
		grrz_in[i]=fabs(grrz_in[i]);

	}
//��ÿ��С���ڵľ�ֵ����������ڹ���ʮ������gx,gy,gz������
	for(k=0;k<optimi_Q;k++)
	{			
		for(i=optimi_M*k;i<optimi_M*(k+1);i++)			
		{
			gx[k]+= grrx_in[i];
			gy[k]+= grry_in[i];
			gz[k]+= grrz_in[i];
		}	
	}				
			
			//ȡ��ֵ
	for(i=0;i<optimi_Q;i++)
	{
	gx[i]/=optimi_M;
	gy[i]/=optimi_M;
	gz[i]/=optimi_M;
//	 printf("%d gx:%0.2f\t,gy:%0.2f\t,gz:%0.2f\r\n",i,gx[i],gy[i],gz[i] );
	}
//ȡ0.75��λ����ȥ0.25��λ����Ϊ������ֵ
			
 	for(i=0;i<optimi_Q;i++)
	{
		g[i]=gx[i]+gy[i]+gz[i];
//	printf("%d  gis %f\t\r\n",i,g[i]);
	}
//�����ó���40����ֵ����
	 for(i=0;i<optimi_Q-1;i++)      
	{
	  for(j=0;j<optimi_Q-i-1;j++)
		{
		  if(g[j]>g[j+1])
			{
				tmp=g[j+1];
				g[j+1]=g[j];
				g[j]=tmp;
			}
		}
	}

//������0.75��λ����ȥ0.25��λ��
// printf("%f\r\n",g[29]*0.75+g[30]*0.25-g[9]*0.25-g[10]*0.75);
	//return(g[29]*0.75f+g[30]*0.25f-g[9]*0.25f-g[10]*0.75f);
	return(g[29]*0.75f+g[30]*0.25f-g[0]);
		
}

/*
�������ܣ����մ�����ٶȺͼ��ٶȵ������㷨�Ը����Ǻͺ������������ж���仯����
���仯���Ƚϴ����ж���Ϊ�˶�������Ϊ����״̬���жϲ�����data_process�У�
*/
//float find_motion(float* Pitch,float* Yaw)
//{
//	short i = 0,j = 0,k = 0;
//	float Pitch_mid[optimi_Q] = {0},Yaw_mid[optimi_Q] = {0};
//	float Pitch_average[optimi_Q] = {0},Yaw_average[optimi_Q] = {0};
//	float data1 = 0,data2[optimi_Q] = {0};
//  //25������Ϊһ��С���ڣ�����ʮ�����ݽ�������ÿ�����ݴ�С��������	
//	for(k = 0;k < optimi_Q;k++)
//	{		
//		for(i = 0;i < (optimi_M-2);i++)      //�Ƚ�optimi_M-1��  
//		{
//		  for(j = (optimi_M*k);j < (optimi_M * (k + 1) - i - 1);j++)		//��һ��
//			{
//				if(Pitch[j] > Pitch[j+1])
//				{
//					data1=Pitch[j+1];
//					Pitch[j+1]=Pitch[j];
//					Pitch[j]=data1;
//				}
//				if(Yaw[j] > Yaw[j+1])
//				{
//					data1=Yaw[j+1];
//					Yaw[j+1]=Yaw[j];
//					Yaw[j]=data1;
//				}
//			}
//		}
//	}
//	
//	//ȡÿ��С���ڵ���λ������40��
//	for(i = 0;i < optimi_Q;i++)
//	{
//		Pitch_mid[i] = Pitch[12+i*25];
//		Yaw_mid[i] = Yaw[12+i*25];
//	}

//  //��ÿ������ȥ��λ������ʮ�飬ԭ�����ֵ�Ѿ��ı�
//	for(k = 0;k < optimi_Q;k++)
//	{
//		for(i = (optimi_M * k);i < (optimi_M * (k + 1));i++)			   //��һ��
//		{
//			Pitch[i] -= Pitch_mid[k];
//			Yaw[i] -= Yaw_mid[k];
//		}
//	}
//			
//	//ȡ����ֵ
//	for(i=0;i<optimi_M*40;i++)
//	{	
//		Pitch[i] = fabs(Pitch[i]);
//		Yaw[i] = fabs(Yaw[i]);

//	}
//  //���㾭���������ÿ�����ڵľ�ֵ
//	for(k=0;k<optimi_Q;k++)
//	{			
//		for(i=optimi_M*k;i<optimi_M*(k+1);i++)			
//		{
//			Pitch_average[k] += Pitch[i];
//			Yaw_average[k] += Yaw[i];
//		}	
//	}				
//			
//			//ȡ��ֵ
//	for(i = 0;i < optimi_Q;i++)
//	{
//	  Pitch_average[i] /= optimi_M;
//	  Yaw_average[i] /= optimi_M;
//		data2[i] = Pitch_average[i] + Yaw_average[i];
//	}
//	
//  //�����ó���40����ֵ����
//	for(i = 0;i < (optimi_Q - 1);i++)      
//	{
//		for(j = 0;j < (optimi_Q - i - 1);j++)
//		{
//			if(data2[j] > data2[j+1])
//			{
//				data1 = data2[j + 1];
//				data2[j + 1] = data2[j];
//				data2[j] = data1;
//			}
//		}
//	}

//  //������ս��
//  return(data2[39] * 0.75f + data2[38] * 0.25f - data2[0]);
//}

