#include "W25Qxx.h"
#include "math.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "spi.h"

/**
 * @brief
 *
 * @param handle 句柄
 * @param buffer 写入的缓冲区
 * @param size 写入大小
 * @return W25Qxx_Status_t
 */
static W25Qxx_Status_t W25Qxx_Writebyte(W25Qxx_Handle_t *handle, uint8_t *buffer, size_t size)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25Qxx_OK;
	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_RESET);

	if (HAL_SPI_Transmit((*handle)->HardWare.spi, buffer, size, (*handle)->timeout) != HAL_OK)
	{
		ret = W25Qxx_SPIERROR;
	}

	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_SET);

	return ret;
}

/**
 * @brief
 *
 * @param handle
 * @param buffer
 * @param size
 * @return W25Qxx_Status_t
 */
static W25Qxx_Status_t W25Qxx_Readbyte(W25Qxx_Handle_t *handle, uint8_t *buffer, size_t size)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25Qxx_OK;
	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_RESET);

	if (HAL_SPI_Receive((*handle)->HardWare.spi, buffer, size, (*handle)->timeout) != HAL_OK)
	{
		ret = W25Qxx_SPIERROR;
	}

	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_SET);

	return ret;
}

/**
 * @brief 写命令
 * 
 * @param handle 句柄
 * @param cmd 命令
 * @return W25Qxx_Status_t 
 */
static W25Qxx_Status_t W25Qxx_WriteCMD(W25Qxx_Handle_t *handle,W25Qxx_CMD_t cmd)
{
	W25Qxx_CheckHandle(handle);

	uint8_t byte = (uint8_t)cmd;
	return W25Qxx_Writebyte(handle,&byte,1);
}

/**
 * @brief 写使能
 * 
 * @param handle 
 * @return W25Qxx_Status_t 
 */
static inline W25Qxx_Status_t W25Qxx_WriteEnable(W25Qxx_Handle_t *handle)
{
    return W25Qxx_WriteCMD(handle, (uint8_t)WRITE_ENABLE); 
}

/**
 * @brief 
 * 
 * @param handle 
 * @param buffer 
 * @param addr 
 * @param size 
 * @return W25Qxx_Status_t 
 */
W25Qxx_Status_t W25Qxx_Write(W25Qxx_Handle_t *handle, uint8_t *buffer, uint32_t addr, size_t size)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25Qxx_OK;

	ret = W25Qxx_WriteEnable(handle); // 写使能
	// W25Qxx_CheckStatus(ret,W25Qxx_WriteEnable(handle));
	W25Qxx_CheckStatus(ret);

	uint8_t Addr[3] = {(addr >> 16) & 0x000000FF, (addr >> 8) & 0x000000FF, addr & 0x000000FF};
	ret = W25Qxx_Writebyte(handle, Addr, 3); // 发24位地址
	W25Qxx_CheckStatus(ret);
	ret = W25Qxx_Writebyte(handle, buffer, size); // 写数据
	W25Qxx_CheckStatus(ret);

	return ret;
}

/**
 * @brief 
 * 
 * @param handle 
 * @param conf 
 * @return W25Qxx_Status_t 
 */
W25Qxx_Status_t W25Qxx_Init(W25Qxx_Handle_t *handle, W25Qxx_Conf_t *conf)
{
	if(handle == NULL || *handle != NULL || conf == NULL) return W25Qxx_ERROR;

	(*handle) = (W25Qxx_Obj *)calloc(1,sizeof(W25Qxx_Obj));

	if((*handle) == NULL) return W25Qxx_ERROR;

	// (*handle)->flag.DMA = conf->DMA;
	(*handle)->HardWare.CS = conf->HardWare.CS;
	(*handle)->HardWare.HOLD = conf->HardWare.HOLD;
	(*handle)->HardWare.RESET = conf->HardWare.RESET;
	(*handle)->HardWare.WP = conf->HardWare.WP;
	(*handle)->HardWare.spi = conf->HardWare.spi;
	(*handle)->timeout = conf->timeout;

	return W25Qxx_OK;
}


