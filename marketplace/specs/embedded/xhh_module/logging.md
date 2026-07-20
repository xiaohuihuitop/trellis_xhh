# 日志规范

> 串口调试输出，不是结构化线上日志。统一 `XHH_PRINTF` 与 `XHH_DEBUG` 宏，全部受同一个编译期开关控制，默认关闭。

---

## 日志宏

| 宏 | 用途 | 控制开关 |
|----|------|----------|
| `XHH_PRINTF(...)` | 通用日志输出 | `XHH_DEBUG_EN` |
| `XHH_DEBUG(...)` | 调试语义日志；等价转发到 `XHH_PRINTF` | `XHH_DEBUG_EN` |

定义在 `xhh_BSP/xhh_BSP_Def.h`，因此 APP、xhh_Module、Components 和 xhh_BSP 都可使用，且不会产生 `xhh_Module -> APP` 反向依赖：

```c
// xhh_BSP_Def.h
#define XHH_DEBUG_EN 0

#if XHH_DEBUG_EN
#include <stdio.h>
#define XHH_PRINTF(...) printf(__VA_ARGS__)
#define XHH_DEBUG(...) XHH_PRINTF(__VA_ARGS__)
#else
#define XHH_PRINTF(...) ((void)0)
#define XHH_DEBUG(...) ((void)0)
#endif
```

`XHH_DEBUG_EN` 为 1 时，当前 Platform 必须已完成 `printf` 输出重定向；为 0 时两个宏均不产生输出。新代码不直接使用厂商 `PRINT`、`printf` 或其他日志宏。

---

## 用法

短小、诊断价值高，放在状态切换、事件处理、协议收发关键点：

```c
// xhh_Module/xhh_Mode/xhh_Mode.c
XHH_DEBUG("s_h:%d-->%d\r\n", xhh_SYS_f, xhh_SYS_n);

// APP/BLE_HANDLE.c
XHH_PRINTF("ble_rx:%u\r\n", length);
```

现有日志常用紧凑标记：`e_h`（event handle）、`s_h`（sys handle）。延续这种风格。

---

## 应记录

- 协议 TX/RX（调试期，加开关）
- 事件号、系统状态切换
- 电源、充电、超时切换
- BLE 连接、断开里程碑
- 上板 bring-up 一次性观察

---

## 不应记录

- 长期开启的高频刷屏日志（影响时序）
- 无诊断价值的纯叙述日志
- 未加开关的大块字节 dump

对本固件而言，“日志太多”往往是时序或噪声问题，不是存储问题。

---

## 调试开关约定

- 所有新日志统一使用 `XHH_PRINTF` 或 `XHH_DEBUG`，并受 `XHH_DEBUG_EN` 控制，默认关闭
- 不新增模块级日志开关，避免多个开关组合导致日志行为不可预测
- 不要用运行时全局变量控制日志开关

---

## 初始化后应补充的项目事实

- `printf` 输出重定向位置
- 哪些模块允许保留调试日志
