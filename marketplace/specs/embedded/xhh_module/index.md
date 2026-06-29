# xhh_Module 开发规范

> 这里的 "xhh_module" 约束 `xhh_Module/` 业务层(Event/Mode/Task),不是平台层,也不是 web 服务。

---

## 概述

- 工程以 C 语言为主
- 核心控制流：协议入口 → 事件分发 → 系统状态机 → 功能 Task 模块
- 重视可控性、可验证性、板级调试，而不是抽象层层封装
- 厂商层（SRC/HAL/LIB/Profile）不改源码，只在 config.h 调宏

---

## 规范索引

### 模式规范（怎么写代码）

| 文档 | 说明 |
|------|------|
| [命名约定](./naming-conventions.md) | xhh_ 前缀、缩写词全大写、_t 后缀、头文件保护宏 |
| [Task 模块模式](./task-module.md) | 四件套接口、使能位守卫、ALL 聚合 |
| [状态机模式](./state-machine.md) | 枚举+子步+计数+switch、集中转换 |
| [事件系统](./event-system.md) | 全局变量单槽、参数编码、Trigger/Handle |
| [中断与关键码](./interrupt.md) | RAM 执行原则、中断禁区、TMOS 调度 |

### 边界规范（各层怎么处理）

| 文档 | 说明 |
|------|------|
| [目录结构](./directory-structure.md) | 分层与目录布局、厂商层约定 |
| [Flash 持久化](./flash.md) | 结构体直存、校验、默认值、集中模块 |
| [异常与边界处理](./error-handling.md) | guard early、返回码、状态切换、无 assert |
| [日志规范](./logging.md) | XHH_DEBUG 宏、编译期开关 |

### 工程规范

| 文档 | 说明 |
|------|------|
| [质量规范](./quality.md) | 格式（Tab/.clang-format/无文件头）、验证、提交、Review 清单 |

### 示例代码骨架（直接套用）

新建模块时复制 `.trellis/examples/` 下对应骨架,改名填逻辑即可。spec 不放代码文件,骨架代码统一在 `.trellis/examples/`（**注意不是** `.trellis/spec/embedded/examples/`——init 落地后必须按 `embedded/README.md` 的"部署后必须步骤"第 2 步把 `examples/` 移到 `.trellis/examples/`，否则 Trellis 会把它误算成一层 spec layer）。

| 骨架 | 用途 |
|------|------|
| `xhh_Task_Template.c/.h` | Task 模块四件套完整模板 |
| `xhh_Event_Template.c/.h` | 事件枚举 + Trigger + Handle 骨架 |
| `xhh_Mode_Template.c` | 状态机枚举 + Change + Handle 骨架 |
| `xhh_Task_Flash_Template.c` | Flash 结构体 + Get/Save/IS_Valid/Clean 骨架 |

---

## Pre-Development Checklist（动手前必读）

写任何固件代码前，按顺序确认：

- [ ] **定位改动落哪一层**：读 [../guides/protocol-event-state-task-flow.md](../guides/protocol-event-state-task-flow.md) 的决策树，确认这段逻辑该放协议/事件/状态机/Task/中断哪一层
- [ ] **命名**：新标识符加 `xhh_` 前缀、缩写词全大写（LED/ADC/BLE）、类型 `_t` 后缀——见 [naming-conventions.md](./naming-conventions.md)
- [ ] **若是新 Task 模块**：规划四件套（`_Init/_DeInit/_Cmd/_Loop`）+ `static volatile` 使能位 + Loop 守卫 + 注册 `xhh_Task_ALL.h`——见 [task-module.md](./task-module.md)
- [ ] **若是新状态/状态转换**：只通过 `xhh_SYS_Change()` 切换，不直接改 `xhh_SYS_n`——见 [state-machine.md](./state-machine.md)
- [ ] **若是多模块联动**：走事件层（`xhh_Event_Trigger` + 事件 case 内集中设置），不在协议层直调各 Task——见 [event-system.md](./event-system.md)
- [ ] **涉及中断**：中断只做清标志/计数/轻量输出，不做协议/事件/Flash——见 [interrupt.md](./interrupt.md) 和 [../guides/isr-vs-main-loop.md](../guides/isr-vs-main-loop.md)
- [ ] **涉及 Flash**：走 `xhh_Task_Flash` 集中模块，不加单字段 Save 接口，结构体直存 + 校验——见 [flash.md](./flash.md)
- [ ] **编辑已有 .c/.h**：确认文件编码是 UTF-8（非 UTF-8 先转码），见 [quality.md](./quality.md) 文件编码章节
- [ ] **新建文件**：直接用 `write` 工具（默认 UTF-8），无文件头注释，Tab 缩进，`__XHH_<MODULE>_H__` 头文件保护
- [ ] **要套骨架**：从 `.trellis/examples/` 复制对应骨架，全局替换 `Template` → 模块名

---

## Quality Check（完成后必验）

代码写完，提交前逐项确认：

- [ ] **编译通过**：MounRiver Studio / RISC-V GCC 编译无错
- [ ] **产物生成**：`.hex` / `.bin` 产出 + post-build CRC 通过
- [ ] **格式**：Tab 缩进、缩写词全大写、无文件头注释、`.clang-format` 无报错——见 [quality.md](./quality.md) Review Checklist
- [ ] **分层链路**：改动是否走了 协议→事件→状态机→Task 链路，有没有绕过事件层直接跨模块调
- [ ] **状态一致性**：事件 case 内多模块联动是否一次性设置完，没有状态改一半
- [ ] **守卫**：Task `_Loop` 首句 `if (en == 0) return;` 在；参数 `if (NULL) return` 在
- [ ] **中断禁区**：中断里没有协议解析/事件触发/Flash 读写
- [ ] **持久化**：Flash 读写经 `xhh_Task_Flash`，没散写 `Flash_Write`；新字段加了 `IS_Valid` + `Clean` 默认值 + 双向 `Update`
- [ ] **路径审查**：涉及协议/状态机/持久化的改动，确认完整读写链路而非只看单函数
- [ ] **TODO 注释**：遗留 TODO 用 `//TODO` 行首格式，便于 grep
- [ ] **调试残留**：没有遗留的 `XHH_DEBUG` 高频日志、测试 hook、编译开关忘关

---

## 使用建议

- 初始化到具体项目后，把"项目事实占位"换成真实值（地址、结构体名、事件清单等）
- `xhh_` 前缀是作者通用前缀，跨项目通用，不需要换
- 每个文件的真实代码示例可替换为目标仓库的等价示例

---

**语言**：说明文档用中文。
