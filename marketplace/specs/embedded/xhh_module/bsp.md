# xhh_BSP 公共层

> 硬件操作集中在项目根目录 `xhh_BSP/`。xhh_BSP 公开平台无关的基础外设参数和操作接口，`xhh_Event/Mode/Task` 与 `Components` 不直接碰厂商 API、寄存器或厂商类型。每个项目只有一个 `Platform/`，xhh_BSP 直接适配当前 MCU。

---

## 目录结构

```text
xhh_BSP/
├── xhh_BSP_Def.h             # 跨类别共用类型
├── xhh_BSP_GPIO.c/.h
├── xhh_BSP_PWM.c/.h
├── xhh_BSP_IWDG.c/.h
├── xhh_BSP_SPI.c/.h
├── xhh_BSP_ADC.c/.h
├── xhh_BSP_Flash.c/.h
├── xhh_BSP_Power.c/.h
├── xhh_BSP_RTC.c/.h
└── xhh_BSP_SYS.c/.h          # 系统初始化、临界区、复位和毫秒延时
```

- 公共头 `.h` 和实现 `.c` 在根目录 `xhh_BSP/` 内，不分芯片子目录
- 每个项目只适配当前唯一 `Platform/`，不在 xhh_BSP 内保留多芯片条件分支
- 更换 MCU 时保持 xhh_BSP 公开接口稳定，重写 `.c` 实现，并同步替换 `Platform/` 和 APP 调度入口

---

## BSP 简洁原则（高优先级）

以下规则优先用于判断 BSP 接口和实现是否合理：

- 跨 MCU 稳定的是公开 `.h` 接口，不是 `.c` 实现。更换 MCU 时直接重写对应 BSP `.c`，不要求不同芯片共用内部实现。
- BSP 是项目开发者手工维护的硬件适配层。引脚、外设实例、通道、分频、模式和时钟源等项目固定参数直接写在 `Init` 实现中。
- 不为了自动适配不同 MCU，引入跨外设通用 ID、运行时对象、Getter、时钟换算层、参数解析器或通用配置结构体。GPIO 使用稳定逻辑信号名，ADC 使用逻辑通道名，二者都只在各自类别内部表达固定资源。
- BSP 的 `Init` 不接收板级配置。端口、引脚、有效极性、频率、最大计数、SPI 模式等当前项目固定参数直接写在对应 BSP `.c`。
- 不为未来可能性预留模式、方向、收发、DMA、中断、DeInit 或其他接口。新增接口必须对应当前真实需求。
- 当前项目未使用的外设不新增空 BSP，也不保留无调用的厂家初始化文件；确认出现真实调用后再增加最小接口和实现。
- 历史项目中手写的 `MX_*`、外设 HAL Handle、DMA 链接、GPIO 复用配置和 MSP 适配应迁入对应 `xhh_BSP_*.c`，Handle 保持文件私有 `static`，不得从公共头暴露。
- 只服务于单个 BSP 的外设 IRQ 入口归对应 `xhh_BSP_*.c`，中断中只调用厂家处理函数或执行该 BSP 所需的最小转发；厂家中断骨架不得反向 include `xhh_BSP`。
- 为保持已确定框架接口或生命周期统一，允许保留占位函数。占位必须是当前项目明确不需要或当前硬件明确不支持的接口，不能借占位提前扩展未确认能力。
- 占位函数体内必须使用 `AI:` 注释说明占位原因、当前行为和启用条件；`.h` 的 Doxygen 同步注明该接口在当前项目为占位。
- `void` 占位函数不得操作硬件；返回状态的占位函数必须返回 `xhh_BSP_ERROR_UNSUPPORTED`。不得用 `xhh_BSP_OK`、0、固定采样值、默认 duty 等伪装成真实实现。
- 简单等待、寄存器写入和少量分支直接写在公开操作函数内，不为隐藏几行代码增加私有辅助函数。
- 只有当前实现已经存在多处复杂重复，且抽取后明显提高可读性和维护性时，才增加 `static` 私有辅助函数。
- 芯片差异完全由各项目 BSP `.c` 承担。禁止为了兼容未知芯片，把当前简单实现改造成计算框架或通用驱动层。
- KISS、YAGNI 优先于形式上的通用性；接口、类型和实现层次保持完成当前需求所需的最小集合。
- 每个公开接口必须在 `.h` 和 `.c` 两处使用 Doxygen 注释：`.h` 记录跨 MCU 稳定的用途、参数、返回值和前置条件；`.c` 记录当前 MCU 的外设、引脚、通道、关键行为及更换 MCU 时的修改点。

---

## 基础外设能力白名单(只按基础能力拆分)

BSP 只按**基础外设能力**拆文件:

| 类别 | 文件 | 典型接口 |
|------|------|----------|
| GPIO | `xhh_BSP_GPIO` | `Init/Read(signal)/Write(signal, active)`，signal 为稳定逻辑信号名 |
| PWM | `xhh_BSP_PWM` | `Init/PWM1_Start/PWM1_Stop/PWM1_Set_Duty` |
| IWDG | `xhh_BSP_IWDG` | `Init/Refresh/DeInit` |
| SPI | `xhh_BSP_SPI` | `SPI1_Init/SPI1_Write_Byte/SPI1_Write` |
| ADC | `xhh_BSP_ADC` | `Init/Read(channel)`，channel 为逻辑通道名 |
| Flash | `xhh_BSP_Flash` | `Erase/Read/Write` |
| RTC | `xhh_BSP_RTC` | `Init/ResetCount/GetCount/GetHz` |
| Power | `xhh_BSP_Power` | `Enter_Wake/Enter_Sleep` |
| SYS | `xhh_BSP_SYS` | `Init/IT_Disable/IT_Enable/Reset/Delay_ms` |

新增 BSP 文件的依据**必须是基础外设能力**(如 I2C/SPI/UART),不是业务设备对象。

---

## 设备型 BSP 黑名单(禁止创建)

这些是 `xhh_Task` 的业务对象,不是基础能力,**禁止**建 BSP 文件:

- ❌ `xhh_BSP_Key` / `xhh_BSP_LED` / `xhh_BSP_Motor` / `xhh_BSP_BAT`
- ❌ `xhh_BSP_HoldPP` / `xhh_BSP_UI` / `xhh_BSP_CSB` / `xhh_BSP_EMS` / `xhh_BSP_RF`

这些对象的业务逻辑放 `xhh_Task_<X>.c/.h`，通过基础 BSP 公开接口使用硬件。

---

## 资源表达策略

GPIO 与 ADC 使用各自类别内的稳定逻辑名，上层不保存真实端口、引脚、有效极性或厂家通道：

```c
typedef enum
{
	xhh_BSP_GPIO_KEY_OK_PRESSED = 0,
	xhh_BSP_GPIO_MOTOR_ENABLE,
	xhh_BSP_GPIO_LCD_SELECT
} xhh_BSP_GPIO_Signal_t;

void xhh_BSP_GPIO_Init(void);
uint8_t xhh_BSP_GPIO_Read(xhh_BSP_GPIO_Signal_t signal);
void xhh_BSP_GPIO_Write(xhh_BSP_GPIO_Signal_t signal, uint8_t active);
```

- `xhh_BSP_GPIO_Init` 一次性配置当前项目实际使用的全部 GPIO，包括模式、上下拉、初始电平和复用。
- `Read` 返回输入逻辑信号是否有效；`Write` 接收输出逻辑信号的有效/无效状态。
- 真实端口、引脚和有效极性只存在于 `xhh_BSP_GPIO.c`，Task、APP 和 Components 不传递硬件配置。
- GPIO 逻辑信号名可以包含 Key、Motor、LCD 等使用语义，但仍统一保留在基础 `xhh_BSP_GPIO` 文件中；禁止据此拆出设备型 BSP。
- 公开头不得出现 `GPIO_TypeDef *`、`GPIO_Pin_*`、HAL Handle、真实引脚号或厂家宏。
- GPIO、PWM、Timer、SPI、ADC 分别设计自己的最小接口，不建立跨类别通用硬件 ID。
- 非法逻辑信号不得静默映射到默认端口、引脚或电平。

Timer 第一版固定提供 100us 和 1ms 两个中断节拍：

```c
typedef void (*xhh_BSP_Timer_Callback_t)(void);

xhh_BSP_Status_t xhh_BSP_Timer_Init_100us(
	xhh_BSP_Timer_Callback_t callback);
xhh_BSP_Status_t xhh_BSP_Timer_Init_1ms(
	xhh_BSP_Timer_Callback_t callback);
```

- Timer 不使用逻辑 ID，不公开硬件定时器实例，也不提供任意周期配置接口。
- `1ms` 节拍必须优先复用 SysTick 实现；若 SysTick 未被其他时基独占，或可通过厂家/RTOS 提供的回调安全挂接，则不得额外占用硬件 Timer。
- 仅当 MCU 没有可用 SysTick，或 SysTick 已被其他时基独占且无法安全复用时，才允许使用硬件 Timer 实现 `1ms` 节拍；实现必须在注释和 README 中说明原因及所用资源。
- `100us` 节拍由项目选择可准确实现该周期的硬件 Timer；不同项目不要求使用相同硬件资源。
- 调用方只初始化实际需要的节拍。未使用的节拍不启动对应硬件或中断，不提供 weak 空回调或静默 fallback。
- 初始化负责检查回调和当前平台能否准确实现固定周期，返回明确状态；调用方必须处理失败结果。
- BSP 中断服务只清除中断标志并调用已注册回调，不直接依赖或调用 `xhh_Task`。
- 回调运行在中断上下文，应只执行置位、计数等短小且确定时间的操作；业务逻辑由主循环消费状态后执行。
- 第一版不提供启动、停止、反初始化、动态改周期和多回调管理接口，出现明确项目需求后再讨论扩展。

IWDG 第一版固定提供三个生命周期接口：

```c
xhh_BSP_Status_t xhh_BSP_IWDG_Init(void);
void xhh_BSP_IWDG_Refresh(void);
void xhh_BSP_IWDG_DeInit(void);
```

- IWDG 超时参数是当前项目的固定实现，不通过公共对象、枚举或 Getter 暴露；更换项目时直接修改 `.c`。
- IWDG 使用的低速时钟由 `xhh_BSP_SYS_Init` 集中开启，IWDG 不自行操作系统时钟源。
- `Init` 返回初始化状态；`Refresh` 是初始化后的直接硬件操作，不返回状态。
- 某些 MCU 的独立看门狗启动后无法由软件关闭。为保持跨项目生命周期接口一致，允许对应平台的 `DeInit` 作为占位函数。
- 占位 `DeInit` 必须在函数体和头文件 Doxygen 中使用 `AI:` 注释明确硬件限制，并保证不操作硬件。

SPI 第一版按项目实际使用的控制器提供独立发送函数族：

```c
void xhh_BSP_SPI1_Init(void);
void xhh_BSP_SPI1_Write_Byte(uint8_t data);
void xhh_BSP_SPI1_Write(const uint8_t *data, uint16_t length);
```

- `SPI1` 是函数名前缀，不是运行时 ID；项目需要第二个 SPI 控制器时，再按真实需求增加 `SPI2` 函数族。
- `Init` 不接收参数；频率、模式、数据宽度、位序、方向、引脚和控制器映射全部由当前项目 `.c` 手工实现。
- `Write_Byte` 和 `Write` 是阻塞发送接口，返回前必须确认硬件已经完成发送。
- SPI 不管理器件 CS、DC、RESET 等信号。外部器件驱动通过 GPIO 控制这些信号，并负责确定一次事务的边界。
- 没有明确需求时，不增加接收、全双工、DMA、中断、DeInit、动态模式切换或通用配置对象。

ADC 使用稳定的逻辑通道名：

```c
typedef enum
{
	xhh_BSP_ADC_CHANNEL_0 = 0,
	xhh_BSP_ADC_CHANNEL_1,
	xhh_BSP_ADC_CHANNEL_2
} xhh_BSP_ADC_Channel_t;

void xhh_BSP_ADC_Init(void);
uint16_t xhh_BSP_ADC_Read(xhh_BSP_ADC_Channel_t channel);
```

- `CHANNEL_0/1/2` 是跨 MCU 稳定的逻辑槽位名，枚举值只用于标识槽位，不代表任何厂家的硬件通道值。
- 公开头只声明当前项目实际使用的逻辑槽位，不出现 `ADC_CHANNEL_5`、`ADC_Channel_8` 等厂家宏，也不按未来可能性预留未使用槽位。
- `Init` 不接收参数；GPIO、ADC 外设、已使用通道、采样时间、对齐方式和校准流程全部由当前项目 `.c` 手工实现。
- 每个 MCU 的 `xhh_BSP_ADC.c` 必须手工把逻辑槽位映射到真实厂家通道值。例如当前项目可将 `CHANNEL_0` 映射到硬件通道 5，换 MCU 后允许映射到其他值。
- 映射使用简单 `switch` 或等价的编译期常量表达，不建设运行时通道注册、Getter 或自动适配框架。
- `Read` 返回对应逻辑槽位的 ADC 原始计数值；非法逻辑槽位不得静默改用默认硬件通道。
- ADC BSP 不执行平均、滤波、电压、温度、电阻或其他业务量换算，这些处理归使用 ADC 的 Task 或 Component。
- 没有明确需求时，不增加连续采样、扫描、DMA、中断、内部参考源、DeInit 或动态采样配置。

Flash 第一版固定提供绝对地址访问接口：

```c
xhh_BSP_Status_t xhh_BSP_Flash_Erase(uint32_t address);
xhh_BSP_Status_t xhh_BSP_Flash_Read(uint32_t address,
									uint8_t *data,
									uint32_t length);
xhh_BSP_Status_t xhh_BSP_Flash_Write(uint32_t address,
									 const uint8_t *data,
									 uint32_t length);
```

- `address` 是当前 MCU 的 Flash 绝对地址，不增加页号、扇区号、区域 ID 或运行时映射层。
- 调用方按项目定义数据地址并避开程序占用区域；`Erase(address)` 擦除该地址所在的一个硬件擦除单元。
- Flash 地址范围、擦除单位和写入单位由项目 `.c` 根据当前 MCU 手工实现。
- `Write` 不自动擦除。调用方必须显式安排擦除和写入顺序，不隐藏数据破坏行为。
- BSP 只读写原始字节，不处理版本、校验、冗余、掉电保护或磨损均衡。

Power 第一版固定提供 MCU 睡眠接口：

```c
void xhh_BSP_Power_Enter_Wake(void);
void xhh_BSP_Power_Enter_Sleep(void);
```

- Power 只负责 MCU 低功耗基础配置和进入睡眠，不包含电池、充电、保持供电或业务关机逻辑。
- `Enter_Wake` 恢复睡眠唤醒后需要重新配置的系统时钟和基础外设；不处理唤醒原因、业务状态或 Event。
- `Enter_Sleep` 在有效中断唤醒后返回；是否允许休眠以及休眠前后的业务处理由上层负责。
- 没有明确需求时，不增加 Stop、Standby、关机、掉电检测、唤醒原因或动态唤醒源接口。

PWM 使用一个类别初始化接口和不包含产品语义的独立操作函数族：

```c
void xhh_BSP_PWM_Init(void);
void xhh_BSP_PWM1_Start(void);
void xhh_BSP_PWM1_Stop(void);
void xhh_BSP_PWM1_Set_Duty(uint16_t duty_count);
```

- `PWM1/2/3` 是函数名前缀，表示当前项目的固定 PWM 接口槽位，不是枚举、对象或运行时 ID；禁止命名为 Motor、HOT、BZ、LED 等产品对象。
- `xhh_BSP_PWM_Init` 初始化并启动当前项目实际使用的全部 PWM 槽位。
- 每路频率、最大占空比计数、初始 duty、定时器和通道都是当前项目固定参数，只在 `xhh_BSP_PWM.c` 中维护。
- Task 不传递 PWM 初始化参数，不保存频率或最大计数的硬件配置，也不调用 PWM Init。
- GPIO、定时器实例、通道和预分频值只存在于 `.c`，换芯片或换项目时允许重写。
- `duty_max_count` 等于 ARR 加 1；`Set_Duty()` 直接写硬件比较计数，不做比例缩放。
- PWM 接口返回 `void`。项目开发者负责在 `.c` 内准确实现当前固定参数组合，不增加自动修正、近似频率或静默 fallback。
- 不同 PWM 若共享同一个硬件定时器，项目实现必须保证频率和周期计数兼容；要求不同频率时应使用独立时基。

---

## 接口命名

| 元素 | 规则 | 示例 |
|------|------|------|
| 公开函数 | `xhh_BSP_<类别>_<动词>`（XHH 分层命名，缩写词全大写 GPIO/PWM/ADC/RTC/IT） | `xhh_BSP_GPIO_Read`、`xhh_BSP_Flash_Write` |
| 公开参数类型 | `xhh_BSP_<类别>_<参数>_t` | `xhh_BSP_GPIO_Signal_t` |
| PWM 函数族 | `xhh_BSP_PWM<n>_<动词>` | `xhh_BSP_PWM1_Set_Duty` |
| SPI 函数族 | `xhh_BSP_SPI<n>_<动词>` | `xhh_BSP_SPI1_Write` |
| ADC 读取 | `xhh_BSP_ADC_Read(channel)` | `xhh_BSP_ADC_Read(xhh_BSP_ADC_CHANNEL_0)` |
| 公开宏 | `XHH_BSP_*` 全大写 | `XHH_BSP_FLASH_LIGHT_CONFIG_SIZE` |
| 内部 static 辅助 | `xhh_BSP_<类别>_<动词>` | `xhh_BSP_GPIO_Resolve`、`xhh_BSP_PWM_Init_Time_Base` |
| 头文件保护 | `XHH_BSP_<类别>_H` | `XHH_BSP_GPIO_H` |

---

## 平台隔离契约

- **公共头 `.h` 不 include 芯片 SDK 头**,只 include `xhh_BSP_Def.h`
  - 禁止 `#include "CH59x_gpio.h"` / `#include "py32f0xx_hal.h"` 出现在 `.h` 里
  - 大小常量(如页大小)在 `.h` 里用自定义宏(如 `XHH_BSP_FLASH_PAGE_SIZE`),不外泄厂商常量
- **实现 `.c` 顶部 `#include "CONFIG.h"`**(WCH)或对应平台头,厂商 API 调用只在 `.c` 内
- 业务层 `#include "xhh_BSP_GPIO.h"` 等公共头,不 include 厂商头

> 若公共头必须暴露某厂商常量(如 `EEPROM_PAGE_SIZE`),改为在 `.h` 里自定义 `XHH_BSP_FLASH_PAGE_SIZE` 宏,而非引厂商头。

---

## 系统级能力（SYS）

`xhh_BSP_SYS` 固定提供以下最小接口：

```c
xhh_BSP_Status_t xhh_BSP_SYS_Init(void);
void xhh_BSP_SYS_IT_Disable(void);
void xhh_BSP_SYS_IT_Enable(void);
void xhh_BSP_SYS_Reset(void);
void xhh_BSP_SYS_ERR_Handle(void);
void xhh_BSP_Delay_ms(uint32_t ms);
```

- `SYS_Init` 必须在其他 BSP 初始化前调用，负责当前项目的系统级确认和已使用外设总线时钟集中开启。
- 系统主时钟只能配置一次。如果 Platform 启动代码已在 `main()` 前完成时钟配置，`SYS_Init` 只确认结果，不重复重配；如果平台要求在应用入口配置，则由当前项目的 `xhh_BSP_SYS.c` 完成。
- GPIO、Timer、PWM、ADC、SPI、USART 等 BSP 不再分别开启自身总线时钟。新增或删除外设 BSP 时，必须同步修改 `SYS_Init` 内的时钟清单。
- 不公开通用“外设时钟 ID”或厂家 RCC 参数；外设时钟清单是当前项目 `xhh_BSP_SYS.c` 的手工配置。
- `IT_Disable/IT_Enable` 使用嵌套深度，并保存最外层进入前的中断状态，防止内层临界区退出时提前打开中断。
- 每次 `IT_Disable` 必须与一次 `IT_Enable` 严格配对。未配对调用的行为未定义，不增加静默忽略或自动修正。
- `Reset` 只处理业务或升级流程明确要求的 MCU 系统复位，不得作为 BSP 参数错误、初始化失败或通信超时的错误处理方式。
- `ERR_Handle` 统一处理 BSP 不可恢复错误，当前固定实现为 `while (1)` 停机，不自动复位、不继续执行，也不增加 fallback。
- 毫秒阻塞延时直接归入 SYS 头文件，不单独建立 Delay BSP；项目实现不得与 Timer BSP 重复占用或破坏同一计时资源。
- APP、xhh_Module、Components 和其他 BSP 需要毫秒延时时统一调用 `xhh_BSP_Delay_ms`，禁止直接调用 `HAL_Delay`、厂家 Delay 或 SDK 延时函数。
- 厂家延时接口只允许出现在 `xhh_BSP_SYS.c` 的 `xhh_BSP_Delay_ms` 平台适配实现中；更换 MCU 时只重写该实现。

跨 Task 共享外设或业务变量的短临界区统一使用 `xhh_BSP_SYS_IT_Disable/IT_Enable` 包夹。临界区内禁止阻塞和耗时操作。

---

## 显示外设低功耗关断

LCD、OLED 等显示外设的背光、显示输出和控制器内部模拟电路是彼此独立的状态。进入 MCU Sleep/Shutdown 前，必须按显示控制器数据手册完成完整关断，不能只关闭背光 GPIO。

- `Display Off` 只关闭像素显示输出，不等于控制器进入低功耗。
- 对支持 Sleep In 的控制器，关机路径必须在关闭 SPI/显示引脚前发送 `Display Off`，再发送 `Sleep In`，并等待数据手册规定的完成时间。
- 控制器休眠序列必须由显示 Component 统一封装，并通过 `xhh_BSP_SPI` 等基础 BSP 接口执行；业务状态机负责在进入低功耗前调用该 Component 接口，不在 Task 的普通 Cmd 中隐式发送。
- 背光 GPIO 必须关闭，但背光关闭只能作为完整关断流程的一步，不能作为低功耗验收依据。
- 冷上电未初始化显示器与“显示器已初始化后关机”必须视为两个不同的电源状态。前者功耗合格不能证明后者合格。
- 唤醒后重新使用显示器时，必须执行控制器规定的 Sleep Out、稳定延时和初始化/恢复流程；禁止假定 Shutdown 后显示控制器自动复位。

典型顺序如下，实际命令与等待时间以控制器数据手册为准：

```text
关闭背光 -> Display Off -> Sleep In -> 等待完成 -> 关闭 SPI/复用/显示引脚 -> MCU Sleep/Shutdown
```

---

## BSP 初始化

- APP 先调用 `xhh_BSP_SYS_Init`，确认成功后直接调用当前项目实际使用的各 `xhh_BSP_*_Init`。
- `xhh_BSP_GPIO_Init` 配置全部固定 GPIO；`xhh_BSP_PWM_Init` 配置全部固定 PWM；ADC、SPI、IWDG、Timer 等由各自 Init 完成初始化。
- APP 不建立 `APP_GPIO_Config`、Board Config 或其他硬件配置整合层。
- Task 不提供 `Config/Init/DeInit`，不接收端口、引脚、频率、通道或有效极性。
- 每个 BSP 的固定参数只在对应 `.c` 维护；新增或删除硬件资源时修改该 BSP Init 和 `xhh_BSP_SYS_Init` 的时钟清单。

---

## BSP ≠ Task

BSP 模块**不**写 `_Loop` / `_Cmd` / 业务使能位，不保存模式、命令、等级等业务状态。HAL Handle、DMA Handle、已注册回调和临界区锁等驱动必需状态允许放在对应 BSP `.c` 的私有 `static` 变量中，禁止通过公共头暴露为可写全局变量。

---

## 业务参数 → 硬件值换算

业务参数(如 Motor 的 level 0~100、Hz 1~100)到硬件值(如 PWM duty、比较值)的换算,**放 Task 内 static 函数**,不进 BSP:

```c
// xhh_Task_Motor.c 内
static uint16_t LevelToDutyCount(uint8_t level) { ... }   // 业务→计数值

// BSP 只接收已换算的硬件计数值
xhh_BSP_PWM2_Set_Duty(LevelToDutyCount(level));
```

PWM 的业务等级到占空比计数换算属于 Task；定时器、ADC 等平台量程换算仍按具体接口判断，平台专属换算用 `static xhh_BSP_<类别>_Scale` 封装。

---

## 扩展流程(新增硬件资源时)

1. 读 `xhh_Task` 列出需要的硬件资源
2. 归类到已有基础 BSP(GPIO/PWM/ADC/Flash/...)
3. 检查根目录 `xhh_BSP/*.h` 的现有参数和接口能否直接表达需求
4. 只有现有参数无法表达已确认需求时，才增加最小必要类型或接口
5. 在 `.c` 内将公开参数转换为当前 Platform 的厂商资源
6. 改 Task 调用为 `xhh_BSP_*` 接口
7. 编译确认 `errors=0`

### 扩展优先级

1. 归入已有基础接口(能否复用 GPIO/PWM/ADC)
2. 扩展已有类别的最小参数或接口
3. 确认是新基础能力(如 I2C/SPI),才新增 `xhh_BSP_<能力>.h/.c`

**禁止因为某个 Task 需要一个设备,就新增设备型 BSP。** 新增 BSP 的依据必须是基础外设能力。

---

## 禁止

- `xhh_Event/Mode/Task` 直接调厂商 API(`GPIOA_ModeCfg`/`HAL_GPIO_PinConfig`/`EEPROM_WRITE`/...)
- 在 `Platform/Src` 中保留项目手写的外设初始化封装、公开 HAL Handle 或仅供单个 BSP 使用的 DMA/IRQ 适配
- 新增设备型 BSP(`xhh_BSP_Key`/`xhh_BSP_LED`/`xhh_BSP_Motor`...)
- 新增 `xhh_Port_*` 转发层(Port + BSP 两层转发)
- 公共头 `.h` include 芯片 SDK 头
- 为编译通过擅自写无注释空实现、fallback 或默认成功
- 在 `xhh_BSP/` 内并存多个芯片实现或通过大段 `#ifdef` 切换平台
- 占位函数没有在函数体内用 `AI:` 注释说明原因、当前行为和启用条件
- 占位函数返回默认成功、伪造硬件值或执行未经确认的 fallback
- 业务参数→硬件值换算放 BSP(应在 Task static)
- BSP 模块写 `_Loop` / `_Cmd` / 使能位
- 同时维护逻辑信号接口和 port/pin 配置接口两套等价 GPIO 抽象
- 在 APP 或 Task 中增加 GPIO/PWM 等硬件 Config 接口
- 在 `xhh_BSP_SYS.c` 以外直接调用厂家毫秒延时接口
