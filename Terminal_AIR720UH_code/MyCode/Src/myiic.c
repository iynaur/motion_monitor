
#include "myiic.h"
#include "delay.h"
#include "i2c.h"

void I2C1_Init(void)
{
	MX_I2C1_Init();
}


#ifndef IIC_USE_HAL_LIB

static ErrorStatus I2C_RequestMemoryWrite(I2C_TypeDef *I2Cx, uint32_t SlaveAddr, uint32_t SlaveAddrSize, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout);
static ErrorStatus I2C_RequestMemoryRead(I2C_TypeDef *I2Cx, uint32_t SlaveAddr, uint32_t SlaveAddrSize, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout);
static ErrorStatus I2C_WaitOnFlagUntilTimeout(I2C_TypeDef *I2Cx, uint32_t Flag, FlagStatus Status, uint32_t Timeout);
static ErrorStatus I2C_WaitOnSTOPFlagUntilTimeout(I2C_TypeDef *I2Cx, uint32_t Timeout);
static ErrorStatus I2C_WaitOnTXISFlagUntilTimeout(I2C_TypeDef *I2Cx, uint32_t Timeout);
static ErrorStatus I2C_IsAcknowledgeFailed(I2C_TypeDef *I2Cx, uint32_t Timeout);
static void I2C_Flush_TXDR(I2C_TypeDef *I2Cx);

/**
  * @brief  Write an amount of data in blocking mode to a specific memory address
	*	@param	I2Cx I2C Instance.
  * @param  SlaveAddr Specifies the slave address to be programmed.
  * @param  SlaveAddrSize This parameter can be one of the following values:
  *         @arg @ref LL_I2C_ADDRSLAVE_7BIT
  *         @arg @ref LL_I2C_ADDRSLAVE_10BIT
  * @param  MemAddress Internal memory address
  * @param  MemAddSize Size of internal memory address
  *         @arg @ref I2C_MEMADD_SIZE_8BIT
  *					@arg @ref I2C_MEMADD_SIZE_16BIT
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval ErrorStatus
*/
ErrorStatus I2C_Mem_Write(I2C_TypeDef *I2Cx, uint32_t SlaveAddr, uint32_t SlaveAddrSize, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint8_t *pBuffPtr = NULL; 
	uint16_t XferCount = 0;
	uint16_t XferSize = 0;

	
	if ((pData == NULL) || (Size == 0U))
	{
		return	ERROR;
	}
	
	/*µÈ´ýI2C×ÜÏß¿ÕÏÐ*/
	if (I2C_WaitOnFlagUntilTimeout(I2Cx, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY) != SUCCESS)
	{
		return ERROR;
	}

	
	/* Prepare transfer parameters */
	pBuffPtr	= pData;
	XferCount = Size;
	
	/* Send Slave Address and Memory Address */
	if (I2C_RequestMemoryWrite(I2Cx, SlaveAddr, SlaveAddrSize, MemAddress, MemAddSize, Timeout) != SUCCESS)
	{
		return ERROR;
	}

	
	/* Set NBYTES to write and reload if hi2c->XferCount > MAX_NBYTE_SIZE */
	if (XferCount > MAX_NBYTE_SIZE)
	{
		XferSize = MAX_NBYTE_SIZE;
		LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_NOSTARTSTOP);
	}
	else
	{
		XferSize = XferCount;
		LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_NOSTARTSTOP);
	}
	
	do
	{
		/* Wait until TXIS flag is set */
		if (I2C_WaitOnTXISFlagUntilTimeout(I2Cx, Timeout) != SUCCESS)
		{
			return ERROR;
		}
	
		/* Write data to TXDR */
		I2Cx->TXDR = *pBuffPtr;
	
		/* Increment Buffer pointer */
		pBuffPtr++;
	
		XferCount--;
		XferSize--;
	
		if ((XferCount != 0U) && (XferSize == 0U))
		{
			/* Wait until TCR flag is set */
			if (I2C_WaitOnFlagUntilTimeout(I2Cx, I2C_FLAG_TCR, RESET, Timeout) != SUCCESS)
			{
				return ERROR;
			}
	
			if (XferCount > MAX_NBYTE_SIZE)
			{
				XferSize = MAX_NBYTE_SIZE;
				LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_NOSTARTSTOP);
			}
			else
			{
				XferSize = XferCount;
				LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_NOSTARTSTOP);
			}
		}
	
	}
	while (XferCount > 0U);
	
	/* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
	/* Wait until STOPF flag is reset */
	if (I2C_WaitOnSTOPFlagUntilTimeout(I2Cx, Timeout) != SUCCESS)
	{
		return ERROR;
	}
	
	/* Clear STOP Flag */
	LL_I2C_ClearFlag_STOP(I2Cx);
	
	/* Clear Configuration Register 2 */
	I2C_RESET_CR2(I2Cx);
		
	return SUCCESS;
}



/**
  * @brief  Read an amount of data in blocking mode from a specific memory address
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  SlaveAddr Specifies the slave address to be programmed.
  * @param  SlaveAddrSize This parameter can be one of the following values:
  *         @arg @ref LL_I2C_ADDRSLAVE_7BIT
  *         @arg @ref LL_I2C_ADDRSLAVE_10BIT
  * @param  MemAddress Internal memory address
  * @param  MemAddSize Size of internal memory address
  *         @arg @ref I2C_MEMADD_SIZE_8BIT
  *					@arg @ref I2C_MEMADD_SIZE_16BIT
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
ErrorStatus I2C_Mem_Read(I2C_TypeDef *I2Cx, uint32_t SlaveAddr, uint32_t SlaveAddrSize, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
	uint8_t *pBuffPtr = NULL; 
	uint16_t XferCount = 0;
	uint16_t XferSize = 0;

  if ((pData == NULL) || (Size == 0U))
  {
    return  ERROR;
  }

  if (I2C_WaitOnFlagUntilTimeout(I2Cx, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY) != SUCCESS)
  {
    return ERROR;
  }


  /* Prepare transfer parameters */
  pBuffPtr  = pData;
  XferCount = Size;

  /* Send Slave Address and Memory Address */
  if (I2C_RequestMemoryRead(I2Cx, SlaveAddr, SlaveAddrSize, MemAddress, MemAddSize, Timeout) != SUCCESS)
  {
    return ERROR;
  }

  /* Send Slave Address */
  /* Set NBYTES to write and reload if hi2c->XferCount > MAX_NBYTE_SIZE and generate RESTART */
  if (XferCount > MAX_NBYTE_SIZE)
  {
    XferSize = MAX_NBYTE_SIZE;
    LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_START_READ);
  }
  else
  {
    XferSize = XferCount;
    LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
  }

  do
  {
    /* Wait until RXNE flag is set */
    if (I2C_WaitOnFlagUntilTimeout(I2Cx, I2C_FLAG_RXNE, RESET, Timeout) != SUCCESS)
    {
      return ERROR;
    }

    /* Read data from RXDR */
    *pBuffPtr = (uint8_t)I2Cx->RXDR;

    /* Increment Buffer pointer */
    pBuffPtr++;

    XferSize--;
    XferCount--;

    if ((XferCount != 0U) && (XferSize == 0U))
    {
      /* Wait until TCR flag is set */
      if (I2C_WaitOnFlagUntilTimeout(I2Cx, I2C_FLAG_TCR, RESET, Timeout) != SUCCESS)
      {
        return ERROR;
      }

      if (XferCount > MAX_NBYTE_SIZE)
      {
        XferSize = MAX_NBYTE_SIZE;
        LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_NOSTARTSTOP);
      }
      else
      {
        XferSize = XferCount;
        LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)XferSize, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_NOSTARTSTOP);
      }
    }
  }
  while (XferCount > 0U);

  /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
  /* Wait until STOPF flag is reset */
  if (I2C_WaitOnSTOPFlagUntilTimeout(I2Cx, Timeout) != SUCCESS)
  {
    return ERROR;
  }

  /* Clear STOP Flag */
  LL_I2C_ClearFlag_STOP(I2Cx);

  /* Clear Configuration Register 2 */
  I2C_RESET_CR2(I2Cx);

  return SUCCESS;
}



/**
  * @brief  Master sends target device address followed by internal memory address for write request.
	*	@param	I2Cx I2C Instance.
  * @param  SlaveAddr Specifies the slave address to be programmed.
  * @param  SlaveAddrSize This parameter can be one of the following values:
  *         @arg @ref LL_I2C_ADDRSLAVE_7BIT
  *         @arg @ref LL_I2C_ADDRSLAVE_10BIT
  * @param  MemAddress Internal memory address
  * @param  MemAddSize Size of internal memory address
  *         @arg @ref I2C_MEMADD_SIZE_8BIT
  *					@arg @ref I2C_MEMADD_SIZE_16BIT
  * @param  Timeout Timeout duration
  * @retval ErrorStatus
  */
static ErrorStatus I2C_RequestMemoryWrite(I2C_TypeDef *I2Cx, uint32_t SlaveAddr, uint32_t SlaveAddrSize, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout)
{
  LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize ,(uint8_t)MemAddSize, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_START_WRITE);

  /* Wait until TXIS flag is set */
  if (I2C_WaitOnTXISFlagUntilTimeout(I2Cx, Timeout) != SUCCESS)
  {
    return ERROR;
  }

  /* If Memory address size is 8Bit */
  if (MemAddSize == I2C_MEMADD_SIZE_8BIT)
  {
    /* Send Memory Address */
    I2Cx->TXDR = I2C_MEM_ADD_LSB(MemAddress);
  }
  /* If Memory address size is 16Bit */
  else
  {
    /* Send MSB of Memory Address */
    I2Cx->TXDR = I2C_MEM_ADD_MSB(MemAddress);

    /* Wait until TXIS flag is set */
    if (I2C_WaitOnTXISFlagUntilTimeout(I2Cx, Timeout) != SUCCESS)
    {
      return ERROR;
    }

    /* Send LSB of Memory Address */
    I2Cx->TXDR = I2C_MEM_ADD_LSB(MemAddress);
  }

  /* Wait until TCR flag is set */
  if (I2C_WaitOnFlagUntilTimeout(I2Cx, I2C_FLAG_TCR, RESET, Timeout) != SUCCESS)
  {
    return ERROR;
  }

  return SUCCESS;
}


/**
  * @brief  Master sends target device address followed by internal memory address for read request.
	*	@param	I2Cx I2C Instance.
  * @param  SlaveAddr Specifies the slave address to be programmed.
  * @param  SlaveAddrSize This parameter can be one of the following values:
  *         @arg @ref LL_I2C_ADDRSLAVE_7BIT
  *         @arg @ref LL_I2C_ADDRSLAVE_10BIT
  * @param  MemAddress Internal memory address
  * @param  MemAddSize Size of internal memory address
  *         @arg @ref I2C_MEMADD_SIZE_8BIT
  *					@arg @ref I2C_MEMADD_SIZE_16BIT
  * @param  Timeout Timeout duration
  * @retval ErrorStatus
  */
static ErrorStatus I2C_RequestMemoryRead(I2C_TypeDef *I2Cx, uint32_t SlaveAddr, uint32_t SlaveAddrSize, uint16_t MemAddress, uint16_t MemAddSize, uint32_t Timeout)
{
  LL_I2C_HandleTransfer(I2Cx, SlaveAddr, SlaveAddrSize, (uint8_t)MemAddSize, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

  /* Wait until TXIS flag is set */
  if (I2C_WaitOnTXISFlagUntilTimeout(I2Cx, Timeout) != SUCCESS)
  {
    return ERROR;
  }

  /* If Memory address size is 8Bit */
  if (MemAddSize == I2C_MEMADD_SIZE_8BIT)
  {
    /* Send Memory Address */
    I2Cx->TXDR = I2C_MEM_ADD_LSB(MemAddress);
  }
  /* If Memory address size is 16Bit */
  else
  {
    /* Send MSB of Memory Address */
    I2Cx->TXDR = I2C_MEM_ADD_MSB(MemAddress);

    /* Wait until TXIS flag is set */
    if (I2C_WaitOnTXISFlagUntilTimeout(I2Cx, Timeout) != SUCCESS)
    {
      return ERROR;
    }

    /* Send LSB of Memory Address */
    I2Cx->TXDR = I2C_MEM_ADD_LSB(MemAddress);
  }

  /* Wait until TC flag is set */
  if (I2C_WaitOnFlagUntilTimeout(I2Cx, I2C_FLAG_TC, RESET, Timeout) != SUCCESS)
  {
    return ERROR;
  }

  return SUCCESS;
}


/**
  * @brief  This function handles I2C Communication Timeout.
	*	@param	I2Cx I2C Instance.
  * @param  Flag Specifies the I2C flag to check.
  * @param  Status The new Flag status (SET or RESET).
  * @param  Timeout Timeout duration
  * @retval ErrorStatus
  */
static ErrorStatus I2C_WaitOnFlagUntilTimeout(I2C_TypeDef *I2Cx, uint32_t Flag, FlagStatus Status, uint32_t Timeout)
{
	uint32_t timecount = Timeout;
	while(I2C_GET_FLAG(I2Cx, Flag) == Status)
	{
		if(LL_SYSTICK_IsActiveCounterFlag())
		{
			if(timecount-- == 0)
			{
				return ERROR;
			}
		}
	}
	return SUCCESS;
}



/**
  * @brief  This function handles I2C Communication Timeout for specific usage of STOP flag.
	*	@param	I2Cx I2C Instance.
  * @param  Timeout Timeout duration
  * @retval ErrorStatus
  */
static ErrorStatus I2C_WaitOnSTOPFlagUntilTimeout(I2C_TypeDef *I2Cx, uint32_t Timeout)
{
	uint32_t timecount = Timeout;
  while (I2C_GET_FLAG(I2Cx, I2C_FLAG_STOPF) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(I2Cx, Timeout) != SUCCESS)
    {
      return ERROR;
    }

    /* Check for the Timeout */
		if(LL_SYSTICK_IsActiveCounterFlag())
		{
			if(timecount-- == 0)
			{
				return ERROR;
			}
		}
  }
  return SUCCESS;
}


/**
  * @brief  This function handles I2C Communication Timeout for specific usage of TXIS flag.
	*	@param	I2Cx I2C Instance.
  * @param  Timeout Timeout duration
  * @retval ErrorStatus
  */
static ErrorStatus I2C_WaitOnTXISFlagUntilTimeout(I2C_TypeDef *I2Cx, uint32_t Timeout)
{	
	uint32_t timecount = Timeout;
	
	while(I2C_GET_FLAG(I2Cx, I2C_FLAG_TXIS) == RESET)
	{
		/* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(I2Cx, Timeout) != SUCCESS)
    {
      return ERROR;
    }
		
		if(LL_SYSTICK_IsActiveCounterFlag())
		{
			if(timecount-- == 0)
			{
				return ERROR;
			}
		}
	}
	return SUCCESS;
}


/**
  * @brief  This function handles Acknowledge failed detection during an I2C Communication.
	*	@param	I2Cx I2C Instance.
  * @param  Timeout Timeout duration
  * @retval ErrorStatus
  */
static ErrorStatus I2C_IsAcknowledgeFailed(I2C_TypeDef *I2Cx, uint32_t Timeout)
{
	uint32_t timecount = Timeout;
	
	if(I2C_GET_FLAG(I2Cx, I2C_FLAG_AF) == SET)
	//if(LL_I2C_IsActiveFlag_NACK(I2Cx))
	{
    /* Wait until STOP Flag is reset */
    /* AutoEnd should be initiate after AF */
    while (I2C_GET_FLAG(I2Cx, I2C_FLAG_STOPF) == RESET)
    //while(LL_I2C_IsActiveFlag_STOP(I2Cx) == 0)
    {
      /* Check for the Timeout */
			if(LL_SYSTICK_IsActiveCounterFlag())
			{
				if(timecount-- == 0)
				{
					return ERROR;
				}
			}
    }
		
    /* Clear NACKF Flag */
    LL_I2C_ClearFlag_NACK(I2Cx);

		/* Clear STOP Flag */
		LL_I2C_ClearFlag_STOP(I2Cx);

		/* Flush TX register */
    I2C_Flush_TXDR(I2Cx);

    /* Clear Configuration Register 2 */
    I2C_RESET_CR2(I2Cx);

		return ERROR;
	}
	return SUCCESS;
}


/**
  * @brief  I2C Tx data register flush process.
	*	@param	I2Cx I2C Instance.
  * @retval None
  */
static void I2C_Flush_TXDR(I2C_TypeDef *I2Cx)
{
  /* If a pending TXIS flag is set */
  /* Write a dummy data in TXDR to clear it */
  if (I2C_GET_FLAG(I2Cx, I2C_FLAG_TXIS) != RESET)
  {
    I2Cx->TXDR = 0x00U;
  }

  /* Flush TX register if not empty */
  if (I2C_GET_FLAG(I2Cx, I2C_FLAG_TXE) == RESET)
  {
  	LL_I2C_ClearFlag_TXE(I2Cx);
  }
}

#endif


