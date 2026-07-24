# 示例代码骨架

> 这些是可以直接套用的文件模板。新建模块时复制改名，填业务逻辑即可。骨架已遵守 Task 无 Init/DeInit、BSP 自初始化、使能位、守卫、命名、头文件保护和 Tab 等约定。

---

## 可用骨架

| 文件 | 用途 | 复制后改成 |
|------|------|-----------|
| `xhh_BSP_Template.c/.h` | BSP 公共层骨架（按当前类别补充硬件接口） | `Template/TEMPLATE` → 类别名，文件名 → 对应 `xhh_BSP_<类别>.c/.h` |
| `xhh_Task_All_Template.h` / `.c` | 聚合层骨架（聚合 include + 转发各模块 Cmd） | 文件名 → `xhh_Task_ALL.h` / `xhh_Task.c`，按需增减 Task 头和 Cmd 转发 |
| `xhh_Task_Template.h` | Task 模块头文件（Cmd/Loop + 类型 + 宏） | `Template` → 你的模块名（如 `Temp`） |
| `xhh_Task_Template.c` | Task 模块实现（无 Init/DeInit + 调 `xhh_BSP_*` + Loop 守卫） | 同上 + 填业务逻辑和换算；真实硬件资源只填入 BSP |
| `xhh_Event_Template.h` | 事件枚举 + 参数 ID 宏 | 加你的事件值 |
| `xhh_Event_Template.c` | Trigger + Handle switch 骨架 | 加你的事件 case |
| `xhh_Mode_Template.h` | 状态机头(状态/子步枚举 + 变量 extern + 接口) | 加你的状态枚举值 |
| `xhh_Mode_Template.c` | 状态机实现(Change + Handle switch 骨架) | 加你的状态 + 子步逻辑 |
| `xhh_Task_Flash_Template.c` | Flash 结构体 + Get/Save/IS_Valid/Clean | 填你的字段 + 默认值 |

---

## 使用方式

1. BSP 骨架复制到根目录 `xhh_BSP/`；Task/Event/Mode 骨架分别复制到 `xhh_Module/` 对应子目录
2. 全局替换 `Template` / `TEMPLATE` 为你的模块名
3. 在 BSP 填真实硬件资源，在 Task 填业务枚举、Loop 处理和状态转换
4. 新 Task 模块注册到 `xhh_Task_ALL.h`；需要统一使能时在 `xhh_Task.c` 增加 Cmd 转发

---

## 约定提醒

骨架已遵守的约定(改的时候别破坏):
- `xhh_` 前缀 + 缩写词全大写(LED/ADC/BLE)
- 头文件保护 `XHH_<MODULE>_H`
- 使能位 `static volatile uint8_t xhh_task_<x>_en`(必须 static)
- Loop 首句 `if (en == 0) return;` 守卫
- 硬件操作走 `xhh_BSP_*` 公共层,Task 不直接调厂商 API(见 bsp.md)
- Task 不提供 `_Init/_DeInit` 或硬件 Config 接口；各 BSP Init 由 APP 直接调用
- 占位函数体内必须用 `AI:` 注释说明原因、当前行为和启用条件
- Tab 缩进,Allman 大括号,无文件头注释
- 编码 UTF-8,行尾 CRLF
