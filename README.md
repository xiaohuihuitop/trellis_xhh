# trellis_spec

这是一个给 Trellis 使用的自定义 Spec Template Marketplace 仓库。

目标：

- 为 MCU 固件类项目提供可复用的 `.trellis/spec/` 初始模板
- 把开发者真实的工程习惯（从代码考古提取）直接编码进模板，而不是空模板让用户从零填
- 避免每个新项目都从默认 web/backend 模板开始手工改写

当前提供的模板：

- `wch-mcu-firmware`：适用于 WCH MCU 固件项目（基于真实代码考古提取的完整约定）

> 原 `wch-mcu-ble-app` 模板已移除（删除 frontend 后与 firmware 模板重复，未来如需 BLE 调试 App 规范再单独建）。

## 模板编码的约定（wch-mcu-firmware）

本模板不是空模板，而是从真实项目代码里提取的开发者工程习惯：

- **分层架构**：协议入口 → 事件 → 状态机 → Task 模块 → BSP；厂商层不改只调 config.h
- **命名**：`xhh_` 前缀 + snake_case + 缩写词全大写 + `_t` 后缀 + `__XHH_<MODULE>_H__` 头文件保护
- **Task 模块四件套**：`_Init/_DeInit/_Cmd(uint8_t)/_Loop` + static volatile 使能位 + Loop 守卫 + ALL 聚合
- **状态机**：枚举状态 + 子步(Entry/Ing/Done) + Loop_Count + 单 switch + 集中 Change
- **事件系统**：全局变量单槽 + 参数位编码 + Trigger/Handle + 无 RTOS 队列
- **Flash**：结构体直存 + 字段校验 + 无效清默认值 + 集中模块
- **中断/关键码**：放 RAM 执行提速 + 中断只做轻量 + 共享 volatile + TMOS 调度
- **日志**：`XHH_DEBUG` 宏 + 编译期开关默认关
- **格式**：统一 Tab + 无文件头 + `.clang-format` 强制

## 仓库结构

```text
trellis_spec/
├── marketplace/
│   ├── index.json                              # 模板索引入口
│   └── specs/
│       └── wch-mcu-firmware/                   # 固件规范模板
│           ├── .clang-format                   # 格式配置（部署到项目根）
│           ├── README.md
│           ├── backend/                        # 固件规范（10 个文件）
│           └── guides/                         # 思考指南（5 个文件）
├── .gitignore
└── README.md
```

说明：

- `marketplace/index.json` 是模板索引入口
- `marketplace/specs/<模板 id>/` 是实际会被复制到目标项目 `.trellis/spec/` 的模板目录
- `path` 字段相对的是仓库根目录，不是 `index.json` 所在目录

## 使用方式

仓库推到远端后，可按 Trellis 官方 registry 方式使用：

```bash
# 交互选择模板
trellis init --registry gh:<你的账号>/<你的仓库名>/marketplace

# 直接指定模板
trellis init --registry gh:<你的账号>/<你的仓库名>/marketplace --template wch-mcu-firmware
```

初始化后：
1. 把模板根的 `.clang-format` 复制到项目根目录
2. 把 backend/guides 里"项目事实占位"换成真实值
3. `xhh_` 前缀是作者通用前缀，跨项目通用，不需要换

## 维护约定

- 模板里放的是"可复用的工程约定"，不是某一个仓库的全部细节
- 具体项目初始化后，仍然应该继续在自己的 `.trellis/spec/` 里本地演化
- 文档、说明、备注尽量使用中文；只有 Trellis 规定的结构字段保持官方要求
- 新增模板时，先补模板目录，再同步更新 `marketplace/index.json`

## 本地验证建议

发布前至少检查：

- `marketplace/index.json` 的 `templates/path` 是否指向正确目录
- 每个模板目录内是否至少包含 `README.md` 和需要暴露的规范文件
- 模板内容里是否带入了某个私有仓库独有的路径、域名、客户信息或任务状态
