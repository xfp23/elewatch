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
 * 上电或断电时，只要 VCC 低于 VWI 阈值，W25QxxJV 都将保持复位状态（参见上电时序和电压电平以及图 43）。
 * 复位期间，所有操作均被禁用，并且不识别任何指令。在上电期间以及 VCC 电压超过 VWI 后，所有编程和擦除相关指令将进一步被禁用，延迟时间为 tPUW。这包括写使能、页编程、扇区擦除、块擦除、芯片擦除和写状态寄存器指令。
 * 请注意，片选引脚 (/CS) 必须在上电时跟踪 VCC 电源电平，直至达到 VCC 最小电平和 tVSL 延迟时间；在断电时也必须跟踪 VCC 电源电平，以防止出现错误的命令序列。如有需要，可以在 /CS 引脚上使用上拉电阻来实现这一点。
 *
 * 上电后，设备自动进入写禁用状态，状态寄存器写使能锁存器 (WEL) 设置为 0。
 * 必须先发出写使能指令，然后才能接受页编程、扇区擦除、块擦除、芯片擦除或写入状态寄存器指令。完成编程、擦除或写入指令后，写使能锁存器 (WEL) 将自动清除为写禁用状态 0。
 *
 * 使用“写入状态寄存器”指令并设置状态寄存器保护位（SRP、SRL）和块保护位（CMP、TB、BP[2:0]）可实现软件控制的写保护。
 * 这些设置允许将部分或整个存储器阵列配置为只读。与写保护 (/WP) 引脚配合使用，可以在硬件控制下启用或禁用对状态寄存器的更改。有关更多信息，请参阅“状态寄存器”部分。
 * 此外，掉电指令提供了额外的写保护级别，因为除“释放掉电指令”外，所有指令都将被忽略。
 *
 * W25QxxJV 还提供另一种使用独立块锁定的写保护方法。每个 64KB 块（顶部和底部块除外，共 62 个块）以及顶部/底部块内的每个 4KB 扇区（共 32 个扇区）都配备了一个独立块锁定位。
 * 当锁定位为 0 时，可以擦除或编程相应的扇区或块；当锁定位设置为 1 时，发送到相应扇区或块的擦除或编程命令将被忽略。
 * 设备上电后，所有独立块锁定位都将为 1，因此整个存储器阵列受到保护，不能被擦除/编程。必须发出“独立块解锁 (39h)”指令才能解锁任何特定的扇区或块。
 *
 * 状态寄存器 3 中的 WPS 位用于决定应使用哪种写保护方案。
 * 当 WPS=0（出厂默认值）时，设备将仅使用 CMP、SEC、TB、BP[2:0] 位来保护阵列的特定区域；当 WPS=1 时，设备将使用独立块锁进行写保护。
 *
 * W25QxxJV 提供三个状态和配置寄存器。读取状态寄存器 1/2/3 指令可用于提供闪存阵列的可用性状态、器件是否启用或禁用写入、写保护状态、四路 SPI 设置、安全寄存器锁定状态、擦除/编程暂停状态以及输出驱动器强度。
 * 写入状态寄存器指令可用于配置器件的写保护功能、四路 SPI 设置、安全寄存器 OTP 锁定以及输出驱动器强度。
 * 状态寄存器的写访问由非易失性状态寄存器保护位 (SRL) 的状态、写入使能指令以及标准/双路 SPI 操作控制。
 *
 *   S7    S6    S5   S4    S3     S2   S1     S0
 * | SPR | SEC | TB | BP2 | BP1 | BP0 | WEL | BUSY |
 *
 *
 * BUSY 是状态寄存器 (S0) 中的一个只读位，当设备正在执行
 * 页面编程、四页编程、扇区擦除、块擦除、芯片擦除、写入状态寄存器或擦除/编程安全寄存器指令时，该位将被设置为 1。
 * 在此期间，设备将忽略除读取状态寄存器和擦除/编程暂停指令之外的后续指令（参见交流特性中的 tW、tPP、tSE、tBE 和 tCE）。
 * 当编程、擦除或写入状态/安全寄存器指令完成后，BUSY 位将被清除为 0，表示设备已准备好执行后续指令。
 *
 * 写使能锁存器 (WEL) 是状态寄存器 (S1) 中的一个只读位，执行写使能指令后该位会被置 1。
 * 当设备处于写禁用状态时，WEL 状态位会被清零。
 * 写禁用状态会在上电时或执行以下任何指令后发生：写禁用、页编程、四页编程、扇区擦除、块擦除、芯片擦除、写状态寄存器、擦除安全寄存器和编程安全寄存器。
 *
 * 块保护位 (BP2、BP1、BP0) 是状态寄存器 (S4、S3 和 S2) 中的非易失性读/写位，用于控制写保护状态。块保护位可以使用写状态寄存器指令设置（参见 AC 特性中的 tW）。
 * 存储器阵列可以全部、全部或部分受到保护，免受编程和擦除指令的侵害（参见状态寄存器存储器保护表）。块保护位的出厂默认设置为 0，即所有阵列均不受保护。
 *
 * 非易失性顶部/底部位 (TB) 控制块保护位 (BP2、BP1、BP0) 是从阵列顶部 (TB=0) 还是底部 (TB=1) 进行保护，如状态寄存器内存保护表所示。
 * 出厂默认设置为 TB=0。TB 位可以通过写入状态寄存器指令设置，具体取决于 SRP/SRL 和 WEL 位的状态。
 *
 * 非易失性扇区/块保护位 (SEC) 控制块保护位 (BP2、BP1、BP0) 是否保护阵列顶部 (TB=0) 或底部 (TB=1) 的 4KB 扇区 (SEC=1) 或 64KB 块 (SEC=0)，如状态寄存器内存保护表所示。默认设置为 SEC=0。
 *
 * 补码保护位 (CMP) 是状态寄存器 (S14) 中的一个非易失性读/写位。它与 SEC、TB、BP2、BP1 和 BP0 位配合使用，为阵列保护提供更灵活的选择。
 * 一旦 CMP 设置为 1，先前由 SEC、TB、BP2、BP1 和 BP0 设置的阵列保护将被撤销。例如，当 CMP=0 时，顶部 64KB 块可以受到保护，而阵列的其余部分不受保护；当 CMP=1 时，顶部 64KB 块将不受保护，而阵列的其余部分将变为只读状态。
 * 有关详细信息，请参阅状态寄存器内存保护表。默认设置为 CMP=0。
 *
 * W25QxxJV 提供三个状态和配置寄存器。读取状态寄存器 1/2/3 指令可用于提供闪存阵列的可用性状态、器件是否启用或禁用写入功能、写保护状态、四路 SPI 设置、安全寄存器锁定状态、擦除/编程暂停状态以及输出驱动器强度。
 * 写入状态寄存器指令可用于配置器件的写保护功能、四路 SPI 设置、安全寄存器 OTP 锁定和输出驱动器。对状态寄存器的写入访问由非易失性状态寄存器保护位（SRP、SRL）的状态、写入使能指令以及在标准/双路 SPI 操作期间的 /WP 引脚控制。
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

// W25X系列/Q系列芯片列表
// W25Q80  ID  0XEF13
// W25Q16  ID  0XEF14
// W25Qxx  ID  0XEF15
// W25Q64  ID  0XEF16
// W25Q128 ID  0XEF17
#define W25Q80 0XEF13
#define W25Q16 0XEF14
#define W25Qxx 0XEF15
#define W25Q64 0XEF16
#define W25Q128 0XEF17

#define W25Qxx_CheckStatus(ret) \
do                          \
{                           \
    if (ret != W25Qxx_OK)   \
    {                       \
        return ret;         \
    }                       \
} while (0)

#define W25Qxx_CheckHandle(handle)               \
do                                           \
{                                            \
    if (handle == NULL || (*handle) == NULL) \
    {                                        \
        return W25Qxx_ERROR;                 \
    }                                        \
} while (0)

// #define W25Qxx_WriteEnable(handle) W25Qxx_WriteCMD(handle,WRITE_ENABLE)

enum
{
    W25Qxx_OFF,
    W25Qxx_ON,
};

typedef enum
{

    W25Qxx_OK,       // OK
    W25Qxx_ERROR,    // 一般错误
    W25Qxx_SPIERROR, // SPI错误

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
    W25QXX_RESET = 0x99         // 执行复位操作（软件重启Flash芯片）

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
    // /HOLD 引脚允许在设备被选中时暂停设备。当 /HOLD 引脚被拉低且 /CS 引脚也保持低电平时，DO 引脚将处于高阻态，DI 和 CLK 引脚上的信号将被忽略（无关）。当 /HOLD 引脚被拉高时，设备可以恢复运行。当多个设备共享相同的 SPI 信号时，/HOLD 功能非常有用。/HOLD 引脚处于低电平有效状态。当状态寄存器 2 的 QE 位设置为四路 I/O 时，/HOLD 引脚功能不可用，因为该引脚用于 IO3。四路 I/O 操作的引脚配置请参见图 1a-c。
    W25Qxx_GPIO_t HOLD; // HOLD 引脚 (IO3)
    // 写保护 (/WP) 引脚可用于防止状态寄存器被写入。与状态寄存器的块保护 (CMP、SEC、TB、BP2、BP1 和 BP0) 位以及状态寄存器保护 (SRP) 位配合使用，可以对小至 4KB 扇区或整个存储器阵列的部分进行硬件保护。/WP 引脚低电平有效。
    W25Qxx_GPIO_t WP;       // WP 引脚 (IO2)
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
    int timeout;
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

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !__W25Qxx_H
