# 质量规范

> 固件质量 = 模块边界清楚 + 状态机可推理 + 持久化边界统一 + 协议/硬件路径可验证 + 不引入影响板级调试的抽象。

---

## 代码格式（强制）

由 `.clang-format` 文件约束，关键规则：

| 项 | 规则 |
|----|------|
| 缩进 | 统一 Tab |
| 大括号 | 与代码库现状一致（Allman 倾向，开括号另起一行） |
| case | 顶格，不缩进 |
| 文件头注释 | **不要**（不写文件头注释块，靠 git 记录作者/日期） |
| 缩写词 | 全大写（LED/USB/BLE/ADC），见 [naming-conventions.md](./naming-conventions.md) |

历史文件若有文件头注释（Eclipse CDT 模板），改动时顺手删掉。

### TODO 注释（强制）

需要用户后续补充或修改的地方，用 `TODO` 标注。**`TODO` 必须在注释行的最前面**，便于 grep 全局定位。

```c
// 正确
//TODO 标定温度阈值
//TODO: VCC 需确认

// 错误（TODO 在中间，grep 难定位）
//这是阈值 TODO 后续标定
//xxx TODO xxx
```

约定：
- `//TODO` 或 `//TODO:` 开头（`//` 后紧跟 `TODO`，可加冒号）
- 一行只标注一个 TODO 事项
- 不要把 TODO 嵌在多行注释的中间段落

---

## 文件编码（强制）

本项目 `.c/.h` 文件编码**固定为 UTF-8**。

**⚠ AI 工具陷阱**：opencode 的 `edit` / `write` / `read` 工具按 UTF-8 读写文件。直接对非 UTF-8 文件（如历史 GBK 文件）用 `edit` 会把全文件中文注释破坏成乱码（GBK 字节被当 UTF-8 解析后重写）。

**编辑非 UTF-8 文件的强制流程**（转码 → 编辑，不转回原编码）：

1. **改前转码**：若目标文件不是 UTF-8（如 GBK），先用 PowerShell 转成 UTF-8 保存：
   ```powershell
   $gbk = [System.Text.Encoding]::GetEncoding(936)
   $utf8 = [System.Text.Encoding]::UTF8
   $b = [System.IO.File]::ReadAllBytes($path)
   $t = $gbk.GetString($b)                    # GBK 字节 -> 字符串
   $t = $t.Replace("`r`n","`n").Replace("`n","`r`n")  # 统一 CRLF
   [System.IO.File]::WriteAllBytes($path, $utf8.GetBytes($t))  # 写 UTF-8
   ```
2. **用 `edit` / `write` 工具修改**（此时文件已是 UTF-8，工具安全）
3. **改后保持 UTF-8**：不转回原编码
4. **验证**：UTF-8 解码中文正常 + CRLF 行数 = 总行数、LF = 0

**新建文件**：直接用 `write` 工具（默认 UTF-8）即可，无需转码。

**子代理约束**：dispatch `trellis-implement` / `trellis-check` 时，prompt 里必须告知"本项目 .c/.h 编码固定 UTF-8，编辑前若发现文件非 UTF-8（如 GBK），先用 PowerShell 转 UTF-8 保存再修改，禁止直接对非 UTF-8 文件用 edit/write 工具"。

**行尾**：保持 CRLF（`core.autocrlf=true`，git 仓库存 LF，工作区 CRLF）。

---

## 必须遵守

- 分层链路稳定：协议入口 → 事件 → 状态机 → Task 模块（见各 pattern 文件）
- 模块状态默认放在 `.c` 内私有 `static` 变量或 context；确需跨模块共享的数据对象必须明确归属，外部只允许按约定读取，不得直接写内部字段
- 持久化数据应用前必须校验
- 注释用中文
- 新代码延续现有 `switch` 驱动风格
- **关键逻辑注释以 `AI:` 开头**，便于追溯 AI 生成内容（如 `//AI: 这里延迟避免上电竞争` / `///AI:`）
- **对外函数接口写 Doxygen 注释**（`@brief` / `@param` / `@return`），`@brief` 可带 `AI:` 前缀
- **占位函数允许用于框架统一**：函数体内必须用 `AI:` 注释说明原因、当前行为和启用条件；不得默认成功、伪造硬件值或执行 fallback
- **代码事实高于文档**：项目文档与代码冲突时，以代码为准，并把冲突列为待确认事项告知用户，不擅自改代码迁就文档

---

## 禁止

- 无明确必要时引入动态内存
- 未经确认擅自增加 fallback / 兜底逻辑（项目负责人要求先确认）
- 已有模块边界明确时再平行复制一套逻辑
- 用通用 web/backend 术语重写 MCU 风格模块

---

## 验证要求

固件验证不靠 lint/typecheck/unit-test，靠：

- **编译通过**：使用当前项目实际 IDE 或构建系统全量编译，无错无警告
- **产物生成**：`.hex` / `.bin` 产出 + post-build CRC 通过
- **路径审查**：涉及协议/状态机/持久化时，看完整读写链路而非单函数
- **硬件验证**（按需）：烧录 + 串口日志确认 + 协议行为确认

### 低功耗验证（涉及 Sleep、Shutdown、关机或显示外设时强制）

低功耗问题必须以电流实测为准，不能仅凭屏幕熄灭、代码调用了低功耗 API 或可被按键唤醒判定通过。

- 至少测量两组工况：冷上电后不操作自动休眠，以及完整开机、使用显示外设后手动关机。
- 两组工况使用相同电池电压、相同外接设备和相同测量量程；记录进入低功耗后的稳定电流，而不是切换瞬态。
- 手动关机功耗必须与冷上电自动休眠处于同一量级；出现明显差异时，优先检查已初始化外设的内部状态，而非只检查 MCU 的 Sleep/Shutdown 调用。
- 显示类外设必须验证背光关闭、显示关闭和控制器 Sleep In 后的电流；仅关闭背光或仅发送 Display Off 不得作为验收结论。
- 需要时加入一次性、可观测的诊断标记确认低功耗 API 是否异常返回；诊断代码完成定位后必须删除，不能保留在正式固件。
- 低功耗修改后必须重新执行完整开机 -> 关机 -> 唤醒 -> 再开机流程，确认显示恢复、按键唤醒和稳定电流均正常。

提交信息里常记录"测试ok"类证据，这是本项目的验证现实。

---

## 提交约定

- 新提交用**中文 Conventional Commits**：`feat:` / `fix:` / `chore:` / `docs:` 前缀 + 中文描述
- 旧历史保留原风格，不追溯重写
- MCU 代码提交必须包含本次全量编译生成的 `.hex` 与 `.bin` 产物，并在提交信息中说明验证方式

---

## Code Review Checklist

- [ ] 源码是否收口在 `APP/`、`xhh_Module/`、`xhh_BSP/`、`Platform/`、`Components/` 五个根目录？
- [ ] 单一 IDE 工程文件是否直接位于 `Project/`，没有重复的工具名子目录？
- [ ] `Platform/` 是否只包含当前项目唯一 MCU，没有多芯片实现或大段平台切换宏？
- [ ] `Platform/Src` 是否只保留厂家 SDK、启动、系统、配置和厂家中断骨架，没有项目手写的 `MX_*`、外设 Handle、DMA 或外设初始化封装？
- [ ] BSP 专属 IRQ 是否归对应 `xhh_BSP_*.c`，且厂家中断骨架没有反向 include `xhh_BSP`？
- [ ] 当前未使用的外设是否已从工程移除，且没有为了目录完整新增空 BSP？
- [ ] 项目根 `README.md` 是否列出当前 MCU、板卡版本、硬件资料路径、已启用基础外设和硬件限制？
- [ ] 本次是否修改了 MCU、板卡、引脚、ADC 映射、外设实例、Timer/DMA、时钟、Flash 分区或调试/唤醒约束？若是，README、Project 和对应 `xhh_BSP_*.c` 是否已同步核对更新？
- [ ] `xhh_Module/` 是否只依赖 `xhh_BSP/` 和 `Components/`，没有直接依赖 `Platform/`？
- [ ] `Components/` 是否保持产品和 MCU 无关；如需硬件能力，是否只 include `xhh_BSP/` 公共头，没有 include `main.h`、`xhh_Module/` 或厂商头？
- [ ] ADC 公开头是否只使用 `xhh_BSP_ADC_CHANNEL_<n>` 逻辑通道名，厂家硬件通道值是否只存在于 `xhh_BSP_ADC.c`？
- [ ] 占位函数是否均在函数体内使用 `AI:` 注释说明原因、当前行为和启用条件，并且没有默认成功、伪造硬件值或静默 fallback？
- [ ] 改动是否尊重现有模块边界？
- [ ] 事件/状态/Task 顺序会不会导致状态改一半？
- [ ] 数值范围、枚举上下界是否校验？
- [ ] 注释语言与附近代码一致（中文）？
- [ ] 是否残留调试代码 / 测试 hook？
- [ ] 新代码缩进是否 Tab？缩写词是否全大写？
- [ ] 是否残留文件头注释（应删）？
- [ ] 中断里是否做了禁止的事（协议/事件/Flash）？
- [ ] Task 内是否残留厂商 API 直接调用（`GPIOA_ModeCfg`/`HAL_*` 等，应改 `xhh_BSP_*`）？
- [ ] Task 函数定义是否遵循“硬件操作接口（`AI:硬件操作接口` 标注）→ Cmd → 其他公开接口 → Loop”的顺序，且 Loop 为文件最后一个函数定义？
- [ ] 参数型 Task 的 `_Set_*` 是否只更新私有参数，没有隐藏调用 `_Set_Mode_Fun`、`_Apply_*`、目标更新函数或硬件接口？需要即时生效的 Event 是否显式执行“Set 参数 → Mode/Apply”？
- [ ] 周期 Task 是否只在效果相位或目标值变化时更新输出，未在每个周期重复写相同硬件值？
- [ ] 外部输入是否在协议/事件参数解析处完成长度、范围和枚举校验；非法数据是否未传入 Task？Task 收到非法枚举或逻辑资源 ID 时，是否在改状态或输出前调用 `xhh_BSP_SYS_ERR_Handle()`，而非静默保持旧状态？
- [ ] Task 既有状态返回接口是否统一使用 `xhh_BSP_Status_t` 和命名状态码，未定义 `xhh_Module_Status_t`，也未用裸 `0/1` 表示成功或失败？
- [ ] 持久化是否经 `xhh_BSP_Flash_*` 而非裸 `EEPROM_*`/`HAL_FLASH_*`？
- [ ] 是否新增了设备型 BSP（`xhh_BSP_Key`/`xhh_BSP_Motor` 等，应禁）？
- [ ] 公共头是否 include 了芯片 SDK 头（应只在 `.c`）？
- [ ] Task 是否只暴露真实需要的 Cmd/Loop/Get/Set，且没有 `_Init/_DeInit` 或硬件 Config 接口？
- [ ] APP 是否直接调用各 `xhh_BSP_*_Init`，Task 中没有硬件初始化？
- [ ] 除 `xhh_BSP_SYS.c` 的平台适配实现外，是否仍有 `HAL_Delay` 等厂家延时调用？
- [ ] 涉及关机、Sleep、Shutdown 或显示外设时，是否实测了冷上电自动休眠与开机后关机两组稳定电流？
- [ ] 显示外设是否执行了数据手册规定的完整低功耗序列，而非仅关闭背光或 Display Off？
- [ ] 临时低功耗诊断标记是否已删除？

---

## 初始化后应补充的项目事实

- 真实构建命令
- 真实硬件验证方式
- 是否提交构建产物
