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
	
	//将数据分组
	for(k=0;k<MATH_Q;k++)
	{
		for(i=MATH_M*k;i<MATH_M*(k+1);i++) 		//第一组0~25
		{
			gx[k]+= grrx_in[i];
			gy[k]+= grry_in[i];
			gz[k]+= grrz_in[i];
		}
	}

	
	//求每组的平均,数组中存放的是均值
	for(i=0;i<MATH_Q;i++)
	{
	 	gx[i]/=MATH_M;
	 	gy[i]/=MATH_M;
	 	gz[i]/=MATH_M;
		//printf("%d gx:%0.2f\t,gy:%0.2f\t,gz:%0.2f\r\n",i,gx[i],gy[i],gz[i] );
	}


	//数组的每一位减去均值，数组存放的值发生改变
	for(k=0;k<MATH_Q;k++)
	{
 		for(i=MATH_M*k;i<MATH_M*(k+1);i++)				//第一组
 		{
		 	grrx_in[i]-=gx[k];
			grry_in[i]-=gy[k];
			grrz_in[i]-=gz[k];
 		}
	}

	//将数组按从小到大排序，数组对应的值已经改变
	for(k=0;k<MATH_Q;k++)
	{
		for(i=0;i<MATH_M-1;i++)      //比较MATH_M-1次  
		{
			for(j=MATH_M*k;j<MATH_M*(k+1)-i-1;j++)		//第一组
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
	
	//取中位数
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

//用后一个减去前一个取差值，数组存放的值已经改变

	for(k=0;k<algor_Q;k++)									//用K代替产生循环
	{
		for(i=algor_M*k;i<algor_M*(k+1)-1;i++)					
		{
			arrx_in[i]=arrx_in[i+1]-arrx_in[i];
			arry_in[i]=arry_in[i+1]-arry_in[i];
			arrz_in[i]=arrz_in[i+1]-arrz_in[i];
		}
		
	}


	
	//将数组按从小到大排序，数组对应的值已经改变
	for(k=0;k<algor_Q;k++)
	{
		for(i=0;i<algor_M-2;i++) 	 //比较MATH_M-2次
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
	
	for(i=0;i<algor_Q-1;i++) 	 //比较49次
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
	
	//对以25个小窗口，共四十组数据进行排序，每组数据从小到大排序
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
	

//对以25个小窗口，共四十组数据进行排序，每组数据从小到大排序
	for(k=0;k<optimi_Q;k++)
	{		
		for(i=0;i<optimi_M-1;i++)      //比较optimi_M-1次  
		{
		  for(j=optimi_M*k;j<optimi_M*(k+1)-i-1;j++)		//第一组
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
	
//取25个小窗口的中位数，共40个
		for(i=0;i<optimi_Q;i++)
		{
			gxmid[i]=grrx_in[12+i*25];
			gymid[i]=grry_in[12+i*25];
			gzmid[i]=grrz_in[12+i*25];
		}

//将每组数减去中位数共四十组，原数组的值已经改变
	for(k=0;k<optimi_Q;k++)
	{
		for(i=optimi_M*k;i<optimi_M*(k+1);i++)			   //第一组
		{
			grrx_in[i]-=gxmid[k];
			grry_in[i]-=gymid[k];
			grrz_in[i]-=gzmid[k];
		}
	}
			
	//取绝对值
	for(i=0;i<optimi_M*40;i++)
	{	
		grrx_in[i]=fabs(grrx_in[i]);
		grry_in[i]=fabs(grry_in[i]);
		grrz_in[i]=fabs(grrz_in[i]);

	}
//将每个小窗口的均值代表这个窗口共四十个放在gx,gy,gz数组中
	for(k=0;k<optimi_Q;k++)
	{			
		for(i=optimi_M*k;i<optimi_M*(k+1);i++)			
		{
			gx[k]+= grrx_in[i];
			gy[k]+= grry_in[i];
			gz[k]+= grrz_in[i];
		}	
	}				
			
			//取均值
	for(i=0;i<optimi_Q;i++)
	{
	gx[i]/=optimi_M;
	gy[i]/=optimi_M;
	gz[i]/=optimi_M;
//	 printf("%d gx:%0.2f\t,gy:%0.2f\t,gz:%0.2f\r\n",i,gx[i],gy[i],gz[i] );
	}
//取0.75中位数减去0.25中位数作为最后输出值
			
 	for(i=0;i<optimi_Q;i++)
	{
		g[i]=gx[i]+gy[i]+gz[i];
//	printf("%d  gis %f\t\r\n",i,g[i]);
	}
//给最后得出的40个均值排序
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

//最后输出0.75中位数减去0.25中位数
// printf("%f\r\n",g[29]*0.75+g[30]*0.25-g[9]*0.25-g[10]*0.75);
	//return(g[29]*0.75f+g[30]*0.25f-g[9]*0.25f-g[10]*0.75f);
	return(g[29]*0.75f+g[30]*0.25f-g[0]);
		
}

/*
函数功能：按照处理角速度和加速度的类似算法对俯仰角和航向角做处理后判断其变化幅度
若变化幅度较大，则判定其为运动，否则为怠速状态（判断部分在data_process中）
*/
//float find_motion(float* Pitch,float* Yaw)
//{
//	short i = 0,j = 0,k = 0;
//	float Pitch_mid[optimi_Q] = {0},Yaw_mid[optimi_Q] = {0};
//	float Pitch_average[optimi_Q] = {0},Yaw_average[optimi_Q] = {0};
//	float data1 = 0,data2[optimi_Q] = {0};
//  //25个数据为一个小窗口，共四十组数据进行排序，每组数据从小到大排序	
//	for(k = 0;k < optimi_Q;k++)
//	{		
//		for(i = 0;i < (optimi_M-2);i++)      //比较optimi_M-1次  
//		{
//		  for(j = (optimi_M*k);j < (optimi_M * (k + 1) - i - 1);j++)		//第一组
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
//	//取每个小窗口的中位数，共40个
//	for(i = 0;i < optimi_Q;i++)
//	{
//		Pitch_mid[i] = Pitch[12+i*25];
//		Yaw_mid[i] = Yaw[12+i*25];
//	}

//  //将每组数减去中位数共四十组，原数组的值已经改变
//	for(k = 0;k < optimi_Q;k++)
//	{
//		for(i = (optimi_M * k);i < (optimi_M * (k + 1));i++)			   //第一组
//		{
//			Pitch[i] -= Pitch_mid[k];
//			Yaw[i] -= Yaw_mid[k];
//		}
//	}
//			
//	//取绝对值
//	for(i=0;i<optimi_M*40;i++)
//	{	
//		Pitch[i] = fabs(Pitch[i]);
//		Yaw[i] = fabs(Yaw[i]);

//	}
//  //计算经上述处理后每个窗口的均值
//	for(k=0;k<optimi_Q;k++)
//	{			
//		for(i=optimi_M*k;i<optimi_M*(k+1);i++)			
//		{
//			Pitch_average[k] += Pitch[i];
//			Yaw_average[k] += Yaw[i];
//		}	
//	}				
//			
//			//取均值
//	for(i = 0;i < optimi_Q;i++)
//	{
//	  Pitch_average[i] /= optimi_M;
//	  Yaw_average[i] /= optimi_M;
//		data2[i] = Pitch_average[i] + Yaw_average[i];
//	}
//	
//  //给最后得出的40个均值排序
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

//  //输出最终结果
//  return(data2[39] * 0.75f + data2[38] * 0.25f - data2[0]);
//}

