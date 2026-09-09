# trellis_spec

这是供 Trellis 初始化使用的 Codex 工作流 Registry。

## 提供内容

- `skill-routed-native`：以 Trellis `native` Workflow 为基线的通用任务工作流。它保留原生任务生命周期、`auto/inline` 执行分支、规划、实施、检查和收尾契约；在此基础上增加当前会话 Skill 路由、项目事实与 Spec 边界、`result.md` 结果追溯、统一项目记录、阶段提示和授权边界。

`skill-routed-native` 不固定绑定或安装任何领域 Skill。任务规划只从当前会话已经声明可用的 Skill 中选择实际相关项，并把使用方式、阶段和原因写入 PRD。项目仍按 Trellis 正式 `codex.dispatch_mode` 配置使用原生 `auto` 或 `inline` 分支；本 Registry 不改变该配置，也不替换官方的实施和检查链路。

工作流先按任务价值和风险分流：简单问答、只读查询和低风险且范围自包含的小任务直接处理；需要跨会话接续、明确项目记录或涉及复杂设计与多范围修改时，先询问是否创建 Trellis Task。直接处理只跳过 Trellis 任务生命周期，不跳过项目规则、当前会话中适用的 Skill 和必要验证；范围扩大时必须重新分流。

发生实际修改时，任务用 `result.md` 记录原因、实际结果和验证状态。功能变化、Bug 修复和用户确认的长期决策统一追加到 `docs/项目记录/项目记录.md`；该文件只供用户快速阅读，AI 查询历史时读取对应任务的 `result.md`。任务首次进入规划、实施、检查和收尾时各输出一次 `[trellis]` 阶段提示。

## 授权与收尾边界

规划审查通过并执行 `task.py start` 后，实施、检查、修复和复查连续进行。以下授权仍严格分开：

```text
批准进入实施 != 批准提交 != 批准收尾 != 批准推送
```

- 修改范围扩大时，先更新范围和验证计划并重新确认。
- 提交前必须展示提交计划并等待用户确认；提交授权不包含 Trellis 收尾或推送。
- 任务完成、代码结果和验证结论必须先向用户报告，并取得“任务完成且结果通过”的明确确认；该确认只允许进入提交计划阶段，不等于已经授权提交。
- 提交完成后必须单独确认是否收尾。收尾授权只包含任务归档和会话记录；归档或日志提交仍需单独展示计划并取得确认，不包含推送。
- 待上板、实物或其他外部验收未完成时默认不收尾；只有用户明确允许带未验证项收尾时才可继续。
- 推送始终是独立请求。

授权顺序示例：实现和检查完成后，先报告“改了什么、保持了什么、哪些验证通过、哪些未验证”。用户确认“结果通过”后，才可以展示 Git 提交计划；用户再次确认提交计划后，才可以提交。提交完成后，再单独询问是否归档和记录会话。任何一步没有明确确认，都停在当前阶段。

Trellis CLI 的任务归档和会话记录支持自动创建记账提交。为避免未经授权改变 Git 历史，本 Registry 要求初始化后的项目默认关闭该能力，并在项目 `.trellis/config.yaml` 中保留以下显式设置：

```yaml
session_auto_commit: false
```

该设置关闭归档/会话记录的自动 Git 提交，不会替代归档授权，也不会影响工作代码提交。若配置缺失、值为 `true` 或值无法解析，工作流不得调用会触发自动提交的归档/会话命令，必须先报告配置问题并由用户处理；不得把默认值当成授权。

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
  --workflow skill-routed-native `
  --workflow-source ssh://git@github.com/<账号>/<仓库>/marketplace#main
```

该命令使用 Git SSH 拉取 Workflow，避免 `gh:` 通过 GitHub Raw HTTP 获取旧缓存。执行机器必须已配置对应 GitHub SSH 凭据；未配置时应先解决 SSH 访问，不能静默切换到其他 Registry 源。

初始化后会得到：

- `.trellis/spec/backend/`、`.trellis/spec/frontend/`：由 Trellis 按项目类型生成的通用开发规范；
- `.trellis/spec/guides/`：Trellis 提供的跨层、跨平台和代码复用指南；
- `.trellis/workflow.md`：从 `skill-routed-native` 安装的任务流程；
- Codex 内置的 Trellis Skill：由 `--codex` 安装；
- 任务需要的其他 Skill：由当前会话提供，并在 PRD 中记录实际路由结果。

`empty-spec` 仅为已经使用该模板的旧项目保留兼容，不再用于新项目初始化。新项目允许使用 Trellis 通用 Spec；项目确有本地代码约束时，可在实际规则基础上补充，但不得把领域 Skill 的整套方法重复复制进来。

例如任务在 `prd.md` 中记录：

```markdown
## Skill 路由

| Skill | 使用方式 | 使用阶段 | 原因或触发条件 |
|---|---|---|---|
| `<skill-name>` | 自动 / 条件 / 待用户调用 | 规划 / 实施 / 检查 / 诊断 / 收尾 | `<为什么需要，或何时触发>` |
```

## 发布前验证

```powershell
$null = Get-Content -Raw -Encoding utf8 marketplace/index.json | ConvertFrom-Json
Test-Path marketplace/workflows/skill-routed-native.md
Test-Path marketplace/specs/empty/README.md
Test-Path marketplace/specs/empty/guides/index.md
Select-String -Path marketplace/workflows/skill-routed-native.md -SimpleMatch 'Git 提交前展示提交计划并取得用户当轮明确授权'
Select-String -Path marketplace/workflows/skill-routed-native.md -SimpleMatch '| Skill | 使用方式 | 使用阶段 | 原因或触发条件 |'
Select-String -Path marketplace/workflows/skill-routed-native.md -SimpleMatch '[trellis] 阶段=提交 动作=等待用户确认'
Select-String -Path marketplace/workflows/skill-routed-native.md -SimpleMatch '[trellis] 阶段=收尾 动作=等待用户确认'
git diff --check
```

还应在提交推送后通过远端 Registry 在空目录执行一次完整 `trellis init`；CLI 不支持本地路径作为 Registry，不得用手工复制代替初始化验证。确认 `.trellis/workflow.md` 来自本 Registry，`.trellis/spec/` 已按项目类型生成官方通用 backend/frontend/guides 规范。检查安装后的 Workflow 同时包含原生 `trellis-brainstorm`、`trellis-before-dev`、`trellis-check`、`codex.dispatch_mode` 分支，以及本 Registry 的 `Skill 路由`、`[trellis]` 阶段提示、`result.md`、统一项目记录和提交/收尾授权。首次复杂任务进入规划时，确认 PRD 只记录实际相关的 Skill。

## 更新已初始化项目

非原生 workflow 被 Trellis 视为用户维护内容，`trellis update` 不会自动覆盖 `.trellis/workflow.md`。要应用本 Registry 的新版工作流，在项目根目录执行：

```powershell
trellis workflow --template skill-routed-native `
  --marketplace ssh://git@github.com/<账号>/<仓库>/marketplace#main `
  --force
```

命令只替换 `.trellis/workflow.md`。执行前先检查项目的本地 workflow 是否有用户维护的内容；有需要保留的项目特例时，先使用 `--create-new` 比对后再合并。

## 维护边界

- Workflow 以当前 Trellis 原生 Workflow 为上游基线，只增加 Skill 路由、结果追溯、项目记录和授权边界；不复制领域 Skill 内容，也不固定绑定某个领域 Skill。Trellis CLI 升级后必须先对照新的原生 Workflow，再迁移本 Registry 扩展。
- 项目事实由 README、产品文档、原理图和权威项目证据维护；任务范围、验收、研究和结果由任务文档维护；项目变更与长期决策统一写入 `docs/项目记录/项目记录.md`，不额外维护项目概览、当前状态或滚动快照。
- 新项目保留 Trellis 按项目类型生成的通用 Spec；`empty-spec` 只为旧项目兼容保留，不再作为初始化默认值。项目本地 Spec 只保存用户确认的项目编码约定，不复制适用 Skill 的整套方法。
- 本 Registry 只维护 Marketplace Workflow 与空 Spec 模板；不修改 npm 安装目录、不强制 `codex.dispatch_mode`、不伪造 Agent 调度配置，也不负责安装其他 Skill。
- 不新增未经 CLI 验证的 Hook、门禁或自动化。
