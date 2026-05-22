#include "font.h"
#include "lcd.h"
#include <string.h>

void Font_DrawString(uint16_t x, uint16_t y, const char *str,
                     uint16_t fg, uint16_t bg, uint8_t size)
{
    Show_Str(x, y, fg, bg, (uint8_t *)str, size, 0);
}

void Font_DrawNum(uint16_t x, uint16_t y, uint32_t num,
                  uint8_t len, uint8_t size, uint16_t fg, uint16_t bg)
{
    LCD_ShowNum(x, y, num, len, size);
    (void)fg; (void)bg;
}

void Font_DrawCenter(uint16_t x, uint16_t y, uint16_t w,
                     const char *str, uint16_t fg, uint16_t bg, uint8_t size)
{
    uint16_t sw = strlen(str) * size / 2;  /* 半宽估算 */
    uint16_t cx = x + (w - sw) / 2;
    Show_Str(cx, y, fg, bg, (uint8_t *)str, size, 0);
}
