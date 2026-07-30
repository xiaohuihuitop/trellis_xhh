# trellis_spec

这是供 Trellis 初始化使用的 XHH MCU Registry。

## 提供内容

- `embedded`：项目内 `.trellis/spec/` 的 XHH 源码模式模板。
- `skill-routed-native`：Codex 单 Agent 的通用任务工作流，要求每个任务自行选择并记录全局 Skill。

`embedded` 只保存目录、依赖、接口和源码模式。硬件事实、Demo 复用、构建与实物验证由当前任务在 PRD 中选择的领域 Skill 和任务文档处理。

`skill-routed-native` 不固定绑定 `xhh-mcu-development`、`grilling` 或其他 Skill。它只要求任务规划时记录必需与条件性 Skill，并在实施和检查前加载。Skill 必须由用户环境全局安装，Registry 不负责安装。

## Registry 路径

- 索引：`marketplace/index.json`
- `type: "spec"` 的 `path` 相对仓库根目录，例如 `marketplace/specs/embedded`。
- `type: "workflow"` 的 `path` 相对 Registry 根目录；本仓库的 Registry 根目录是 `marketplace/`，因此写为 `workflows/skill-routed-native.md`。
- `spec` 路径是目录；`workflow` 路径必须是单个 Markdown 文件。

```text
trellis_spec/
├── marketplace/
│   ├── index.json
│   ├── specs/
│   │   └── embedded/
│   └── workflows/
│       └── skill-routed-native.md
└── README.md
```

## 新项目初始化

将 `<账号>/<仓库>` 替换为此 Registry 的远端仓库地址：

```powershell
trellis init --codex --no-monorepo -u <用户名> -y `
  --registry gh:<账号>/<仓库>/marketplace `
  --template embedded `
  --workflow skill-routed-native `
  --workflow-source gh:<账号>/<仓库>/marketplace
```

初始化后会得到：

- `.trellis/spec/`：从 `embedded` 复制的源码模式；
- `.trellis/workflow.md`：从 `skill-routed-native` 安装的任务流程；
- Codex 内置的 Trellis Skill：由 `--codex` 安装；
- 领域 Skill：由任务 PRD 选择，并由用户环境提供。

例如 XHH MCU 任务在 `prd.md` 中记录：

```markdown
## Skill 路由

- 必需：`xhh-mcu-development`，原因：涉及 XHH 分层与硬件事实核对。
- 条件性：`grilling`，触发条件：硬件路径、协议或状态机方案未决。
```

## 发布前验证

```powershell
$null = Get-Content -Raw -Encoding utf8 marketplace/index.json | ConvertFrom-Json
Test-Path marketplace/specs/embedded/README.md
Test-Path marketplace/workflows/skill-routed-native.md
git diff --check
```

还应在空目录执行一次完整 `trellis init`，确认 `.trellis/spec/` 与 `.trellis/workflow.md` 都来自本 Registry。

## 维护边界

- 新增 Spec 规则前，确认它能落到目录、依赖、接口或源码写法。
- 项目硬件事实、任务范围、验收、调研过程与单次结论不进入 Spec。
- Workflow 只定义任务阶段与 Skill 路由，不复制领域 Skill 内容，也不固定绑定某个领域 Skill。
- 不新增未经 CLI 验证的 Hook、门禁或自动化。
