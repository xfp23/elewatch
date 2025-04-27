#ifndef SOFTWAREUART_H
#define SOFTWAREUART_H

#pragma anon_unions

#include "stdio.h"
#include "main.h"
#include "stdint.h"
#include "string.h"
#include "stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

typedef enum
{
    BITS_9600 = 104,
    BITS_19200 = 58,
    BITS_115200 = 9,

} SoftWareUart_baud_t;

typedef enum 
{
    UART_OK,
    UART_ERROR,

}SoftWareUart_Status_t;

// 枚举类型标记当前位
typedef enum
{
    COM_START_BIT, // 起始位
    COM_D0_BIT,
    COM_D1_BIT,
    COM_D2_BIT,
    COM_D3_BIT,
    COM_D4_BIT,
    COM_D5_BIT,
    COM_D6_BIT,
    COM_D7_BIT,
    COM_STOP_BIT, // 停止位
} SoftWare_Bit_t;

typedef struct
{
    GPIO_TypeDef *TXport;
    uint16_t TXpin;
    GPIO_TypeDef *RXport;
    uint16_t RXpin;
    TIM_HandleTypeDef *DelayHtime;     // 延時定時器
    TIM_HandleTypeDef *InterruptHtime; // 主中斷定時器
} SoftWareUart_HardWare_t;

typedef struct
{
    SoftWareUart_baud_t baud; // 波特率
    SoftWareUart_HardWare_t HardWare;
    uint8_t *rxbuffer;
    size_t rx_size;
} SoftWareUart_Conf_t;

typedef struct
{
    uint8_t Recivedata :1;
    uint8_t Firstdata :1;
    uint8_t Reserve_bits :6;
}SoftWare_flag_t;
typedef union
{
    struct
    {
        SoftWareUart_baud_t baud;
        SoftWareUart_HardWare_t HardWare;
        SoftWare_Bit_t recvStat; // 当前位
        uint8_t recvData;
        uint8_t *rxbuffer;
        size_t rx_size;
        volatile uint16_t rx_write_index;
        volatile uint16_t rx_read_index;
        SoftWare_flag_t flag;

    };
} SoftWareUart_t;

typedef SoftWareUart_t *SoftWareUart_Handle_t;

/**
 * @brief 软件串口发送单字节
 *
 * @param handle 软件串口句柄
 * @param byte 要发送的字节
 */
extern SoftWareUart_Status_t SoftWareUart_Sendbyte(SoftWareUart_Handle_t handle, uint8_t byte);

/**
 * @brief 软件串口对外发送buffer
 *
 * @param handle 句柄
 * @param buffer buffer地址
 * @param size buffer大小
 */
extern SoftWareUart_Status_t SoftWareUart_SendBuffer(SoftWareUart_Handle_t handle, const uint8_t *buffer, size_t size);

/**
 * @brief 软件串口初始化
 *
 * @param handle 句柄
 * @param conf 配置信息
 */
extern SoftWareUart_Status_t SoftWareUART_Init(SoftWareUart_Handle_t *handle, SoftWareUart_Conf_t *conf);

/**
 * @brief 软件串口的外部中断回调
 * @note 将RX引脚配置为下降沿中断，然后将此函数放入对应的引脚中断处 stm32xx_it.c
 * @param handle 句柄
 */
extern void SoftWareUart_RXCallback(SoftWareUart_Handle_t *handle);

/**
 * @brief 软件巉口的定时器回调
 *
 * @param handle 将此函数放入配置信息中的 InterruptHtime 定时器回调触发中
 */
extern void SoftWareUart_TimeCallback(SoftWareUart_Handle_t *handle);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !SOFTWAREUART_H
