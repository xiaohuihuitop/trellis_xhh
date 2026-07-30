# XHH 源码模式索引

本目录约束 `APP/`、`xhh_Module/`、`xhh_BSP/`、`Platform/` 与 `Components/` 的代码边界和接口模式。

项目硬件事实、Demo 复用、构建和实物验证不在本索引重复定义，由全局 `xhh-mcu-development` Skill 与当前任务文档处理。

## 规范索引

| 文档 | 约束范围 |
|------|----------|
| [目录结构](./directory-structure.md) | 顶层目录职责和依赖方向 |
| [BSP 公共层](./bsp.md) | 基础外设能力、公开接口和初始化归属 |
| [Task 模块模式](./task-module.md) | Cmd/Loop、私有状态、函数顺序和局部协作 |
| [事件系统](./event-system.md) | 单槽 Event、参数编码、Trigger 与 Handle |
| [状态机模式](./state-machine.md) | 状态、子步和集中转换 |
| [中断与关键码](./interrupt.md) | ISR 可做与禁止做的源码边界 |
| [Flash 持久化](./flash.md) | Flash 接口、数据校验和运行时同步 |
| [异常与边界处理](./error-handling.md) | 参数边界、错误处理和占位接口 |
| [日志规范](./logging.md) | `XHH_PRINTF`、`XHH_DEBUG` 与编译开关 |
| [命名约定](./naming-conventions.md) | 标识符、类型、文件和头保护宏 |
| [质量规范](./quality.md) | 源码格式、编码、注释和静态检查 |

## 编码前检查

- [ ] 改动所在目录符合 [目录结构](./directory-structure.md) 的职责和依赖方向。
- [ ] 新标识符遵守 [命名约定](./naming-conventions.md)。
- [ ] 新 Task 只提供真实需要的接口，状态私有，周期逻辑遵守 [Task 模块模式](./task-module.md)。
- [ ] 状态迁移只通过 [状态机模式](./state-machine.md) 的集中接口完成。
- [ ] 协议入口、状态迁移和跨域编排遵守 [事件系统](./event-system.md)；Task 局部协作保持单向且无调用环。
- [ ] Task 和 Components 只使用 `xhh_BSP_*` 公开接口，不直接依赖厂家 API、寄存器或 `Platform/`。
- [ ] ISR、Flash、日志和错误处理分别阅读对应规范，避免跨层复制实现。
- [ ] 新建或编辑源码符合 [质量规范](./quality.md) 的编码、格式和注释要求。

## 源码检查

- [ ] 没有反向依赖、循环依赖或 Task 内硬件初始化。
- [ ] Event、状态机和 Task 的调用顺序不会形成半更新状态。
- [ ] Task 私有状态未暴露为可写全局变量，`_Loop` 有使能守卫。
- [ ] Task 函数顺序符合“硬件操作接口 → Cmd → 其他公开接口 → Loop”。
- [ ] 参数型 `_Set_*` 只更新私有状态；需要输出时由明确的 Apply/Mode 接口执行。
- [ ] 中断中未执行协议解析、Event Handle、Flash 读写或普通 Task Loop。
- [ ] Flash 数据经集中模块读写，新增字段具备有效性校验和默认清理。
- [ ] 公开头未泄露厂家类型，源码未保留无说明的占位、调试或兜底逻辑。
