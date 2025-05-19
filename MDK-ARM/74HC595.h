#ifndef __HC595_H
#define __HC595_H

#include "stdint.h"

//#define HC595_OENUM 1 // OE 1 为单片 0 为多片 
// 用户根据接线修改这些宏定义



// 串行数据输入，接 MCU
#define __74CH595_SER_GPIO_Port     GPIOB
#define __74CH595_SER_Pin           GPIO_PIN_5

// 移位时钟，接 MCU
#define __74CH595_SRCLK_GPIO_Port   GPIOB
#define __74CH595_SRCLK_Pin         GPIO_PIN_4

// 锁存时钟，接 MCU
#define __74CH595_RCLK_GPIO_Port    GPIOB
#define __74CH595_RCLK_Pin          GPIO_PIN_6

// 输出使能，一般接地（低电平有效）
//#define __74CH595_OE_GPIO_Port      GPIOB
//#define __74CH595_OE_Pin            GPIO_PIN_4

// #define __74595_SRCLR_GPIO_Port   GPIOA
// #define __74CH595_SRCLR_Pin         GPIO_PIN_4


//#if HC595_OENUM != 1
//#define HC595_OE1_GPIO_Port      GPIOA
//#define HC595_OE1_Pin            GPIO_PIN_5
//#endif
typedef enum
{
    DISPLAY_T1 = 0, // Q0、Q1（数码管3、4）
    DISPLAY_T2 = 2, // Q2、Q3（数码管5、6）
    DISPLAY_T3 = 4  // Q4、Q5（数码管7、8）
} DisplayArea_t;


// 控制宏
#define HC595_SER_H()      HAL_GPIO_WritePin(__74CH595_SER_GPIO_Port, __74CH595_SER_Pin, GPIO_PIN_SET)
#define HC595_SER_L()      HAL_GPIO_WritePin(__74CH595_SER_GPIO_Port, __74CH595_SER_Pin, GPIO_PIN_RESET)

#define HC595_SRCLK_H()    HAL_GPIO_WritePin(__74CH595_SRCLK_GPIO_Port, __74CH595_SRCLK_Pin, GPIO_PIN_SET)
#define HC595_SRCLK_L()    HAL_GPIO_WritePin(__74CH595_SRCLK_GPIO_Port, __74CH595_SRCLK_Pin, GPIO_PIN_RESET)

#define HC595_RCLK_H()     HAL_GPIO_WritePin(__74CH595_RCLK_GPIO_Port, __74CH595_RCLK_Pin, GPIO_PIN_SET)
#define HC595_RCLK_L()     HAL_GPIO_WritePin(__74CH595_RCLK_GPIO_Port, __74CH595_RCLK_Pin, GPIO_PIN_RESET)

#define HC595_OE_ENABLE()  HAL_GPIO_WritePin(__74CH595_OE_GPIO_Port, __74CH595_OE_Pin, GPIO_PIN_RESET) // OE低有效
#define HC595_OE_DISABLE() HAL_GPIO_WritePin(__74CH595_OE_GPIO_Port, __74CH595_OE_Pin, GPIO_PIN_SET)

//#if HC595_OENUM != 1
//#define HC595_OE1_ENABLE()  HAL_GPIO_WritePin(HC595_OE1_GPIO_Port, HC595_OE1_Pin, GPIO_PIN_RESET) // OE低有效
//#define HC595_OE1_DISABLE() HAL_GPIO_WritePin(HC595_OE1_GPIO_Port, HC595_OE1_Pin, GPIO_PIN_SET)
//#endif

// #define HC595_CLR_ENABLE() HAL_GPIO_WritePin(__74595_SRCLR_GPIO_Port, __74CH595_SRCLR_Pin, GPIO_PIN_RESET)
// #define HC595_CLR_DISABLE() HAL_GPIO_WritePin(__74595_SRCLR_GPIO_Port, __74CH595_SRCLR_Pin, GPIO_PIN_SET)

extern void HC595_Init(void);
extern void HC595_WriteByte(uint8_t data);
extern void HC595_DisplayNumber(int number);
extern void HC595_DisplayByArea(int num, DisplayArea_t area);

#endif
