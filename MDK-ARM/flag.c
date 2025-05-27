#include "flag.h"

volatile UserTim_t UserTim = {0}; // 定时器标志

volatile UserCommon_t UserCommon = {0}; // 用户公共体

SoftwareUART_Handle_t SoftUart = NULL;

Buzzer_Handle_t beep = NULL;

RTC_DateTypeDef GetData; // 获取日期结构体

RTC_TimeTypeDef GetTime; // 获取时间结构体


uint8_t uart_index = 0;
uint8_t json_received = 0;

/*******************************************************************************
Name            : void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
Syntax          : HAL_UART_RxCpltCallback(&huart1)
Func		    : 串口回调重定义
Return value    : -
Description     :  stm32xx_it.c调用
Date			: 2025/4/21
|******************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        if (uart_index < UART_BUFFSIZE - 1)
        {
            UserCommon.uartReceive[uart_index++] = UserCommon.rx_byte;

            // 判断是否接收到完整 JSON，最后一个 '}' 结尾
            if (UserCommon.rx_byte == '}')
            {
                UserCommon.uartReceive[uart_index] = '\0';  // 添加字符串结尾
                json_received = 1;
                uart_index = 0;
				UserCommon.flag.isUartReceive = ON;
            }
        }
        else
        {
            // 超过最大长度，清空重来
            uart_index = 0;
        }

        // 继续接收下一个字节
        HAL_UART_Receive_IT(&huart1, &UserCommon.rx_byte, 1);
    }
}

/*******************************************************************************
Name            : -
Syntax          : -
Func		    : 串口重定向
Return value    : -
Description     :  不需要调用此函数
Date			: 2025/4/21
|******************************************************************************/
int fputc(int ch, FILE *f)
{
  static uint16_t delayUtil = 0;

  HAL_UART_Transmit(&huart1, (unsigned char *)&ch, 1, 0xFFFF);

  while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET)
  {
    if (++delayUtil > 2000)
    {
      delayUtil = 0;
      break;
    }
  };

  return ch;
}

/*******************************************************************************
Name            : HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
Syntax          : -
Func		    : 定时器回调函数
Return value    : -
Description     :  此函数不做调用
Date			: 2025/4/21
|******************************************************************************/
// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	if (htim->Instance == TIM1)
//   {
//     SoftwareUART_TickHandler(&SoftUart);
//   }
// }

/*******************************************************************************
Name            : void Set_PWM_DutyCycle(float duty)
Syntax          : Set_PWM_DutyCycle(float duty)
Func		    : 设置PWM的占空比
Return value    : -
Description     :  pwm输出占空比
Date			: 2024/12/11
|******************************************************************************/
void Set_PWM_DutyCycle(float duty)
{
  if (duty > 100)
    duty = 100; // 防止超过 100%
  //	uint32_t compare_value = (htim1.Init.Period + 1) * duty / 100;
  //	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, compare_value);
}

void RunTime(void *p)
{
  while (1)
  {
    //   UserCommon.second++;
    //   if (UserCommon.second >= 60)
    //   {
    //     UserCommon.second = 0;
    // UserCommon.flag.isUpdateTime = ON;
    //     UserCommon.minute++;
    //     if (UserCommon.minute >= 60)
    //     {
    //       UserCommon.minute = 0;
    //       UserCommon.hour++;
    //       if (UserCommon.hour >= 24)
    //       {
    //         UserCommon.hour = 0;
    //       }
    //     }
    //   }
    HAL_RTC_GetTime(&hrtc, &GetTime, RTC_FORMAT_BIN); // 获取时间
    HAL_RTC_GetDate(&hrtc, &GetData, RTC_FORMAT_BIN); // 获取日期
    printf("%d年%d月%d日 %d:%d:%d\n", 2000 + GetData.Year, GetData.Month, GetData.Date, GetTime.Hours, GetTime.Minutes, GetTime.Seconds);
    vTaskDelay(1000);
  }
}

void DisplayTime(void *p)
{
  while (1)
  {
    if (UserCommon.flag.isUpdateTime == ON)
    {
      UserCommon.flag.isUpdateTime = OFF;
      HC595_DisplayNumber(UserCommon.hour * 100 + UserCommon.minute);
    }
  }
}

void LedTask(void *p)
{
  while (1)
  {
    HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    vTaskDelay(500);
  }
}

void UartTest(void *p)
{
  while (1)
  {
    uint8_t data[20] = {0};

    if (SoftwareUART_CheckRevice(SoftUart) == UART_OK)
    {
      sprintf(data, "%d :: %d :: %d", UserCommon.hour, UserCommon.minute, UserCommon.second);
      SoftwareUART_Transmit(SoftUart, data, strlen(data) + 1);
      SoftwareUART_Clearbuffer(&SoftUart);
    }

    //		uint8_t data = 0x03;
    //		SoftwareUART_Sendbyte(SoftUart,data);
    Dealwith_Uartdata();
    vTaskDelay(500);
  }
}

void beepTask(void *p)
{
  Buzzer_TickHandler(&beep);
  vTaskDelay(1);
}
