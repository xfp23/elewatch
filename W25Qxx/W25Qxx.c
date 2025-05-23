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
static W25Qxx_Status_t W25Qxx_WriteCMD(W25Qxx_Handle_t *handle, W25Qxx_CMD_t cmd)
{
	W25Qxx_CheckHandle(handle);

	uint8_t byte = (uint8_t)cmd;
	return W25Qxx_Writebyte(handle, &byte, 1);
}

/**
 * @brief 写使能
 *
 * @param handle
 * @return W25Qxx_Status_t
 */
static inline W25Qxx_Status_t W25Qxx_WriteEnable(W25Qxx_Handle_t *handle)
{
	return W25Qxx_WriteCMD(handle, WRITE_ENABLE);
}

static inline W25Qxx_Status_t W25Qxx_ReadEnable(W25Qxx_Handle_t *handle)
{
	return W25Qxx_WriteCMD(handle, READ_DATA);
}

static inline W25Qxx_Status_t W25Qxx_WriteDisable(W25Qxx_Handle_t *handle)
{
	return W25Qxx_WriteCMD(handle, WRITE_DISABLE);
}

W25Qxx_Status_t W25Qxx_WeekUP(W25Qxx_Handle_t *handle)
{
	W25Qxx_Status_t ret = W25Qxx_WriteCMD(handle, RELEASE_POWERDOWN_ID);
	HAL_Delay(1);
	return ret;
}

/**
 * @brief
 *
 * @param handle
 * @return W25Qxx_Status_t
 */
W25Qxx_Status_t W25Qxx_ReadID(W25Qxx_Handle_t *handle, W25Qxx_ID_t *buffer)
{

	W25Qxx_CheckHandle(handle);

	uint8_t data[6] = {(uint8_t)MANUFACT_DEV_ID, 0x00, 0x00, 0x00, 0x00, 0x00};

	W25Qxx_Status_t ret = W25Qxx_Writebyte(handle, data, 4);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Readbyte(handle, data + 4, 2);
	W25Qxx_ChECKERR(ret);
	uint16_t id = (uint16_t)data[4] << 8 | data[5];
	switch (id)
	{
	case (uint16_t)W25Q80:
		*buffer = W25Q80;
		break;
	case (uint16_t)W25Q16:
		*buffer = W25Q16;
		break;
	case (uint16_t)W25Q32:
		*buffer = W25Q32;
		break;
	case (uint16_t)W25Q64:
		*buffer = W25Q64;
		break;
	case (uint16_t)W25Q128:
		*buffer = W25Q128;
		break;
	default:
		*buffer = W25QXX_IDUNKNOW;
	}

	return ret;
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

	W25Qxx_ChECKERR(ret);

	uint8_t Addr[3] = {(addr >> 16) & 0x000000FF, (addr >> 8) & 0x000000FF, addr & 0x000000FF};
	ret = W25Qxx_Writebyte(handle, Addr, 3); // 发24位地址
	W25Qxx_ChECKERR(ret);
	ret = W25Qxx_Writebyte(handle, buffer, size); // 写数据
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_WriteDisable(handle); // 写功能禁用
	W25Qxx_ChECKERR(ret);

	return ret;
}

W25Qxx_Status_t W25Qxx_Read(W25Qxx_Handle_t *handle, uint8_t *buffer, uint32_t addr, size_t size)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25Qxx_OK;

	ret = W25Qxx_ReadEnable(handle);
	W25Qxx_ChECKERR(ret);

	uint8_t Addr[3] = {(uint8_t)((addr >> 16) & 0x000000FF), (uint8_t)((addr >> 8) & 0x000000FF), (uint8_t)((addr) & 0x000000FF)};

	ret = W25Qxx_Writebyte(handle, Addr, 3);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Readbyte(handle, buffer, size);

	W25Qxx_ChECKERR(ret);
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
	W25Qxx_CheckHandle(handle);

	(*handle) = (W25Qxx_Obj *)calloc(1, sizeof(W25Qxx_Obj));

	if ((*handle) == NULL)
		return W25Qxx_ERROR;

	// (*handle)->flag.DMA = conf->DMA;
	(*handle)->HardWare.CS = conf->HardWare.CS;
	(*handle)->HardWare.HOLD = conf->HardWare.HOLD;
	(*handle)->HardWare.RESET = conf->HardWare.RESET;
	(*handle)->HardWare.WP = conf->HardWare.WP;
	(*handle)->HardWare.spi = conf->HardWare.spi;
	(*handle)->timeout = conf->timeout;
	W25Qxx_Status_t ret = W25Qxx_WeekUP(handle);
	W25Qxx_ChECKERR(ret);
	ret = W25Qxx_ReadID(handle, &(*handle)->ID);
	W25Qxx_ChECKERR(ret);
	return W25Qxx_OK;
}

W25Qxx_Status_t W25Qxx_Powerdown(W25Qxx_Handle_t *handle)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25Qxx_WriteCMD(handle, POWER_DOWN);
	HAL_Delay(1);

	return ret;
}

W25Qxx_Status_t W25Qxx_ReadSR(W25Qxx_Handle_t *handle, uint8_t *buffer)
{
	W25Qxx_CheckHandle(handle);
	W25Qxx_Status_t ret = W25Qxx_WriteCMD(handle, READ_STATUS_REG1);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Readbyte(handle, buffer, 1);

	W25Qxx_ChECKERR(ret);

	return ret;
}

static W25Qxx_Status_t W25Qxx_Waitbusy(W25Qxx_Handle_t *handle)
{
	W25Qxx_CheckHandle(handle);

	uint8_t data = 0x00;
	W25Qxx_Status_t ret = W25Qxx_OK;
	do
	{
		ret = W25Qxx_ReadSR(handle, &data);
		W25Qxx_ChECKERR(ret);
	} while ((data & 0x01) == 0x01);

	return ret;
}

W25Qxx_Status_t W25Qxx_EraseChip(W25Qxx_Handle_t *handle)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25Qxx_WriteEnable(handle);

	W25Qxx_ChECKERR(ret);
	ret = W25Qxx_Waitbusy(handle);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_WriteCMD(handle, CHIP_ERASE);

	W25Qxx_ChECKERR(ret);
	ret = W25Qxx_Waitbusy(handle);

	return ret;
}

// uint32_t W25QXX_ReadCapacity(void)
// {
// 	int i = 0;
// 	uint8_t arr[4] = {0,0,0,0};
//     W25QXX_CS_L();
//     W25QXX_SPI_ReadWriteByte(0x5A);
//     W25QXX_SPI_ReadWriteByte(0x00);
//     W25QXX_SPI_ReadWriteByte(0x00);
//     W25QXX_SPI_ReadWriteByte(0x84);
// 	W25QXX_SPI_ReadWriteByte(0x00);
// 	for(i = 0; i < sizeof(arr); i++)
// 	{
// 		arr[i] = W25QXX_SPI_ReadWriteByte(0xFF);
// 	}
//     W25QXX_CS_H();
//     return ((((*(uint32_t *)arr)) + 1) >> 3);
// }capacity

W25Qxx_Status_t W25Qxx_ReadCapcity(W25Qxx_Handle_t *handle, uint32_t *buffer)
{
	W25Qxx_CheckHandle(handle);

	uint8_t data[9] = {(uint8_t)READ_SFDP_REG, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00};
	W25Qxx_Status_t ret = W25Qxx_Writebyte(handle, data, 5);
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Readbyte(handle, data + 5, 4);

	W25Qxx_ChECKERR(ret);

	(*handle)->capacity = (uint32_t)(data[6] << 16) | (uint16_t)(data[7] << 8) | (data[8]);
	*buffer = (*handle)->capacity;

	return ret;
}

W25Qxx_Status_t W25Qxx_EraseSector(W25Qxx_Handle_t *handle, uint32_t addr)
{
	W25Qxx_CheckHandle(handle);

	addr *= 4096;
	W25Qxx_Status_t ret = W25Qxx_WriteEnable(handle);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Waitbusy(handle);

	W25Qxx_ChECKERR(ret);

	uint8_t data[4] = {SECTOR_ERASE_4KB, addr >> 16, addr >> 8, addr & 0x000000FF};
	ret = W25Qxx_Waitbusy(handle);

	W25Qxx_ChECKERR(ret);

	return ret;
}

W25Qxx_Status_t W25QXX_WritePage(W25Qxx_Handle_t *handle, uint8_t *buffer,uint32_t addr, size_t size)
{
	W25Qxx_CheckHandle(handle);
	W25Qxx_Status_t ret = W25Qxx_WriteEnable(handle);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_WriteCMD(handle,PAGE_PROGRAM);
	W25Qxx_ChECKERR(ret);

	uint8_t data[] = {addr >> 16 & 0x000000FF,addr >> 8 & 0x000000FF,addr & 0x000000FF};

	ret = W25Qxx_Writebyte(handle,data,3);
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Writebyte(handle,buffer,size);
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Waitbusy(handle);
	W25Qxx_ChECKERR(ret);

	return ret;
}

W25Qxx_Status_t W25QXX_WriteNoCheck(W25Qxx_Handle_t *handle, uint8_t *buffer, uint32_t addr, size_t size)
{
	W25Qxx_CheckHandle(handle);
	W25Qxx_Status_t ret = W25Qxx_OK;
	uint16_t pageremain = 256 - addr % 256;

	if (size <= pageremain)
	{
		pageremain = size;
	}

	while (1)
	{
		 ret = W25QXX_WritePage(handle, buffer, addr, pageremain);
		W25Qxx_ChECKERR(ret);

		if (addr == pageremain)
			break;
		else
		{
			buffer += pageremain;
			addr += pageremain;

			size -= pageremain;
			if (size > 256)
				pageremain = 256; // 一次可以写入256个字节
			else
				pageremain = size; // 不够256个字节了
		}
	}

	return ret;
}