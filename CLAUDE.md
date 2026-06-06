# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

推箱子(Sokoban)嵌入式游戏 — STM32F107VCT6 (Cortex-M3), ILI9341 3.2" TFT 320×240 横屏。
IDE: Keil MDK-ARM V5, 编译器: ARMCC V5 (C89), 标准外设库 V3.5。

## 编译与烧录

- 打开 `Project/Project.uvprojx` → Rebuild (F7) → Download (F8)
- 工程宏定义: `USE_STDPERIPH_DRIVER,STM32F10X_CL`
- 横屏宏 `USE_HORIZONTAL=1` 已在 `User/Lcd/lcd.h` 第9行定义，不在编译器命令行重复
- 中文字符串使用 GB2312 hex 转义 (`\xHH\xHH`)，不要直接写中文字符

## 关卡工具

```bash
python tools/xsb2c.py levels/my_level.xsb 0 "Level Name"   # 单关: XSB → C 数组
python tools/xsb2c.py --batch levels/                       # 批量转换
python verify_levels.py                                     # 验证15关数据一致性/死锁/可达性
```

## 架构

**核心**: main.c 驱动 FSM 主循环，`Input_Poll()` 产生事件（按键扫描 10ms/SysTick + USART1 串口 115200），`g_state` 分发到各状态处理器。
主循环每帧无条件调用 `Audio_Update()` / `Motor_Update()` / `LED_Update()`，反馈均为非阻塞。

```
main.c (主循环 + 状态机分发 + 非阻塞反馈驱动)
├── App层:  menu.c / game.c / select.c  — 各状态页面逻辑
├── Logic层: map.c (双层地图) / move.c (碰撞+推箱+胜利) / undo.c (环形栈50步) / score.c (BKP高分+设置存取)
├── Render层: tile.c (程序化绘制6种瓦片,零位图) / font.c / ui.c / anim.c
├── Drv层:   audio.c (PC0蜂鸣器, DWT非阻塞音频状态机)
└── HAL:     lcd.c (ILI9341 FSMC 8080并口) / hw_config.c (GPIO/USART/NVIC/电机振动状态机) / delay.c (DWT)
```

**状态转换**: `SPLASH → MENU ↔ GAME ↔ SELECT → SETTINGS → HELP → MENU`

**关键数据**:
- `MapData`: 双层分离存储 (`ground[]` 地面 + `entities[]` 实体), 运行时一份, Flash常量数组一份
- `MoveRecord`: 9字节记录玩家/箱子位移, 用于脏矩形重绘(最多4格)和撤销
- `config.h` 中的 `TEST_MODE`: 注释掉=正常逐关解锁, 定义=全解锁任意跳关
- BKP 备份寄存器: DR1=magic 0x50C0, DR2~DR16=每关最佳步数, DR17=设置(音效/振动开关, 0xA5魔数)

**游戏内渲染**: 增量脏矩形重绘，非全屏刷新。移动后仅重绘变化的 3~4 个瓦片。
菜单/选关页面也使用增量绘制，只更新光标变化的两个按钮/卡片，避免闪烁。

## 非阻塞反馈系统

所有反馈（音频/振动/LED）均为非阻塞——`_Play/_Pulse/_Start` 设置状态立即返回，`_Update` 函数在主循环中每帧驱动：

| 系统 | 驱动方式 | 状态文件 |
|------|---------|---------|
| 音频 | DWT 周期计数器 (72MHz, µs 精度相位计算) | audio.c |
| 振动 | SysTick (1ms 粒度) | hw_config.c |
| LED | SysTick (1ms 粒度) | game.c |

音频在 `g_settings.sound_enabled=0` 时静默跳过，振动同理。设置存储于 BKP_DR17。

## UI 设计

**配色体系**: 质感风(瓦片) + 现代扁平风(菜单/信息栏)。
- 信息栏底色 `COLOR_CHARCOAL` (0x1082), 卡片色 `COLOR_CARD` (0x18E3)
- 瓦片墙体暖灰石材 + 砖缝纹理 + 3D 高光/阴影边缘
- 箱子立体高光/阴影, 玩家圆头+梯形身体+脚
- 按钮选中态: 左边黄色竖条 + 双线黄框
- 选关: 已通关金卡, 已解锁亮卡, 未解锁暗卡+锁图标

新增 Settings 页面 (Sound/Vibration 开关), 菜单 4 项。

## 新增文件

- `docs/superpowers/specs/2026-06-06-ui-redesign-design.md` — UI 翻新设计文档
