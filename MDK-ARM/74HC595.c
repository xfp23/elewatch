#include "main.h"
#include "74HC595.h"
// 段码定义：0~9（共阳极，0 点亮为低电平）
 static const uint8_t num_table[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90}; // 低电平点亮

// 段码定义: 0~9 (共阴极, 1 点亮为高电平)
// static const uint8_t num_table[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f}; // 高电平点亮

// 初始化函数
void HC595_Init(void)
{
//    HC595_OE_ENABLE();
//#if HC595_OENUM != 1
//    HC595_OE1_ENABLE();
//#endif
//    HC595_CLR_DISABLE();
    HC595_RCLK_L();
    HC595_SRCLK_L();
    HC595_SER_L();
}

// 写一个字节进 595
void HC595_WriteByte(uint8_t data)
{
    for (int i = 0; i < 8; i++)
    {
        if (data & 0x80)
            HC595_SER_H();
        else
            HC595_SER_L();

        data <<= 1;

        HC595_SRCLK_H();
        HC595_SRCLK_L();
    }

    HC595_RCLK_H();
    HC595_RCLK_L();
}

// 写两个字节，段选+位选
void HC595_WriteSegmentAndDigit(uint8_t seg, uint8_t digitSel)
{
    HC595_WriteByte(seg);       // 段码（第一个 595）
    HC595_WriteByte(digitSel);  // 位码（第二个 595）
}


// 显示 4 位数字（数码管从左到右）
void HC595_DisplayNumber(int number)
{
    uint8_t digits[4] = {0};

    // 拆分数字（从低位到高位）
    digits[3] = number % 10;
    digits[2] = (number / 10) % 10;
    digits[1] = (number / 100) % 10;
    digits[0] = (number / 1000) % 10;

    // 写入段码（从左到右分别送给第1~第4个595）
    // 注意：74HC595 是先送最高位的数据（最左边的数码管），后送最低位（最右边）
    HC595_WriteByte(num_table[digits[3]]); // 最右边的数码管
    HC595_WriteByte(num_table[digits[2]]);
    HC595_WriteByte(num_table[digits[1]]& ~0x80);
    HC595_WriteByte(num_table[digits[0]]); // 最左边的数码管
}




/**
 * @brief 通用显示两位数字（指定区域）
 * 
 * @param num      两位数字
 * @param area     显示区域，枚举类型 DisplayArea
 */
void HC595_DisplayByArea(int num, DisplayArea_t area)
{
    uint8_t digits[2] = {0};
    uint8_t segment, digitSel;

    digits[0] = num % 10;      // 个位
    digits[1] = num / 10;      // 十位

    for (uint8_t i = 0; i < 2; i++)
    {
        segment = num_table[digits[i]];
        digitSel = ~(1 << (i + (uint8_t)area));  // 按区域偏移位选
        HC595_WriteSegmentAndDigit(segment, digitSel);
        HAL_Delay(1);
    }
}
