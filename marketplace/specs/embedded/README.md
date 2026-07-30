# 嵌入式 MCU XHH 代码模式模板

本模板定义 XHH MCU 项目的目录边界、接口形态和源码写法。它是项目内 `.trellis/spec/` 的初始代码模式库，不保存硬件事实、任务过程或通用 MCU 开发流程。

## 适用范围

- 以 C 语言、状态机和模块化 Task 为主的 XHH MCU 固件项目。
- 使用 `APP/`、`xhh_Module/`、`xhh_BSP/`、`Platform/`、`Components/` 分层的工程。

## 模板边界

本模板只保存可由源码目录和接口追溯的稳定模式：

- 各层目录职责和依赖方向；
- Task、Event、状态机、BSP、Flash、日志与异常处理的接口模式；
- 命名、注释、编码和格式等源码规则。

以下内容不属于 Spec，由全局 `xhh-mcu-development` Skill 和当前任务文档处理：

- README、Doc、原理图、数据手册中的项目与板级事实；
- 参考 Demo 选择、复用差异与新板迁移；
- Keil 构建、烧录、实物验证和调试流程；
- 本次任务范围、验收条件、未决项和调研过程。

## 目录结构

```text
embedded/
├── README.md
└── xhh_module/
    ├── index.md
    ├── directory-structure.md
    ├── bsp.md
    ├── task-module.md
    ├── event-system.md
    ├── state-machine.md
    ├── interrupt.md
    ├── flash.md
    ├── error-handling.md
    ├── logging.md
    ├── naming-conventions.md
    └── quality.md
```

## 使用方式

```bash
trellis init --registry <仓库地址> --template embedded
```

初始化后，代码改动按 `xhh_module/index.md` 定位适用模式。项目事实、开发流程和验证要求由全局 Skill 与当前任务 PRD/design 约束。

## 维护约定

- 模板仅沉淀跨项目稳定的 XHH 代码模式。
- 新规则必须能明确落到目录、依赖、接口或源码写法；否则应进入全局 Skill、任务文档或项目记忆。
- 禁止写入 MCU 型号、板卡版本、端口、引脚、AF、时钟、Flash 地址、产品阈值和单次任务结论。
- 文档使用中文 UTF-8 无 BOM。
