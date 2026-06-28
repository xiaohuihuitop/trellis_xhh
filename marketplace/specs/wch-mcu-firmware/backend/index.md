# 后端（固件）开发规范

> 这里的 "backend" 指 MCU 固件本体，不是 web 服务。

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
| [Task 模块模式](./task-module-pattern.md) | 四件套接口、使能位守卫、ALL 聚合 |
| [状态机模式](./state-machine-pattern.md) | 枚举+子步+计数+switch、集中转换 |
| [事件系统](./event-system.md) | 全局变量单槽、参数编码、Trigger/Handle |
| [中断与关键码](./interrupt-and-critical-code.md) | RAM 执行原则、中断禁区、TMOS 调度 |

### 边界规范（各层怎么处理）

| 文档 | 说明 |
|------|------|
| [目录结构](./directory-structure.md) | 分层与目录布局、厂商层约定 |
| [Flash 持久化](./flash-guidelines.md) | 结构体直存、校验、默认值、集中模块 |
| [异常与边界处理](./error-handling.md) | guard early、返回码、状态切换、无 assert |
| [日志规范](./logging-guidelines.md) | XHH_DEBUG 宏、编译期开关 |

### 工程规范

| 文档 | 说明 |
|------|------|
| [质量规范](./quality-guidelines.md) | 格式（Tab/.clang-format/无文件头）、验证、提交、Review 清单 |

### 示例代码骨架（直接套用）

新建模块时复制 `../examples/` 下对应骨架,改名填逻辑即可。见 [examples/README.md](../examples/README.md)。

| 骨架 | 用途 |
|------|------|
| `xhh_Task_Template.c/.h` | Task 模块四件套完整模板 |
| `xhh_Event_Template.c/.h` | 事件枚举 + Trigger + Handle 骨架 |
| `xhh_Mode_Template.c` | 状态机枚举 + Change + Handle 骨架 |
| `xhh_Task_Flash_Template.c` | Flash 结构体 + Get/Save/IS_Valid/Clean 骨架 |

---

## 使用建议

- 初始化到具体项目后，把"项目事实占位"换成真实值（地址、结构体名、事件清单等）
- `xhh_` 前缀是作者通用前缀，跨项目通用，不需要换
- 每个文件的真实代码示例可替换为目标仓库的等价示例

---

**语言**：说明文档用中文。
