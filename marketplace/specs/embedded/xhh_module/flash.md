# Flash 持久化规范

> 持久化集中在一个模块 `xhh_Task_Flash`：Flash 结构体直存、字段校验与运行时对象同步均由该模块管理，不分散写。

---

## 底层读写封装

底层 Flash 操作由 `xhh_BSP_Flash` 提供(见 [bsp.md](./bsp.md)),本层只调统一签名:

| 函数 | 职责 |
|------|------|
| `xhh_BSP_Flash_Erase(address)` | 擦除绝对地址所在的一个硬件擦除单元 |
| `xhh_BSP_Flash_Read(address, data, length)` | 读原始字节 |
| `xhh_BSP_Flash_Write(address, data, length)` | 写原始字节，不自动擦除 |

`xhh_Task_Flash` 只调这层签名,不直接碰厂商 API(`EEPROM_*`/`HAL_FLASH_Program` 等)。具体实现依 MCU,见 [bsp.md](./bsp.md)。

> 注:BSP 路线下,业务层 Flash 管理仍集中在 `xhh_Task_Flash`;`xhh_BSP_Flash` 只提供原语,不持有业务结构体。

---

## 存储模式:结构体直存

- 用一个集中结构体承载所有用户配置/运行记忆
- 全局唯一运行副本可按实际模块需要定义
- 地址用项目宏定义，不在通用规范中提供示例地址

当前项目在 `xhh_Task_Flash.h` 中定义 `XHH_TASK_FLASH_USER_DATA_ADDRESS`，其值必须是已确认的 Flash 绝对地址；通用规范不提供具体数值。

```c
// xhh_Module/xhh_Task/xhh_Task_Flash.h
typedef struct
{
	/* AI:只保留当前项目明确需要持久化的字段。 */
} xhh_Task_Flash_User_Data_t;

extern xhh_Task_Flash_User_Data_t g_xhh_user_data;
```

---

## 读写函数命名(必须使用这 4 个函数名)

| 函数 | 方向 |
|------|------|
| `xhh_Task_Flash_Get_User_Data` | Flash → 结构体 |
| `xhh_Task_Flash_Save_User_Data` | 结构体 → Flash |
| `xhh_Task_Flash_Update_User_Data` | 结构体 → 各 Task 运行时对象 |
| `xhh_Task_Flash_Update_Flash_Data` | 各 Task 运行时对象 → 结构体 |

---

## 有效性校验

- 读取后**先校验再使用**,逐字段范围检查
- 非法数据不得部分信任
- 利用 Flash 默认全 1 特性判断首次上电

```c
void xhh_Task_Flash_Get_User_Data(xhh_Task_Flash_user_data_t *data) {
    xhh_BSP_Flash_Read(XHH_TASK_FLASH_USER_DATA_ADDRESS, (uint8_t *)data, sizeof(*data));
    if (xhh_Task_Flash_User_Data_IS_Valid(data) == 0)
    {
        /* AI:按当前项目明确的无效数据策略处理。 */
    }
}
```

`IS_Valid` 逐字段范围校验枚举与数值范围。无效数据的处理方式是产品规则，必须由当前项目明确选择；通用规范不预设默认值、自动重写或其他兜底策略。

---

## 启动 / 关机保存时机

- **启动**:`Get_User_Data` → 校验 → `Update_User_Data`(推到各 Task 运行时)
- **关机**:`Update_Flash_Data`(各 Task 收集到结构体)→ `Save_User_Data`

```c
// 关机时(xhh_Mode.c 关机状态)
xhh_Task_Flash_Update_Flash_Data(&g_xhh_user_data);
xhh_Task_Flash_Save_User_Data(&g_xhh_user_data);
```

禁止在运行中频繁写 Flash(Flash 擦写寿命有限)。配置改动先存内存,关机时统一保存。

---

## 持久化边界

- 持久化集中在 `xhh_Task_Flash.*`,其他模块(Motor/BAT/Timeout)不直接调 `xhh_BSP_Flash_Write`
- `xhh_Task_Flash` 模块**只做整体管理**:`Get` / `Set` / `Clean` / `IS_Valid` / `Update` / `Cmd`，不提供 `Init` 或 `DeInit`
- **禁止在 Flash 模块内放单字段 Save 接口**(如 `Save_Temp_Max` / `Save_BAT` / `Save_Level`)——这些由业务模块在自己的 .c 里实现
- 单字段 Save 接口(业务模块内)**只更新 RAM**(`g_xhh_user_data.xxx = value`),**不立即写 Flash**;整体写 Flash 由关机流程集中调用
- 需要立即持久化的场景(如设置即生效):调用方更新运行副本后调用 `xhh_Task_Flash_Save_User_Data(&g_xhh_user_data);`
- 读取直接用 `g_xhh_user_data.xxx`,不用函数接口
- OTA 标志是否独立保存、保存位置与字段布局均由当前项目定义

---

## 禁止

- 把这一层写成数据库抽象(CRUD 术语 / KV / migration)
- 各 Task 模块分散写 Flash(必须经 `xhh_Task_Flash`)
- `xhh_Task_Flash` 之外直接调 `xhh_BSP_Flash_Write`(其他业务模块也不行)
- 未校验就把 Flash 内容应用到运行时
- 在中断里读写 Flash
