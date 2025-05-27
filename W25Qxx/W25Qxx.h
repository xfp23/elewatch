#ifndef __W25QXX_H
#define __W25QXX_H

#pragma anon_unions

/**
 * @file W25Qxx.h
 * @author https://github.com/xfp23
 * @brief W25Qxx系列flash芯片驱动
 * @note 此驱动不会初始化你的gpio以及spi硬件资源
 *
 * SPI最大频率 : 104MHZ
 * 低极性 第二边沿锁存
 *
 * @version 0.1
 * @date 2025-05-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "stdint.h"
#include "stdbool.h"
#include "main.h"

#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

#define ENABLE_IO W25QXX_OFF // 是否使用其余两个IO功能

typedef enum
{
    W25QXX_IDUNKNOW,
    W25Q80 = 0XEF13,
    W25Q16 = 0XEF14,
    W25Q32 = 0XEF15,
    W25Q64 = 0XEF16,
    W25Q128 = 0XEF17,
} W25Qxx_ID_t;

#define W25Qxx_ChECKERR(ret)  \
do                        \
{                         \
    if (ret != W25QXX_OK) \
    {                     \
        return ret;       \
    }                     \
} while (0)

#define W25Qxx_CheckHandle(handle)               \
do                                           \
{                                            \
    if (handle == NULL || (*handle) == NULL) \
    {                                        \
        return W25QXX_ERROR;                 \
    }                                        \
} while (0)

enum
{
    W25QXX_OFF,
    W25QXX_ON,
};

typedef enum
{

    W25QXX_OK,       // OK
    W25QXX_ERROR,    // 一般错误
    W25QXX_SPIERROR, // SPI错误

} W25Qxx_Status_t;

typedef enum
{
    WRITE_ENABLE = 0x06,    // 使能写操作（必须在写、擦除前调用）
    SR_WRITE_ENABLE = 0x50, // 状态寄存器写使能（允许写状态寄存器）

    WRITE_DISABLE = 0x04, // 禁止写操作

    READ_STATUS_REG1 = 0x05, // 读取状态寄存器1
    READ_STATUS_REG2 = 0x35, // 读取状态寄存器2
    WRITE_STATUS_REG = 0x01, // 写入状态寄存器1和2

    PAGE_PROGRAM = 0x02, // 页编程（最多写256字节）

    SECTOR_ERASE_4KB = 0x20, // 擦除一个4KB扇区
    BLOCK_ERASE_32KB = 0x52, // 擦除一个32KB块
    BLOCK_ERASE_64KB = 0xD8, // 擦除一个64KB块
    CHIP_ERASE = 0xC7,       // 整片擦除（擦除整个芯片）

    SUPEND = 0x75, // 暂停擦除或编程操作
    RESUME = 0x7A, // 恢复擦除或编程操作

    POWER_DOWN = 0xB9,           // 进入掉电模式（省电）
    RELEASE_POWERDOWN_ID = 0xAB, // 退出掉电模式并读取设备ID

    READ_DATA = 0x03, // 标准读取数据（低速）
    FAST_READ = 0x0B, // 快速读取数据（支持高速读取）

    MANUFACT_DEV_ID = 0x90, // 读取制造商和设备ID
    JEDEC_ID = 0x9F,        // 读取JEDEC ID（三字节：厂商ID+存储类型+容量）
    READ_UNIQUE_ID = 0x4B,  // 读取唯一ID（64位唯一序列号）
    READ_SFDP_REG = 0x5A,   // 读取SFDP寄存器（序列化闪存发现参数）

    ERASE_SECURITY_REG = 0x44,   // 擦除安全寄存器
    PROGRAM_SECURITY_REG = 0x42, // 写安全寄存器
    READ_SECURITY_REG = 0x48,    // 读安全寄存器

    ENABLE_RESET = 0x66, // 使能复位命令（必须先发该命令再发RESET）
    W25QXX_RESET = 0x99  // 执行复位操作（软件重启Flash芯片）

} W25Qxx_CMD_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} W25Qxx_GPIO_t; // gpio类型

typedef union
{
    uint8_t full;
    struct
    {
        uint8_t DMA : 1;
        uint8_t Reserve_bits : 7; // 保留位
    };

} W25Qxx_Flag_t; // 标志类型

typedef struct
{
    W25Qxx_GPIO_t CS; // 片选引脚
    // SOIC-16 和 TFBGA 封装均提供专用硬件 /RESET 引脚。当该引脚被驱动至低电平并持续约 1µS 时，该器件将终止任何外部或内部操作并返回到上电状态。
    W25Qxx_GPIO_t RESET; // 复位引脚 如果不使用复位功能可以上拉到VCC或者浮空
#if ENABLE_IO
    // /HOLD 引脚允许在设备被选中时暂停设备。当 /HOLD 引脚被拉低且 /CS 引脚也保持低电平时，DO 引脚将处于高阻态，DI 和 CLK 引脚上的信号将被忽略（无关）。当 /HOLD 引脚被拉高时，设备可以恢复运行。当多个设备共享相同的 SPI 信号时，/HOLD 功能非常有用。/HOLD 引脚处于低电平有效状态。当状态寄存器 2 的 QE 位设置为四路 I/O 时，/HOLD 引脚功能不可用，因为该引脚用于 IO3。四路 I/O 操作的引脚配置请参见图 1a-c。
    W25Qxx_GPIO_t HOLD; // HOLD 引脚 (IO3)
    // 写保护 (/WP) 引脚可用于防止状态寄存器被写入。与状态寄存器的块保护 (CMP、SEC、TB、BP2、BP1 和 BP0) 位以及状态寄存器保护 (SRP) 位配合使用，可以对小至 4KB 扇区或整个存储器阵列的部分进行硬件保护。/WP 引脚低电平有效。
    W25Qxx_GPIO_t WP;       // WP 引脚 (IO2)
#endif

    SPI_HandleTypeDef *spi; // 硬件spi
} W25Qxx_HardWare_t;        // 硬件配置

typedef struct
{
    W25Qxx_HardWare_t HardWare; // 硬件
    // bool DMA;
    int timeout; // 超时时间，-1表示永远等待
} W25Qxx_Conf_t;

typedef struct
{
    W25Qxx_HardWare_t HardWare; // 硬件
    W25Qxx_Flag_t flag;
    W25Qxx_ID_t ID;
    unsigned int capacity;
    int timeout;
    uint8_t buffer[4096];
} W25Qxx_Obj;

typedef W25Qxx_Obj *W25Qxx_Handle_t; // 句柄

/**
 * @brief 初始化W25Qxx
 *
 * @param handle 句柄地址
 * @param conf  配置结构体地址
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25Qxx_Init(W25Qxx_Handle_t *handle, W25Qxx_Conf_t *conf);

/**
 * @brief 读取芯片数据
 * 
 * @param handle 句柄
 * @param buffer 读取buffer
 * @param addr 读取的地址
 * @param size 大小
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25Qxx_Read(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size);

/**
 * @brief 向W25Qxx写入数据
 *
 * @param handle 句柄
 * @param buffer 写入的buffer
 * @param addr 向芯片写入的地址
 * @param size 大小
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25Qxx_Write(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size);

/**
 * @brief 读取ID，型号
 *
 * @param handle 句柄
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25Qxx_ReadID(W25Qxx_Handle_t *handle, W25Qxx_ID_t *buffer);

/**
 * @brief 唤醒
 *
 * @param handle 句柄地址
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25Qxx_WeekUP(W25Qxx_Handle_t *handle);

/**
 * @brief 掉电休眠
 *
 * @param handle
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25Qxx_Powerdown(W25Qxx_Handle_t *handle);

/**
 * @brief 擦除整个芯片
 *
 * @param handle 句柄
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25Qxx_EraseChip(W25Qxx_Handle_t *handle);

/**
 * @brief 读取容量
 *
 * @param handle 句柄
 * @param buffer 存储容量的buffer
 * @return W25Qxx_Status_t  状态
 */
extern W25Qxx_Status_t W25Qxx_ReadCapcity(W25Qxx_Handle_t *handle, unsigned int *buffer);

/**
 * @brief
 *
 * @param handle
 * @param addr
 * @return W25Qxx_Status_t
 */
extern W25Qxx_Status_t W25Qxx_EraseSector(W25Qxx_Handle_t *handle, unsigned int addr);

/**
 * @brief 向页写入数据
 *
 * @param handle 句柄
 * @param buffer 写入buffer
 * @param addr 页地址
 * @param size 大小
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25QXX_WritePage(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size);

/**
 * @brief 不做校验直接往芯片写值
 *
 * @param handle 句柄地址
 * @param buffer 写入的buffer
 * @param addr 写入数据的地址
 * @param size 写入的大小 ： 字节
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25QXX_WriteNoCheck(W25Qxx_Handle_t *handle, uint8_t *buffer, unsigned int addr, size_t size);

/**
 * @brief 卸载W25Qxx
 *
 * @param handle 句柄地址
 *
 * @return W25Qxx_Status_t 状态
 */
extern W25Qxx_Status_t W25QXX_Delete(W25Qxx_Handle_t *handle);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !__W25Qxx_H
