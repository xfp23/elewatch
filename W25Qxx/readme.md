
---

# 📘 W25Qxx 驱动 API 文档

> 作者：[xfp23](https://github.com/xfp23)
> 驱动适用于 W25Qxx 系列 SPI Flash，**不包含 GPIO 与 SPI 初始化**。
> SPI 最大频率：104 MHz，CPOL = 0，CPHA = 1（低电平空闲，第二边沿采样）

---

## 初始化与卸载

```c

W25Qxx_Handle_t handle = NULL; // 每个句柄对应一个设备

```

### `W25Qxx_Status_t W25Qxx_Init(W25Qxx_Handle_t *handle, W25Qxx_Conf_t *conf);`

初始化 W25Qxx 芯片。

### `W25Qxx_Status_t W25QXX_Delete(W25Qxx_Handle_t *handle);`

释放资源，卸载驱动。

---

## 读写操作

### `W25Qxx_Status_t W25Qxx_Read(W25Qxx_Handle_t *handle, uint8_t *buffer, uint32_t addr, size_t size);`

读取数据。

### `W25Qxx_Status_t W25Qxx_Write(W25Qxx_Handle_t *handle, uint8_t *buffer, uint32_t addr, size_t size);`

写入数据（自动分页）。

### `W25Qxx_Status_t W25QXX_WritePage(W25Qxx_Handle_t *handle, uint8_t *buffer, uint32_t addr, size_t size);`

向指定页写入数据（最多 256 字节，起始地址需页对齐）。

### `W25Qxx_Status_t W25QXX_WriteNoCheck(W25Qxx_Handle_t *handle, uint8_t *buffer, uint32_t addr, size_t size);`

无擦除判断直接写入，使用前需手动擦除。

---

## 擦除操作

### `W25Qxx_Status_t W25Qxx_EraseSector(W25Qxx_Handle_t *handle, uint32_t addr);`

擦除 4KB 扇区。

### `W25Qxx_Status_t W25Qxx_EraseChip(W25Qxx_Handle_t *handle);`

擦除整个芯片。

---

## 信息获取

### `W25Qxx_Status_t W25Qxx_ReadID(W25Qxx_Handle_t *handle, W25Qxx_ID_t *buffer);`

获取 JEDEC ID 与设备型号。

### `W25Qxx_Status_t W25Qxx_ReadCapcity(W25Qxx_Handle_t *handle, uint32_t *buffer);`

获取芯片容量（单位：字节）。

---

## 电源管理

### `W25Qxx_Status_t W25Qxx_WeekUP(W25Qxx_Handle_t *handle);`

唤醒芯片。

### `W25Qxx_Status_t W25Qxx_Powerdown(W25Qxx_Handle_t *handle);`

进入掉电休眠状态。

---

## 枚举与状态

```c
typedef enum
{
    W25QXX_OK,       // OK
    W25QXX_ERROR,    // 一般错误
    W25QXX_SPIERROR, // SPI错误
} W25Qxx_Status_t;
```

---

## 注意事项

* 写入前必须擦除对应区域，除非使用 `W25QXX_WriteNoCheck`；
* 所有读写地址必须在芯片有效范围内；
* 初始化前需先配置 SPI 与引脚（驱动不负责初始化硬件）；
* 推荐 SPI 模式 0（CPOL=0，CPHA=1）；
* 频率建议小于或等于 80 MHz，最大可支持 104 MHz（具体看芯片型号和板级条件）；

---
