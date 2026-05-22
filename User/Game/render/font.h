#ifndef __GAME_FONT_H
#define __GAME_FONT_H

#include "../config.h"

/* 简化文字绘制 (复用 lcd.h 的 Show_Str) */
void Font_DrawString(uint16_t x, uint16_t y, const char *str,
                     uint16_t fg, uint16_t bg, uint8_t size);
void Font_DrawNum(uint16_t x, uint16_t y, uint32_t num,
                  uint8_t len, uint8_t size, uint16_t fg, uint16_t bg);
/* 居中文字 */
void Font_DrawCenter(uint16_t x, uint16_t y, uint16_t w,
                     const char *str, uint16_t fg, uint16_t bg, uint8_t size);

#endif
