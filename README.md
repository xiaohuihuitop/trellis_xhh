# trellis_spec

这是供 Trellis 初始化使用的 Codex 工作流 Registry。

## 提供内容

- `skill-routed-native`：通用任务工作流，先分类直接修改、当前任务扩展与新任务，再主动发现、评估并路由当前会话可用的全局 Skill。进入工作流的用户可见状态以 `[trellis]` 标识；涉及实物或外部系统的任务使用单变量测试卡，连续两次同症状失败后必须先诊断复盘；收尾发现可复发规则缺口时必须请求用户确认归属。

`skill-routed-native` 不固定绑定任何领域 Skill。它要求任务规划时按领域、决策、构建、测试、诊断、审查与交付等通用能力维度筛选候选，并在 PRD 中记录必需、条件性、不使用或待用户调用的结论。Skill 必须由用户环境全局安装，Registry 不负责安装。主 Agent 负责实施、整合和最终结论；仅已路由且独立只读的调研或审查可以使用子 Agent。

## Registry 路径

- 索引：`marketplace/index.json`
- `type: "workflow"` 的 `path` 相对 Registry 根目录；本仓库的 Registry 根目录是 `marketplace/`，因此写为 `workflows/skill-routed-native.md`。
- `workflow` 路径必须是单个 Markdown 文件。

```text
trellis_spec/
├── marketplace/
│   ├── index.json
│   └── workflows/
│       └── skill-routed-native.md
└── README.md
```

## 新项目初始化

将 `<账号>/<仓库>` 替换为此 Registry 的远端仓库地址：

```powershell
trellis init --codex --no-monorepo -u <用户名> -y `
  --registry ssh://git@github.com/<账号>/<仓库>/marketplace#main `
  --template empty-spec `
  --workflow skill-routed-native `
  --workflow-source ssh://git@github.com/<账号>/<仓库>/marketplace#main
```

该命令使用 Git SSH 拉取 Workflow，避免 `gh:` 通过 GitHub Raw HTTP 获取旧缓存。执行机器必须已配置对应 GitHub SSH 凭据；未配置时应先解决 SSH 访问，不能静默切换到其他 Registry 源。

初始化后会得到：

- `.trellis/spec/README.md`：空白技术模板，不包含领域代码规则；
- `.trellis/spec/guides/index.md`：原始 `before-dev` 所需的空索引，不包含通用指南；
- `.trellis/workflow.md`：从 `skill-routed-native` 安装的任务流程；
- Codex 内置的 Trellis Skill：由 `--codex` 安装；
- 领域 Skill：由任务 PRD 选择，并由用户环境提供。

`empty-spec` 仅抑制 Trellis 默认生成的通用 Spec。若项目确有无法进入全局 Skill 的本地代码特例，可由用户明确授权后在 `.trellis/spec/` 中维护；不得复制全局 Skill 的规则。

例如任务在 `prd.md` 中记录：

```markdown
## Skill 路由

候选发现：已根据任务范围、技术栈、风险和验证目标筛选当前会话可用的 Skill。

| Skill | 决定 | 使用阶段 | 触发范围 | 原因 |
|---|---|---|---|---|
| `<skill-name>` | 必需 / 条件性 / 不使用 / 待用户调用 | 规划 / 实施 / 检查 / 诊断 / 收尾 | `<条件或边界>` | `<结论依据>` |
```

## 发布前验证

```powershell
$null = Get-Content -Raw -Encoding utf8 marketplace/index.json | ConvertFrom-Json
Test-Path marketplace/workflows/skill-routed-native.md
Test-Path marketplace/specs/empty/README.md
Test-Path marketplace/specs/empty/guides/index.md
git diff --check
```

还应在空目录执行一次完整 `trellis init`，确认 `.trellis/workflow.md` 来自本 Registry，`.trellis/spec/` 仅含空白模板和空指南索引，且未自动安装通用 backend/frontend Spec。首次复杂任务进入规划时，确认 PRD 会记录 Skill 候选发现与路由结论。

## 更新已初始化项目

非原生 workflow 被 Trellis 视为用户维护内容，`trellis update` 不会自动覆盖 `.trellis/workflow.md`。要应用本 Registry 的新版工作流，在项目根目录执行：

```powershell
trellis workflow --template skill-routed-native `
  --marketplace ssh://git@github.com/<账号>/<仓库>/marketplace#main `
  --force
```

命令只替换 `.trellis/workflow.md`。执行前先检查项目的本地 workflow 是否有用户维护的内容；有需要保留的项目特例时，先使用 `--create-new` 比对后再合并。

## 维护边界

- Workflow 只定义任务阶段与 Skill 路由，不复制领域 Skill 内容，也不固定绑定某个领域 Skill。
- 领域代码规则由相应全局 Skill 维护；项目事实、任务范围、验收、调研过程与单次结论由项目事实和任务文档维护。
- `empty-spec` 只用于抑制 CLI 默认 Spec；`.trellis/spec/` 只允许保存经用户确认的项目本地特例，不得复制或覆盖全局 Skill 的默认规则。
- 本 Registry 只维护 Marketplace workflow 与空 Spec 模板；不修改 npm 安装目录或伪造 Agent 调度配置，领域 Skill 仍由用户环境维护。
- 不新增未经 CLI 验证的 Hook、门禁或自动化。
