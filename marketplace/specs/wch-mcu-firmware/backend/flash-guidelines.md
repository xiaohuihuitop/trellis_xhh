# Flash 持久化规范

> 固件没有数据库。持久化 = Flash 结构体直存 + 字段校验 + 无效清默认值。集中在一个模块，不分散写。

---

## 统一读写封装

底层 Flash 操作集中在 `APP/flash/flash.c`，提供四个原语：

| 函数 | 职责 |
|------|------|
| `Flash_Erase(addr)` | 擦除一页 |
| `Flash_Read(buff, len, addr)` | 读 |
| `Flash_Write(buff, len, addr)` | 擦+写 |
| `Flash_NoCheck_Write(buff, len, addr)` | 只写不擦 |
| `Flash_Write_Any(...)` | 跨页写（读整页→擦→合并→写） |

基于厂商 `EEPROM_*` 宏实现，页大小由宏 `FLASH_DATA_PAGE_SIZE` 定义。

```c
// APP/flash/flash.c:13-42
void Flash_Erase(uint32_t addr) { EEPROM_ERASE(addr, FLASH_DATA_PAGE_SIZE); }
void Flash_Read(uint8_t *buff, uint16_t len, uint32_t addr) { EEPROM_READ(addr, buff, len); }
void Flash_Write(uint8_t *buff, uint16_t len, uint32_t addr) {
    Flash_Erase(addr);
    EEPROM_WRITE(addr, buff, len);
}
```

---

## 存储模式：结构体直存

- 用一个集中结构体承载所有用户配置/运行记忆
- 全局唯一运行副本 `g_xhh_user_data`
- 地址用宏定义，不硬编码数字

```c
// xhh_Module/xhh_Task/xhh_Task_Flash.h:10-18
#define FLASH_USER_DATA_ADDR 0

typedef struct {
    uint16_t time;
    xhh_Mode_t mode;
    Motor_Obj_t motor;
    xhh_Task_BAT_Val_t bat_val;
} xhh_Task_Flash_user_data_t;

xhh_Task_Flash_user_data_t g_xhh_user_data;   // 跨文件全局
```

---

## 读写函数命名

| 函数 | 方向 |
|------|------|
| `xhh_Task_Flash_Get_User_Data` | Flash → 结构体 |
| `xhh_Task_Flash_Save_User_Data` | 结构体 → Flash |
| `xhh_Task_Flash_Update_User_Data` | 结构体 → 各 Task 运行时对象 |
| `xhh_Task_Flash_Update_Flash_Data` | 各 Task 运行时对象 → 结构体 |

---

## 有效性校验 + 默认值

- 读取后**先校验再使用**，逐字段范围检查
- 非法数据不部分信任，整体清默认值重写
- 利用 Flash 默认全 1 特性判断首次上电

```c
// xhh_Module/xhh_Task/xhh_Task_Flash.c:58-77
void xhh_Task_Flash_Get_User_Data(xhh_Task_Flash_user_data_t *data) {
    Flash_Read((uint8_t *)data, sizeof(*data), FLASH_USER_DATA_ADDR);  // 用 uint8_t* 匹配签名；历史代码用 uint32_t* 强转是类型隐患，新代码按 uint8_t*
    if (xhh_Task_Flash_User_Data_IS_Valid(data) == 0) {
        xhh_Task_Flash_User_Data_Clean(data);          // 清默认值
        xhh_Task_Flash_Save_User_Data(data);           // 写回
    }
}
```

`IS_Valid` 逐字段范围校验（mode/motor.mode/fun/level_save/hz_power/hz/level），`Clean` 写默认值（MOTOR_MODE_NORMAL / LEVEL_SAVE_1=50 等）。

---

## 启动 / 关机保存时机

- **启动**：`Get_User_Data` → 校验 → `Update_User_Data`（推到各 Task 运行时）
- **关机**：`Update_Flash_Data`（各 Task 收集到结构体）→ `Save_User_Data`

```c
// xhh_Module/xhh_Mode/xhh_Mode.c:313-314（关机时）
xhh_Task_Flash_Update_Flash_Data(&g_xhh_user_data);
xhh_Task_Flash_Save_User_Data(&g_xhh_user_data);
```

---

## 持久化边界

- 持久化集中在 `xhh_Task_Flash.*`，其他模块（Motor/BAT/Timeout）不直接调 `Flash_Write`
- `xhh_Task_Flash` 模块**只做整体管理**：`Get_Flash` / `Set_Flash` / `Clean` / `IS_Valid` / `Init` / `DeInit` / `Cmd`
- **禁止在 Flash 模块内放单字段 Save 接口**（如 `Save_Temp_Max` / `Save_BAT` / `Save_Level`...）——这些由业务模块在自己的 .c 里实现
- 单字段 Save 接口（业务模块内）**只更新 RAM**（`g_xhh_user_data.xxx = value`），**不立即写 Flash**；整体写 Flash 由关机流程集中调用
- 需要立即持久化的场景（如设置即生效）：调用方直接 `g_xhh_user_data.xxx = value; xhh_Task_Flash_User_Data_Set_Flash(&g_xhh_user_data);`
- 读取直接用 `g_xhh_user_data.xxx`，不用函数接口
- OTA 标志单独存固定地址（如 `OTA_DATAFLASH_ADD`），不进用户数据结构体

---

## 禁止

- 把这一层写成数据库抽象（CRUD 术语 / KV / migration）
- 各 Task 模块分散写 Flash
- 未校验就把 Flash 内容应用到运行时
- 在中断里读写 Flash

---

## 初始化后应补充的项目事实

- 实际持久化结构体字段全集
- 实际默认值清单
- 实际读写地址 / 分区表
- 实际启动/关机保存调用点
- 是否启用 CRC（按项目实际宏配置）
