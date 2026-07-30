# 按任务选 Skill 的单 Agent 工作流

本工作流管理 Trellis 任务生命周期，并要求每个任务在规划阶段选择适用的全局 Skill。它不复制、安装或固定绑定任何领域 Skill。

## 职责边界

| 归属 | 负责内容 |
|---|---|
| Trellis | 任务创建、PRD、设计、实现计划、上下文、检查、收尾与项目记忆 |
| 当前任务选择的领域 Skill | 领域事实、实施方法、验证要求与专业门禁 |
| `.trellis/spec/` | 可从目录、依赖、接口和源码追溯的项目代码模式 |
| README、Doc、原理图、数据手册 | 当前项目真实事实 |

## Skill 路由记录

创建任务后，必须在 `prd.md` 增加以下章节：

```markdown
## Skill 路由

- 必需：`<skill-name>`，原因：<该任务为何必须使用>
- 条件性：`<skill-name>`，触发条件：<何时加载>
- 不使用：<没有适用 Skill 时的原因>
```

- 规划时根据任务领域、风险和当前环境中实际可用的全局 Skill 选择，不因 Workflow 存在而假定某个 Skill 必然可用。
- `必需` Skill 必须在实施前和检查前加载；`条件性` Skill 只在触发条件成立时加载。
- 已记录的必需 Skill 缺失时，报告缺失项并暂停相关阶段，不用未经确认的替代流程。
- Skill 路由随范围变化更新；不要把单次任务的路由写入 `.trellis/spec/`。

### 已知领域映射示例

| 任务类型 | 必需 Skill | 条件性 Skill |
|---|---|---|
| XHH MCU 开发、修复、审查、Demo 复用或硬件事实核对 | `xhh-mcu-development` | 方案存在高风险未决项时使用 `grilling` |
| 重复出现的相同故障 | `diagnosing-bugs` | 修复验证后使用 `trellis-break-loop` 记录经验 |

示例是任务分类规则，不使 Trellis 与其中任何 Skill 形成固定依赖。其他领域任务应按实际可用 Skill 另行选择。

## 阶段

```text
Plan -> Skill 路由与事实核对 -> PRD/design/implement -> Execute -> Check -> Finish
```

### Phase 1: Plan

1. 按 Trellis 规则取得创建任务的用户许可后创建任务。
2. 在 `prd.md` 记录范围、约束、验收条件、已确认事实、未决项和 Skill 路由。
3. 复杂任务在实施前完成 `design.md` 与 `implement.md`。`design.md` 记录边界、调用链与取舍；`implement.md` 记录修改顺序、验证方法和回退点。
4. 领域事实缺失或冲突时，先加载已选择的领域 Skill；仍无法确认时在 PRD 记录证据与阻塞项，并向用户确认。
5. 高风险且方案未决时，按 Skill 路由加载 `grilling`。结论和未决项必须回写 `prd.md`；涉及技术方案时同步回写 `design.md`。

### Phase 2: Execute

1. 仅在任务状态进入 `in_progress` 后实施。
2. 实施前加载 PRD 中所有必需 Skill，并读取 PRD/design/implement 和相关 `.trellis/spec/`。
3. 只修改当前任务必需的文件和调用链；领域事实未闭环时，只实施不依赖该事实的部分，或停止并请求确认。
4. 同类问题连续失败时，按 PRD 路由加载诊断 Skill；已验证修复后，才更新项目记忆。

### Phase 3: Check And Finish

1. 检查前再次加载 PRD 中所有必需 Skill，按其要求核对领域事实、修改范围和验证结果。
2. 将代码验证、事实确认和实物或运行验证分别记录；不得以单一验证结果替代其他验证结论。
3. 仅当发现跨项目稳定、可由源码追溯的代码模式时更新 `.trellis/spec/`。
4. 收尾前更新任务文档中的完成项、未决项和 Skill 路由，完成检查后再按用户授权提交。

## 工作流状态

[workflow-state:no_task]
先判断请求是否需要 Trellis 任务。复杂开发、修复、迁移或审查任务先取得用户许可，再创建任务并进入规划；普通问答不创建任务。
[/workflow-state:no_task]

[workflow-state:planning]
保持规划阶段。在 PRD 记录 Skill 路由、项目事实和验收条件；复杂任务完成 design.md 与 implement.md。高风险未决方案按路由加载对应 Skill，结论回写任务文档后才能 start。
[/workflow-state:planning]

[workflow-state:planning-inline]
保持规划阶段。在 PRD 记录 Skill 路由、项目事实和验收条件；复杂任务完成 design.md 与 implement.md。高风险未决方案按路由加载对应 Skill，结论回写任务文档后才能 start。
[/workflow-state:planning-inline]

[workflow-state:in_progress]
实施与检查前加载 PRD 中的必需 Skill，读取任务文档和相关 Spec。事实缺失时停止相关实现；重复失败按路由诊断，验证修复后再更新项目记忆。完成检查和任务文档收尾后，按用户授权提交。
[/workflow-state:in_progress]

[workflow-state:in_progress-inline]
实施与检查前加载 PRD 中的必需 Skill，读取任务文档和相关 Spec。事实缺失时停止相关实现；重复失败按路由诊断，验证修复后再更新项目记忆。完成检查和任务文档收尾后，按用户授权提交。
[/workflow-state:in_progress-inline]

[workflow-state:completed]
任务已完成。确认任务文档、验证结论与提交状态一致，再执行归档。
[/workflow-state:completed]
