# XHH MCU 单 Agent 工作流

本工作流用于 XHH MCU 固件项目的单 Agent 开发。它管理 Trellis 任务生命周期与任务文档，并在合适阶段路由已全局安装的 Skill；不复制、不安装、也不修改全局 Skill。

## 职责边界

| 归属 | 负责内容 |
|---|---|
| Trellis | 任务创建、PRD、设计、实现计划、上下文、检查、收尾与项目记忆 |
| `xhh-mcu-development` | 项目事实核对、硬件事实门禁、XHH 分层、Demo 复用边界、构建与实物验证状态 |
| `grilling` | 高风险决策的逐项追问与方案压力测试 |
| `.trellis/spec/` | 可从目录、依赖、接口和源码追溯的 XHH 代码模式 |
| README、Doc、原理图、数据手册 | 当前项目真实硬件与产品事实 |

Trellis 任务文档的创建与更新由本工作流负责。`xhh-mcu-development` 只负责领域事实和实施门禁，不管理 `.trellis/` 生命周期文件。

## Skill 前置条件

- 本工作流依赖全局 `xhh-mcu-development` Skill。
- 高风险决策依赖全局 `grilling` Skill。
- Workflow 不能自动安装 Skill。缺失时必须报告缺失项并停止对应阶段，不得用未验证的替代流程。

## 阶段

```text
Plan -> 事实核对与决策 -> PRD/design/implement -> Execute -> Check -> Finish
```

### Phase 1: Plan

1. 按 Trellis 规则取得创建任务的用户许可后创建任务。
2. 在 `prd.md` 记录范围、约束、验收条件、已确认事实与未决项。
3. 非简单 MCU 任务必须加载 `xhh-mcu-development`，先核对 README、Doc、原理图、Platform、xhh_BSP 与已确认 Demo。
4. 硬件资源、极性、时钟、外设实例、阈值或产品行为存在缺失或冲突时，停止相关实现，在 PRD 写明证据和阻塞项，并向用户确认。
5. 复杂任务在实施前完成 `design.md` 与 `implement.md`。`design.md` 记录边界、调用链、取舍与兼容性；`implement.md` 记录修改顺序、验证方法和回退点。

### 高风险决策门槛

仅在以下决策尚未明确时加载 `grilling`：

- 新板或 MCU 迁移、BSP 资源归属、时钟与低功耗路径；
- 协议字段、Event 参数编码或状态机转换语义；
- Flash 数据布局与掉电策略；
- 安全、不可逆操作或跨 Task 调用环风险。

不对普通局部修改、已确认方案或单纯代码实现无条件使用 `grilling`。完成逐项确认后，必须把结论和未决项回写 `prd.md`；涉及技术方案时同步回写 `design.md`，再进入实施。

### Phase 2: Execute

1. 仅在任务状态进入 `in_progress` 后实施。
2. 实施前加载 `xhh-mcu-development`，并读取本任务 PRD/design/implement 及相关 `.trellis/spec/`。
3. 只修改当前任务必需的文件和调用链。Demo 只能作为已确认的实现基线，不得直接带入 Platform、厂家配置或未确认板级参数。
4. 硬件事实仍未闭环时，只允许完成不依赖该事实的纯逻辑、接口和私有状态；禁止以默认映射、空实现或静默 fallback 代替确认。
5. 同类问题连续失败时使用 `diagnosing-bugs` 定位根因；已验证修复后，才使用 `trellis-break-loop` 更新项目记忆。

### Phase 3: Check And Finish

1. 检查前再次加载 `xhh-mcu-development`，核对事实状态、分层边界、修改范围和验证要求。
2. MCU 源码变更按项目实际方式执行构建；构建、硬件事实和实物验证必须分别报告，禁止把编译通过表述为实物功能正确。
3. 仅当发现跨项目稳定、可由源码追溯的代码模式时更新 `.trellis/spec/`。板级事实、单次过程和任务结论不得写入 Spec。
4. 收尾前更新 PRD/design 中已解决项与未决项，完成检查后再按用户授权提交。

## 路由规则

- 规划 MCU 任务、修改 BSP/Components/xhh_Module/APP、复用 Demo、核对 Keil 或硬件事实：加载 `xhh-mcu-development`。
- 高风险且方案未决：在 `trellis-brainstorm` 完成需求收集后加载 `grilling`；其结论必须持久化到 PRD/design。
- 重复失败：加载 `diagnosing-bugs`；修复经验证后才允许 `trellis-break-loop` 写入项目记忆。
- 普通对话、无代码修改的简单问答：不强制创建 Trellis 任务。

## 工作流状态

[workflow-state:no_task]
先判断请求是否需要 Trellis 任务。复杂 MCU 开发、修复、迁移或审查任务先取得用户许可，再创建任务并进入规划；普通问答不创建任务。
[/workflow-state:no_task]

[workflow-state:planning]
保持规划阶段。MCU 任务加载 xhh-mcu-development，核对项目事实并写入 PRD；复杂任务完成 design.md 和 implement.md。高风险未决方案才加载 grilling，结论回写 PRD/design 后才能 start。
[/workflow-state:planning]

[workflow-state:planning-inline]
保持规划阶段。MCU 任务加载 xhh-mcu-development，核对项目事实并写入 PRD；复杂任务完成 design.md 和 implement.md。高风险未决方案才加载 grilling，结论回写 PRD/design 后才能 start。
[/workflow-state:planning-inline]

[workflow-state:in_progress]
实施与检查前加载 xhh-mcu-development，读取任务文档和相关 Spec；硬件事实缺失时停止相关实现。重复失败使用 diagnosing-bugs，验证修复后才用 trellis-break-loop。完成检查和任务文档收尾后，按用户授权提交。
[/workflow-state:in_progress]

[workflow-state:in_progress-inline]
实施与检查前加载 xhh-mcu-development，读取任务文档和相关 Spec；硬件事实缺失时停止相关实现。重复失败使用 diagnosing-bugs，验证修复后才用 trellis-break-loop。完成检查和任务文档收尾后，按用户授权提交。
[/workflow-state:in_progress-inline]

[workflow-state:completed]
任务已完成。确认任务文档、验证结论与提交状态一致，再执行归档。
[/workflow-state:completed]
