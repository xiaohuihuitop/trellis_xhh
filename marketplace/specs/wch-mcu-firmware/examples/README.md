# 示例代码骨架

> 这些是可以直接套用的文件模板。新建模块时复制改名,填业务逻辑即可。骨架已遵守 spec 全部约定(四件套/使能位/守卫/命名/头文件保护/Tab),不需要再补结构。

---

## 可用骨架

| 文件 | 用途 | 复制后改成 |
|------|------|-----------|
| `xhh_Task_Template.h` | Task 模块头文件(四件套 + 类型 + 宏) | `Template` → 你的模块名(如 `Temp`) |
| `xhh_Task_Template.c` | Task 模块实现(四件套 + BSP static + Loop 守卫) | 同上 + 填 GPIO/硬件配置 + Loop 逻辑 |
| `xhh_Event_Template.h` | 事件枚举 + 参数 ID 宏 | 加你的事件值 |
| `xhh_Event_Template.c` | Trigger + Handle switch 骨架 | 加你的事件 case |
| `xhh_Mode_Template.c` | 状态机枚举 + Change + Handle 骨架 | 加你的状态 + 子步逻辑 |
| `xhh_Task_Flash_Template.c` | Flash 结构体 + Get/Save/IS_Valid/Clean | 填你的字段 + 默认值 |

---

## 使用方式

1. 复制对应骨架到项目目录,重命名(如 `xhh_Task_Template.c` → `xhh_Task_Temp.c`)
2. 全局替换 `Template` / `TEMPLATE` 为你的模块名
3. 填业务逻辑(GPIO 引脚、枚举值、Loop 处理、状态转换)
4. 新 Task 模块记得注册到 `xhh_Task_ALL.h` + `xhh_Task.c`(见 task-module-pattern.md)

---

## 约定提醒

骨架已遵守的约定(改的时候别破坏):
- `xhh_` 前缀 + 缩写词全大写(LED/ADC/BLE)
- 头文件保护 `__XHH_<MODULE>_H__`(双下划线包裹)
- 使能位 `static volatile uint8_t xhh_task_<x>_en`(必须 static)
- Loop 首句 `if (en == 0) return;` 守卫
- BSP 函数 static,不进头文件,.c 顶部前向声明
- Tab 缩进,Allman 大括号,无文件头注释
- 编码 UTF-8,行尾 CRLF
