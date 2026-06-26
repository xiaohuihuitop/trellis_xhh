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

---

## 文件编码（强制）

本项目历史 `.c/.h` 文件是 **GBK 编码**（MounRiver Studio 默认）。新编辑的文件接受 **UTF-8**。项目允许 GBK/UTF-8 混合，但**绝不允许乱码**。

**⚠ AI 工具陷阱**：opencode 的 `edit` / `write` / `read` 工具按 UTF-8 读写文件。直接对 GBK 文件用 `edit` 会把全文件中文注释破坏成乱码（GBK 字节被当 UTF-8 解析后重写）。

**编辑 GBK 文件的强制流程**（转码 → 编辑 → 保持 UTF-8，不转回 GBK）：

1. **改前转码**：用 PowerShell 把目标文件 GBK → UTF-8（保留 CRLF）：
   ```powershell
   $gbk = [System.Text.Encoding]::GetEncoding(936)
   $utf8 = [System.Text.Encoding]::UTF8
   $b = [System.IO.File]::ReadAllBytes($path)
   $t = $gbk.GetString($b)                    # GBK 字节 -> 字符串
   $t = $t.Replace("`r`n","`n").Replace("`n","`r`n")  # 统一 CRLF
   [System.IO.File]::WriteAllBytes($path, $utf8.GetBytes($t))  # 写 UTF-8
   ```
2. **用 `edit` / `write` 工具修改**（此时文件已是 UTF-8，工具安全）
3. **改后保持 UTF-8**：不再转回 GBK（用户接受 UTF-8）
4. **验证**：UTF-8 解码中文正常 + CRLF 行数 = 总行数、LF = 0

**新建文件**：直接用 `write` 工具（默认 UTF-8）即可，无需转码。

**子代理约束**：dispatch `trellis-implement` / `trellis-check` 时，prompt 里必须告知"本项目 .c/.h 可能是 GBK 编码，编辑前先用 PowerShell 转 UTF-8（GetEncoding(936)），禁止直接对 GBK 文件用 edit/write 工具"。

**行尾**：保持 CRLF（`core.autocrlf=true`，git 仓库存 LF，工作区 CRLF）。

---

## 必须遵守

- 分层链路稳定：协议入口 → 事件 → 状态机 → Task 模块（见各 pattern 文件）
- 模块状态放模块 static 变量或明确共享结构体，不用隐藏堆对象
- 持久化数据应用前必须校验
- 注释用中文
- 新代码延续现有 `switch` 驱动风格

---

## 禁止

- 无明确必要时引入动态内存
- 未经确认擅自增加 fallback / 兜底逻辑（项目负责人要求先确认）
- 已有模块边界明确时再平行复制一套逻辑
- 用通用 web/backend 术语重写 MCU 风格模块

---

## 验证要求

固件验证不靠 lint/typecheck/unit-test，靠：

- **编译通过**：MounRiver Studio / RISC-V GCC 编译无错
- **产物生成**：`.hex` / `.bin` 产出 + post-build CRC 通过
- **路径审查**：涉及协议/状态机/持久化时，看完整读写链路而非单函数
- **硬件验证**（按需）：烧录 + 串口日志确认 + 协议行为确认

提交信息里常记录"测试ok"类证据，这是本项目的验证现实。

---

## 提交约定

- 新提交用**中文 Conventional Commits**：`feat:` / `fix:` / `chore:` / `docs:` 前缀 + 中文描述
- 旧历史保留原风格，不追溯重写
- 构建产物（hex/bin）是否随提交：按项目约定，在提交信息里说明是否刷新

---

## Code Review Checklist

- [ ] 改动是否尊重现有模块边界？
- [ ] 事件/状态/Task 顺序会不会导致状态改一半？
- [ ] 数值范围、枚举上下界是否校验？
- [ ] 注释语言与附近代码一致（中文）？
- [ ] 是否残留调试代码 / 测试 hook？
- [ ] 新代码缩进是否 Tab？缩写词是否全大写？
- [ ] 是否残留文件头注释（应删）？
- [ ] 中断里是否做了禁止的事（协议/事件/Flash）？
- [ ] 持久化是否经 `xhh_Task_Flash` 而非散写？
- [ ] 新 Task 模块是否四件套齐全 + 注册到 `xhh_Task_ALL`？

---

## 初始化后应补充的项目事实

- 真实构建命令
- 真实硬件验证方式
- 是否提交构建产物
