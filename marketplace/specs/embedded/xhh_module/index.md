# xhh_Module 开发规范

> 这里的 "xhh_module" 主要约束 `xhh_Module/` 业务层(Event/Mode/Task)，并定义它与根目录 `APP/`、`xhh_BSP/`、`Platform/`、`Components/` 的边界。

---

## 概述

- 工程以 C 语言为主
- 固定工程结构：五个源码目录 `APP/`、`xhh_Module/`、`xhh_BSP/`、`Platform/`、`Components/`，外加 `Project/`、`Doc/` 两个辅助目录
- 核心控制流：协议入口 → 事件分发 → 系统状态机 → 功能 Task 模块/Components → xhh_BSP 公共层
- 重视可控性、可验证性、板级调试，而不是抽象层层封装
- 每个项目只有一个 MCU 平台，厂商层全部收口到根目录 `Platform/`
- 硬件操作集中在根目录 `xhh_BSP/`，Task 和 Components 不直接碰厂商 API

---

## 规范索引

### 模式规范（怎么写代码）

| 文档 | 说明 |
|------|------|
| [命名约定](./naming-conventions.md) | xhh_ 前缀、缩写词全大写、_t 后缀、头文件保护宏 |
| [Task 模块模式](./task-module.md) | 按需 Cmd/Loop、使能位守卫、ALL Cmd 聚合、调 `xhh_BSP_*` |
| [状态机模式](./state-machine.md) | 枚举+子步+计数+switch、集中转换 |
| [事件系统](./event-system.md) | Event 私有单槽、参数编码、Trigger/Handle |
| [中断与关键码](./interrupt.md) | 跨平台原则、时敏输出/安全关断例外 |
| [BSP 公共层](./bsp.md) | BSP 简洁原则、SYS 集中初始化、Timer/PWM/IWDG/SPI/ADC 接口和平台隔离 |

### 边界规范（各层怎么处理）

| 文档 | 说明 |
|------|------|
| [目录结构](./directory-structure.md) | 五个源码目录、Project/Doc 辅助目录、单 MCU 平台和依赖方向 |
| [Flash 持久化](./flash.md) | 结构体直存、校验、默认值、集中模块 |
| [异常与边界处理](./error-handling.md) | guard early、返回码、状态切换、无 assert |
| [日志规范](./logging.md) | XHH_PRINTF / XHH_DEBUG 宏、XHH_DEBUG_EN 编译期开关 |

### 工程规范

| 文档 | 说明 |
|------|------|
| [质量规范](./quality.md) | 格式（Tab/.clang-format/无文件头）、验证、提交、Review 清单 |

### 示例代码骨架（直接套用）

新建模块时复制 `.trellis/examples/` 下对应骨架,改名填逻辑即可。spec 不放代码文件,骨架代码统一在 `.trellis/examples/`（**注意不是** `.trellis/spec/embedded/examples/`——init 落地后必须按 `embedded/README.md` 的"部署后必须步骤"第 2 步把 `examples/` 移到 `.trellis/examples/`，否则 Trellis 会把它误算成一层 spec layer）。

| 骨架 | 用途 |
|------|------|
| `xhh_BSP_Template.c/.h` | BSP 公共层骨架（逻辑信号 + BSP 内固定硬件映射） |
| `xhh_Task_All_Template.h/.c` | 聚合层骨架（聚合 include + 转发各模块 Cmd） |
| `xhh_Task_Template.c/.h` | Task 按需 Cmd/Loop/Get/Set 模板 |
| `xhh_Event_Template.c/.h` | 事件枚举 + Trigger + Handle 骨架 |
| `xhh_Mode_Template.c/.h` | 状态机枚举 + Change + Handle 骨架(.h 放枚举定义,.c 放实现) |
| `xhh_Task_Flash_Template.c` | Flash 结构体 + Get/Save/IS_Valid/Clean 骨架 |

---

## Pre-Development Checklist（动手前必读）

写任何固件代码前，按顺序确认：

- [ ] **定位改动落哪一层**：读 [../guides/protocol-event-state-task-flow.md](../guides/protocol-event-state-task-flow.md) 的决策树，确认这段逻辑该放协议/事件/状态机/Task/中断哪一层
- [ ] **命名**：新标识符加 `xhh_` 前缀、缩写词全大写（LED/ADC/BLE）、类型 `_t` 后缀——见 [naming-conventions.md](./naming-conventions.md)
- [ ] **若是新 Task 模块**：只规划真实需要的 `_Cmd/_Loop/Get/Set`，使用私有状态和 Loop 守卫，并注册 `xhh_Task_ALL.h`——见 [task-module.md](./task-module.md)
- [ ] **若是新状态/状态转换**：只通过 `xhh_SYS_Change()` 切换，不直接改 `xhh_SYS_n`——见 [state-machine.md](./state-machine.md)
- [ ] **若是多模块联动**：走事件层（`xhh_Event_Trigger` + 事件 case 内集中设置），不在协议层直调各 Task——见 [event-system.md](./event-system.md)
- [ ] **涉及中断**：中断只做清标志/计数/轻量输出，不做协议/事件/Flash——见 [interrupt.md](./interrupt.md) 和 [../guides/isr-vs-main-loop.md](../guides/isr-vs-main-loop.md)
- [ ] **涉及硬件操作**：Task 不直接调厂商 API/引脚号/寄存器，集中走 `xhh_BSP_*` 公共层——见 [bsp.md](./bsp.md)
- [ ] **涉及硬件事实**：先阅读项目根 `README.md` 的 MCU、板卡、硬件资料和限制；硬件信息与原理图、Project 或 BSP 冲突时先核对，不按猜测修改——见 [directory-structure.md](./directory-structure.md)
- [ ] **涉及 ADC**：上层只使用 `xhh_BSP_ADC_CHANNEL_<n>` 逻辑通道名，硬件通道值只在当前 MCU 的 `xhh_BSP_ADC.c` 中映射——见 [bsp.md](./bsp.md)
- [ ] **涉及 Components 硬件访问**：Components 只调用 `xhh_BSP_*`，不包含 `main.h`、产品层或厂家头——见 [directory-structure.md](./directory-structure.md)
- [ ] **涉及延时**：统一调用 `xhh_BSP_Delay_ms`；厂家延时接口只允许出现在 `xhh_BSP_SYS.c` 的平台适配实现中——见 [bsp.md](./bsp.md)
- [ ] **涉及关机、Sleep、Shutdown 或显示外设**：确认显示控制器、背光和通信接口均按数据手册完整关断；`Display Off` 不等于 `Sleep In`——见 [bsp.md](./bsp.md)
- [ ] **存在占位函数**：确认占位是为了框架统一，函数体内有 `AI:` 注释说明原因、当前行为和启用条件，且未返回默认成功或伪造硬件值——见 [error-handling.md](./error-handling.md)
- [ ] **涉及 Flash**：走 `xhh_Task_Flash` 集中模块，不加单字段 Save 接口，结构体直存 + 校验——见 [flash.md](./flash.md)
- [ ] **编辑已有 .c/.h**：确认文件编码是 UTF-8（非 UTF-8 先转码），见 [quality.md](./quality.md) 文件编码章节
- [ ] **新建文件**：使用 UTF-8 无 BOM，无文件头注释，Tab 缩进，`XHH_<MODULE>_H` 头文件保护
- [ ] **要套骨架**：从 `.trellis/examples/` 复制对应骨架，全局替换 `Template` → 模块名

---

## Quality Check（完成后必验）

代码写完，提交前逐项确认：

- [ ] **编译通过**：使用当前项目实际 IDE 或构建系统全量编译，无错无警告
- [ ] **构建产物**：MCU 代码提交已包含本次全量编译生成的 `.hex` / `.bin`
- [ ] **产物生成**：`.hex` / `.bin` 产出 + post-build CRC 通过
- [ ] **格式**：Tab 缩进、缩写词全大写、无文件头注释、`.clang-format` 无报错——见 [quality.md](./quality.md) Review Checklist
- [ ] **分层链路**：改动是否走了 协议→事件→状态机→Task 链路，有没有绕过事件层直接跨模块调
- [ ] **状态一致性**：事件 case 内多模块联动是否一次性设置完，没有状态改一半
- [ ] **守卫**：Task `_Loop` 首句 `if (en == 0) return;` 在；参数 `if (NULL) return` 在
- [ ] **Task 函数顺序**：硬件操作接口以 `AI:硬件操作接口` 标注并置顶，随后 Cmd、其他公开接口，Loop 为文件最后一个函数定义——见 [task-module.md](./task-module.md)
- [ ] **中断禁区**：中断里没有协议解析/事件触发/Flash 读写
- [ ] **持久化**：Flash 读写经 `xhh_Task_Flash`，没散写 `xhh_BSP_Flash_Write`；新字段加了 `IS_Valid` + `Clean` 默认值 + 双向 `Update`
- [ ] **路径审查**：涉及协议/状态机/持久化的改动，确认完整读写链路而非只看单函数
- [ ] **TODO 注释**：遗留 TODO 用 `//TODO` 行首格式，便于 grep
- [ ] **调试残留**：没有遗留的 `XHH_DEBUG` 高频日志、测试 hook、编译开关忘关
- [ ] **低功耗实测**：涉及低功耗或显示外设时，已对比冷上电自动休眠与开机后关机的稳定电流；已验证唤醒后显示恢复——见 [quality.md](./quality.md)

---

## 使用建议

- 初始化到具体项目后，把"项目事实占位"换成真实值（地址、结构体名、事件清单等）
- `xhh_` 前缀是作者通用前缀，跨项目通用，不需要换
- 每个文件的真实代码示例可替换为目标仓库的等价示例

---

**语言**：说明文档用中文。
