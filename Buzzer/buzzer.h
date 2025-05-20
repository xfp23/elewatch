#ifndef __BUZZER_H
#define __BUZZER_H

#pragma anon_unions

/**
 * @file buzzer.h
 * @author https://github.com/xfp23
 * @brief 蜂鸣器驱动
 * @note 本库不会帮你初始化任何硬件外设
 * 如果使用无源蜂鸣器，请准备好定时器初始化配置
 * 本库的设计初衷是为了避免传统的驱动方式阻塞主程序
 * @version 0.1
 * @date 2025-05-20
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "stdint.h"
#include "main.h"
#include "tim.h"

#ifdef __cplusplus
extern "C"
{
#endif // !__cplusplus

typedef enum
{
    BUZZER_OK,
    BUZZER_ERROR,
    BUZZER_TYPE_ERROR,
} Buzzer_Status_t;

enum
{
    BUZZER_OFF,
    BUZZER_ON,
};

typedef enum
{
    BUZZER_PASSIVE = 0x01, // 无源蜂鸣器
    BUZZER_ACTIVE          // 有源蜂鸣器
} Buzzer_Type_t;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} Buzzer_GPIO_t;

typedef union
{
    uint8_t full;
    struct
    {
        uint8_t EN : 1;           // 蜂鸣器使能
        uint8_t CNT_INT : 1;      // 初始化计数器
        uint8_t BEEP : 1;         // 蜂鸣
        uint8_t Reserve_bits : 5; // 保留位
    };

} Buzzer_Flag_t;

typedef struct
{
    Buzzer_Type_t type;     // 蜂鸣器类型
    Buzzer_GPIO_t beep;     // 蜂鸣器引脚
    TIM_HandleTypeDef *tim; // 定时器
    unsigned int channel;   // 定时器输出通道
} Buzzer_Conf_t;

typedef struct
{
    Buzzer_GPIO_t beep;     // 蜂鸣器引脚
    TIM_HandleTypeDef *tim; // 定时器
    unsigned int channel;   // 定时器输出通道
} Buzzer_HardWare_t;

typedef struct
{
    Buzzer_Type_t type; // 蜂鸣器类型
    Buzzer_HardWare_t Hardware;
    Buzzer_Flag_t flag; // 标志
    unsigned int dur;   // duration，单次响声持续时间（单位：ms）
    unsigned int rpt;   // repeat，响的次数
    unsigned int beep_1ms_count;

} Buzzer_Obj;

typedef Buzzer_Obj *Buzzer_Handle_t; // 句柄

/**
 * @brief 初始化蜂鸣器
 *
 * @param handle 句柄地址
 * @param conf 配置结构体地址
 * @return Buzzer_Status_t 状态
 */
extern Buzzer_Status_t Buzzer_Init(Buzzer_Handle_t *handle, Buzzer_Conf_t *conf);

/**
 * @brief 蜂鸣器回调，将此函数放入1ms定时器中断中
 *
 * @param handle 句柄地址
 * @return Buzzer_Status_t 状态
 */
extern Buzzer_Status_t Buzzer_TickHandler(Buzzer_Handle_t *handle);

/**
 * @brief 设置蜂鸣器蜂鸣次数与时长
 *
 * @param handle 句柄地址
 * @param dur 单次响声持续时间（单位：ms）
 * @param rpt 响的次数
 * @param value 音量，仅在 BUZZER_PASSIVE 类型下有效，不想设置请传递 -1
 * @return Buzzer_Status_t 状态
 */
extern Buzzer_Status_t Buzzer_StartBeep(Buzzer_Handle_t *handle, uint32_t dur, uint32_t rpt, int value);

/**
 * @brief 删除蜂鸣器
 *
 * @param handle 句柄地址
 * @return Buzzer_Status_t 状态
 */
extern Buzzer_Status_t Buzzer_Delete(Buzzer_Handle_t *handle);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !__BUZZER_H
