# 日志规范

> 串口调试输出，不是结构化线上日志。统一 `XHH_DEBUG` 宏，编译期开关控制，默认关。

---

## 日志宏

| 宏 | 用途 | 控制开关 |
|----|------|----------|
| `XHH_DEBUG(...)` | 运行时跟踪点、事件/状态切换 | `XHH_DEBUG_EN` |
| `XHH_DEBUG_BLE(...)` | BLE 透传日志 | `XHH_DEBUG_BLE_EN` |
| `PRINT(...)` | 厂商代码启动/调试 | 厂商宏 |

定义在 `APP/main.h` 顶部，编译期控制，默认 0（关闭）：

```c
// APP/main.h:17-39
#define XHH_DEBUG_EN 0
#define XHH_DEBUG_BLE_EN 0

#if XHH_DEBUG_EN
#define XHH_DEBUG(X...) printf(X)
#else
#define XHH_DEBUG(X...) ((void)0)
#endif
```

---

## 用法

短小、诊断价值高，放在状态切换/事件处理/协议收发关键点：

```c
// xhh_Module/xhh_Mode/xhh_Mode.c:38
XHH_DEBUG("s_h:%d-->%d\r\n", xhh_SYS_f, xhh_SYS_n);

// xhh_Module/xhh_Event/xhh_Event.c:28
XHH_DEBUG("e_h:%d\r\n", event_temp);
```

现有日志常用紧凑标记：`e_h`（event handle）、`s_h`（sys handle）。延续这种风格。

---

## 应记录

- 协议 TX/RX（调试期，加开关）
- 事件号、系统状态切换
- 电源/充电/超时切换
- BLE 连接/断开里程碑
- 上板 bring-up 一次性观察

---

## 不应记录

- 长期开启的高频刷屏日志（影响时序）
- 无诊断价值的纯叙述日志
- 未加开关的大块字节 dump

对本固件而言，"日志太多"往往是时序/噪声问题，不是存储问题。

---

## 调试开关约定

- 所有调试输出用编译期宏包裹，默认关闭
- 模块级调试开关命名 `<MODULE>_DEBUG_EN`，如 `BLE_SLAVE_DEBUG_EN`、`BLE_DEBUG_EN`
- 不要用运行时全局变量控制日志开关

---

## 初始化后应补充的项目事实

- 实际日志宏清单
- 实际调试开关名
- 哪些模块允许保留调试日志
