# 推箱子(Sokoban) — STM32F107VCT6 嵌入式课程设计

## 一、打开 Keil 工程

在 Keil uVision V5 中打开 `Project/Project.uvprojx`。

**现有工程组 (从LCD实验例程继承, 已配置好):**
- `Libraries/CMSIS` — core_cm3.c, system_stm32f10x.c, startup_stm32f10x_cl.s
- `Libraries/STM32F10x_StdPeriph_Driver` — 所有标准外设库 .c 文件
- `User/Lcd` — lcd.c, NUAA107_32_Driver_IO16.c
- `User/Delay` — delay.c
- `User/Main` — main.c, hw_config.c, stm32f10x_it.c

## 二、需要新添加到 Keil 工程的文件

在 Keil 中右键 Target → **Add Group**, 创建以下组并添加对应文件:

| 组名 | 添加的文件 |
|------|-----------|
| `Game/App` | `User/Game/app/app.c`, `menu.c`, `game.c`, `select.c` |
| `Game/Logic` | `User/Game/logic/map.c`, `move.c`, `undo.c`, `score.c` |
| `Game/Render` | `User/Game/render/tile.c`, `font.c`, `ui.c`, `anim.c` |
| `Game/Drv` | `User/Game/drv/audio.c` |
| `Game/Data` | `User/Game/data/levels.c` |

## 三、需要添加的 Include 路径

Keil → Options for Target → C/C++ → Include Paths, **在现有路径基础上追加**:

```
..\User\Game
..\User\Game\app
..\User\Game\logic
..\User\Game\render
..\User\Game\drv
..\User\Game\data
```

(工程已含 `..\User\Main`, `..\User\Lcd`, `..\User\Delay`, `..\Libraries\...` 等路径, 无需重复添加)

## 四、工程宏定义确认

Options → C/C++ → Define 应包含:

```
USE_STDPERIPH_DRIVER,STM32F10X_CL
```

横屏模式 `USE_HORIZONTAL=1` 已在 `User/Lcd/lcd.h` 第9行直接定义，**不需要在编译器命令行重复定义**。

## 五、编译与烧录

- 连接 J-Link 或 ST-Link (SWD接口)
- Keil → Rebuild all (F7) → 确认 0 Error, 0 Warning
- Keil → Download (F8) → 烧录到 Flash
- 按 RESET 键运行

## 六、运行时操作

### 板载按键
| 按键 | 引脚 | 游戏功能 | 菜单功能 |
|------|------|---------|---------|
| KEY1 | PD11 | 上移 | 上一项 |
| KEY2 | PD12 | 下移 | 下一项 |
| KEY3 | PC13 | 左移 | 确认选择 |
| KEY4 (WAKEUP) | PA0 | 右移 | 返回 |

**长短按:**
- 短按 (< 0.8s): 方向移动
- 长按 (0.8~2s, 任意键): 撤销
- 超长按 (> 2s, 仅 KEY4): 返回菜单

### 串口 PC 键盘 (加分项)
- 串口参数: 115200-8-N-1 (USART1: PA9 TX, PA10 RX)
- 按键映射:
  - `W` / `S` / `A` / `D` → 上/下/左/右移动
  - `U` → 撤销, `R` → 重置, `M` → 返回菜单
  - 空格/回车 → 确认选择

## 七、硬件引脚速查

| 外设 | 引脚 |
|------|------|
| LCD 数据线 | PE0 ~ PE15 |
| LCD 背光 | PB14 |
| LCD 控制 | CS=PC6, RS=PD13, WR=PD14, RD=PD15 |
| 按键 KEY1-4 | PD11, PD12, PC13, PA0 |
| LED1-4 | PD2, PD3, PD4, PD7 |
| LED5 | PB13 |
| 蜂鸣器 | PC0 |
| USART1 | TX=PA9, RX=PA10 |
| 电机 (保留) | PB8, PB9 |

## 八、项目文件清单

```
pushBit/
├── README.md                     # 本文件
├── Project/                      # Keil 工程 (.uvprojx)
├── Libraries/                    # CMSIS + STM32F10x 标准外设库 V3.5
├── tools/xsb2c.py                # XSB→C 关卡转换工具
├── levels/                       # XSB 关卡源文件
├── doc/
│   ├── report_outline.md         # 结课报告大纲
│   └── ppt_outline.md            # 答辩 PPT 大纲
└── User/
    ├── Main/                     # 入口 + 硬件配置 + 中断服务
    ├── Lcd/                      # ILI9341 TFT 驱动 (横屏 320×240)
    ├── Delay/                    # 软件延时
    └── Game/
        ├── config.h              # 全局宏定义
        ├── app/                  # 状态机 (菜单/游戏/选关)
        ├── logic/                # 地图/移动/撤销/计分
        ├── render/               # 瓦片绘制/字体/UI/动画
        ├── drv/                  # 蜂鸣器音频
        └── data/                 # 15 关数据
```
