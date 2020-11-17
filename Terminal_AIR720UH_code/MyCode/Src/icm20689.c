#include "icm20689.h"
#include "myiic.h"
#include "i2c.h"
#include "delay.h"
#include "myusart.h"

static uint8_t icm20689_write_reg(uint8_t reg, uint8_t *data, uint16_t len)
{
	uint8_t ret;
	
#ifndef IIC_USE_HAL_LIB
	ret = I2C_Mem_Write(ICM_I2C_BUS_NUM, (ICM_I2C_ADDR_L << 1), LL_I2C_ADDRSLAVE_7BIT,
												 reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
#else
	ret = HAL_I2C_Mem_Write(&hi2c1, (ICM_I2C_ADDR_L << 1), reg,
                      I2C_MEMADD_SIZE_8BIT, data, len, 1000);
#endif
	//debug_print("write ret is %d.\r\n", ret);
	return ret;
}

static uint8_t icm20689_read_reg(uint8_t reg, uint8_t *data, uint16_t len)
{
	uint8_t ret;
#ifndef IIC_USE_HAL_LIB
	ret = I2C_Mem_Read(ICM_I2C_BUS_NUM, (ICM_I2C_ADDR_L << 1), LL_I2C_ADDRSLAVE_7BIT,
												 reg, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
#else
	ret = HAL_I2C_Mem_Read(&hi2c1, (ICM_I2C_ADDR_L << 1), reg,
                      I2C_MEMADD_SIZE_8BIT, data, len, 1000);
#endif
	
	//debug_print("read ret is %d.\r\n", ret);
	return ret;
}


uint8_t ICM20689_Init(void)
{
	uint8_t res;
	ICM20689_Soft_Reset();//复位ICM20689
	delay_ms(100);
	ICM20689_Power_ON();//唤醒ICM20689 
	ICM20689_Set_Gyro_Fsr(ICM_GYRO_500dps);//陀螺仪传感器,±500dps
	ICM20689_Set_Accel_Fsr(ICM_ACCEL_4g);//加速度传感器,±4g
	ICM20689_Set_Rate(ICM20689_HZ);//设置采样率
	res = 0x01;
	icm20689_write_reg(ICM_INT_EN_REG, &res, 1);//打开所有中断
	res = 0x00;
	icm20689_write_reg(ICM_USER_CTRL_REG, &res, 1);//I2C主模式关闭
	icm20689_write_reg(ICM_FIFO_EN_REG, &res, 1);//关闭FIFO
	res = 0x80;
	icm20689_write_reg(ICM_INTBP_CFG_REG, &res, 1);//INT引脚低电平有效

	ICM20689_Get_DeviceID(&res);
	debug_print("ID is 0x%02x.\r\n", res);
	if(res == ICM20689_ID || res == MPU6050_ID)//器件ID正确
	{
		res = 0x01;
		icm20689_write_reg(ICM_PWR_MGMT1_REG, &res, 1);//设置CLKSEL,PLL X轴为参考
		res = 0x00;
		icm20689_write_reg(ICM_PWR_MGMT2_REG, &res, 1);//加速度与陀螺仪都工作
	}
	else return 1;

	return 0;

	
}


uint8_t ICM20689_Soft_Reset(void)
{
	uint8_t res = 0x80;
	return icm20689_write_reg(ICM_PWR_MGMT1_REG,&res,1);

}

uint8_t ICM20689_Power_ON(void)
{
	uint8_t res = 0x00;
	return icm20689_write_reg(ICM_PWR_MGMT1_REG, &res, 1);
}


uint8_t ICM20689_Low_Power_Mode(void)
{
	uint8_t res = 0x40;
	return icm20689_write_reg(ICM_PWR_MGMT1_REG, &res, 1);
}


uint8_t ICM20689_Set_Gyro_Fsr(icm20689_gyro_fsr gyro_fsr)
{
	uint8_t res, ret;
	//ret = icm20689_read_reg(ICM_GYRO_CFG_REG,&res,1);
	//debug_print("first read gyro fsr is %d.\r\n", res);
	//if(ret == SUCCESS)
	//{
		//res &= (~(0x03 << 3));
		res =  gyro_fsr << 3;
		ret = icm20689_write_reg(ICM_GYRO_CFG_REG,&res,1);
		//icm20689_read_reg(ICM_GYRO_CFG_REG,&res,1);
		//debug_print("second read gyro fsr is %d.\r\n", res);
	//}
	return ret;
}

uint8_t ICM20689_Set_Accel_Fsr(icm20689_accel_fsr accel_fsr)
{
	uint8_t res, ret;
	//ret = icm20689_read_reg(ICM_ACCEL_CFG_REG,&res,1);
	//debug_print("first read accel fsr is %d.\r\n", res);
	//if(ret == SUCCESS)
	//{
		//res &= (~(0x03 << 3));
		res =  accel_fsr << 3;
		ret = icm20689_write_reg(ICM_ACCEL_CFG_REG,&res,1);
	//	icm20689_read_reg(ICM_ACCEL_CFG_REG,&res,1);
	//	debug_print("second read accel fsr is %d.\r\n", res);
	//}
	return ret;
}

uint8_t ICM20689_Set_Rate(uint16_t rate)
{
	u8 res , ret;
	if(rate > 1000) rate = 1000;
	if(rate < 4) rate = 4;
	res = 1000 / rate - 1;

	ret = icm20689_write_reg(ICM_SAMPLE_RATE_REG,&res,1);

	if(ret == SUCCESS)
	{
		ret = ICM20689_Set_LPF(rate/2);
	}

	return ret;
}

uint8_t ICM20689_Set_LPF(uint16_t lpf)
{
	uint8_t res;
	
	if(lpf>=188)res=1;
	else if(lpf>=98)res=2;
	else if(lpf>=42)res=3;
	else if(lpf>=20)res=4;
	else if(lpf>=10)res=5;
	else res=6; 
	
	return icm20689_write_reg(ICM_CFG_REG,&res,1);
}

uint8_t ICM20689_Get_DeviceID(uint8_t *id)
{
	return icm20689_read_reg(ICM_DEVICE_ID_REG, id, 1);
}


uint8_t ICM20689_Get_Temperature(short *temp)
{
	uint8_t ret;
	uint8_t buf[2] = {0};
	short raw = 0;
	float tmp = 0;

	ret = icm20689_read_reg(ICM_TEMP_OUTH_REG,buf,2);

	if(ret == SUCCESS)
	{
		raw=((u16)buf[0]<<8)|buf[1];  
	  tmp=25+((double)raw)/326.8f;  
	  *temp = tmp*100;
	}

	return ret;
}

uint8_t ICM20689_Get_Gyroscope(short *gx, short *gy, short *gz)
{
	uint8_t ret, buf[6];

	ret = icm20689_read_reg(ICM_GYRO_XOUTH_REG,buf,6);

	if(ret == SUCCESS)
	{
		*gx=((u16)buf[0]<<8)|buf[1];  
		*gy=((u16)buf[2]<<8)|buf[3];  
		*gz=((u16)buf[4]<<8)|buf[5];
	}

	return ret;
}

uint8_t ICM20689_Get_Accelerometer(short *ax, short *ay, short *az)
{
	uint8_t ret, buf[6];

	ret = icm20689_read_reg(ICM_ACCEL_XOUTH_REG,buf,6);

	if(ret == SUCCESS)
	{
		*ax=((u16)buf[0]<<8)|buf[1];  
		*ay=((u16)buf[2]<<8)|buf[3];  
		*az=((u16)buf[4]<<8)|buf[5];
	}

	return ret;
}


uint8_t ICM20689_Get_INT_Status(uint8_t *status)
{
	return icm20689_read_reg(ICM_INT_STA_REG, status, 1);
}

