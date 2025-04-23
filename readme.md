# 基于 STM32F103C8T6 的电子时钟设计

> **说明：** 本项目以 STM32F103C8T6 为主控，驱动 74HC595 数码管显示时间。重点在于演示如何将 `FreeRTOS` 移植到用户工程中，而非电子表功能本身。

---

## 🚀 FreeRTOS 移植步骤

### 1. 添加 FreeRTOS 源码

- 将 `FreeRTOS` 源码目录复制到你的工程目录中。

### 2. 在 Keil 中创建分组

- 创建两个新的代码组：`FreeRTOS_Src` 和 `FreeRTOS_Port`
- 将 `FreeRTOS` 根目录下所有 `.c` 文件添加至 `FreeRTOS_Src`
- 将 `FreeRTOS/portable/MemMang/heap_4.c` 添加至 `FreeRTOS_Port`
- 从 `FreeRTOS/portable/RVDS/ARM_CM3` 中添加 `port.c` 到 `FreeRTOS_Port`（针对 Cortex-M3）

### 3. 设置 Include 路径

- 打开 Keil 魔术棒 → `C/C++` → 添加以下路径：
  - `FreeRTOS/include`
  - `FreeRTOS/portable/RVDS/ARM_CM3`

### 4. 配置 CubeMX

- 在 CubeMX 中：
  - **SysTick** 不可用作 FreeRTOS 的时基；使用一个 **硬件定时器** 替代
  - **NVIC** 设置：
    - `Priority Group` 设为 **4 bits**
    - 勾选 `Sort by Pre-emption Priority and Sub Priority`

---

## ⚙️ 中断服务函数修改（stm32f1xx_it.c）

### 1. 添加外部函数声明

```c
/* USER CODE BEGIN EV */
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);
extern void vPortSVCHandler(void);
/* USER CODE END EV */
```

### 2. 修改系统中断回调

#### `SVC_Handler`
```c
void SVC_Handler(void)
{
  vPortSVCHandler();
}
```

#### `PendSV_Handler`
```c
void PendSV_Handler(void)
{
  xPortPendSVHandler();
}
```

#### `SysTick_Handler`
```c
void SysTick_Handler(void)
{
  xPortSysTickHandler();
}
```

---

## 📦 创建 FreeRTOS 任务示例

### 任务函数
```c
void LedTask(void *p)
{
    while (1)
    {
        HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
```

### 主函数中创建任务
```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    xTaskCreate(LedTask, "LED", 128, NULL, 2, NULL);
    vTaskStartScheduler();

    while (1) {}
}
```

---

## 📝 注意事项

- 请确保 `configTOTAL_HEAP_SIZE` 在 `FreeRTOSConfig.h` 中配置合理
- 如果使用硬件定时器替代 SysTick，请在 FreeRTOS 配置中启用 `configUSE_TICKLESS_IDLE` 选项（可选）
- 如果中断优先级设置不当，可能导致系统进入 HardFault

---

> 📁 本项目适用于学习 FreeRTOS 在 STM32 上的最小系统移植，有助于后续进行多任务控制的应用开发。

