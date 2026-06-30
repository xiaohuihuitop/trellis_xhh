# Flash 持久化规范

> 持久化走 `xhh_BSP_Flash` 逻辑分区 + 各业务 Task 自管 `magic/size/checksum` 头。集中模块不再唯一,按子区独立读写,互不影响。

---

## 底层 Flash 抽象(见 bsp.md)

Flash 硬件操作封装在 `xhh_Module/xhh_BSP/xhh_BSP_Flash.c/.h`,提供逻辑分区接口:

| 函数 | 职责 |
|------|------|
| `xhh_BSP_Flash_Init(void)` | 初始化 |
| `xhh_BSP_Flash_Read(id, offset, buf, len)` | 读逻辑分区 |
| `xhh_BSP_Flash_Erase(id)` | 擦除逻辑分区 |
| `xhh_BSP_Flash_Write(id, offset, buf, len)` | 写逻辑分区(内部读-改-擦-写) |

逻辑分区 ID 枚举:

```c
// xhh_BSP_Flash.h
typedef enum {
    xhh_BSP_FLASH_ID_LIGHT_CONFIG = 0,
    xhh_BSP_FLASH_ID_MOTOR_CONFIG,
    // 按项目业务域注册
} xhh_BSP_FLASH_ID_t;
```

每个 ID 对应固定地址 + 大小,映射在 `xhh_BSP_Flash.c` 内 `static bool app_flash_get_region(id, &addr, &size)`。业务层只认 ID,不碰地址。

`Write` 内部做读-改-擦-写(因 Flash 页擦粒度):

```c
void xhh_BSP_Flash_Write(xhh_BSP_FLASH_ID_t id, uint16_t offset, uint8_t *buf, uint16_t len) {
    uint32_t addr, size;
    if (!app_flash_get_region(id, &addr, &size)) return;
    // 边界检查(offset+len <= size)
    EEPROM_READ(addr, page_buf, size);
    memcpy(&page_buf[offset], buf, len);
    EEPROM_ERASE(addr, size);
    EEPROM_WRITE(addr, page_buf, size);
}
```

具体 EEPROM/HAL 实现按平台,见 [bsp.md](./bsp.md)。

---

## 存储模式:各业务 Task 自管子区

每个业务 Task 维护**自己的存储结构体 + 校验头**,通过 `xhh_BSP_Flash_*` 读写自己的逻辑分区:

```c
// xhh_Task_Light.h
#define APP_LIGHT_CONFIG_MAGIC    0x5848U
#define APP_LIGHT_CONFIG_CHECK_XOR 0xA35AC65A
#define APP_LIGHT_STORE_DELAY_10MS 300   // 改配置后延迟 3s 合并写

typedef struct {
    uint16_t magic;       // 校验头
    uint16_t size;
    uint32_t checksum;    // 滚动移位 XOR
    // 业务字段
    uint8_t brightness;
    uint8_t mode;
    // ...
} xhh_Task_Light_Config_t;
```

### 校验头三件套(magic + size + checksum)

- `magic`:固定标识,初值区分"空 Flash(0xFF)"和"已写入"
- `size`:结构体自身大小,检测版本/布局变化
- `checksum`:32位滚动移位 XOR,检测数据损坏

```c
// 滚动 XOR 算法(项目5 验证)
static uint32_t config_checksum(const uint8_t *data, uint16_t len) {
    uint32_t checksum = APP_LIGHT_CONFIG_CHECK_XOR;
    for (uint16_t i = 0; i < len; i++) {
        checksum = (checksum << 5) | (checksum >> 27);
        checksum ^= data[i];
    }
    return checksum;
}
```

---

## 有效性校验 + 默认值

每个 Task 自己做校验,不全局统一:

```c
void xhh_Task_Light_LoadConfig(void) {
    xhh_BSP_Flash_Read(xhh_BSP_FLASH_ID_LIGHT_CONFIG, 0, &cfg, sizeof(cfg));
    if (cfg.magic != APP_LIGHT_CONFIG_MAGIC || cfg.size != sizeof(cfg) || !Config_Valid(&cfg)) {
        Config_CleanDefaults(&cfg);   // 整体清默认值
        xhh_Task_Light_SaveConfig();  // 写回
    }
}
```

校验失败:**该子区整体清默认值重写**,不影响其他子区。其他 Task 的配置不受影响——这是逻辑分区相对单结构体的核心优势。

---

## 编译期分区检查

每个 Flash 子区结构体必须配静态断言,确保放得下分区:

```c
// xhh_BSP_Flash.c
#define APP_FLASH_STATIC_ASSERT(cond, name) typedef char name[(cond) ? 1 : -1]
APP_FLASH_STATIC_ASSERT(
    sizeof(xhh_Task_Light_Config_t) <= XHH_BSP_FLASH_LIGHT_CONFIG_SIZE,
    light_config_too_large
);
```

结构体增大超过分区大小时,编译期报错,不会运行时溢出。

---

## 写 Flash 时机

| 场景 | 做法 |
|------|------|
| 日常改配置 | 改内存值 + 设置 10ms 倒计时延迟合并写(`APP_LIGHT_STORE_DELAY_10MS`) |
| 倒计时到 | 在 Task `_Loop` 里倒计时归零时调 `xhh_BSP_Flash_Write` |
| 关机集中 | 各 Task 在关机事件/状态里调自己的保存函数 |
| 中断里 | **禁止**写 Flash |

延迟合并写避免配置连续变化时频繁擦写,关机集中写保证关机前落盘。

---

## 简化方案:小项目单结构体

字段少(<5 个)且全项目一个配置对象时,可简化为集中单结构体:

```c
typedef struct {
    xhh_Mode_t mode;
    Motor_Obj_t motor;
} xhh_UserData_t;
xhh_UserData_t g_xhh_user_data;
```

用 `xhh_BSP_Flash` 单分区存储 + magic 校验。各 Task 通过 `g_xhh_user_data.xxx` 读写,关机时 `xhh_BSP_Flash_Write` 整体写。

**判断标准:** 业务域 ≤2 个(如只有电机+电池)用简化;≥3 个(灯光+电机+电池+触摸+日历)用逻辑分区。

---

## 禁止

- `xhh_Task` 直接调 `EEPROM_*` / `HAL_FLASH_Program`(必须经 `xhh_BSP_Flash_*`)
- 在中断里写 Flash
- 未校验就把 Flash 内容应用到运行时
- 一个子区的损坏影响其他子区(逻辑分区的核心保证)
- 给新增结构体不加 `APP_FLASH_STATIC_ASSERT`
- 写 Flash 不做延迟合并(连续改配置时每次立即擦写)