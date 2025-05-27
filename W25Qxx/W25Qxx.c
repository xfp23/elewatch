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

	W25Qxx_Status_t ret = W25QXX_OK;
	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_RESET);

	if (HAL_SPI_Transmit((*handle)->HardWare.spi, buffer, size, (*handle)->timeout) != HAL_OK)
	{
		ret = W25QXX_SPIERROR;
	}

	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_SET);

	return ret;
}

/**
 * @brief 读字节
 *
 * @param handle 句柄地址
 * @param buffer 读取数据存放缓存
 * @param size 大小
 * @return W25Qxx_Status_t 状态
 */
static W25Qxx_Status_t W25Qxx_Readbyte(W25Qxx_Handle_t *handle, uint8_t *buffer, size_t size)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25QXX_OK;
	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_RESET);

	if (HAL_SPI_Receive((*handle)->HardWare.spi, buffer, size, (*handle)->timeout) != HAL_OK)
	{
		ret = W25QXX_SPIERROR;
	}

	HAL_GPIO_WritePin((*handle)->HardWare.CS.port, (*handle)->HardWare.CS.pin, GPIO_PIN_SET);

	return ret;
}

/**
 * @brief 写命令
 *
 * @param handle 句柄
 * @param cmd 命令
 * @return W25Qxx_Status_t 状态
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
 * @return W25Qxx_Status_t 状态
 */
static inline W25Qxx_Status_t W25Qxx_WriteEnable(W25Qxx_Handle_t *handle)
{
	return W25Qxx_WriteCMD(handle, WRITE_ENABLE);
}

/**
 * @brief 读使能
 *
 * @param handle
 * @return W25Qxx_Status_t 状态
 */
static inline W25Qxx_Status_t W25Qxx_ReadEnable(W25Qxx_Handle_t *handle)
{
	return W25Qxx_WriteCMD(handle, READ_DATA);
}

/**
 * @brief 写禁用
 *
 * @param handle
 * @return W25Qxx_Status_t 状态
 */
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
W25Qxx_Status_t W25Qxx_Write(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size)
{
	W25Qxx_CheckHandle(handle);

	unsigned int secpos;
	uint16_t secoff;
	uint16_t secremain;
	uint16_t i;
	uint8_t *W25QXX_BUF;
	W25Qxx_Status_t ret = W25QXX_OK;
	W25QXX_BUF = (*handle)->buffer;
	secpos = addr / 4096;	   // 扇区地址
	secoff = addr % 4096;	   // 在扇区内的偏移
	secremain = 4096 - secoff; // 扇区剩余空间大小
	if (size <= secremain)
		secremain = size; // 不大于4096个字节
	while (1)
	{
		ret = W25Qxx_Read(handle, W25QXX_BUF, secpos * 4096, 4096); // 读出整个扇区的内容

		W25Qxx_ChECKERR(ret);

		for (i = 0; i < secremain; i++) // 校验数据
		{
			if (W25QXX_BUF[secoff + i] != 0XFF)
				break; // 需要擦除
		}
		if (i < secremain) // 需要擦除
		{
			ret = W25Qxx_EraseSector(handle,secpos); // 擦除这个扇区

			W25Qxx_ChECKERR(ret);
			for (i = 0; i < secremain; i++) // 复制
			{
				W25QXX_BUF[i + secoff] = buffer[i];
			}
			ret = W25QXX_WriteNoCheck(handle, W25QXX_BUF, secpos * 4096, 4096); // 写入整个扇区

			W25Qxx_ChECKERR(ret);
		}
		else
			ret = W25QXX_WriteNoCheck(handle, buffer, addr, secremain); // 写已经擦除了的,直接写入扇区剩余区间.

		W25Qxx_ChECKERR(ret);
		if (size == secremain)
			break; // 写入结束了
		else	   // 写入未结束
		{
			secpos++;	// 扇区地址增1
			secoff = 0; // 偏移位置为0

			buffer += secremain; // 指针偏移
			addr += secremain;	 // 写地址偏移
			size -= secremain;	 // 字节数递减
			if (size > 4096)
				secremain = 4096; // 下一个扇区还是写不完
			else
				secremain = size; // 下一个扇区可以写完了
		}
	};

	return ret;
}

W25Qxx_Status_t W25Qxx_Read(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25QXX_OK;

	ret = W25Qxx_ReadEnable(handle);
	W25Qxx_ChECKERR(ret);

	uint8_t Addr[3] = {(uint8_t)((addr >> 16) & 0x000000FF), (uint8_t)((addr >> 8) & 0x000000FF), (uint8_t)((addr) & 0x000000FF)};

	ret = W25Qxx_Writebyte(handle, Addr, 3);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Readbyte(handle, buffer, size);

	W25Qxx_ChECKERR(ret);

	return ret;
}


W25Qxx_Status_t W25Qxx_Init(W25Qxx_Handle_t *handle, W25Qxx_Conf_t *conf)
{
	W25Qxx_CheckHandle(handle);

	(*handle) = (W25Qxx_Obj *)calloc(1, sizeof(W25Qxx_Obj));

	if ((*handle) == NULL)
		return W25QXX_ERROR;

	// (*handle)->flag.DMA = conf->DMA;
	(*handle)->HardWare.CS = conf->HardWare.CS;
	(*handle)->HardWare.RESET = conf->HardWare.RESET;
#if ENABLE_IO
	(*handle)->HardWare.HOLD = conf->HardWare.HOLD;
	(*handle)->HardWare.WP = conf->HardWare.WP;
#endif
	(*handle)->HardWare.spi = conf->HardWare.spi;
	(*handle)->timeout = conf->timeout;
	W25Qxx_Status_t ret = W25Qxx_WeekUP(handle);
	W25Qxx_ChECKERR(ret);
	ret = W25Qxx_ReadID(handle, &(*handle)->ID);
	W25Qxx_ChECKERR(ret);

	return ret;
}

W25Qxx_Status_t W25Qxx_Powerdown(W25Qxx_Handle_t *handle)
{
	W25Qxx_CheckHandle(handle);

	W25Qxx_Status_t ret = W25Qxx_WriteCMD(handle, POWER_DOWN);
	HAL_Delay(1);

	return ret;
}

static W25Qxx_Status_t W25Qxx_ReadSR(W25Qxx_Handle_t *handle, uint8_t *buffer)
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
	W25Qxx_Status_t ret = W25QXX_OK;
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

W25Qxx_Status_t W25Qxx_ReadCapcity(W25Qxx_Handle_t *handle, unsigned int *buffer)
{
	W25Qxx_CheckHandle(handle);

	uint8_t data[9] = {(uint8_t)READ_SFDP_REG, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00};
	W25Qxx_Status_t ret = W25Qxx_Writebyte(handle, data, 5);
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Readbyte(handle, data + 5, 4);

	W25Qxx_ChECKERR(ret);

	(*handle)->capacity = (unsigned int)(data[6] << 16) | (uint16_t)(data[7] << 8) | (data[8]);
	*buffer = (*handle)->capacity;

	return ret;
}

W25Qxx_Status_t W25Qxx_EraseSector(W25Qxx_Handle_t *handle, unsigned int addr)
{
	W25Qxx_CheckHandle(handle);

	addr *= 4096;
	W25Qxx_Status_t ret = W25Qxx_WriteEnable(handle);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Waitbusy(handle);

	W25Qxx_ChECKERR(ret);

	uint8_t data[4] = {SECTOR_ERASE_4KB, addr >> 16, addr >> 8, addr & 0x000000FF};

	ret = W25Qxx_Writebyte(handle,data,4);
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Waitbusy(handle);

	W25Qxx_ChECKERR(ret);

	return ret;
}

W25Qxx_Status_t W25QXX_WritePage(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size)
{
	W25Qxx_CheckHandle(handle);
	W25Qxx_Status_t ret = W25Qxx_WriteEnable(handle);

	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_WriteCMD(handle, PAGE_PROGRAM);
	W25Qxx_ChECKERR(ret);

	uint8_t data[] = {addr >> 16 & 0x000000FF, addr >> 8 & 0x000000FF, addr & 0x000000FF};

	ret = W25Qxx_Writebyte(handle, data, 3);
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Writebyte(handle, buffer, size);
	W25Qxx_ChECKERR(ret);

	ret = W25Qxx_Waitbusy(handle);
	W25Qxx_ChECKERR(ret);

	return ret;
}

W25Qxx_Status_t W25QXX_WriteNoCheck(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size)
{
	W25Qxx_CheckHandle(handle);
	W25Qxx_Status_t ret = W25QXX_OK;
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

W25Qxx_Status_t W25QXX_Delete(W25Qxx_Handle_t *handle)
{
	W25Qxx_CheckHandle(handle);

	free((*handle));
	(*handle) = NULL;

	return W25QXX_OK;
}
