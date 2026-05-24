# 推箱子(Sokoban) 详细设计文档

> **平台**: MDV-STM32-F107 (NUAA_CM3_107) | **MCU**: STM32F107VCT6  
> **LCD**: ILI9341 3.2" TFT 320×240 横屏 | **IDE**: Keil MDK-ARM V5 (ARMCC V5)  
> **标准库**: STM32F10x_StdPeriph_Driver V3.5

---

## 1. 硬件资源分配

### 1.1 GPIO 引脚总表

| 外设 | 信号 | GPIO | 模式 | 说明 |
|------|------|------|------|------|
| LCD_D0~D15 | DATA[15:0] | PE0~PE15 | Out_PP, 50MHz | FSMC 16位并行数据 |
| LCD_CS | 片选 | PC6 | Out_PP | 低有效 |
| LCD_RS | 寄存器选择 | PD13 | Out_PP | 0=命令, 1=数据 |
| LCD_WR | 写使能 | PD14 | Out_PP | 低有效 |
| LCD_RD | 读使能 | PD15 | Out_PP | 低有效 |
| LCD_RST | 复位 | PC5 | Out_PP | 低有效(原理图标注NC) |
| LCD_LED | 背光 | PB14 | Out_PP | 1=亮, 0=灭 |
| KEY1 | 上方向 | PD11 | IPU | 按下=0 |
| KEY2 | 下方向 | PD12 | IPU | 按下=0 |
| KEY3 | 左方向/确认 | PC13 | IPU | 按下=0 |
| KEY4 | 右方向/WAKEUP | PA0 | IPU | 按下=0 |
| LED1 | 指示灯1 | PD2 | Out_PP | 0=亮 |
| LED2 | 指示灯2 | PD3 | Out_PP | 0=亮 |
| LED3 | 指示灯3 | PD4 | Out_PP | 0=亮 |
| LED4 | 指示灯4 | PD7 | Out_PP | 0=亮 |
| LED5 | 指示灯5 | PB13 | Out_PP | 0=亮 |
| BEEP | 蜂鸣器 | PC0 | Out_PP | 1=响, 0=停 |
| USART1_TX | 串口发送 | PA9 | AF_PP | PC键盘 |
| USART1_RX | 串口接收 | PA10 | IN_FLOATING | PC键盘 |

### 1.2 时钟与外设

- **系统时钟**: 72MHz (外部晶振 25MHz, PLL×9, SYSCLK=HCLK=PCLK2=72MHz, PCLK1=36MHz)
- **SysTick**: 1ms 中断 (`SystemCoreClock / 1000`)
- **NVIC**: 优先级分组2 (Preemption 2bit, Sub 2bit)
  - SysTick: Pre=0, Sub=0
  - USART1: Pre=1, Sub=1
- **FSMC**: Bank1, 16bit NOR/PSRAM 模式, 驱动ILI9341 (8080并口)
- **PWR + BKP**: `PWR_BackupAccessCmd` 使能备份域访问, 备份寄存器存储高分 (DR1=magic, DR2~DR16=每关最佳步数)

### 1.3 硬件连接与跳线设置

> 实验板型号: MDV-STM32-F107（NUAA_CM3_107 核心板 + 底板）

#### 1.3.1 基本连接（所有实验通用）

（1）连接 +5V 电源到开发板的 USB Mini-B 接口（底板自带），打开电源开关。板载 LDO 会将 5V 降压为 3.3V 供 MCU 及外设使用。

（2）将 J-Link 或 ST-Link 仿真器连接到 MDV-STM32-F107 嵌入式系统实验开发板的 20Pin JTAG 插座上（兼容 SWD 接口：SWDIO=PA13, SWCLK=PA14）。将仿真器的 USB 插头连接到 PC 的 USB 插口。

> 注意: 如果开发板已通过 USB Mini-B 供电，仿真器无需再提供 3.3V 供电，两种供电方式不可同时使用。

（3）LCD 模块（ILI9341, 3.2 寸 TFT 320×240 横屏）通过 40Pin 排线直接插接在底板 J1 排母上，无需额外飞线。LCD 以 FSMC 8080 并口方式与 MCU 通信：
- 16 位数据总线 D0~D15 → PE0~PE15
- 片选 CS → PC6，寄存器选择 RS → PD13
- 写使能 WR → PD14，读使能 RD → PD15
- 背光 BL → PB14（1=亮, 0=灭）

（4）按键、LED、蜂鸣器均焊接在底板上，通过 PCB 走线直连 MCU，无需额外设置跳线：
- KEY1（上）→ PD11，KEY2（下）→ PD12
- KEY3（左/确认）→ PC13，KEY4（右/返回）→ PA0 (WAKEUP)
- LED1~4 → PD2, PD3, PD4, PD7（低电平点亮）
- LED5 → PB13（低电平点亮）
- 蜂鸣器 BEEP → PC0（1=响, 0=停，软件 PWM 驱动）

（5）如需使用 PC 键盘操控（USART1 串口通信，115200-8-N-1）：用杜邦线将 USB 转 TTL 串口模块（如 CH340、CP2102）与底板连接：
```
MCU  PA9  (TX)  ────  串口模块 RX
MCU  PA10 (RX)  ────  串口模块 TX
底板 GND        ────  串口模块 GND
底板 3.3V       ────  串口模块 VCC（切勿接 5V!）
```
将串口模块 USB 端插入 PC，打开串口终端（波特率 115200，数据位 8，无校验，停止位 1），即可使用 W/A/S/D 控制方向、U 撤销、R 重置、M 返回菜单。

> 提示: 如果底板有 USART1 跳线（JP23、JP24），将两者短接到右端，使 PA9 连接 U1TX、PA10 连接 U1RX，则可直接使用底板上的 RS-232 接口（DB9）连接 PC，无需外部串口模块。

#### 1.3.2 软件准备与编译下载

（1）复制本工程 "pushBit" 文件夹中的所有内容到 D 盘（或任意路径），双击 `pushBit\Project\Project.uvprojx` 工程文件，在 Keil uVision V5 (ARMCC V5) 中打开。

（2）打开后阅读 `User/Main/main.c`（主循环与状态机）、`User/Main/hw_config.c`（GPIO/USART/NVIC 初始化）、`User/Lcd/lcd.c`（ILI9341 FSMC 驱动）了解程序结构。

（3）按 F7 功能键编译并链接工程，确认 0 Error, 0 Warning。

（4）按 Ctrl+F5 键或单击调试（Debug）按钮，进入集成调试环境。

（5）按 F5 功能键全速运行，观察 LCD 上显示的启动画面 → 菜单界面。按 KEY1(上)/KEY2(下) 切换菜单项，按 KEY3(左) 确认。选关页面超长按 (≥1.5s) KEY3/KEY4 返回菜单。详见 §5 输入系统。

（6）进入游戏后按方向键移动玩家: 短按 (<0.8s) 移动, 长按 (0.8~1.5s) 撤销, 超长按 (≥1.5s) KEY3/KEY4 返回菜单。通关后自动进入下一关，播放胜利动画与音效。

（7）如需查看串口控制功能：连接 USB 转串口模块（步骤 1.3.1-(5)），打开 PC 端串口助手（115200-8-N-1），键入 W/A/S/D 控制方向，U=撤销，R=重置，M=返回菜单，观察 LCD 同步变化。

#### 1.3.3 引脚速查

| 外设 | MCU 引脚 | 连接方式 |
|------|---------|---------|
| LCD D0~D15 | PE0 ~ PE15 | 40Pin 排线 → 底板 J1 |
| LCD CS/RS/WR/RD | PC6, PD13, PD14, PD15 | 同上 |
| LCD 背光 | PB14 | 同上 |
| KEY1~4 | PD11, PD12, PC13, PA0 | 底板焊接 |
| LED1~4 | PD2, PD3, PD4, PD7 | 底板焊接 |
| LED5 | PB13 | 底板焊接 |
| 蜂鸣器 | PC0 | 底板焊接 |
| USART1 TX/RX | PA9, PA10 | 杜邦线 → 串口模块 |
| SWD 调试 | PA13(SWDIO), PA14(SWCLK) | 20Pin JTAG 插座 |

---

## 2. 屏幕布局

横屏 320×240 像素 (`USE_HORIZONTAL=1`)。

```
┌────────────────────────┬──────────┐
│                        │ SOKOBAN  │  y=0
│                        │ Level:01 │
│     游戏区域           │ Steps:   │
│     (240×240)          │  00128   │
│                        │ Best:    │
│     瓦片居中绘制       │  00067   │
│                        │          │
│   关卡≤10格: 22px/格   │ Key:Move │
│   关卡>10格: 16px/格   │ Hold0.8s│
│                        │  :Undo   │
│                        │ Hold1.5s │
│                        │  :Menu   │
│                        │ Serial:  │
│                        │  WASD    │
└────────────────────────┴──────────┘
 ←── 240px ──→             ← 80px ─→
```

### 2.1 瓦片尺寸策略

| 关卡最大维度 | 瓦片尺寸 | 最大格子数(240/ts) | 应用关卡 |
|-------------|---------|-------------------|---------|
| ≤ 10格 | 22px | 10×10 | Lv1~8 (8x8~10x9) |
| > 10格 | 16px | 15×15 | Lv9~15 (11x9~12x10) |

实际关卡宽度 8~12，22px/16px 两档完全覆盖。

### 2.2 颜色编码 (RGB565)

| 常量名 | 值 | 用途 |
|--------|-----|------|
| COLOR_LGRAY (0xC618) | 浅灰 | 地板底色 |
| COLOR_GRAY (0x8430) | 灰 | 网格线/边框 |
| COLOR_ORANGE (0xFC00) | 橙 | 箱子填充 |
| COLOR_BROWN (0xBC40) | 棕 | 箱子交叉线/墙壁 |
| COLOR_BLUE (0x001F) | 蓝 | 玩家 |
| COLOR_RED (0xF800) | 红 | 目标点菱形 |
| COLOR_GREEN (0x07E0) | 绿 | 到位箱子外框 |
| COLOR_DARKBLUE (0x01CF) | 深蓝 | 信息栏背景 |
| COLOR_YELLOW (0xFFE0) | 黄 | 高亮/选中 |
| COLOR_BLACK (0x0000) | 黑 | 背景/屏幕清除 |
| COLOR_WHITE (0xFFFF) | 白 | 文字 |
| 0x8A45 | 深暖棕 | 墙壁填充 |

---

## 3. 软件架构

### 3.1 分层架构

```
┌────────────────────────────────────────────┐
│ main.c: 主循环, 状态机分发                   │
├────────────────────────────────────────────┤
│ App层: app.c → menu.c / game.c / select.c   │
│        状态: SPLASH → MENU ↔ GAME ↔ SELECT  │
├────────────────────────────────────────────┤
│ Logic层: map.c   → 地图数据管理              │
│          move.c  → 碰撞检测 + 推动 + 胜利判定 │
│          undo.c  → 环形撤销栈(50步)          │
│          score.c → 步数 + BKP高分存储        │
├────────────────────────────────────────────┤
│ Render层: tile.c → 瓦片程序化绘制(6种)       │
│           font.c → 文字渲染封装              │
│           ui.c   → 菜单按钮/关卡网格/进度条   │
│           anim.c → 启动画面/过关动画          │
├────────────────────────────────────────────┤
│ Drv层: audio.c → 蜂鸣器音效(软件PWM)        │
│  (stm32f10x_it.c 中: 按键扫描+串口解析)      │
├────────────────────────────────────────────┤
│ HAL: lcd.c/h (ILI9341 FSMC),              │
│      hw_config.c/h (GPIO/USART/NVIC),      │
│      delay.c/h (DWT硬件延时 + SysTick)     │
└────────────────────────────────────────────┘
```

### 3.2 主循环流程

```
main()
  ├── SystemInit() + SysTick_Config()
  ├── Delay_Init()           // DWT 周期计数器初始化
  ├── GPIO_Configuration()  // LED, 按键, 蜂鸣器
  ├── NVIC_Configuration()  // USART1, SysTick 中断优先级
  ├── USART_Configuration() // 115200-8-N-1
  ├── LCD_Init()            // ILI9341 FSMC 初始化
  ├── Score_Load()          // 从BKP寄存器读取高分
  ├── Audio_Init()          // 蜂鸣器初始静音
  ├── Anim_BootLogo()       // 启动画面(3秒或按键跳过)
  │
  └── while(1):
        ev = Input_Poll()      // 按键扫描 + 串口接收
        switch(g_state):
          STATE_MENU:   Menu_Update(ev); Menu_Draw()
          STATE_GAME:   Game_Update(ev)        // 增量重绘 + 音效
          STATE_SELECT: Select_Update(ev); Select_Draw()
          STATE_HELP:   Help_Draw(); 等待按键
```

### 3.3 测试模式

`User/Game/config.h` 中的 `TEST_MODE` 宏控制选关行为:

```c
#define TEST_MODE       /* 测试版: 15关全解锁, 任意跳关 */
// #define TEST_MODE    /* 正常版: 逐关解锁, 需通关才能解锁下一关 */
```

切换模式只需注释/取消注释这一行，重新编译即可。

### 3.4 状态转换图

```
SPLASH ──(超时/按键)──▶ MENU ──(Start)──▶ GAME ──(过关)──▶ WIN动画 ──▶ GAME(下一关)
                           │                  │                    │
                           ├──(Select)──▶ SELECT ──(选关)──▶ GAME │
                           │    ▲            │                    │
                           │    └──(Back)────┘                    │
                           │                                      │
                           ├──(Help)──▶ HELP ──(按键)──▶ MENU    │
                           │                                      │
                           └──(Menu键←)────────────── GAME ──────┘
```

---

## 4. 核心数据结构

### 4.1 地图 (map.c)

```c
// 双层分离存储, 每格 1 字节, 最大 20×20
typedef struct {
    uint8_t width, height;
    uint8_t ground[MAP_MAX_SIZE];    // 0=空地, 1=墙, 2=目标
    uint8_t entities[MAP_MAX_SIZE];  // 0=空, 1=玩家, 2=箱子
    uint8_t player_x, player_y;
} MapData;

// 关卡定义(Flash只读)
typedef struct {
    uint8_t id;
    char    name[24];
    uint8_t width, height;
    const uint8_t *ground;    // 指向Flash中的常量数组
    const uint8_t *entities;
    uint8_t player_x, player_y;
} LevelDef;
```

**内存占用**: `MapData` (运行时) = 2 + 2×400 + 2 = 804 字节, 远小于 64KB SRAM。  
**Flash占用**: 15关 × ~800字节 = ~12KB, 远小于 256KB Flash。

### 4.2 移动记录与撤销 (undo.c)

```c
typedef struct {
    uint8_t player_from_x, player_from_y;
    uint8_t player_to_x,   player_to_y;
    uint8_t box_from_x,    box_from_y;
    uint8_t box_to_x,      box_to_y;
    uint8_t has_box;  // 0=纯移动, 1=推动了箱子
} MoveRecord;

// 环形缓冲区
static MoveRecord g_undo_stack[UNDO_DEPTH];  // 50 × 9字节 = 450字节
static uint8_t g_undo_head, g_undo_count;
```

### 4.3 计分 (score.c)

```c
uint32_t g_steps;                     // 当前局步数
uint32_t g_best_steps[MAX_LEVELS];    // 每关最佳(0=未通关)

// 备份寄存器映射:
// BKP_DR1  = 0x50C0 (magic number)
// BKP_DR2  = 关卡0最佳
// BKP_DR3  = 关卡1最佳
// ...
// BKP_DR16 = 关卡14最佳
```

### 4.4 移动算法

```
Move_Execute(dx, dy, &rec):
  nx = player_x + dx    // 下一步坐标
  ny = player_y + dy

  // 第1关: 边界+墙壁检查
  if nx,ny 超出地图 OR ground[ny][nx] == WALL → return MOVE_WALL

  // 第2关: 前方有箱子
  if entities[ny][nx] == BOX:
      bx = nx + dx      // 箱子目标坐标
      by = ny + dy
      if bx,by 超出 OR ground[by][bx] == WALL OR entities[by][bx] == BOX
          → return MOVE_BOX_BLOCKED

      // 推箱子
      entities[ny][nx] = EMPTY
      entities[by][bx] = BOX
      rec.has_box = 1, 记录箱子位移

  // 第3步: 移动玩家
  entities[player_y][player_x] = EMPTY
  entities[ny][nx] = PLAYER
  player_x = nx, player_y = ny
  return OK (或 BOX_OK)

// 胜利判定:
Move_CheckWin():
  for all (x,y): if ground[y][x]==TARGET AND entities[y][x]!=BOX → 未完成
  return 1  // 所有目标点都有箱子
```

### 4.5 脏矩形渲染策略

```
Game_Update(方向):
  px_before = player_x (旧位置)
  py_before = player_y

  Move_Execute(方向, &rec)   // 更新地图数据
  Score_AddStep()
  Undo_Push(&rec)

  // 只重绘变化的格子(最多3-4个)
  Tile_Draw(px_before, py_before, ..., ENT_EMPTY)   // 玩家离开
  Tile_Draw(player_x, player_y, ..., ENT_PLAYER)    // 玩家到达
  if 推了箱子:
      Tile_Draw(box_to_x, box_to_y, ..., ENT_BOX)   // 箱子到达

  Tile_UpdateInfo(...)  // 更新信息栏步数

  if Move_CheckWin():
      Score_SaveBest()
      Anim_WinSequence()
      → 下一关 / 通关画面
```

---

## 5. 输入系统

### 5.1 事件生成层 (stm32f10x_it.c)

物理按键 `短按/长按/超长按` 由 `Input_Scan()` 根据 **按下持续时长** 统一生成对应事件:

| 按下时长 | 生成事件 | 有效按键 |
|---------|---------|---------|
| 短按 (<0.8s) 后释放 | `INPUT_UP / DOWN / LEFT / RIGHT` | 对应的方向键 |
| 长按 (0.8s ~ 1.5s) | `INPUT_UNDO` | **全部 4 键** |
| 超长按 (≥1.5s) | `INPUT_MENU` | **KEY3(左键) + KEY4(右键)** |

串口键盘 (USART1, 115200-8-N-1) 产生独立事件:

| 输入字符 | 事件 |
|---------|------|
| W / A / S / D | INPUT_UP / LEFT / DOWN / RIGHT |
| U | INPUT_UNDO |
| R | INPUT_RESET |
| M | INPUT_MENU |
| 空格 / 回车 | INPUT_CONFIRM |

### 5.2 各页面按键行为详表

#### 5.2.1 启动动画 (Splash)

| 操作 | 行为 |
|------|------|
| 任意键短按 | 跳过动画 → 进入主菜单 |
| 3 秒超时 | 自动进入主菜单 |

#### 5.2.2 主菜单 (Menu)

| 按键 | 短按 (<0.8s) | 长按 (0.8~1.5s) | 超长按 (≥1.5s) |
|------|-------------|-----------------|---------------|
| **KEY1 上** | 光标上移 (循环) | UNDO (无响应) | — |
| **KEY2 下** | 光标下移 (循环) | UNDO (无响应) | — |
| **KEY3 左** | **确认选中项** | UNDO (无响应) | MENU (已是根页,无响应) |
| **KEY4 右** | (无响应) | UNDO (无响应) | MENU (已是根页,无响应) |

> 菜单项: Start Game / Select Level / Help，共 3 项循环

#### 5.2.3 选关页面 (Select)

| 按键 | 短按 (<0.8s) | 长按 (0.8~1.5s) | 超长按 (≥1.5s) |
|------|-------------|-----------------|---------------|
| **KEY1 上** | 光标上移一行 (-4) | UNDO = **确认进关卡** | — |
| **KEY2 下** | 光标下移一行 (+4) | UNDO = **确认进关卡** | — |
| **KEY3 左** | 左移一列; **光标=0 → 返回菜单** | UNDO = **确认进关卡** | MENU = 返回菜单 |
| **KEY4 右** | 右移一列 (+1) | UNDO = **确认进关卡** | MENU = 返回菜单 |

> 仅可进入已解锁关卡 (通关后自动解锁下一关)。长按任意键 = 确认，无需串口键盘。

#### 5.2.4 游戏页面 (Game)

| 按键 | 短按 (<0.8s) | 长按 (0.8~1.5s) | 超长按 (≥1.5s) |
|------|-------------|-----------------|---------------|
| **KEY1 上** | 玩家向上移动 | UNDO = 撤销一步 | — |
| **KEY2 下** | 玩家向下移动 | UNDO = 撤销一步 | — |
| **KEY3 左** | 玩家向左移动 | UNDO = 撤销一步 | MENU = 返回菜单 |
| **KEY4 右** | 玩家向右移动 | UNDO = 撤销一步 | MENU = 返回菜单 |

> 撞墙/推箱受阻 → 播放 `SOUND_BLOCKED`。撤销深度 50 步。通关后自动播放胜利动画并进入下一关。

#### 5.2.5 帮助页面 (Help)

| 操作 | 行为 |
|------|------|
| 任意键 | 返回主菜单 |

#### 5.2.6 胜利动画 (Win)

| 操作 | 行为 |
|------|------|
| 任意键 | 动画自动播放 (不可跳过)，完成后自动下一关 |

### 5.3 按键扫描实现

- **扫描频率**: 10ms (SysTick 驱动, `g_systick` 差值判断)
- **去抖**: 2 阶段消抖 — 连续两次读取值一致才确认状态变更
- **事件队列**: 环形缓冲 8 个 InputEvent, 防止事件丢失
- **优先级**: 物理按键事件优先于串口事件
- **队列清空**: `Input_Flush()` 在状态切换时调用，清空事件队列并重置所有按键长按状态，防止残留事件（如长按确认后未松手产生的 MENU 事件）干扰新页面

```
按下沿 → 记录 press_time
  ├── 释放 (held < 0.8s)  未发过长按 → 短按事件 (方向)
  ├── 持续 (held > 0.8s)  未发过      → 长按事件 (UNDO)
  └── 持续 (held ≥ 1.5s)  KEY3/KEY4   → 超长按事件 (MENU)
```

---

## 6. 渲染系统

### 6.1 瓦片程序化绘制 (tile.c)

全部用 LCD 基本绘图函数实现, 零位图资源:

| 瓦片 | 绘制方法 | 配色 |
|------|---------|------|
| 地板 | Fill 浅灰 + Rectangle 灰色网格线 | LGRAY + GRAY |
| 墙壁 | Fill 深棕 + 十字砖缝线(DrawLine) | 0x8A45 + BLACK |
| 目标点 | DrawLine 菱形 + Fill 红色填充 | RED |
| 箱子 | Fill 橙色 + Rectangle 棕框 + 交叉对角线 | ORANGE + BROWN |
| 到位箱子 | 同上 + Rectangle 绿色双层外框 | GREEN |
| 玩家 | DrawCircle 头 + 梯形Fill 身体 | BLUE |

### 6.2 文字渲染 (font.c)

封装 `lcd.h` 的 `Show_Str` / `LCD_ShowNum`:
- `Font_DrawString(x, y, str, fg, bg, size)` — 左对齐
- `Font_DrawNum(x, y, num, len, size, fg, bg)` — 数字
- `Font_DrawCenter(x, y, w, str, fg, bg, size)` — 居中

文字尺寸: 12px / 16px / 24px / 32px (依赖 lcd.h 的字库)

---

## 7. 音频系统 (audio.c)

### 7.1 蜂鸣器驱动

PC0 无硬件 TIM 通道, 采用软件 GPIO 翻转 + DWT 硬件延时 (阻塞式, 音效短):

```c
void Audio_Beep(uint16_t freq, uint16_t duration_ms) {
    uint32_t half_us = 500000 / freq;
    uint32_t cycles = duration_ms * 1000 / (half_us * 2);
    for (i = 0; i < cycles; i++) {
        BEEP(1); Delay_us(half_us);
        BEEP(0); Delay_us(half_us);
    }
}
```

`Delay_us()` 基于 DWT 周期计数器 (Cortex-M3 硬件), 不受编译器优化影响, 且 SysTick 中断在延时期间持续递增 `g_systick`。

### 7.2 音效表

| 音效 | 频率 | 时长 | 触发位置 |
|------|------|------|---------|
| SOUND_MOVE | 300Hz | 30ms | game.c: 玩家纯移动 |
| SOUND_PUSH | 500Hz | 80ms | game.c: 推动箱子 |
| SOUND_BLOCKED | 120Hz | 120ms | game.c: 撞墙/被挡 |
| SOUND_WIN | 523→659→784→1047Hz | 多段 | game.c: 过关 |
| SOUND_SELECT | 800Hz | 25ms | menu.c + select.c: 菜单/选关操作 |
| SOUND_UNDO | 500→300Hz | 30+40ms | game.c: 撤销 |

### 7.3 初始化

`Audio_Init()` 在 `main.c` 中调用 (位于 `Score_Load()` 之后), 调用 `BEEP(0)` 确保蜂鸣器初始为静音。`BEEP_Init()` (hw_config.c) 启用 TIM2 时钟备用, 当前使用软件翻转无需 TIM 通道。

---

## 8. 15 关设计

### 8.1 难度梯度

| 关卡 | 名称 | 尺寸 | 箱子数 | 难度 |
|------|------|------|--------|------|
| 1 | First Push | 8×8 | 2 | ★ 入门: 直线推箱 |
| 2 | Side Step | 9×7 | 2 | ★ 入门: 基础移动 |
| 3 | Corner | 8×8 | 2 | ★☆ 初级: 避开墙角 |
| 4 | Line Up | 9×7 | 3 | ★☆ 初级: 三箱一排 |
| 5 | Split | 9×8 | 3 | ★★ 基础: 分隔区域 |
| 6 | Through | 10×8 | 3 | ★★ 基础: 穿越布局 |
| 7 | Square | 9×9 | 4 | ★★☆ 中级: 方形对称 |
| 8 | Rooms | 10×9 | 4 | ★★☆ 中级: 多房间 |
| 9 | Cross Path | 10×9 | 4 | ★★★ 中级: 十字路径 |
| 10 | ZigZag | 11×9 | 4 | ★★★ 进阶: 四角目标 |
| 11 | Maze | 11×10 | 5 | ★★★ 进阶: 柱子迷宫 |
| 12 | Warehouse I | 11×10 | 5 | ★★★☆ 进阶: 仓库布局 |
| 13 | Labyrinth | 12×10 | 5 | ★★★☆ 高级: 走廊迷宫 |
| 14 | Castle | 12×10 | 5 | ★★★★ 高级: 城堡 |
| 15 | Fortress | 12×10 | 6 | ★★★★★ 终极: 6箱挑战 |

全部 15 关通过 Python 脚本自动验证: T=B 数量匹配、玩家坐标正确、无墙角死锁、玩家可达所有推箱位置。

### 8.2 关卡制作 (Python 工具)

`tools/xsb2c.py` 支持从标准 XSB 格式转换为 C 数组:

```bash
# 单关转换
python tools/xsb2c.py levels/mylevel.xsb 0 "My Level"

# 批量转换
python tools/xsb2c.py --batch levels/
```

XSB 字符映射: `#`=墙, ` `=空地, `.`=目标, `$`=箱, `*`=箱在目标上,
`@`=玩家, `+`=玩家在目标上。

---

## 9. 内存预算

### 9.1 SRAM (总计 ~9KB / 64KB)

| 区域 | 大小 | 说明 |
|------|------|------|
| .data + .bss 系统 | ~2KB | 编译器生成 |
| g_map (MapData) | 804B | 运行时地图 |
| g_undo_stack | 450B | 撤销环形缓冲 |
| g_levels[] extern (Flash常量, 不占SRAM) | 0 | `const` 在 Flash |
| g_best_steps[15] | 60B | 高分缓存 |
| 按键状态变量 | ~50B | 去抖/计时 |
| USART环形缓冲 | 32B | 串口接收 |
| 事件队列 | 8B | 输入事件 |
| 栈 | ~4KB | 函数调用 |
| **合计** | **~8.5KB** | 安全, 大量余量 |

### 9.2 Flash (总计 ~50KB / 256KB)

| 区域 | 大小 | 说明 |
|------|------|------|
| .text (代码) | ~30KB | 游戏+驱动+标准库 |
| .rodata (关卡+字库) | ~15KB | 15关 + font.h |
| 常量数据 | ~5KB | 字符串/数组 |
| **合计** | **~50KB** | 远小于 256KB |

---

## 10. 答辩演示流程

| 时间 | 内容 | 重点 |
|------|------|------|
| 0:00-0:15 | 上电 → 启动画面 → 菜单 | Logo + 标题显示 |
| 0:15-0:30 | 菜单 → 选关界面 | 摇杆导航, 关卡网格 |
| 0:30-1:00 | 选第1关 → 演示移动/推箱/过关 | 基本操作流畅 |
| 1:00-1:30 | 选第7关 → 展示复杂度 + 撤销功能 | 长按撤销 |
| 1:30-2:00 | 过关动画 + 音效展示 | 撞墙/推箱/胜利三音 |
| 2:00-2:30 | 串口PC键盘演示 (加分) | 插USB转串口, 开终端 |
| 2:30-3:00 | PPT翻页: 架构图 + 代码片段 | 1页架构, 1页亮点 |
| 3:00-5:00 | Q&A + 结语 | 回答老师提问 |
