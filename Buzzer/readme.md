
---

# 📘 Buzzer API 使用说明

---

## 📌 返回类型说明

```c
typedef enum
{
    BUZZER_OK,         // 操作成功
    BUZZER_ERROR,     // 空指针、内存失败或其他错误
    BUZZER_TYPE_ERROR, // 类型不合法（既不是有源也不是无源）
} Buzzer_Status_t;
```

---

## 🔧 初始化蜂鸣器

```c
Buzzer_Status_t Buzzer_Init(Buzzer_Handle_t *handle, Buzzer_Conf_t *conf);
```

> 分配内存并初始化蜂鸣器。使用前必须调用。

---

## ⏱ 轮询处理蜂鸣器状态

```c
Buzzer_Status_t Buzzer_TickHandler(Buzzer_Handle_t *handle);
```

> 需在定时器中周期性（1ms）调用，用于驱动蜂鸣器控制逻辑。

---

## 📣 启动蜂鸣器鸣叫

```c
Buzzer_Status_t Buzzer_StartBeep(Buzzer_Handle_t *handle, uint32_t dur, uint32_t rpt, int value);
```

> 开始一次蜂鸣器鸣叫控制：

* `dur`: 每次鸣叫持续时间（ms）
* `rpt`: 鸣叫次数
* `value`: 占空比（仅无源，0–100）

---

## ❌ 释放蜂鸣器对象

```c
Buzzer_Status_t Buzzer_Delete(Buzzer_Handle_t *handle);
```

> 释放内存并清空句柄，避免内存泄漏。

---

