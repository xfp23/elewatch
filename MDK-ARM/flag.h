#ifndef __FLAG_H
#define __FLAG_H

#pragma anon_unions

#include "main.h"
//#include "tim.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "Logic.h"
#include "math.h"
#include "usart.h"
#include "SoftwareUART.h"
//#include "Logic.h"
//#include "cJSON.h"
#include "stdint.h"
#include "74HC595.h"
#include "FreeRTOSConfig.h" 
#include "FreeRTOS.h" 
#include "task.h"
#include "W25Q32.h"
#include "buzzer.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define UART_BUFFSIZE 128

#define ON (1)
#define OFF (0)

typedef union
{
    struct
    {
        uint8_t systim_1ms : 1;
        uint8_t systim_5ms : 1;
        // uint8_t systim_10ms : 1;
        uint8_t systim_100ms : 1;
        uint8_t systim_250ms : 1;
        uint8_t systim_500ms : 1;
        uint8_t systim_1000ms : 1;
        uint8_t Reserve_bit : 2; // 保留位，没有实际意义
    } bytes;
} UserTim_t;

typedef struct
{
	uint8_t isUpdateTime :1;   // 是否更新
    uint8_t isUartReceive : 1; // 串口是否接收到数据
    uint8_t Reserve_bits : 6;
} UserFlag_t;

typedef enum
{
    USER_MODE_SWING, // 摇摆模式
    USER_MODE_ROCK   // 晃荡模式
} UserMode_t;

typedef union
{
    struct
    {
        UserFlag_t flag;         // 标志位
        uint8_t minute;         // 分钟
		uint8_t second;          // 秒
		uint8_t hour;            // 小时
        uint8_t uartReceive[UART_BUFFSIZE];
    };

} UserCommon_t;

extern volatile UserTim_t UserTim;       // 系统定时器
extern volatile UserCommon_t UserCommon; // 用户公共体
extern SoftwareUART_Handle_t SoftUart;

extern void Set_PWM_DutyCycle(float duty);
//extern void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
extern void RunTime(void *p);
extern void LedTask(void *p);
extern void DisplayTime(void *p);
extern void UartTest(void *p);

#ifdef __cplusplus
}
#endif

#endif
