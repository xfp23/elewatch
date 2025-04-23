#include "flag.h"

volatile UserTim_t UserTim = {0}; // 定时器标志

volatile UserCommon_t UserCommon = {0}; // 用户公共体

/*******************************************************************************
Name            : void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
Syntax          : HAL_UART_RxCpltCallback(&huart1)
Func		    : 串口回调重定义
Return value    : -
Description     :  stm32xx_it.c调用
Date			: 2025/4/21
|******************************************************************************/
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART1)
//    {

//        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) != RESET)
//        {
//            __HAL_UART_CLEAR_IDLEFLAG(&huart1);
//            HAL_UART_DMAStop(&huart1); // 停止dma接收
//            UserCommon.flag.isUartReceive = ON;
//            HAL_UART_Receive_DMA(&huart1, (uint8_t*)UserCommon.uartReceive, UART_BUFFSIZE); // 重开dma
//        }
//    }
//}


/*******************************************************************************
Name            : -
Syntax          : -
Func		    : 串口重定向
Return value    : -
Description     :  不需要调用此函数
Date			: 2025/4/21
|******************************************************************************/
//int fputc(int ch, FILE *f)
//{
//    static uint16_t delayUtil = 0;

//    HAL_UART_Transmit(&huart1, (unsigned char *)&ch, 1, 0xFFFF);

//    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) != SET)
//    {
//        if (++delayUtil > 2000)
//        {
//            delayUtil = 0;
//            break;
//        }
//    };

//    return ch;
//}

/*******************************************************************************
Name            : HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
Syntax          : -
Func		    : 定时器回调函数
Return value    : -
Description     :  此函数不做调用
Date			: 2025/4/21
|******************************************************************************/
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//    static uint8_t sys1ms_count = 0;
//    static uint8_t sys5ms_count = 0;
//    static uint8_t sys100ms_count = 0;
//    static uint8_t sys250ms_count = 0;
//    static uint16_t sys500ms_count = 0;
//    static uint16_t sys1000ms_count = 0;
//    if (htim->Instance == TIM1)
//    {

//        if (++sys1ms_count >= 1)
//        {
//            sys1ms_count = 0;
//            UserTim.bytes.systim_1ms = ON;
//        }
//        if (++sys5ms_count >= 5)
//        {
//            sys5ms_count = 0;
//            UserTim.bytes.systim_5ms = ON;
//        }

//        if (++sys100ms_count >= 100)
//        {
//            sys100ms_count = 0;
//            UserTim.bytes.systim_100ms = ON;
//        }
//        if (++sys250ms_count >= 250)
//        {
//            sys250ms_count = 0;
//            UserTim.bytes.systim_250ms = ON;
//        }
//        if (++sys500ms_count >= 500)
//        {
//            sys500ms_count = 0;
//            UserTim.bytes.systim_500ms = ON;
//        }

//        if (++sys1000ms_count >= 1000)
//        {
//            UserTim.bytes.systim_1000ms = ON;
//			sys1000ms_count = 0;
////			UserCommon.flag.isUpdateTime = ON;
////		if(++UserCommon.second >= 60)
////		{
////			UserCommon.second = 0;
//////			UserCommon.flag.isUpdateTime = ON;
////		if(++UserCommon.minute >= 60)
////		{
////			UserCommon.minute = 0;
////			
////		if(++UserCommon.hour >= 24)
////		{
////			UserCommon.hour = 0;
////		}
////		}
////		}

//        }
//    }
//}

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
    while(1)
    {
	  UserCommon.flag.isUpdateTime = ON;
      UserCommon.second++;
      if (UserCommon.second >= 60)
      {
        UserCommon.second = 0;
        UserCommon.minute++;
        if (UserCommon.minute >= 60)
        {
          UserCommon.minute = 0;
          UserCommon.hour++;
          if (UserCommon.hour >= 24)
          {
            UserCommon.hour = 0;
          }
        }
      }
	  vTaskDelay(1000);

    }
}

void DisplayTime(void *p)
{
    while(1)
    {
      if (UserCommon.flag.isUpdateTime == ON)
      {
        UserCommon.flag.isUpdateTime = OFF;
        HC595_DisplayNumber(UserCommon.minute * 100 + UserCommon.second);
      }
    }
}


void LedTask(void *p)
{
    while(1)
    {
		HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
		vTaskDelay(500);
    }
}

