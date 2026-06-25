# 状态机模式

> 系统级状态机用"枚举状态 + 枚举子步 + Loop_Count 计数 + 单 switch"范式。转换集中在一个函数，在主循环周期推进。

---

## 范式四要素

1. **枚举状态** `xxx_t`：定义系统所有可能状态
2. **枚举子步** `xxx_Step_t`：每个状态内的执行阶段（Entry / Ing / Done）
3. **Loop_Count 计数**：在 Ing 阶段计时/计次，到点触发转换
4. **单 switch**：一个 `xhh_SYS_Handle()` 内 switch 状态，每个 case 内再分支 step

不用函数指针表、不用状态结构体表驱动。理由：switch 直读、调试可跟、嵌入式编译器优化友好。

---

## 枚举定义

```c
// xhh_Module/xhh_Mode/xhh_Mode.h:19-39
typedef enum {
    xhh_SYS_Null = 0, xhh_SYS_Init, xhh_SYS_PowerON, xhh_SYS_PowerOFF,
    xhh_SYS_Run, xhh_SYS_Bright, xhh_SYS_Charge, xhh_SYS_ERR,
    xhh_SYS_Wake, xhh_SYS_PP_Start, xhh_SYS_PP_Stop
} xhh_SYS_t;

typedef enum {
    xhh_SYS_Step_Entry = 0,   // 进入状态首做的事
    xhh_SYS_Step_Ing,         // 持续运行/等待
    xhh_SYS_Step_Done,        // 收尾（可选）
} xhh_SYS_Step_t;
```

---

## 集中转换函数

状态转换**只**通过 `xhh_SYS_Change(new)` 触发，它同时做四件事：

```c
// xhh_Module/xhh_Mode/xhh_Mode.c:31-39
void xhh_SYS_Change(xhh_SYS_t new_sys)
{
    xhh_SYS_f = xhh_SYS_n;          // 保存前一状态（former）
    xhh_SYS_n = new_sys;            // 切到新状态
    xhh_SYS_Step_n = xhh_SYS_Step_Entry;  // 重置子步
    xhh_SYS_Loop_Count = 0;         // 重置计数
    XHH_DEBUG("s_h:%d-->%d\r\n", xhh_SYS_f, xhh_SYS_n);
}
```

- `xhh_SYS_f` 保留前一状态，供回溯判断（如"从 PowerON 来的"）
- 不要直接写 `xhh_SYS_n = xxx;` 绕过 `Change`，会漏掉 step/count 重置

转换触发点有两类：
1. 事件层 `xhh_Event_Handle` 内（外部输入驱动）：`xhh_Event.c:46` `xhh_SYS_Change(xhh_SYS_Wake)`
2. 状态机自身子步计时到点（内部时序驱动）：`xhh_Mode.c:218` `xhh_SYS_Change(xhh_SYS_PowerOFF)`

---

## Handle 推进

`xhh_SYS_Handle()` 在主循环 10ms 周期调用，switch 状态 + 分支子步：

```c
// xhh_Module/xhh_Mode/xhh_Mode.c:175-287（节选）
void xhh_SYS_Handle(void)
{
    motor_start_flag = 0;
    switch (xhh_SYS_n)
    {
    case xhh_SYS_Wake:
        if (xhh_SYS_Step_n == xhh_SYS_Step_Entry) {
            xhh_Task_Key_Cmd(1);
            xhh_Task_ADC_Cmd(1);
            xhh_SYS_Loop_Count = 0;
            xhh_SYS_Step_n++;            // Entry 做完进 Ing
        }
        else if (xhh_SYS_Step_n == xhh_SYS_Step_Ing) {
            if (xhh_SYS_Loop_Count < 1000) { xhh_SYS_Loop_Count++; }
            if (xhh_SYS_Loop_Count >= 200) { xhh_SYS_Change(xhh_SYS_PowerOFF); }
        }
        break;
    ...
```

子步推进模式：
- Entry 内做完初始化后 `xhh_SYS_Step_n++` 进 Ing
- Ing 内累加 `Loop_Count`，到阈值调 `xhh_SYS_Change` 切走
- `Loop_Count` 上限保护：`if (xhh_SYS_Loop_Count < 1000) xhh_SYS_Loop_Count++;`

---

## 子状态机

按键、Touch 等局部状态机用同范式但更细的 step（0→1→2→3...），每组状态一组 `key_X_step` / `key_X_interval` 变量。见 `xhh_Task_Key.c:67-169`。

---

## 禁止

- 用函数指针表 / 状态转移表替代 switch（本项目不采用）
- 绕过 `xhh_SYS_Change` 直接改 `xhh_SYS_n`
- 在状态机里做协议解析或 Flash 写（这些在事件层 / Task 层）
- Entry 阶段漏掉 `xhh_SYS_Step_n++`，导致卡在 Entry

---

## 初始化后应补充的项目事实

- 实际状态枚举全集
- 实际状态转换图（哪些状态能切到哪些）
- 哪些转换是事件驱动、哪些是时序驱动
