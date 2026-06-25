# Flash 数据布局思考指南

> 新增/修改持久化字段时，先想清楚结构体布局、校验、默认值、读写时机，再动手。

---

## 决策清单

新增一个持久化字段前，回答这些问题：

- [ ] 这个字段真的需要持久化吗？（瞬态调度计数器、BLE 连接状态、UI 渲染状态不该存）
- [ ] 字段类型和范围是什么？（决定校验上下界）
- [ ] 默认值是什么？（Flash 擦除后/首次上电的值）
- [ ] 加进 `xhh_Task_Flash_user_data_t` 结构体的哪个位置？
- [ ] `IS_Valid` 校验函数加了对应字段检查吗？
- [ ] `Clean` 默认值函数加了对应字段赋值吗？
- [ ] `Update_User_Data`（Flash→运行时）和 `Update_Flash_Data`（运行时→Flash）双向同步都加了吗？
- [ ] 结构体大小变化会影响跨页写吗？

---

## 结构体设计原则

- **单一结构体**：所有用户数据集中在 `xhh_Task_Flash_user_data_t`，不要建多个分散结构体
- **定长字段**：用 `uint8_t/uint16_t/uint32_t` 和定长数组，不用指针
- **字段顺序**：按更新频率/相关性分组，频繁一起改的放一起
- **对齐**：注意编译器对齐，跨平台读写时结构体大小要可预测

```c
typedef struct {
    uint16_t time;              // 运行时长
    xhh_Mode_t mode;            // 当前模式
    Motor_Obj_t motor;          // 电机对象（含子字段）
    xhh_Task_BAT_Val_t bat_val; // 电池校准
} xhh_Task_Flash_user_data_t;
```

---

## 校验设计

每个字段都要有上下界校验。枚举校验枚举范围，数值校验 min/max：

```c
uint8_t xhh_Task_Flash_User_Data_IS_Valid(xhh_Task_Flash_user_data_t *data) {
    if (data->mode < MODE_MIN || data->mode > MODE_MAX) return 0;
    if (data->motor.level < MOTOR_LEVEL_MIN || data->motor.level > MOTOR_LEVEL_MAX) return 0;
    ...
    return 1;
}
```

校验失败策略：**整体清默认值重写**，不部分信任。理由：Flash 擦除后是 0xFF，部分字段可能合法部分非法，部分信任会导致混合状态。

---

## 首次上电判断

利用 Flash 默认全 1 特性：

- 擦除后 Flash 字节 = 0xFF
- 校验函数对 0xFF 的字段必然返回非法
- 因此 `Get_User_Data` 读到非法 → `Clean` → `Save`，自动完成首次初始化

不要单独建"首次上电标志位"，靠校验函数即可。

---

## 读写时机

| 时机 | 操作 | 调用点 |
|------|------|--------|
| 开机 | Flash → 结构体 → 校验 → 各 Task 运行时 | 启动流程 `Get_User_Data` |
| 关机 | 各 Task 运行时 → 结构体 → Flash | `xhh_Mode.c` 关机状态 |
| 运行中改配置 | 改运行时对象 → 标记脏 → 关机时统一存 | 不要运行中频繁写 Flash |

**不要在运行中频繁写 Flash**。Flash 擦写次数有限，配置改动先存运行时，关机统一保存。

---

## 常见错误

| 错误 | 后果 | 正确做法 |
|------|------|----------|
| 新增字段没加校验 | 0xFF 值被当合法用 | `IS_Valid` 加上下界 |
| 新增字段没加 Clean | 首次上电该字段是 0xFF | `Clean` 加默认值 |
| 只加了 Flash→运行时，漏了运行时→Flash | 关机存不下来 | 双向 Update 都加 |
| 各 Task 自己写 Flash | 保存时机混乱 | 集中在 `xhh_Task_Flash` |
| 运行中频繁写 Flash | 磨损、时序问题 | 关机统一存 |
| 结构体里放指针 | 重启后指针失效 | 只用定长值/数组 |
