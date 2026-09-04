# Trellis 原生工作流与通用 Skill 路由

---

> 上游基线：Trellis CLI `0.6.14` 的 `native` Workflow，源模板 SHA-256：`E2C5AB7004FF83A5A804B50DF81746AA1D558DD4480463287622605F86A82A76`。
>
> 维护要求：本文件保留上游状态机、平台标记和任务契约；升级 Trellis CLI 后，必须先对照新的原生 Workflow 复核，再迁移本文件中的 Registry 扩展。不得将本文件重新简化为独立流程。
>
> 语言说明：本文件正文使用简体中文，便于项目开发时直接阅读。状态标签、平台标记、命令、路径和机器可读字段保持原样，确保 Trellis 解析与升级核对不受影响。其中 `## Phase Index` 和 `## Phase 1: Plan` 是官方解析器使用的精确边界标题，禁止翻译、添加中文后缀或调整字符。

## Registry 自定义索引

本索引只标识相对官方 `native 0.6.14` 发生的行为差异，不把中文翻译视为自定义。标题以 `Registry 扩展` 开头的章节均为本 Registry 新增；下表列出其余嵌入原生阶段的自定义。未列入本索引的内容保持官方原生语义。

| 位置 | 类型 | Registry 自定义内容 |
|---|---|---|
| 核心原则 5-7 | 改写 / 新增 | 将原生的统一 Spec 回写改为按归属持久化，并增加全局 Skill 发现和事实唯一归属。 |
| `Registry 扩展：全局 Skill 发现与路由` | 新增 | 发现当前会话可用 Skill，将选用阶段、条件和原因记录到 PRD。 |
| `Registry 扩展：项目与 Demo 路由` | 新增 | 先核对项目事实，再决定是否查询 Demo，并记录允许复用的边界。 |
| `Registry 扩展：事实、项目 Spec 与复用边界` | 新增 | 区分项目事实、任务产物、项目记录、本地 Spec 和全局 Skill 的职责。 |
| Trellis 系统 / Spec 系统 | 增补 | 保留官方 Spec 基线，只把已确认的项目约定写入本地 Spec。 |
| Trellis 系统 / Task 系统 | 增补 | 将 `result.md` 纳入任务产物。 |
| Trellis 系统 / 授权与外部状态门禁 | 新增 | 分离规划、实施、检查、提交、归档、会话记录和推送授权。 |
| Phase Index / 请求分流 | 改写 | 简单、只读或低风险自包含工作可直接处理；需要持续追踪时才创建任务。 |
| Phase Index / 规划产物 | 增补 | PRD 增加 Skill 路由和复用决策，并增加 `result.md` 结果产物。 |
| Phase Index / 状态标签、活动任务路由和护栏 | 增补 / 改写 | 增加 Skill/Demo 路由、结果记录、长期记录归属、可见 `[trellis]` 阶段提示和独立授权门禁。 |
| Phase 1 / 1.0、1.1、1.4、1.5 | 增补 | 增加阶段提示、Skill/Demo 评估、`task.py validate` 预检和路由就绪条件。 |
| Phase 2 / 2.1、2.2 | 增补 | 实现前加载已路由 Skill；每批代码按路由检查，并区分代码、构建和外部验证。 |
| Phase 2 / 外部验证与失败停止 | 新增 | 使用单变量测试卡；同一现象连续失败后停止试错并进入诊断复盘。 |
| Phase 2 / 2.4 记录结果 | 新增 | 生成 `result.md`，记录实际变更、验证状态、Bug 根因和持久化候选。 |
| Phase 3 / 3.3 | 改写 | 将原生的统一 Spec 更新改为候选归属判断，再按确认结果更新项目记录、本地 Spec 或全局 Skill。 |
| Phase 3 / 3.4、3.5 | 改写 | 提交、归档、会话记录和推送分别授权；存在未决外部验证时默认不归档。 |
| 自定义 Trellis / 修改步骤含义 | 增补 | 同步本 Registry 的直接处理边界以及提交、归档前置要求。 |

## 核心原则

1. **先规划再编码**：开始实现前，先明确要解决的问题、边界和验收方式。
2. **注入规范而不是凭记忆**：通过 Hook 或 Skill 注入规范，不要求 AI 依靠上下文记住全部规则。
3. **持久化重要信息**：研究、决策和经验写入文件；对话可能被压缩，文件不会因此丢失。
4. **增量开发**：一次处理一个可验证的任务。
5. **记录可复用结论**：任务结束后识别长期有效的结论，只写入已经确认的归属位置。
6. **先发现 Skill 再路由**：选择任务能力前，检查当前会话实际可用的全局 Skill。
7. **每类事实都有唯一归属**：项目事实写入权威项目文档；任务决策和已验证根因写入任务产物；长期项目决策写入项目决策记录；可复用方法写入所属全局 Skill。

---

## Registry 扩展：全局 Skill 发现与路由

Trellis 负责任务生命周期、任务产物、上下文和通用检查。它不会安装、复制或永久绑定某个领域、框架或产品 Skill。在进入实现规划前，先发现当前会话可用的全局 Skill，阅读候选 Skill 的说明，再把路由结果记录到 `{TASK_DIR}/prd.md`。

```markdown
## Skill 路由

候选发现：已根据任务范围、技术栈、风险和验证目标筛选当前会话可用的 Skill。

| Skill | 决定 | 使用阶段 | 触发范围 | 原因 |
|---|---|---|---|---|
| `<skill-name>` | 必需 / 条件性 / 不使用 / 待用户调用 | 规划 / 实施 / 检查 / 诊断 / 收尾 | `<条件或边界>` | `<结论依据>` |
```

- 候选评估至少覆盖：领域与技术栈、需求与架构、实现与构建、测试与验证、诊断与调试、审查与质量、文档与交付。不相关的类别可以合并记录为“不使用”，不需要列出全部已安装 Skill。
- 标注为必须手动调用的 Skill，只能记录为“待用户调用”，不能声称它会自动运行。
- 必需 Skill 必须在对应阶段开始前加载；条件性 Skill 只有在触发条件成立后才加载。
- 任务范围、技术栈、风险或验证目标发生变化时，重新评估路由，并在继续之前更新 `prd.md`。
- 不要在本 Workflow 中维护“领域到 Skill”的固定映射表。Skill 自己负责匹配和实现规则，本 Workflow 只记录当前任务的实际决定。

## Registry 扩展：项目与 Demo 路由

Trellis 判断任务是否需要读取当前项目事实或参考 Demo；具体的查询和复用规则由选中的领域 Skill 负责。项目事实和验收标准明确后，在 `prd.md` 中加入下面这张任务专属表：

```markdown
## 复用决策

| 来源 | 操作 | 决定 | 触发条件或查询目标 | 结果与边界 |
|---|---|---|---|---|
| 当前项目 | 读取事实 | 直接处理 / 需要补充事实 | `<README、Doc、原理图、代码或官方资料>` | `<已确认事实或待确认项>` |
| 参考 Demo | 查询 | 必须 / 条件性 / 不查询 | `<接口形式、目录结构或稳定写法>` | `<允许复用与禁止照搬的边界>` |
```

- 先读取权威项目事实。影响当前设计或实现的事实缺失时必须暂停；不能用 Demo 的猜测补齐。
- 当领域 Skill 要求保持接口形式、目录结构或已验证写法一致时，才查询 Demo。允许复用的内容由领域 Skill 决定；未经当前项目确认，不得从 Demo 复制板级、产品、凭据、协议或环境事实。
- 当前项目事实已经充分且没有 Demo 触发条件时，可以直接推理，不要为了形式执行查询。
- 任务范围、现象、技术栈或验证目标发生实质变化时，继续之前同时更新 `Skill 路由` 和 `复用决策`。

## Registry 扩展：事实、项目 Spec 与复用边界

- README、产品文档、原理图、权威 API 和项目证据负责保存真实项目事实。
- `.trellis/spec/` 以 Trellis 提供的包/层级指南为基础，可以根据已确认的项目编码约定进行细化。它必须与真实代码保持一致，不能把某个全局 Skill 的完整领域流程复制进来。
- 任务的 `prd.md`、`design.md`、`implement.md`、`result.md` 和 `research/` 只记录本次任务的范围、决策、证据、调查、实际结果和验证。
- 写入项目记录前先确定唯一归属。项目已有等价的变更记录或决策记录时直接复用；没有时，行为变化和 Bug 修复使用 `docs/变更记录/项目变更记录.md`，用户确认的长期决策使用 `docs/决策/项目决策记录.md`。只有出现第一条符合条件的记录时才创建文件，不要为同一职责维护并行文件。
- 任务中发现的长期结论，必须先完成归属分类再写入。归属只能是项目决策记录、全局 Skill、项目本地 Spec、项目事实文档或仅任务结果，并遵守对应归属的确认和审计规则。
- 不要重新创建 `项目概览`、`当前状态` 或滚动任务快照作为另一套记忆系统。权威项目文档、Trellis 任务产物、上述两个项目索引和 Git 已经承担这些职责。

## Trellis 系统

### 开发者身份

首次使用时初始化开发者身份：

```bash
python3 ./.trellis/scripts/init_developer.py <your-name>
```

命令会创建 `.trellis/.developer`（由 Git 忽略）和 `.trellis/workspace/<your-name>/`。

### Spec 系统

`.trellis/spec/` 按包和层级保存编码指南。

- `.trellis/spec/<package>/<layer>/index.md` 是入口，包含**开发前检查**和**质量检查**；具体规则写在它引用的 `.md` 文件中。
- `.trellis/spec/guides/index.md` — cross-package thinking guides.

Registry 规则：保留 Trellis 提供的 backend/frontend/guides 基线，并根据真实代码示例确认过的项目约定进行细化。通用领域方法和生命周期流程继续放在负责它们的全局 Skill 中。

```bash
python3 ./.trellis/scripts/get_context.py --mode packages   # list packages / layers
```

**何时更新项目本地 Spec**：项目形成了已确认且长期有效的编码约定，或 Trellis 基线需要根据真实代码进行项目化细化；并且该结论不属于全局 Skill 或项目事实文档，用户也确认由本地 Spec 负责。

### Task 系统

每个任务在 `.trellis/tasks/{MM-DD-name}/` 下拥有独立目录，包含 `task.json`、`prd.md`，以及可选的 `design.md`、`implement.md`、完成后的 `result.md`、`research/` 和面向支持子代理平台的上下文清单（`implement.jsonl`、`check.jsonl`）。

```bash
# 任务生命周期
python3 ./.trellis/scripts/task.py create "<title>" [--slug <name>] [--parent <dir>]
python3 ./.trellis/scripts/task.py start <name>          # set active task (session-scoped when available)
python3 ./.trellis/scripts/task.py current --source      # 显示活动任务及其来源
python3 ./.trellis/scripts/task.py finish                # clear active task (triggers after_finish hooks)
python3 ./.trellis/scripts/task.py archive <name>        # 移动到 archive/{year-month}/
python3 ./.trellis/scripts/task.py list [--mine] [--status <s>]
python3 ./.trellis/scripts/task.py list-archive

# 代码规范上下文（通过 JSONL 注入 implement/check 代理）。
# 支持子代理的平台会在 `task create` 时生成 `implement.jsonl` / `check.jsonl` 初始文件；
# 规划阶段由 AI 按需整理真实 Spec 和研究条目。
python3 ./.trellis/scripts/task.py add-context <name> <action> <file> <reason>
python3 ./.trellis/scripts/task.py list-context <name> [action]
python3 ./.trellis/scripts/task.py validate <name>

# 任务元数据
python3 ./.trellis/scripts/task.py set-branch <name> <branch>
python3 ./.trellis/scripts/task.py set-base-branch <name> <branch>    # PR target
python3 ./.trellis/scripts/task.py set-scope <name> <scope>

# 层级关系（父任务/子任务）
python3 ./.trellis/scripts/task.py add-subtask <parent> <child>
python3 ./.trellis/scripts/task.py remove-subtask <parent> <child>

# 创建 PR
python3 ./.trellis/scripts/task.py create-pr [name] [--dry-run]
```

> 使用 `python3 ./.trellis/scripts/task.py --help` 查看当前版本的权威命令列表。

**当前任务机制**：`task.py create` 创建任务目录；当会话身份可用时，还会自动设置当前会话的活动任务指针，使规划提示立即生效。`task.py start` 写入同一个指针（已设置时保持幂等），并把 `task.json.status` 从 `planning` 改为 `in_progress`。状态保存在 `.trellis/.runtime/sessions/`。如果 Hook 输入、`TRELLIS_CONTEXT_ID` 或平台原生会话环境变量都没有提供上下文键，则不会存在活动任务，`task.py start` 会失败并提示如何设置会话身份。`task.py finish` 删除当前会话文件，但不改变任务状态。`task.py archive <task>` 写入 `status=completed`，将任务目录移动到 `archive/`，并删除仍指向该任务的运行时会话文件。

### 授权与外部状态门禁

Trellis 任务状态和 Git 状态彼此独立。本轮将下列操作视为相互独立的授权：

```text
批准规划  !=  批准实施
批准实施  !=  批准检查
批准检查  !=  批准提交
批准提交  !=  批准归档
批准归档  !=  批准推送
```

- `批准实施` 只授权当前预检/检查点中列出的文件和动作，绝不包含 `git add`、`git commit`、`task.py archive`、`/finish-work`、`add_session.py` 或 `git push`。
- 只有在紧接着已展示的提交计划之后，`ok` 或“行”才表示批准提交。批准规划、批准实施或笼统的“继续/完成”都不表示批准提交或归档。
- 如果要编辑当前允许范围之外的差异，必须先停止，更新范围和验证计划，并取得扩大范围的批准。
- 工作提交、任务归档、日志/会话记录和推送是相互独立的外部状态操作。每次操作前都要输出可见的 `[trellis]` 行，说明阶段、动作和所需确认；只执行用户在当前轮明确确认的操作，不能从之前的批准推断下一步。
- 任何外部、实物或用户验收项仍为 `待用户验证` 或未勾选时，任务保持进行中，默认不得归档。带未决验证项归档必须得到用户明确批准，并在 `result.md` 保留未决项。构建成功或静态审查通过不等于外部验证通过。
- Trellis 不会静默改变 CLI 的记账默认值。如果项目不希望归档/会话命令创建 Git 提交，应设置 `session_auto_commit: false`，或在归档已获授权后使用 `task.py archive <task> --no-commit`。该设置不等于归档授权，也不影响工作提交。

### 工作区系统

工作区系统把每次 AI 会话记录到 `.trellis/workspace/<developer>/`，用于跨会话追踪。

- `journal-N.md` 是会话日志。**每个文件最多 2000 行**，超过后自动创建 `journal-(N+1).md`。
- `index.md` 是个人索引，记录会话总数和最近活动。

```bash
python3 ./.trellis/scripts/add_session.py --title "Title" --commit "hash" --summary "Summary"
```

### 上下文脚本

```bash
python3 ./.trellis/scripts/get_context.py                            # full session runtime
python3 ./.trellis/scripts/get_context.py --mode packages            # available packages + spec layers
python3 ./.trellis/scripts/get_context.py --mode phase --step <X.Y>  # detailed guide for a workflow step
```

---

<!--
  工作流状态提示契约（编辑下方标签块前必须阅读）

  下方 ## Phase Index 中的 [workflow-state:STATUS] 标签块，是所有支持平台的
  UserPromptSubmit Hook 每轮读取的 `<workflow-state>` 提示的唯一事实源。
  inject-workflow-state.py（Python 平台）和 inject-workflow-state.js（OpenCode 插件）
  只负责解析这些标签；v0.5.0-rc.0 之后，脚本中不再内置 fallback 字典。

  STATUS 字符集必须是 [A-Za-z0-9_-]+。Hook 找不到标签时，会降级为通用提示
  "Refer to workflow.md for current step."；该提示故意保持可见，以便发现并修复损坏的 workflow.md。

  不变量（test/regression.test.ts）：
    每个标记为 `[required · once]` 的工作流步骤，都必须在所属阶段的
    [workflow-state:*] 标签块中有对应的强制执行提示。该提示是每轮唯一的状态通道；
    如果必需步骤没有写进去，AI 可能静默跳过它（Phase 1 规划门禁和 Phase 3.4
    提交门禁都曾因该缺口被跳过）。

  标签与阶段的对应关系：
[workflow-state:no_task]      → 没有活动任务；Phase 1 之前
[workflow-state:planning]     → Phase 1 全部内容（status='planning'）
[workflow-state:planning-inline] → Codex inline 模式的 Phase 1
    [workflow-state:in_progress]  → Phase 2 + Phase 3.2-3.4
                                    （从 task.py start 到 task.py archive 前，状态保持为 in_progress）
[workflow-state:in_progress-inline] → Codex inline 模式的 Phase 2/3
    [workflow-state:completed]    → 当前不会触发：cmd_archive 在同一次调用中修改状态并
                                    移动任务目录，解析器因此丢失指针（保留此块供未来显式的
                                    in_progress->completed 状态迁移使用）

  编辑检查清单：
    - 修改 [workflow-state:STATUS] 标签块时，同时检查所属阶段的
      `[required · once]` 步骤是否同步
    - 编辑后运行 `trellis update`，将新正文按标签块替换到下游项目
    - 完整运行时契约：
      .trellis/spec/cli/backend/workflow-state-contract.md
-->

## Phase Index

```
Phase 1: 规划    → 判断任务类型；需要追踪时取得同意并写入规划产物
Phase 2: 实施    → 只有任务状态为 in_progress 后才开始编码
Phase 3: 收尾    → 确定记录归属，在允许时更新长期记录，提交并完成收尾
```

### 请求分流

- 简单对话、只读查询或低风险且自包含的小任务：直接处理，不要求创建 Trellis 任务。这里只跳过 Trellis 任务生命周期；仓库规则、适用的全局 Skill 和适当的验证仍然适用。
- 需要跨会话连续性或明确项目记录的轻量任务：先请求创建任务的同意，再优先使用只有 PRD 的路径；如果出现设计或执行复杂度，再升级为完整规划。
- 复杂任务：先询问是否可以创建 Trellis 任务并进入规划。如果用户不同意，不进行大范围的直接实现，而是解释、澄清范围或建议拆分为更小任务。
- 如果直接处理的工作超出自包含小任务范围，必须在大范围实现前停止并重新分流。用户同意创建任务不等于同意开始实现；仍需先完成规划。

### 规划产物

- `prd.md`：需求、约束和验收标准。不要在这里写技术设计或执行清单。
- `prd.md` 同时记录当前任务的 `## Skill 路由` 结果。它说明需要哪些能力以及由哪个阶段负责，不是技术设计，也不能替代所属 Skill。
- `prd.md` 记录当前任务的 `## 复用决策`：当前项目事实和参考 Demo。只记录决定与证据，不复制 Demo 代码。
- `design.md`：复杂任务的技术设计，包括边界、契约、数据流、取舍、兼容性和发布/回滚形态。
- `implement.md`：复杂任务的执行计划，包括有序清单、验证命令、审查门禁和回滚点。
- `result.md`：实际修改原因、结果、验证状态、未决风险和可选的 Bug 分析。实现和最终检查后创建；它不是第二份 PRD，也不是 diff 的复制品。
- `implement.jsonl` / `check.jsonl`：注入子代理上下文的 Spec 和研究清单，不能替代 `implement.md`。
- 轻量任务可以只有 PRD。复杂任务必须在 `task.py start` 前具备并审查 `prd.md`、`design.md` 和 `implement.md`。

### 父子任务树

当一个用户请求包含多个可以独立验证的交付物时，使用父任务。父任务负责原始需求、任务地图、跨子任务验收标准和最终集成审查；除非父任务本身也有直接实现工作，否则它通常不作为编码目标。

对可以独立规划、实现、检查和归档的交付物使用子任务。父子结构不是依赖系统：如果一个子任务必须等待另一个子任务，必须在该子任务的 `prd.md` / `implement.md` 中写明顺序，并保证每个子任务的验收标准可测试。

使用 `task.py create "<title>" --slug <name> --parent <parent-dir>` 创建子任务；使用 `task.py add-subtask <parent> <child>` 关联已有任务；关联错误时使用 `task.py remove-subtask <parent> <child>` 解除关联。

<!-- 每轮提示：没有活动任务时显示（Phase 1 之前） -->

[workflow-state:no_task]
当前没有活动任务。先判断本轮请求；只有确实需要 Trellis 追踪时才请求创建任务。
简单对话、只读查询或低风险且自包含的小任务：直接处理，不要求创建任务。仓库规则、适用的全局 Skill 和适当的验证仍然适用。如果范围扩大，必须在大范围实现前停止并重新分流。
需要跨会话连续性或明确项目记录的轻量工作：请求创建任务的同意，然后优先使用只有 PRD 的路径。
复杂任务：询问用户是否允许创建 Trellis 任务并进入规划。如果用户不同意，解释情况、澄清范围或建议拆成更小的任务。
[/workflow-state:no_task]

### Phase 1: 规划
- 1.0 创建任务 `[required · once]`（仅在用户同意创建任务后）
- 1.1 需求探索 `[required · repeatable]`（写入 `prd.md`；复杂任务还需要 `design.md` 和 `implement.md`）
- 1.2 研究 `[optional · repeatable]`
- 1.3 配置上下文 `[required · once]`（仅适用于派发子代理的平台；inline 平台跳过）
- 1.4 激活任务 `[required · once]`（审查通过后运行 `task.py start`；状态变为 `in_progress`）
- 1.5 完成条件

<!-- 每轮提示：Phase 1 全程显示（status='planning'） -->

[workflow-state:planning]
加载 `trellis-brainstorm`，保持在规划阶段。
轻量任务：可以只使用 `prd.md`。复杂任务：完成 `prd.md`、`design.md` 和 `implement.md`，并在 `task.py start` 前请求审查。
多个交付物：考虑使用父任务和可独立验证的子任务；依赖关系必须写入子任务产物，不能依靠树的位置暗示。
子代理模式：在 start 前整理 `implement.jsonl` 和 `check.jsonl`，作为 Spec/研究清单。
激活前，发现并把当前任务的全局 Skill 路由和复用决定记录到 `prd.md`；加载必需的规划 Skill，把只能手动调用的 Skill 记录为“待用户调用”。
[/workflow-state:planning]

<!-- 每轮提示：codex.dispatch_mode=inline 时在 Phase 1 全程显示。
     这是 Codex 专用的可选路径，用于替代 [workflow-state:planning]。
     主会话在 Phase 2 直接编辑代码，因此跳过 JSONL 整理；inline 工作流通过
     `trellis-before-dev` 加载上下文，而不是将 JSONL 注入子代理。 -->

[workflow-state:planning-inline]
加载 `trellis-brainstorm`，保持在规划阶段。
轻量任务：可以只使用 `prd.md`。复杂任务：完成 `prd.md`、`design.md` 和 `implement.md`，并在 `task.py start` 前请求审查。
多个交付物：考虑使用父任务和可独立验证的子任务；依赖关系必须写入子任务产物，不能依靠树的位置暗示。
Inline 模式：跳过 JSONL 整理；Phase 2 通过 `trellis-before-dev` 读取产物和 Spec。
激活前，发现并把当前任务的全局 Skill 路由和复用决定记录到 `prd.md`；加载必需的规划 Skill，把只能手动调用的 Skill 记录为“待用户调用”。
[/workflow-state:planning-inline]

### Phase 2: 实施
- 2.1 实现 `[required · repeatable]`
- 2.2 质量检查 `[required · repeatable]`
- 2.3 回滚 `[on demand]`
- 2.4 记录结果 `[required · once]`

<!-- 每轮提示：status='in_progress' 时显示。
     范围是 Phase 2 和 Phase 3.2-3.4 的全部内容（从 task.py start 到
     task.py archive 前状态一直是 in_progress，只有 archive 会改变状态）。因此这里
     必须覆盖从实现到提交的每个必需步骤，包括 Phase 3.3 的归属/长期记录更新和 Phase 3.4 提交。 -->

子代理派发协议适用于所有平台和子代理，包括使用原生 Codex `SubagentStart` 注入上下文并保留子代理主动读取方式的平台、Gemini/Qoder/Copilot/Reasonix/Trae/Grok/Kimi Code、由 Hook 支持的 ZCode/Snow，以及 `trellis-research`。每个派发提示都必须先写 `Active task: <task path from task.py current>`，再写角色说明。在 Grok Build 中，使用 `spawn_subagent`，并将 `subagent_type` 设置为 Trellis 代理名称（例如 `trellis-implement`）。在 Kimi Code 中，使用内置的 `coder` / `explore` 子代理，并遵循对应的 `.kimi-code/skills/trellis-<role>/SKILL.md` 指令。

[workflow-state:in_progress]
工具：`trellis-implement` / `trellis-research` 仅是子代理类型（由 Task/Agent 工具使用，不是 Skill；不存在同名 Skill）。`trellis-check` 同时存在两种形式；代码修改后进行验证时优先使用 Agent 形式。只有在用户确认某候选属于项目本地 Spec 后，才可以加载 `trellis-update-spec`。
流程：`trellis-implement` -> `trellis-check` -> 记录 `result.md`（Phase 2.4）-> 审查候选归属并更新长期记录（Phase 3.3）-> 展示提交计划并等待用户明确确认 -> 只执行提交 -> 展示归档状态并等待用户明确确认 -> 可选执行 `/trellis:finish-work`。
外部状态门禁：`批准实施` 不包含提交、归档、会话记录或推送。每次执行这些动作前都输出 `[trellis] 阶段=<阶段> 动作=<动作> 状态=等待用户确认`；外部/实物验证未完成时，除非用户明确批准带未决项归档，否则任务保持进行中。
主会话默认派发 implement/check 子代理。子代理自豁免：如果当前已经是 `trellis-implement`，不要再派发 `trellis-implement` 或 `trellis-check`；如果当前已经是 `trellis-check`，不要再派发 `trellis-check` 或 `trellis-implement`。派发动作只由主会话执行。
派发提示必须以 `Active task: <task path from task.py current>` 开头。上下文读取顺序：JSONL 条目 -> `prd.md` -> 存在时读取 `design.md` -> 存在时读取 `implement.md`。
派发前读取 `prd.md` 中的 Skill 路由和复用决策。实现或检查代理必须能够使用任务要求的 Skill；除正式的 implement/check 链之外，只能使用文档明确允许的、独立且只读的子代理。最终集成和结论由主会话负责。
[/workflow-state:in_progress]

<!-- 每轮提示：codex.dispatch_mode=inline 且 status='in_progress' 时显示。
     这是 Codex 专用的可选路径，用于替代 [workflow-state:in_progress]；
     主会话直接编辑代码，不派发子代理。 -->

[workflow-state:in_progress-inline]
流程：`trellis-before-dev` -> 编辑 -> `trellis-check` -> 验证 -> 记录 `result.md`（Phase 2.4）-> 审查候选归属并更新长期记录（Phase 3.3）-> 展示提交计划并等待用户明确确认 -> 只执行提交 -> 展示归档状态并等待用户明确确认 -> 可选执行 `/trellis:finish-work`。
外部状态门禁：`批准实施` 不包含提交、归档、会话记录或推送。每次执行这些动作前都输出 `[trellis] 阶段=<阶段> 动作=<动作> 状态=等待用户确认`；外部/实物验证未完成时，除非用户明确批准带未决项归档，否则任务保持进行中。
Inline 模式不派发 implement/check 子代理。
上下文读取顺序：`prd.md` -> 存在时读取 `design.md` -> 存在时读取 `implement.md`，以及由 Skill 加载的相关 Spec/研究资料。
执行 `trellis-before-dev` 后，读取 `prd.md` 中的 Skill 路由和复用决策，然后在编辑前加载必需的实现或验证 Skill。条件性 Skill 必须先记录触发条件，再加载使用。
[/workflow-state:in_progress-inline]

### Phase 3: 收尾
- 3.2 调试回顾 `[on demand]`
- 3.3 候选归属与长期记录更新 `[required · once]`
- 3.4 提交修改 `[required · once]`
- 3.5 收尾提醒

> 注意：3.1 已并入 2.2（最后一轮全范围检查）和 3.4（提交前置检查）。保留原编号是为了不破坏外部引用。

<!-- 每轮提示：status='completed' 时显示。
     当前正常流程中不会触发：cmd_archive 在移动任务目录到 archive/ 的同一次调用中
     写入 status='completed'，活动任务解析器随后丢失指针，因此归档任务不会触发 Hook。
     保留此标签块是为未来的状态迁移设计（例如显式的 in_progress->completed 命令）准备。
     修改方式与其他活动标签块相同。 -->

[workflow-state:completed]
任务已由用户明确归档。不要从此状态推断需要归档或推送；推送仍是单独授权的操作。如果任务只是代码完成但外部验证仍未完成，不得进入此状态。
[/workflow-state:completed]

### 规则

1. 先确定当前处于哪个 Phase，再从该阶段的下一步继续。
2. 每个 Phase 内按顺序执行；`[required]` 步骤不能跳过。
3. Phase 可以回退（例如实施阶段发现 PRD 缺陷，应返回规划阶段修正后再进入实施）。
4. 标记为 `[once]` 的步骤如果产物已经存在则跳过，不要重复执行。
5. 根据产物是否存在判断下一步；轻量任务缺少 `design.md` / `implement.md` 是允许的，复杂任务缺少它们则表示规划未完成。

### 活动任务路由

在活动任务中，如果用户请求匹配下面某个意图，先完成路由，再按需加载对应阶段的详细步骤。

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

- 规划或需求不清晰 -> `trellis-brainstorm`。
- `in_progress` 状态下的实现/检查 -> 派发 `trellis-implement` / `trellis-check`。
- 重复调试 -> `trellis-break-loop`；更新任何 Spec 前先审查候选归属。只有用户确认由项目本地 Spec 负责后，才加载 `trellis-update-spec`。

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

[codex-inline, Kilo, Antigravity, Devin]

- 规划或需求不清晰 -> `trellis-brainstorm`。
- 编辑前 -> `trellis-before-dev`；编辑后 -> `trellis-check`。
- 重复调试 -> `trellis-break-loop`；更新任何 Spec 前先审查候选归属。只有用户确认由项目本地 Spec 负责后，才加载 `trellis-update-spec`。
- 任务范围、事实、技术栈、风险或验证目标发生实质变化时，继续之前重新评估 `prd.md` 中的 Skill 路由和复用决策；普通阶段转换只读取已有决定。

[/codex-inline, Kilo, Antigravity, Devin]

### 护栏

- 同意创建任务不等于同意实现；实现必须等待产物审查通过并执行 `task.py start`。
- 只有 PRD 对轻量任务是有效的；复杂任务需要 `design.md` 和 `implement.md`。
- 规划必须持久化到任务产物；完成检查并在 `result.md` 记录实际结果后，才能报告任务完成。

### 加载步骤详情

在每个步骤中运行以下命令获取详细指引：

```bash
python3 ./.trellis/scripts/get_context.py --mode phase --step <step>
# e.g. python3 ./.trellis/scripts/get_context.py --mode phase --step 1.1
```

---

## Phase 1: Plan

目标：判断请求类型；只有需要 Trellis 追踪时才进入本阶段；取得创建任务的同意；生成实现前必须具备的规划产物。

#### 1.0 创建任务 `[required · once]`

只有在用户同意创建任务后才创建任务目录。命令会把状态设为 `planning`、写入 `task.json`、创建默认 `prd.md`；会话身份可用时，还会自动把新任务设为当前目标：

创建任务前输出 `[trellis] 阶段=规划 动作=进入`。每个任务只输出一次阶段进入标记，不要在每轮规划中重复输出。

```bash
python3 ./.trellis/scripts/task.py create "<task title>" --slug <name>
```

`--slug` 只填写人类可读的名称。**不要**包含 `MM-DD-` 日期前缀；`task.py create` 会自动添加。

使用任务树时，先创建父任务，再用 `--parent <parent-dir>` 创建各个子任务。不要因为存在子任务就启动父任务；应启动负责下一个可独立验证交付物的子任务。

命令成功后，每轮提示会自动切换到 `[workflow-state:planning]`，要求 AI 留在规划阶段。

这里仅运行 `create`，不要同时运行 `start`。`start` 会把状态改为 `in_progress`，在规划产物审查前就切换到实现阶段。`start` 应留到 1.4 步骤执行。

如果 `python3 ./.trellis/scripts/task.py current --source` 已经指向任务，则跳过此步骤。

#### 1.1 需求探索 `[required · repeatable]`

加载 `trellis-brainstorm` Skill，并按照该 Skill 的指引与用户交互探索需求。

brainstorm Skill 会引导你：
- 一次只问一个问题
- 能研究确认的内容优先研究，不直接询问用户
- 优先提供选项，而不是开放式提问
- 每次用户回答后立即更新 `prd.md`
- 当交付物可以独立验证时，把大范围拆成父任务和子任务
- 保持 `prd.md` 只关注需求和验收标准
- 复杂任务必须在开始实现前生成 `design.md` 和 `implement.md`

需求和验收标准足够清晰后，发现当前会话可用的全局 Skill，并把 Registry 的 `## Skill 路由` 和 `## 复用决策` 表加入 `prd.md`。按能力类别评估候选 Skill，再独立判断是否需要当前项目事实或参考 Demo。实现开始前，用户可以修正任一项路由结论。

需要查询 Demo 时，记录选中的参考、复用边界以及被拒绝的板级/产品事实。如果 Demo 与当前权威项目事实冲突，以当前事实为准并记录冲突。

考虑父子任务拆分时：
- 一个请求包含多个可独立验证的交付物时使用父任务。
- 父任务负责原始需求、子任务映射、跨子任务验收标准和最终集成审查。
- 子任务负责可独立规划、实现、检查和归档的实际交付物。
- 父子结构不是依赖系统。如果子任务 B 依赖子任务 A，必须在 B 的 `prd.md` / `implement.md` 中写明顺序。
- 启动负责下一个交付物的子任务。除非父任务本身有直接实现工作，否则不要启动父任务。

需求发生变化时返回此步骤，并修改对应产物。

#### 1.2 研究 `[optional · repeatable]`

研究可以在需求探索期间的任何时候进行，不限于本地代码。可以使用当前可用的工具（MCP 服务、Skill、网页搜索等）查询外部信息，例如第三方库文档、行业实践和 API 参考。

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

派发研究子代理：

- **代理类型**：`trellis-research`
- **任务说明**：研究 <具体问题>
- **关键要求**：研究结果必须持久化到 `{TASK_DIR}/research/`

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

[codex-inline, Kilo, Antigravity, Devin]

在主会话中直接研究，并把结论写入 `{TASK_DIR}/research/`。`codex-inline` 是明确要求在主会话中完成研究的模式。

[/codex-inline, Kilo, Antigravity, Devin]

**研究产物约定**：
- 每个研究主题使用一个文件（例如 `research/auth-library-comparison.md`）。
- 在文件中记录第三方库用法示例、API 参考和版本约束。
- 记录发现的相关 Spec 路径，供后续引用。

brainstorm 和研究可以交错进行：遇到技术问题时先暂停对话完成研究，再返回与用户讨论。

**关键原则**：研究结果必须写入文件，不能只留在对话中。对话可能被压缩，文件不会因此丢失。

#### 1.3 配置上下文 `[required · once]`

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

整理 `implement.jsonl` 和 `check.jsonl`，让 Phase 2 的子代理获得正确的 Spec/研究上下文。`task create` 会为这两个文件写入一行自描述的 `_example` 初始内容；此步骤需要补充真实条目。

**位置**：`{TASK_DIR}/implement.jsonl` 和 `{TASK_DIR}/check.jsonl`（已存在）。

**格式**：每行一个 JSON 对象：`{"file": "<path>", "reason": "<why>"}`。路径相对于仓库根目录。

**应填写的内容**：
- **Spec 文件**：与任务相关的 `.trellis/spec/<package>/<layer>/index.md` 及具体指南文件（如 `error-handling.md`、`conventions.md`）。
- **研究文件**：子代理需要查阅的 `{TASK_DIR}/research/*.md`。

**不要填写的内容**：
- 代码文件（`src/**`、`packages/**/*.ts` 等），子代理会在实现时读取，不需要在这里预注册。
- 即将修改的文件，理由相同。

**两个文件的分工**：
- `implement.jsonl` -> 实现子代理正确编码所需的 Spec 和研究资料。
- `check.jsonl` -> 检查子代理所需的 Spec（质量指南、检查约定，以及必要时的同一份研究资料）。

这些清单不能替代 `implement.md`。`implement.md` 是复杂任务的人类可读执行计划；JSONL 文件只列出需要注入或加载的上下文文件。

**如何发现相关 Spec**：

```bash
python3 ./.trellis/scripts/get_context.py --mode packages
```

列出所有包、Spec 层级及路径。选择与当前任务领域匹配的条目。

**如何追加条目**：

可以直接在编辑器中修改 JSONL 文件，也可以使用：

```bash
python3 ./.trellis/scripts/task.py add-context "$TASK_DIR" implement "<path>" "<reason>"
python3 ./.trellis/scripts/task.py add-context "$TASK_DIR" check "<path>" "<reason>"
```

存在真实条目后可以删除 `_example` 初始行（可选；消费者会自动跳过它）。

就绪门禁：执行 `task.py start` 前，`implement.jsonl` 和 `check.jsonl` 都必须至少包含一条真实的 `{"file": "...", "reason": "..."}` 条目。只有 `_example` 初始行不算就绪。

只有两个文件都已经有整理好的真实条目时，才跳过此步骤。

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

[codex-inline, Kilo, Antigravity, Devin]

跳过此步骤。Phase 2 由 `trellis-before-dev` Skill 直接加载上下文。

[/codex-inline, Kilo, Antigravity, Devin]

#### 1.4 激活任务 `[required · once]`

完成产物审查后，将任务状态改为 `in_progress`：

运行 `start` 前，验证任务产物，并将验证结果记录到 `implement.md`：

```bash
python3 ./.trellis/scripts/task.py validate <task-dir>
```

如果验证报告产物、上下文清单或状态不完整，返回对应的规划步骤。不要把运行时的兼容性容忍当成规划已就绪。

```bash
python3 ./.trellis/scripts/task.py start <task-dir>
```

轻量任务可以只有 `prd.md`。复杂任务必须在 start 前具备并审查 `prd.md`、`design.md` 和 `implement.md`。在派发子代理的平台上，`implement.jsonl` 和 `check.jsonl` 也必须在 start 前各自包含真实整理条目。运行时消费者为了兼容可以容忍清单缺失或只有初始行，但这不代表规划已经就绪。

命令成功后，每轮提示自动切换到 `[workflow-state:in_progress]`，随后进入 Phase 2/3 的剩余流程。

第一次实现动作前输出 `[trellis] 阶段=实施 动作=进入`。每个任务只输出一次阶段进入标记，不要在每轮实现中重复输出。

如果 `task.py start` 因会话身份错误而失败（Hook 输入、`TRELLIS_CONTEXT_ID` 和平台原生会话环境都没有上下文键），按错误提示设置会话身份后重试。

#### 1.5 完成条件

| 条件 | 必需 |
|------|:---:|
| `prd.md` exists | ✅ |
| `prd.md` 记录了 Skill 发现和路由 | ✅ |
| `prd.md` records current-project / Demo reuse decisions | ✅ |
| 对应阶段已加载必需的规划/实现 Skill | ✅ |
| 用户确认任务可以进入实现阶段 | ✅ |
| `task.py start` has been run (status = in_progress) | ✅ |
| `research/` 有研究产物（复杂任务） | 建议 |
| `design.md` exists (complex tasks) | ✅ |
| `implement.md` exists (complex tasks) | ✅ |

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

| `implement.jsonl` 和 `check.jsonl` 各自至少包含一条真实整理条目（初始行不计入） | ✅ |

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

---

## Phase 2: 实施

目标：把已经审查过的规划产物转化为通过质量检查的代码。

#### 2.1 实现 `[required · repeatable]`

[Claude Code, Cursor, OpenCode, codex-sub-agent, CodeBuddy, Droid, Pi, ZCode, Snow, Oh My Pi]

派发实现子代理：

- **代理类型**：`trellis-implement`
- **任务说明**：按照已经审查的任务产物实现代码，查阅 `{TASK_DIR}/research/` 下的资料；完成后运行项目 lint 和类型检查。
- **派发提示约束**：提示必须以 `Active task: <task path>` 开头，然后明确说明该代理已经是 `trellis-implement`，必须直接实现，不能再次派发 `trellis-implement` / `trellis-check`。

平台 Hook/插件会自动处理：
- 读取 `implement.jsonl`，将其中引用的 Spec/研究文件注入代理提示。
- 注入存在时的 `prd.md`、`design.md` 和 `implement.md`。
- 对 Codex，`SubagentStart` 提供原生上下文注入；代理配置保留子代理主动读取机制。

[/Claude Code, Cursor, OpenCode, codex-sub-agent, CodeBuddy, Droid, Pi, ZCode, Snow, Oh My Pi]

[Gemini, Qoder, Copilot, Reasonix, Trae, Grok, Kimi Code]

派发实现子代理：

- **代理类型**：`trellis-implement`
- **任务说明**：按照已经审查的任务产物实现代码，查阅 `{TASK_DIR}/research/` 下的资料；完成后运行项目 lint 和类型检查。
- **派发提示约束**：提示必须以 `Active task: <task path>` 开头，并明确说明该代理已经是 `trellis-implement`，必须直接实现，不能再次派发 `trellis-implement` / `trellis-check`。

主动读取型子代理定义会自动处理上下文加载：
- 使用 `task.py current --source` 解析活动任务，然后读取 `prd.md`、存在时的 `design.md` 和 `implement.md`。
- 读取 `implement.jsonl`，并要求代理在编码前加载其中引用的每个 Spec/研究文件。

[/Gemini, Qoder, Copilot, Reasonix, Trae, Grok, Kimi Code]

[Kiro]

派发实现子代理：

- **代理类型**：`trellis-implement`
- **任务说明**：按照已经审查的任务产物实现代码，查阅 `{TASK_DIR}/research/` 下的资料；完成后运行项目 lint 和类型检查。
- **派发提示约束**：明确告诉代理它已经是 `trellis-implement` 子代理，必须直接实现，不能再次派发 `trellis-implement` / `trellis-check`。

平台前置流程会自动处理上下文加载要求：
- 读取 `implement.jsonl`，并将其中引用的 Spec/研究文件注入代理提示。
- 注入存在时的 `prd.md`、`design.md` 和 `implement.md`。

[/Kiro]

[codex-inline, Kilo, Antigravity, Devin]

1. 加载 `trellis-before-dev` Skill，读取项目指南。
2. 读取 `{TASK_DIR}/prd.md`（包括其中的 `## Skill 路由` 和 `## 复用决策`），然后读取存在时的 `design.md` 和 `implement.md`。
3. 编辑前加载分配给实现或验证阶段的所有必需 Skill；条件性 Skill 在触发条件成立并记录后才能加载。
4. 查阅 `{TASK_DIR}/research/` 下的资料。
5. 按照已审查的产物实现代码。
6. 运行项目 lint 和类型检查。

[/codex-inline, Kilo, Antigravity, Devin]

#### 2.2 质量检查 `[required · repeatable]`

第一次质量检查动作前输出 `[trellis] 阶段=检查 动作=进入`。每个任务只输出一次阶段进入标记，后续重复检查不再输出。

[Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

派发检查子代理：

- **代理类型**：`trellis-check`
- **任务说明**：根据 Spec 和任务产物审查所有代码变更；直接修复发现的问题；确保 lint 和类型检查通过。
- **派发提示约束**：提示必须以 `Active task: <task path>` 开头，然后明确说明该代理已经是 `trellis-check`，必须直接审查/修复，不能再次派发 `trellis-check` / `trellis-implement`。

检查代理的职责：
- 根据 Spec 审查代码变更。
- 根据 `prd.md`、存在时的 `design.md` 和 `implement.md` 审查代码变更。
- 直接修复发现的问题。
- 运行 lint 和类型检查进行验证。

[/Claude Code, Cursor, OpenCode, codex-sub-agent, Kiro, Gemini, Qoder, CodeBuddy, Copilot, Droid, Pi, Oh My Pi, ZCode, Snow, Reasonix, Trae, Grok, Kimi Code]

[codex-inline, Kilo, Antigravity, Devin]

加载 `trellis-check` Skill，并按照其指引验证代码：
- 是否符合 Spec。
- lint / 类型检查 / 测试。
- 跨层一致性（变更涉及多个层时）。

发现问题后，修复并重新检查，直到通过。

[/codex-inline, Kilo, Antigravity, Devin]

**最终检查（Phase 3.4 提交前）**：任务最后一次 2.2 必须覆盖全部变更范围，不能只检查最新一批实现。使用 `python3 ./.trellis/scripts/get_context.py --mode packages` 列出所有受影响的包，然后加载每个包的 Spec 索引中的质量检查部分。这可以发现中途局部 2.2 无法发现的跨层/多包问题。

每完成一批编码，都要运行当前 Skill 路由要求的检查。如果条件性 Skill 没有触发，在 `implement.md` 中记录原因；“未使用”不能证明已经通过必需的测试、构建、审查或外部验证。

#### 2.3 回滚 `[on demand]`

- `check` 发现 PRD 缺陷 -> 返回 Phase 1，修复 `prd.md`，然后重新执行 2.1。
- 实现出现错误 -> 回退代码，重新执行 2.1。
- 需要更多研究 -> 按 Phase 1.2 进行研究，并将结论写入 `research/`。

### 外部验证与失败停止

当结论依赖设备、部署、构建产物、客户端、对端系统、网络服务或其他外部系统时，在请求验证前，先在 `implement.md` 中记录最小测试卡：

```text
基线：<提交 / 构建产物 / 目标设备或对端版本>
假设：<本轮要验证的唯一根因或行为>
唯一变量：<相对基线只改变了什么>
预期现象：<可观察、可判定的结果>
实际结果：<通过 / 失败 / 未执行，以及原始日志、图片或路径>
```

- 一次外部测试只改变一个可归因变量。存在依赖关系的变化要拆成多张测试卡。
- 同一可观察现象连续两张测试卡失败后，停止继续修改该路径的产品代码。加载已路由的诊断能力和 `trellis-break-loop`，记录已确认事实、已排除假设、按优先级排序的新假设，以及重试前的下一个最小实验。
- 不要把本地构建、日志或代码路径说成外部功能验证。代码验证、事实确认和外部/运行时验证必须分开记录。

#### 2.4 记录结果 `[required · once]`

创建或更新最终结果前输出 `[trellis] 阶段=收尾 动作=进入`。每个任务只输出一次阶段进入标记。

完成最终全范围检查后，创建或更新 `{TASK_DIR}/result.md`。记录实际结果，不要记录计划结果，也不要复制 diff：

```markdown
# 变更结果

## 基本信息

- 完成日期：YYYY-MM-DD
- 变更类型：功能 / Bug修复 / 重构 / 文档 / 构建

## 修改原因

<引用 PRD 中的目标或问题，说明为什么必须修改。>

## 实际修改

<说明最终行为、关键边界和与原计划的差异。>

## 验证结果

- 代码检查：通过 / 失败 / 未执行
- 构建验证：通过 / 失败 / 未执行
- 自动化测试：通过 / 失败 / 未执行
- 外部或实物验证：通过 / 失败 / 未执行
- 未验证项：<没有则写“无”>

## Bug分析

- 异常现象：<非 Bug 任务删除本节>
- 已确认根因：
- 修复方式：
- 防止复发：

## 复用记录

- Demo：未查询 / 参考路径及复用边界
- 项目决策：无 / 已记录 `DEC-NNN`

## 持久化候选

本次无持久化候选。
```

- 每个完成的工作任务都必须有 `result.md`，包括轻量任务。未验证的工作必须明确写为 `未执行`；代码检查或构建成功不能改称为运行时、外部或实物验证。
- 存在长期候选时，用下表替换 `本次无持久化候选`。Phase 3.3 会在写入前完成归属分类。没有候选时保留这一行结论，不要创建空表。

  ```markdown
  | 候选ID | 核心结论 | 证据 | 建议归属 | 验证状态与边界 | 处理状态 |
  |---|---|---|---|---|---|
  | C1 | `<可复用或长期生效的结论>` | `<result、代码、测试或用户确认>` | `<项目决策 / 全局 Skill / 本地 Spec / README事实 / 仅任务>` | `<验证状态、适用与不适用边界>` | `<待分类 / 已新增 / 已更新 / 待确认 / 已跳过 / 失败>` |
  ```
- Bug 结果只有在证据闭合“现象 -> 根因 -> 修复 -> 验证”链路时，才能写 `已确认根因`。否则把未解决的假设写入 `未验证项`。
- 如果任务改变用户可见功能、协议行为、持久化数据或状态机行为，或修复了 Bug，更新对应的项目变更记录，并新增一行日期、类型、模块、原因、实际结果、验证状态和任务路径。纯格式、注释和不改变行为的重构只保留在 `result.md` 和 Git 中。
- 如果用户明确确认了会影响后续工作的长期项目技术/产品决策，并且该结论不属于已有等价格式，则更新对应的项目决策记录。使用连续 ID（`DEC-001`、`DEC-002` 等）。被替代的决策保留原行，将状态改为 `已替代（DEC-NNN）`，再新增一行；不要重写决策历史。

  ```markdown
  | ID | 日期 | 状态 | 领域 | 决策 | 原因 | 影响范围 | 关联任务 |
  |---|---|---|---|---|---|---|---|
  | DEC-001 | YYYY-MM-DD | 生效 | `<模块或领域>` | `<确认结论>` | `<选择原因>` | `<后续受影响范围>` | `<Trellis任务路径>` |
  ```

- 不要把实现细节、任务计划、未确认选项、临时绕过方案、项目事实、编码约定或跨项目经验写入项目决策记录。它们应根据归属分别留在任务产物、权威项目文档、项目本地 Spec 或全局 Skill 中。
- Git 仍然是精确 diff 和提交时间的来源。不要在 `result.md` 或项目变更记录中逐文件重复记录修改内容。

---

## Phase 3: 收尾

目标：确保代码质量，记录经验，并完成工作记录。

#### 3.2 调试回顾 `[on demand]`

如果本任务涉及重复调试（同一个问题被多次修复），加载 `trellis-break-loop` Skill 来：
- 分类根因。
- 解释之前的修复为什么失败。
- 提出防止复发的措施。

目标是记录调试经验，避免同类问题再次出现。

#### 3.3 候选归属与长期记录更新 `[required · once]`

读取 `{TASK_DIR}/result.md`，检查本任务是否产生了值得记录的长期结论：
- 新发现的模式或约定。
- 遇到的陷阱。
- 新的技术决策。
- 已验证的根因、修复方式或跨项目经验。

如果 `result.md` 写明 `本次无持久化候选`，记录 Phase 3.3 已完成，不创建表格并继续。否则先对每个候选完成归属分类再写入。`result.md` 中的“持久化候选”表是任务事实源，下面示例说明如何作出归属决定：

| 候选内容 | 证据 | 建议归属 | 处理状态 |
|---|---|---|---|
| `<结论>` | `<result.md、代码定位、测试或已确认事实>` | 项目决策记录 / 全局 Skill / 项目本地 Spec / README项目事实 / 仅任务记录 | `<已处理 / 待用户确认 / 跳过及原因>` |

- 用户确认、长期有效并影响后续任务的项目技术/产品决策，归入对应的项目决策记录。讨论历史和任务内选择保留在任务产物中。长期意图不清晰时，写入前询问用户。
- 已验证的根因、修复方式和陷阱保留在 `result.md`，除非它们同时形成了明确的项目决策、项目本地约定、项目事实或由全局 Skill 负责的可复用方法。
- 全局可复用方法只能更新所属 Skill，不要复制到 `.trellis/spec/`。
- 只有用户确认某候选是长期项目编码约定或 Trellis 基线的项目化细化后，才加载 `trellis-update-spec` 并写入 `.trellis/spec/`。
- 已确认的项目事实归入 README 或其他权威项目文档。
- 未验证、暂停、一次性或临时的调试记录保留在 `result.md`、`research/` 或验证状态中，不进入长期记录。
- 没有长期候选时，在 `result.md` 保留 `本次无持久化候选`。
- 任务仍有未解决的归属决定时，不要继续提交或归档。

#### 3.4 提交修改 `[required · once, user-confirmed]`

**长期记录前置检查**：拟定提交前，确认 `result.md` 已存在，并询问本任务是否修复了 Bug、形成了长期项目决策，或发现了需要归属处理的非显然长期结论。如果有，先返回 Phase 3.3。候选归属未处理完时，不要归档任务。

AI 为本任务的代码变更准备分批提交计划。提交属于外部状态变化，需要单独的用户确认；`批准实施` 和 `批准检查` 都不包含提交授权。先提交工作代码，然后停止并等待单独的归档授权。不要从一次模糊确认中推断要同时执行工作提交、归档、日志记录或推送。

**执行步骤**：

1. **检查工作区脏状态**：
   ```bash
   git status --porcelain
   ```
   记录所有脏路径。如果工作区干净，跳到 3.5。

2. **从最近历史学习提交风格**（让拟定的提交信息与仓库一致）：
   ```bash
   git log --oneline -5
   ```
   记录前缀约定（`feat:` / `fix:` / `chore:` / `docs:` 等）、语言（中文/English）和长度风格。

3. **将脏文件分成两组**：
   - **本会话由 AI 修改**：本会话中通过 Edit/Write/Bash 工具写入或修改的文件，明确知道改了什么以及为什么改。
   - **无法识别**：本会话没有触碰的脏文件（可能是用户手动修改、之前遗留的 WIP 或无关工作）。不要静默把它们加入提交。

4. **拟定提交计划**。把本会话修改的文件按逻辑分成多个提交（一个连贯变更单元一个提交，不是一个文件一个提交）。每项包含 `<commit message>` 和文件列表，并在末尾单独列出无法识别的文件。

5. **只展示一次计划，并请求一次性提交确认**。格式如下：
   ```
提交计划（按执行顺序）：
  1. <提交信息>
        - <file>
        - <file>
  2. <提交信息>
        - <file>

无法识别的脏文件（不会加入任何提交，请确认包含/排除）：
     - <file>
     - <file>

回复 `ok` / “行”只执行这些工作提交；回复修改意见，或“我自己来”/`manual` 取消。
   ```

6. **用户确认后**：按顺序对每一批运行 `git add <files>` 和 `git commit -m "<msg>"`。不要 amend，不要推送。

7. **工作提交完成后**：输出 `[trellis] 阶段=归档 动作=等待用户确认 状态=待归档` 并停止。本轮不要调用 `task.py archive`、`/finish-work`、`add_session.py` 或 `git push`。

8. **用户拒绝时**（回复“不行”/“我自己来”/“manual”，或对计划提出异议）：停止，不要重新拟定第二份计划。由用户自行提交；在用户明确决定下一个生命周期动作前，任务保持进行中。

**规则**：
- 任何地方都不能使用 `git commit --amend`，采用三阶段三类提交流程（工作提交 -> 归档提交 -> 日志提交）。
- 此步骤绝不推送远程。
- 如果用户只要求修改提交信息而接受文件分组，修改信息后重新确认一次；如果用户拒绝文件分组，转为用户手动处理。
- 分批计划只请求一次确认，不要每个提交分别询问。
- 提交确认只授权展示的工作提交，不授权归档、日志/会话记录或推送。

#### 3.5 收尾提醒

归档是单独的用户授权操作。调用 `/finish-work` 或 `task.py archive` 前，确认所有外部/实物验收项已完成。如果仍有未决项，保持任务进行中并报告；只有用户明确批准带未决项归档后才可归档。归档获授权后输出 `[trellis] 阶段=归档 动作=执行 状态=用户已确认`；除非用户另行授权记账提交，否则使用 `task.py archive <task> --no-commit`。会话记录同样需要单独授权，不能推断执行。推送从不属于收尾流程，必须由用户单独提出。

---

## 自定义 Trellis（用于维护分支）

本节面向需要修改 Trellis Workflow 本身的维护者。所有自定义都通过编辑本文件完成；脚本只负责解析，不内置工作流正文。

### 修改步骤含义

编辑上方 Phase 1/2/3 中对应步骤的正文。关键不变量：
- 没有活动任务时必须先分流。直接处理仅限简单、只读或低风险自包含范围；任何 Trellis 任务都需要先取得创建任务的同意。
- 规划必须区分轻量 PRD-only 任务和复杂任务；复杂任务在 start 前必须具备 `prd.md`、`design.md` 和 `implement.md`。
- 所有必需执行路径都必须能在 `/trellis:finish-work` 前到达 Phase 3.4 的提交提醒。

所有标签块都位于上方 `## Phase Index` 部分，并紧跟对应阶段摘要：

| 范围 | 对应标签 |
|---|---|
| 没有活动任务（Phase 1 之前） | `[workflow-state:no_task]`（Phase Index ASCII 图之后） |
| Phase 1 全部内容（创建任务 -> 可实现） | `[workflow-state:planning]`（Phase 1 摘要之后） |
| Codex inline 模式的 Phase 1 | `[workflow-state:planning-inline]` |
| Phase 2 + Phase 3.2-3.4（实现、检查和收尾） | `[workflow-state:in_progress]`（Phase 2 摘要之后） |
| Codex inline 模式的 Phase 2 + Phase 3.2-3.4 | `[workflow-state:in_progress-inline]` |
| Phase 3.5 之后（已归档） | `[workflow-state:completed]`（Phase 3 摘要之后；**当前不会触发**） |

### 修改每轮提示文字

直接编辑对应 `[workflow-state:STATUS]` 标签块的正文。编辑后，如果你是模板维护者则运行 `trellis update`；如果只是自定义自己的项目，则重启 AI 会话。不需要修改脚本。

### 添加自定义状态

Add a new block:

```
[workflow-state:my-status]
your per-turn prompt text
[/workflow-state:my-status]
```

约束：
- STATUS 字符集必须是 `[A-Za-z0-9_-]+`（允许下划线和连字符，例如 `in-review`、`blocked-by-team`）。
- 必须有生命周期 Hook 将 `task.json.status` 写成自定义值，否则永远不会读取该标签。
- 生命周期 Hook 位于 `task.json.hooks.after_*`，可绑定 `after_create`、`after_start`、`after_finish` 或 `after_archive`。

### 添加生命周期 Hook

在 `task.json` 中添加 `hooks` 字段：

```json
{
  "hooks": {
    "after_finish": [
      "your-script-or-command-here"
    ]
  }
}
```

支持的事件：`after_create / after_start / after_finish / after_archive`。注意，`after_finish` 不等于状态变化（它只清除活动任务指针）；需要发送“任务完成”通知时使用 `after_archive`。

### 完整契约

关于工作流状态机的运行时契约、所有状态写入位置、伪状态（`no_task` / `stale_<source_type>`）、Hook 可达性矩阵及其他细节，参见：

- `.trellis/spec/cli/backend/workflow-state-contract.md`：运行时契约、写入者表和测试不变量。
- `.trellis/scripts/inject-workflow-state.py`：实际解析器（只读取 workflow.md，不内嵌正文）。
