# 嵌入式 MCU 固件规范模板

适用场景:

- 嵌入式 MCU 固件项目(WCH/PY32 等多平台通用)
- 以 C 语言、状态机、模块化 Task 为主的工程
- 需要 Flash 持久化、串口调试日志、构建产物管理的工程

## 模板编码的约定

本模板基于真实项目代码考古提取，编码了以下开发者的工程习惯（非空模板）：

### 分层架构
固定五个源码目录 `APP/`、`xhh_Module/`、`xhh_BSP/`、`Platform/`、`Components/`，并使用 `Project/`、`Doc/` 两个工程辅助目录。控制流为协议入口 → 事件分发 → 状态机 → Task 模块/Components → xhh_BSP → 当前唯一 Platform；厂商源码原则上不改，只调整允许的配置文件。

### 核心模式(见 xhh_module/ 各 pattern 文件)
- **命名约定**：`xhh_` 前缀 + XHH 分层命名 + 缩写词全大写 + `_t` 后缀 + 头文件保护 `XHH_<MODULE>_H`
- **Task 模块接口**：不提供 `_Init/_DeInit`；使用 `_Cmd(uint8_t)` 控制业务状态，周期模块使用 `_Loop` + static volatile 使能位 + 首句守卫
- **状态机**：枚举状态 + 子步(Entry/Ing/Done) + Loop_Count + 单 switch + 集中 `xhh_SYS_Change`
- **事件系统**：Event 私有单槽 + 参数位编码 + Trigger/Handle + 无 RTOS 队列
- **Flash 持久化**：结构体直存 + 字段校验 + 无效清默认值 + 集中模块
- **中断/关键码**：放 RAM 执行提速（段/属性依 MCU）+ 中断只做轻量 + 共享 volatile + TMOS 调度
- **日志**：`XHH_PRINTF` / `XHH_DEBUG` 宏 + `XHH_DEBUG_EN` 编译期开关默认关
- **占位接口**：允许为框架统一保留，占位函数体内必须用 `AI:` 注释明确原因、当前行为和启用条件，不得伪造成功或硬件值

### 格式规约
- 统一 Tab 缩进
- 不要文件头注释
- `.clang-format` 强制格式（见模板根 `.clang-format`，部署时复制到项目根目录）

## 目录结构

```text
embedded/
├── .clang-format                 # ⚠ 辅助资源：部署到项目根目录（不是 spec layer）
├── README.md                     # 本文件
├── xhh_module/                   # ✅ spec layer：xhh_Module 业务层规范(11 个 .md)
│   ├── index.md                  # 索引 + Pre-Dev Checklist + Quality Check
│   ├── naming-conventions.md     # 命名约定
│   ├── task-module.md    # Task Cmd/Loop 与 BSP 初始化边界
│   ├── state-machine.md  # 状态机范式
│   ├── event-system.md           # 事件系统
│   ├── interrupt.md  # 中断与关键码
│   ├── directory-structure.md    # 目录结构
│   ├── flash.md       # Flash 持久化
│   ├── error-handling.md         # 异常与边界处理
│   ├── logging.md     # 日志规范
│   └── quality.md     # 质量规范（格式/验证/提交/Review）
└── guides/                       # ✅ spec layer：思考指南
    ├── index.md                  # 索引 + Pre-Dev Checklist + Quality Check
    ├── protocol-event-state-task-flow.md  # 核心链路推导
    ├── isr-vs-main-loop.md                # 中断 vs 主循环
    ├── flash-data-layout.md               # Flash 数据布局
    ├── code-reuse-thinking-guide.md       # 代码复用
    ├── mcu-development-checklist.md       # MCU 开发检查清单
    └── cross-layer-thinking-guide.md      # 跨层链路
```

> `.clang-format` 是随模板部署的格式配置文件，不参与 spec layer 识别；Trellis 会将模板根文件部署到 `.trellis/spec/`，初始化后复制到项目根目录，再从 `.trellis/spec/.clang-format` 移除即可。代码复用统一从 `D:\workspace\xhh_项目参考` 中的真实参考 Demo 获取，不再维护独立代码骨架。

## 不直接包含的内容

- 某个仓库独有的函数名（除 `xhh_` 通用前缀外）
- 特定芯片寄存器或 BSP 细节
- 某个项目特有的 DRIVER_ID、地址、产品名

## 使用方式

```bash
trellis init --registry <仓库地址> --template embedded
```

初始化后：

1. **复制 `.clang-format` 到项目根目录**
   ```powershell
   Move-Item .trellis\spec\.clang-format .\.clang-format
   ```
   不做这一步：`.clang-format` 留在 `.trellis/spec/` 内没人引用，clang-format 在项目根跑不到它。

2. 在项目根 `README.md` 建立或更新硬件事实清单：MCU、板卡版本、硬件资料路径、已启用 BSP 能力和硬件限制；与原理图、Project 和 BSP 实现核对后再开始开发。

3. 把 `xhh_module/` 与 `guides/` 里"项目事实占位"换成真实值（地址、结构体名、事件清单等）。

4. 确认 `D:\workspace\xhh_项目参考` 中可用的参考项目，并按 `guides/code-reuse-thinking-guide.md` 的规则登记复用来源；该目录可以包含多个独立项目，且只作为复制和检索来源。

5. `xhh_` 前缀是作者通用前缀，跨项目通用，不需要换。

## 维护约定

- 模板里放的是"可复用的工程约定"，不是某一个仓库的全部细节
- 具体项目初始化后，继续在自己的 `.trellis/spec/` 里本地演化
- 文档用中文
- 新增模式/指南时,先补文件,再同步更新 `xhh_module/index.md` 或 `guides/index.md`
