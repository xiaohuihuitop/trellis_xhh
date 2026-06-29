# 跨层链路思考指南

> MCU 固件项目里，很多 bug 出在"边界顺序"而不是单个函数实现。一次改动碰 3 层以上时，先把链路画清楚再改。

---

## 本项目的层链路

```text
协议入口 ─> 事件层 ─> 状态机层 ─> Task 模块层 ─> UI/输出/持久化
(BLE_HANDLE) (xhh_Event) (xhh_Mode)  (xhh_Task_*)   (Flash/LCD/LED/Motor)
```

一次改动若同时碰到其中 3 层以上，先把链路写清楚再改。

---

## 重点检查点

- [ ] 谁负责做最终输入校验？（协议层校验帧，事件层校验状态合法性）
- [ ] 状态是不是可能先改了一半？（事件 case 内要么全改要么不改）
- [ ] UI、输出、timeout、持久化会不会分叉？（多模块联动走事件层）
- [ ] 是否应该通过统一事件而不是散写直接调用？
- [ ] 关机/异常路径下各层状态是否一致？

---

## 常见跨层 bug

### Bug 1：状态改一半

**症状**：事件 case 内先改了 motor，后面 break 前漏改了 timeout，导致 timeout 没重置。

**预防**：事件 case 内一次性设置所有联动模块，最后再 `xhh_SYS_Change`。不要在 case 中间 break/return。

### Bug 2：协议层直接改状态机

**症状**：在 `BLE_HANDLE.c` 里直接 `xhh_SYS_Change`，绕过事件层，导致 UI/LED 没同步。

**预防**：协议层只 `xhh_Event_Trigger`，多模块联动在事件 case 内协调。

### Bug 3：Task 间直接互调

**症状**：`xhh_Task_Motor_Loop` 内直接调 `xhh_Task_LED_Set_*`，耦合 + 难调度。

**预防**：Task 间通过事件或状态机协调，不直接互调。

### Bug 4：关机路径漏存 Flash

**症状**：关机状态 Entry 里改了运行时对象，但漏调 `Update_Flash_Data` + `Save`。

**预防**：关机状态必须按"收集 → 保存"顺序，见 [flash.md](../embedded/flash.md)。

### Bug 5：中断和主循环竞态

**症状**：中断改的标志主循环没读到，或主循环改的值中断读到旧值。

**预防**：共享变量加 `volatile`，见 [isr-vs-main-loop.md](./isr-vs-main-loop.md)。

---

## 改动前画链路

把这次改动涉及的层和调用顺序写出来，例如：

```
需求：BLE 下发"切换模式"命令
链路：
  BLE_HANDLE.c 解析帧
  → xhh_Event_Trigger(xhh_Event_Change_Mode, 参数)
  → xhh_Event_Handle case xhh_Event_Change_Mode:
      - xhh_Mode_Change(新模式)
      - xhh_Task_Motor_Apply_Mode_Fun()
      - xhh_Task_UI_Set_*(新UI)
      - xhh_Task_TIMEOUT_Set_*(重置)
      - xhh_SYS_Change(xhh_SYS_Run)
  → xhh_SYS_Handle 推进 Run 状态
  → 各 xhh_Task_*_Loop 执行
```

画完再改，能提前发现"漏改某层"。

---

## Checklist

改动前：
- [ ] 画了完整链路
- [ ] 标出每层要改什么
- [ ] 确认多模块联动走事件层
- [ ] 确认关机/异常路径覆盖

改动后：
- [ ] 沿链路验证每层都执行了
- [ ] 状态没改一半
- [ ] 中断/主循环共享数据有 volatile
- [ ] 持久化双向同步完整
