# trellis_spec

这是供 Trellis 初始化使用的 XHH MCU Registry。

## 提供内容

- `embedded`：项目内 `.trellis/spec/` 的 XHH 源码模式模板。
- `xhh-mcu-native`：Codex 单 Agent 的 XHH MCU 任务工作流。

`embedded` 只保存目录、依赖、接口和源码模式。硬件事实、Demo 复用、Keil 构建与实物验证由全局 `xhh-mcu-development` Skill 和当前任务文档处理。

`xhh-mcu-native` 只路由全局 Skill，不包含其副本，也不能自动安装 Skill。使用前需在 Codex 全局安装：

- `xhh-mcu-development`
- `grilling`，仅用于高风险且未决的方案

## Registry 路径

- 索引：`marketplace/index.json`
- `type: "spec"` 的 `path` 相对仓库根目录，例如 `marketplace/specs/embedded`。
- `type: "workflow"` 的 `path` 相对 Registry 根目录；本仓库的 Registry 根目录是 `marketplace/`，因此写为 `workflows/xhh-mcu-native.md`。
- `spec` 路径是目录；`workflow` 路径必须是单个 Markdown 文件。

```text
trellis_spec/
├── marketplace/
│   ├── index.json
│   ├── specs/
│   │   └── embedded/
│   └── workflows/
│       └── xhh-mcu-native.md
└── README.md
```

## 新项目初始化

将 `<账号>/<仓库>` 替换为此 Registry 的远端仓库地址：

```powershell
trellis init --codex --no-monorepo -u <用户名> -y `
  --registry gh:<账号>/<仓库>/marketplace `
  --template embedded `
  --workflow xhh-mcu-native `
  --workflow-source gh:<账号>/<仓库>/marketplace
```

初始化后会得到：

- `.trellis/spec/`：从 `embedded` 复制的源码模式；
- `.trellis/workflow.md`：从 `xhh-mcu-native` 安装的工作流；
- Codex 内置的 Trellis Skill：由 `--codex` 安装；
- 全局 XHH Skill：由用户环境提供，Registry 不负责安装。

## 发布前验证

```powershell
$null = Get-Content -Raw -Encoding utf8 marketplace/index.json | ConvertFrom-Json
Test-Path marketplace/specs/embedded/README.md
Test-Path marketplace/workflows/xhh-mcu-native.md
git diff --check
```

还应在空目录执行一次完整 `trellis init`，确认 `.trellis/spec/` 与 `.trellis/workflow.md` 都来自本 Registry。

## 维护边界

- 新增 Spec 规则前，确认它能落到目录、依赖、接口或源码写法。
- 项目硬件事实、任务范围、验收、调研过程与单次结论不进入 Spec。
- Workflow 只定义任务阶段与 Skill 路由，不复制领域 Skill 内容。
- 不新增未经 CLI 验证的 Hook、门禁或自动化。
