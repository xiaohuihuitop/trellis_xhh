# WCH MCU 固件规范模板

适用场景：

- WCH 系列 MCU 固件项目
- 以 C 语言、状态机、模块化 Task 为主的嵌入式工程
- 需要 Flash 持久化、串口调试日志、构建产物管理的工程

## 模板编码的约定

本模板基于真实项目代码考古提取，编码了以下开发者的工程习惯（非空模板）：

### 分层架构
协议入口 → 事件分发 → 状态机 → Task 模块 → BSP 支撑。厂商层（SRC/HAL/LIB/Profile）不改源码，只调 config.h。

### 核心模式（见 backend/ 各 pattern 文件）
- **命名约定**：`xhh_` 前缀 + snake_case + 缩写词全大写 + `_t` 后缀 + 头文件保护 `__XHH_<MODULE>_H__`
- **Task 模块四件套**：`_Init/_DeInit/_Cmd(uint8_t)/_Loop` + static volatile 使能位 + Loop 首句守卫 + `xhh_Task_ALL.h` 聚合
- **状态机**：枚举状态 + 子步(Entry/Ing/Done) + Loop_Count + 单 switch + 集中 `xhh_SYS_Change`
- **事件系统**：全局变量单槽 + 参数位编码 + Trigger/Handle + 无 RTOS 队列
- **Flash 持久化**：结构体直存 + 字段校验 + 无效清默认值 + 集中模块
- **中断/关键码**：放 RAM 执行提速（段/属性依 MCU）+ 中断只做轻量 + 共享 volatile + TMOS 调度
- **日志**：`XHH_DEBUG` 宏 + 编译期开关默认关

### 格式规约
- 统一 Tab 缩进
- 不要文件头注释
- `.clang-format` 强制格式（见模板根 `.clang-format`，部署时复制到项目根目录）

## 目录结构

```text
wch-mcu-firmware/
├── .clang-format                 # 格式配置（部署到项目根目录）
├── README.md                     # 本文件
├── backend/                      # 固件规范
│   ├── index.md                  # 索引
│   ├── naming-conventions.md     # 命名约定
│   ├── task-module-pattern.md    # Task 模块四件套
│   ├── state-machine-pattern.md  # 状态机范式
│   ├── event-system.md           # 事件系统
│   ├── interrupt-and-critical-code.md  # 中断与关键码
│   ├── directory-structure.md    # 目录结构
│   ├── flash-guidelines.md       # Flash 持久化
│   ├── error-handling.md         # 异常与边界处理
│   ├── logging-guidelines.md     # 日志规范
│   └── quality-guidelines.md     # 质量规范（格式/验证/提交/Review）
└── guides/                       # 思考指南
    ├── index.md
    ├── protocol-event-state-task-flow.md  # 核心链路推导
    ├── isr-vs-main-loop.md                # 中断 vs 主循环
    ├── flash-data-layout.md               # Flash 数据布局
    ├── code-reuse-thinking-guide.md       # 代码复用
    └── cross-layer-thinking-guide.md      # 跨层链路
```

## 不直接包含的内容

- 某个仓库独有的函数名（除 `xhh_` 通用前缀外）
- 特定芯片寄存器或 BSP 细节
- 某个项目特有的 DRIVER_ID、地址、产品名

## 使用方式

```bash
trellis init --registry <仓库地址> --template wch-mcu-firmware
```

初始化后：
1. 把 `.clang-format` 复制到项目根目录
2. 把 backend/guides 里"项目事实占位"换成真实值
3. `xhh_` 前缀是作者通用前缀，跨项目通用，不需要换

## 维护约定

- 模板里放的是"可复用的工程约定"，不是某一个仓库的全部细节
- 具体项目初始化后，继续在自己的 `.trellis/spec/` 里本地演化
- 文档用中文
- 新增模式/指南时，先补文件，再同步更新 `backend/index.md` 或 `guides/index.md`
