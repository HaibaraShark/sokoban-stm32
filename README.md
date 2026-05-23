# 推箱子(Sokoban) — STM32F107VCT6 嵌入式课程设计

## 一、打开 Keil 工程

在 Keil uVision V5 中打开 `Project/Project.uvprojx`。

**工程已预配置，打开即可编译，无需手动添加任何文件或路径。**

已包含的工程组:

| 组名 | 文件 |
|------|------|
| `User/Main` | main.c, hw_config.c, stm32f10x_it.c, stm32f10x_adc.c |
| `Libraries/StdPeriph_Driver` | flash, gpio, rcc, usart, misc, exti, bkp, fsmc, tim, pwr |
| `Libraries/CMSIS` | core_cm3.c, system_stm32f10x.c, startup_stm32f10x_cl.s |
| `Lcd` | lcd.c |
| `Delay` | delay.c |
| `Game/App` | app.c, menu.c, game.c, select.c |
| `Game/Logic` | map.c, move.c, undo.c, score.c |
| `Game/Render` | tile.c, font.c, ui.c, anim.c |
| `Game/Drv` | audio.c |
| `Game/Data` | levels.c |
| `Doc` | information.txt |

## 二、工程宏定义确认

Options → C/C++ → Define 已包含:

```
USE_STDPERIPH_DRIVER,STM32F10X_CL
```

横屏模式 `USE_HORIZONTAL=1` 已在 `User/Lcd/lcd.h` 第9行直接定义，**不需要在编译器命令行重复定义**。

Include 路径已预配置，包含 `..\User\Game` 及其全部子目录。

## 三、编译与烧录

**编译器要求: ARMCC V5 (Keil MDK-ARM V5)**

> 注意: 中文字符串使用 GB2312 hex 转义格式(`\xHH\xHH`)，确保 ARMCC V5 兼容。

- 连接 J-Link 或 ST-Link (SWD接口)
- Keil → Rebuild all (F7) → 确认 0 Error, 0 Warning
- Keil → Download (F8) → 烧录到 Flash
- 按 RESET 键运行

## 四、运行时操作

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

## 五、硬件引脚速查

### 实验板: MDV-STM32-F107 (NUAA_CM3_107)

| 外设 | MCU 引脚 | 连接说明 |
|------|---------|---------|
| LCD D0~D15 | PE0 ~ PE15 | 40Pin 排线插接底板 J1 |
| LCD CS | PC6 | 同上 |
| LCD RS | PD13 | 同上 |
| LCD WR | PD14 | 同上 |
| LCD RD | PD15 | 同上 |
| LCD 背光 | PB14 | 同上 |
| KEY1 (上) | PD11 | 底板焊接 |
| KEY2 (下) | PD12 | 底板焊接 |
| KEY3 (左/确认) | PC13 | 底板焊接 |
| KEY4 (右/返回) | PA0 (WAKEUP) | 底板焊接 |
| LED1~4 | PD2, PD3, PD4, PD7 | 底板焊接, 低电平亮 |
| LED5 | PB13 | 底板焊接, 低电平亮 |
| 蜂鸣器 | PC0 | 底板焊接, 软件 PWM |
| USART1 TX | PA9 | 杜邦线 → USB转串口模块 RX |
| USART1 RX | PA10 | 杜邦线 → USB转串口模块 TX |
| 电机 (保留) | PB8, PB9 | 未使用 |
| SWD 调试 | PA13 (SWDIO), PA14 (SWCLK) | 20Pin JTAG 插座 |

### 串口模块接线 (PC 键盘)

```
MCU  PA9  (TX) ──── 串口模块 RX
MCU  PA10 (RX) ──── 串口模块 TX
底板 GND      ──── 串口模块 GND
底板 3.3V     ──── 串口模块 VCC (勿接 5V!)
```

模块 USB 端接 PC，打开串口终端 (115200-8-N-1)，用 W/A/S/D 控制方向。

## 六、项目文件清单

```
pushBit/
├── README.md                     # 本文件
├── Project/                      # Keil 工程 (.uvprojx)
├── Libraries/                    # CMSIS + STM32F10x 标准外设库 V3.5
├── tools/xsb2c.py                # XSB→C 关卡转换工具
├── levels/                       # XSB 关卡源文件
├── Doc/
│   ├── design.md                 # 详细设计文档
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
