# 思考指南

> 这些 guide 不是教"代码怎么写"，而是提醒动手前先想什么。固件项目里，大部分 bug 出在"边界顺序"和"该走事件没走"，而不是单函数实现。

---

## 可用指南

| 指南 | 目的 | 适用时机 |
|------|------|----------|
| [协议→事件→状态机→Task 链路指南](./protocol-event-state-task-flow.md) | 新需求该落哪一层、怎么沿链路推导 | 拿到新功能需求时 |
| [中断 vs 主循环思考指南](./isr-vs-main-loop.md) | 一段逻辑该放中断还是主循环 | 涉及中断/时敏/共享数据时 |
| [Flash 数据布局思考指南](./flash-data-layout.md) | 持久化结构体怎么设计、校验怎么做 | 新增/修改持久化字段时 |
| [代码复用思考指南](./code-reuse-thinking-guide.md) | 判断是否应该复用/扩展现有模块 | 改常量、加 helper、批量改时 |
| [跨层链路思考指南](./cross-layer-thinking-guide.md) | 检查协议/事件/状态机/Task 边界 | 一次改动影响 3 层以上时 |

---

## 快速判断

- **新功能需求** → 先读 [协议→事件→状态机→Task 链路指南](./protocol-event-state-task-flow.md)
- **碰中断/定时器/共享变量** → 先读 [中断 vs 主循环思考指南](./isr-vs-main-loop.md)
- **加/改持久化字段** → 先读 [Flash 数据布局思考指南](./flash-data-layout.md)
- **改常量/枚举/协议字段** → 先 grep 全局引用，再读 [代码复用思考指南](./code-reuse-thinking-guide.md)
- **改动跨 3 层** → 先画链路，再读 [跨层链路思考指南](./cross-layer-thinking-guide.md)

---

**核心原则**：30 分钟思考省 3 小时调试。固件调试成本远高于写代码成本。
