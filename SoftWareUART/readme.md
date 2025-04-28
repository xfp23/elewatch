
---

# 📚 SoftwareUART 软件串口驱动库 API 文档

---

## 简介
SoftwareUART 是基于 STM32 HAL库的一个软件实现的 UART 驱动库，适用于没有硬件串口或者串口资源受限的场景。  
需要预配置两个定时器以及相关 GPIO，支持基本的单字节、缓冲区收发与状态检测。

---

## 初始化配置

### `SoftwareUART_Init()`
> 软件串口初始化

- **函数原型：**
  ```c
  SoftwareUART_Status_t SoftwareUART_Init(SoftwareUART_Handle_t *handle, SoftwareUART_Conf_t *conf);
  ```
- **参数说明：**
  - `handle`：软件串口句柄指针
  - `conf`：串口配置信息（包括波特率、硬件资源等）
- **返回值：**
  - `UART_OK`：初始化成功
  - `UART_ERROR`：初始化失败

---

## 数据发送

### `SoftwareUART_Sendbyte()`
> 发送单个字节

- **函数原型：**
  ```c
  SoftwareUART_Status_t SoftwareUART_Sendbyte(SoftwareUART_Handle_t handle, uint8_t byte);
  ```
- **参数说明：**
  - `handle`：软件串口句柄
  - `byte`：待发送的单个字节
- **返回值：**
  - `UART_OK`：发送成功
  - `UART_ERROR`：发送失败

---

### `SoftwareUART_SendBuffer()`
> 发送一段缓冲区数据

- **函数原型：**
  ```c
  SoftwareUART_Status_t SoftwareUART_SendBuffer(SoftwareUART_Handle_t handle, const uint8_t *buffer, size_t size);
  ```
- **参数说明：**
  - `handle`：软件串口句柄
  - `buffer`：数据缓冲区指针
  - `size`：缓冲区大小（字节数）
- **返回值：**
  - `UART_OK`：发送成功
  - `UART_ERROR`：发送失败

---

## 接收相关

### `SoftwareUART_RXCallback()`
> 外部中断回调（接收起始位触发）

- **函数原型：**
  ```c
  void SoftwareUART_RXCallback(SoftwareUART_Handle_t *handle);
  ```
- **使用说明：**
  - 需要在 RX 引脚的下降沿中断中调用。
  - 通常放置在 `stm32xx_it.c` 的外部中断处理函数中。

---

### `SoftwareUART_TimeCallback()`
> 定时器中断回调（接收后续数据位）

- **函数原型：**
  ```c
  void SoftwareUART_TimeCallback(SoftwareUART_Handle_t *handle);
  ```
- **使用说明：**
  - 需要在主定时器（InterruptHtime）的中断服务函数中调用。
  - 用于接收数据流中的各个位。

---

### `SoftwareUART_CheckRevice()`
> 检查是否接收到完整的一帧数据

- **函数原型：**
  ```c
  SoftwareUART_Status_t SoftwareUART_CheckRevice(SoftwareUART_Handle_t handle);
  ```
- **参数说明：**
  - `handle`：软件串口句柄
- **返回值：**
  - `UART_OK`：接收到完整数据，用户可以读取缓冲区
  - `UART_ERROR`：尚未接收到完整数据

- **注意事项：**
  - 调用频率不要太高，建议配合主循环定时调用或在定时任务中调用。

---

### `SoftwareUART_Clearbuffer()`
> 清空接收缓冲区

- **函数原型：**
  ```c
  SoftwareUART_Status_t SoftwareUART_Clearbuffer(SoftwareUART_Handle_t *handle);
  ```
- **参数说明：**
  - `handle`：软件串口句柄
- **返回值：**
  - `UART_OK`：清除成功
  - `UART_ERROR`：清除失败

- **使用场景：**
  - 当数据处理完成后，手动清空接收缓冲区，准备接收下一帧数据。

---

## 支持的波特率

目前支持以下波特率：
- 9600 bps
- 19200 bps

（通过配置定时器重装载值 ARR 实现，枚举值定义在 `SoftwareUART_baud_t`）

---

## 主要数据结构

- `SoftwareUART_Handle_t`  
  软件串口操作句柄。
  
- `SoftwareUART_Conf_t`  
  初始化配置参数（包含波特率、定时器、GPIO引脚等）。

- `SoftwareUART_Status_t`  
  返回状态枚举（`UART_OK`、`UART_ERROR`）。

- `SoftWare_Bit_t`  
  接收状态位（起始位、数据位、停止位）。

---

## 使用流程示例

```c
SoftwareUART_Handle_t huart;
SoftwareUART_Conf_t conf = {
    .baud = BITS_9600,
    .HardWare = {
        .TXport = GPIOA,
        .TXpin = GPIO_PIN_9,
        .RXport = GPIOA,
        .RXpin = GPIO_PIN_10,
        .DelayHtime = &htim1,
        .InterruptHtime = &htim2,
    },
    .rxbuffer = rx_buf,
    .rx_size = sizeof(rx_buf),
};

SoftwareUART_Init(&huart, &conf);

// 外部中断和定时器回调中分别调用
// SoftwareUART_RXCallback(&huart);
// SoftwareUART_TimeCallback(&huart);
```

---

# 🔥 额外提醒
- `DelayHtime` 定时器不需要开中断，只用来计时。
- `InterruptHtime` 定时器必须开中断，并且频率设置为 1MHz。
- 使用时注意中断优先级，避免和其他高优先级外设冲突。

---