#include "SoftWareUart.h"

/**
 * @brief
 *
 * @param handle
 * @param nTime
 */
static void SoftWareUart_delayus(SoftWareUart_Handle_t handle, volatile uint32_t nTime)
{
    if (handle == NULL)
        return;
    uint16_t tmp;

    tmp = __HAL_TIM_GetCounter(handle->HardWare.DelayHtime);

    if (tmp + nTime <= 65535)
    {
        while ((__HAL_TIM_GetCounter(handle->HardWare.DelayHtime) - tmp) < nTime)
            ;
    }
    else
    {
        __HAL_TIM_SET_COUNTER(handle->HardWare.DelayHtime, 0); // 修正！
        while (__HAL_TIM_GetCounter(handle->HardWare.DelayHtime) < nTime)
            ;
    }
}

/**
 * @brief
 *
 * @param handle
 * @param byte
 */
SoftWareUart_Status_t SoftWareUart_Sendbyte(SoftWareUart_Handle_t handle, uint8_t byte)
{
    if (handle == NULL)
        return UART_ERROR;
    uint32_t i, tmp;
    // 开始位
    // iouart1_TXD(0); //将TXD的引脚的电平置低
    HAL_GPIO_WritePin(handle->HardWare.TXport, handle->HardWare.TXpin, GPIO_PIN_RESET);

    SoftWareUart_delayus(handle, handle->baud);
    for (i = 0; i < 8; i++)
    {
        tmp = (byte >> i) & 0x01;
        if (tmp == 0)
        {
            HAL_GPIO_WritePin(handle->HardWare.TXport, handle->HardWare.TXpin, GPIO_PIN_RESET);
            SoftWareUart_delayus(handle, handle->baud);
        }
        else
        {
            //   iouart1_TXD(1);
            HAL_GPIO_WritePin(handle->HardWare.TXport, handle->HardWare.TXpin, GPIO_PIN_SET);
            SoftWareUart_delayus(handle, handle->baud);
        }
    }
    // 结束位
    // iouart1_TXD(1);//将TXD的引脚的电平置?
    HAL_GPIO_WritePin(handle->HardWare.TXport, handle->HardWare.TXpin, GPIO_PIN_SET);
    SoftWareUart_delayus(handle, handle->baud);
    return UART_OK;
}

/**
 * @brief
 *
 * @param handle
 * @param buffer
 * @param size
 */
SoftWareUart_Status_t SoftWareUart_SendBuffer(SoftWareUart_Handle_t handle, const uint8_t *buffer, size_t size)
{
    if (handle == NULL)
        return UART_ERROR;
    for (int i = 0; i < size; i++)
    {
        SoftWareUart_Sendbyte(handle, buffer[i]);
    }
    return UART_OK;
}
/**
 * @brief
 *
 * @param handle
 * @param conf
 */
SoftWareUart_Status_t SoftWareUART_Init(SoftWareUart_Handle_t *handle, SoftWareUart_Conf_t *conf)
{
    if (handle == NULL || conf == NULL)
        return UART_ERROR;

    if (*handle != NULL)
        return UART_ERROR;

    *handle = (SoftWareUart_Handle_t)calloc(1, sizeof(SoftWareUart_t));
    if (*handle == NULL)
        return UART_ERROR;
    (*handle)->baud = conf->baud;
    (*handle)->rxbuffer = conf->rxbuffer;
    (*handle)->rx_size = conf->rx_size;
    (*handle)->HardWare.TXport = conf->HardWare.TXport;
    (*handle)->HardWare.RXport = conf->HardWare.RXport;
    (*handle)->HardWare.TXpin = conf->HardWare.TXpin;
    (*handle)->HardWare.RXpin = conf->HardWare.RXpin;
    (*handle)->HardWare.DelayHtime = conf->HardWare.DelayHtime;
    (*handle)->HardWare.InterruptHtime = conf->HardWare.InterruptHtime;
    (*handle)->recvData = 0;
    (*handle)->recvStat = COM_STOP_BIT;
    HAL_TIM_Base_Start((*handle)->HardWare.DelayHtime);
    return UART_OK;
}

/**
 * @brief
 *
 * @param handle
 */
void SoftWareUart_TimeCallback(SoftWareUart_Handle_t *handle)
{
    if (handle == NULL || *handle == NULL)
        return;

    SoftWareUart_Handle_t uart = *handle;

    uart->recvStat++; // recvStat 作为采样第几位计数器

    if (uart->recvStat <= 8) // 只接收8个数据位
    {
        if (HAL_GPIO_ReadPin(uart->HardWare.RXport, uart->HardWare.RXpin))
        {
            uart->recvData |= (1 << (uart->recvStat - 1)); // 按位存到recvData
        }
        else
        {
            uart->recvData &= ~(1 << (uart->recvStat - 1));
        }
    }
    else
    {
        // 接收结束了（通常第9次是停止位，可以检查也可以直接结束）
        if(uart->rxbuffer != NULL)
        {
            uart->rxbuffer[uart->rx_write_index++] = uart->recvData;
            if (uart->rx_write_index >= uart->rx_size)
            {
                uart->rx_write_index = 0; // 循环
            }
        }
        HAL_TIM_Base_Stop_IT(uart->HardWare.InterruptHtime); // 停止计时器中断
        uart->recvStat = COM_STOP_BIT;                       // 重置状态，准备下一次接收
    }
}

void SoftWareUart_RXCallback(SoftWareUart_Handle_t *handle)
{
    if (handle == NULL || *handle == NULL)
        return;

    SoftWareUart_Handle_t uart = *handle;

    if (HAL_GPIO_ReadPin(uart->HardWare.RXport, uart->HardWare.RXpin) == 0)
    {
        if (uart->recvStat == COM_STOP_BIT) // 空闲状态，发现起始位
        {
            uart->recvStat = COM_START_BIT;                       // 标记开始采样
            uart->recvData = 0;                                   // 清空上次接收的数据
            if(!uart->flag.Firstdata)
            {
                uart->flag.Firstdata = 1;
                SoftWareUart_delayus(uart,uart->baud * 1.5); 
            } else {
                SoftWareUart_delayus(uart,uart->baud / 2);        					 // 跳过起始位等待
            }

            HAL_TIM_Base_Start_IT(uart->HardWare.InterruptHtime); // 开启定时器中断，定时采样
        }
    }
}