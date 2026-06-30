# BSP 公共层

> 硬件操作集中在 `xhh_Module/xhh_BSP/` 公共层,通过逻辑 ID 抽象。`xhh_Event/Mode/Task` 不直接碰厂商 API、寄存器、引脚号、Flash 地址。换 MCU 只改 `xhh_BSP_*.c` 实现,业务层零改动。

---

## 目录结构

```text
xhh_Module/
├── xhh_Event/
├── xhh_Mode/
├── xhh_Task/
└── xhh_BSP/                      # 公共 BSP 层(头 + 实现在同一目录)
    ├── xhh_BSP_Def.c/.h          # 跨类别共用类型/工具
    ├── xhh_BSP_GPIO.c/.h
    ├── xhh_BSP_PWM.c/.h
    ├── xhh_BSP_ADC.c/.h
    ├── xhh_BSP_Flash.c/.h
    ├── xhh_BSP_RTC.c/.h
    ├── xhh_BSP_Power.c/.h
    ├── xhh_BSP_System.c/.h       # 临界区等系统级能力
    └── xhh_BSP_Delay.c/.h        # 按需
```

- 公共头 `.h` 和实现 `.c` 在同一目录,不分芯片子目录
- 换 MCU 时改 `.c` 实现内容,目录名/文件名/接口不变
- 同一套 `.h` 函数名跨芯片一致,只替换 `.c`

---

## 基础外设能力白名单(只按基础能力拆分)

BSP 只按**基础外设能力**拆文件:

| 类别 | 文件 | 典型接口 |
|------|------|----------|
| GPIO | `xhh_BSP_GPIO` | `Init/Read(id)/Write(id, level)` |
| PWM | `xhh_BSP_PWM` | `Init(id)/Set(id, duty)/Update(id_mask)` |
| ADC | `xhh_BSP_ADC` | `Init/Read(id)/Simp_Loop(obj)` |
| Flash | `xhh_BSP_Flash` | `Init/Read(id, off, buf, len)/Erase/Write` |
| RTC | `xhh_BSP_RTC` | `Init/ResetCount/GetCount/GetHz` |
| Power | `xhh_BSP_Power` | `Init/IsLost/EnterSleep` |
| System | `xhh_BSP_System` | `IT_Disable/IT_Enable`(可重入临界区) |
| Delay | `xhh_BSP_Delay` | `Ms/Us`(按需) |

新增 BSP 文件的依据**必须是基础外设能力**(如 I2C/SPI/UART),不是业务设备对象。

---

## 设备型 BSP 黑名单(禁止创建)

这些是 `xhh_Task` 的业务对象,不是基础能力,**禁止**建 BSP 文件:

- ❌ `xhh_BSP_Key` / `xhh_BSP_LED` / `xhh_BSP_Motor` / `xhh_BSP_BAT`
- ❌ `xhh_BSP_HoldPP` / `xhh_BSP_UI` / `xhh_BSP_CSB` / `xhh_BSP_EMS` / `xhh_BSP_RF`

这些对象的业务逻辑放 `xhh_Task_<X>.c/.h`,通过基础 BSP 逻辑 ID 用硬件。

---

## 逻辑 ID 抽象(核心)

每类 BSP 用枚举逻辑 ID 解耦业务语义和物理引脚:

```c
// xhh_BSP_GPIO.h
typedef enum {
    xhh_BSP_GPIO_ID_KEY_MATCH = 0,
    xhh_BSP_GPIO_ID_KEY_POWER,
    xhh_BSP_GPIO_ID_LED_TEST,
} xhh_BSP_GPIO_ID_t;

void xhh_BSP_GPIO_Init(void);
uint8_t xhh_BSP_GPIO_Read(xhh_BSP_GPIO_ID_t id);
void xhh_BSP_GPIO_Write(xhh_BSP_GPIO_ID_t id, uint8_t level);
```

**业务层只认逻辑 ID,看不到 `GPIO_Pin_*` / `CH_PWM*` / `addr`。** 物理映射只在 `.c` 内:

```c
// xhh_BSP_GPIO.c
#define APP_GPIO_KEY_MATCH_PIN  GPIO_Pin_22
#define APP_GPIO_KEY_POWER_PIN  GPIO_Pin_4
#define APP_GPIO_LED_TEST_PIN   GPIO_Pin_15

void xhh_BSP_GPIO_Init(void) {
    GPIOB_ModeCfg(APP_GPIO_KEY_MATCH_PIN, GPIO_ModeIN_PU);
    GPIOA_ModeCfg(APP_GPIO_KEY_POWER_PIN, GPIO_ModeIN_PD);
    GPIOB_ModeCfg(APP_GPIO_LED_TEST_PIN, GPIO_ModeOut_PP_5mA);
}
// Read/Write 用 switch(id) 映射到具体 GPIOA/B 端口
```

同类:PWM 的 `xhh_BSP_PWM_ID_t`、Flash 的 `xhh_BSP_FLASH_ID_t`、ADC 的 `xhh_BSP_ADC_ID_t`。

> 未被使用的逻辑 ID 不应被 Task 调用;BSP 实现只做"已定义逻辑 ID 到真实硬件"映射。

---

## 接口命名

| 元素 | 规则 | 示例 |
|------|------|------|
| 公开函数 | `xhh_BSP_<类别>_<动词>`(snake_case,缩写词全大写 GPIO/PWM/ADC/RTC/IT) | `xhh_BSP_GPIO_Read`、`xhh_BSP_Flash_Write` |
| 逻辑 ID 枚举 | `xhh_BSP_<类别>_ID_t`,枚举值 `xhh_BSP_<类别>_ID_<对象>` | `xhh_BSP_GPIO_ID_t`、`xhh_BSP_GPIO_ID_KEY_MATCH` |
| 公开宏 | `XHH_BSP_*` 全大写 | `XHH_BSP_FLASH_LIGHT_CONFIG_SIZE` |
| 内部 static 辅助 | `app_<类别>_<动词>` | `app_pwm_scale`、`app_flash_get_region`、`app_system_irq_status` |
| 物理参数宏 | `APP_<类别>_<对象>_*` 全大写 | `APP_GPIO_KEY_MATCH_PIN`、`APP_FLASH_LIGHT_CONFIG_ADDR` |
| 头文件保护 | `__XHH_BSP_<类别>_H__` | `__XHH_BSP_GPIO_H__` |

---

## 平台隔离契约

- **公共头 `.h` 不 include 芯片 SDK 头**,只 include `xhh_BSP_Def.h`
  - 禁止 `#include "CH59x_gpio.h"` / `#include "py32f0xx_hal.h"` 出现在 `.h` 里
  - 大小常量(如页大小)在 `.h` 里用自定义宏(如 `XHH_BSP_FLASH_PAGE_SIZE`),不外泄厂商常量
- **实现 `.c` 顶部 `#include "CONFIG.h"`**(WCH)或对应平台头,厂商 API 调用只在 `.c` 内
- 业务层 `#include "xhh_BSP_GPIO.h"` 等公共头,不 include 厂商头

> 若公共头必须暴露某厂商常量(如 `EEPROM_PAGE_SIZE`),改为在 `.h` 里自定义 `XHH_BSP_FLASH_PAGE_SIZE` 宏,而非引厂商头。

---

## 临界区(System)

跨 Task 共享外设(如 RTC 计数)的读写必须用 `xhh_BSP_System_IT_Disable/IT_Enable` 包夹:

```c
// 可重入:用 lock_depth 计数,可嵌套调用不破坏
void xhh_BSP_System_IT_Disable(void) {
    if (app_system_irq_lock_depth == 0U)
        SYS_DisableAllIrq(&app_system_irq_status);
    app_system_irq_lock_depth++;
}
void xhh_BSP_System_IT_Enable(void) {
    if (app_system_irq_lock_depth > 0U) {
        app_system_irq_lock_depth--;
        if (app_system_irq_lock_depth == 0U)
            SYS_RecoverIrq(app_system_irq_status);
    }
}
```

各 Task 内对跨 Task 共享业务变量的保护也走同一对。临界区实现按平台(上例为 WCH)。

---

## BSP 初始化入口

由 APP 层 `main_task.c` 集中调一次:

```c
// main_task.c
void BSP_Init(void) {
    xhh_BSP_GPIO_Init();
    xhh_BSP_PWM_Init(xhh_BSP_PWM_ID_LIGHT_W);
    xhh_BSP_Flash_Init();
    xhh_BSP_Power_Init();
}
```

`BSP_Init()` 在 `MAIN_START_EVT` 内调用。各 `xhh_BSP_<类别>_Init` 按需接受 ID 参数(如 PWM 需初始化多个通道)。

---

## BSP ≠ Task

BSP 模块**不**写 `_Loop` / `_Cmd` / `volatile uint8_t en`。BSP 是无状态服务层(最多一对静态锁变量),状态归属仍在业务 Task 内。

---

## 业务参数 → 硬件值换算

业务参数(如 Motor 的 level 0~100、Hz 1~100)到硬件值(如 PWM duty、比较值)的换算,**放 Task 内 static 函数**,不进 BSP:

```c
// xhh_Task_Motor.c 内
static uint16_t LevelToDuty(uint8_t level) { ... }   // 业务→duty
static uint16_t HzToDuty(uint8_t hz) { ... }

// BSP 只接收已换算的 duty
xhh_BSP_PWM_Set(xhh_BSP_PWM_ID_MOTOR_LEVEL, LevelToDuty(level));
```

BSP 内若有平台相关的 scale(如 1000→255),那是 BSP 自己的实现细节,用 `static app_<类别>_scale` 封装。

---

## 扩展流程(新增硬件资源时)

1. 读 `xhh_Task` 列出需要的硬件资源
2. 归类到已有基础 BSP(GPIO/PWM/ADC/Flash/...)
3. 检查 `xhh_BSP/*.h` 是否已有对应逻辑 ID
4. 缺则只扩展最小必要枚举(不预留未用 ID)
5. 在 `.c` 内把逻辑 ID 映射到真实硬件
6. 改 Task 调用为 `xhh_BSP_*` 接口
7. 编译确认 `errors=0`

### 扩展优先级

1. 归入已有基础接口(能否复用 GPIO/PWM/ADC)
2. 加逻辑 ID(在已有类别加枚举值)
3. 确认是新基础能力(如 I2C/SPI),才新增 `xhh_BSP_<能力>.h/.c`

**禁止因为某个 Task 需要一个设备,就新增设备型 BSP。** 新增 BSP 的依据必须是基础外设能力。

---

## 禁止

- `xhh_Event/Mode/Task` 直接调厂商 API(`GPIOA_ModeCfg`/`HAL_GPIO_PinConfig`/`EEPROM_WRITE`/...)
- 新增设备型 BSP(`xhh_BSP_Key`/`xhh_BSP_LED`/`xhh_BSP_Motor`...)
- 新增 `xhh_Port_*` 转发层(Port + BSP 两层转发)
- 公共头 `.h` include 芯片 SDK 头
- 为编译通过擅自写空实现 / fallback / 默认成功
- 把芯片实现目录写死示例名(必须用当前工程实际芯片名)
- 业务参数→硬件值换算放 BSP(应在 Task static)
- BSP 模块写 `_Loop` / `_Cmd` / 使能位