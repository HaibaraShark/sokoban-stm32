#ifndef __107_32_Driver_IO16_H_
#define __107_32_Driver_IO16_H_
#include "hw_config.h"

//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08

#define PAout(n)	*((volatile unsigned long *)(0x42000000+((GPIOA_ODR_Addr-0x40000000)<<5)+(n<<2)))
#define PBout(n)	*((volatile unsigned long *)(0x42000000+((GPIOB_ODR_Addr-0x40000000)<<5)+(n<<2)))
#define PCout(n)	*((volatile unsigned long *)(0x42000000+((GPIOC_ODR_Addr-0x40000000)<<5)+(n<<2)))
#define PDout(n)	*((volatile unsigned long *)(0x42000000+((GPIOD_ODR_Addr-0x40000000)<<5)+(n<<2)))
#define PEout(n)	*((volatile unsigned long *)(0x42000000+((GPIOE_ODR_Addr-0x40000000)<<5)+(n<<2))) 

#define LCD_CS(n)			PCout(6)=n	
#define LCD_RS(n)			PDout(13)=n
#define LCD_WR(n)			PDout(14)=n
#define LCD_RD(n)			PDout(15)=n

#define DataToWrite(DATA)	GPIO_Write(GPIOE, DATA)  
#define DataToRead()	    GPIO_ReadInputData(GPIOE) 

#define LCD_CS_PIN			GPIO_Pin_6	
#define LCD_RS_PIN			GPIO_Pin_13
#define LCD_WR_PIN			GPIO_Pin_14
#define LCD_RD_PIN			GPIO_Pin_15

#define RCC_LCD_DATA_GPIO		RCC_APB2Periph_GPIOE
#define RCC_LCD_CONTROL_GPIO	RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD
#define LCD_DATA_GPIO			GPIOE  
 

/*Private define-------------------------*/
#define POINT_COLOR RED
#define BACK_COLOR	0xFFFF
/*--------16位颜色值---------------*/
#define WHITE        0xFFFF
#define BLACK        0x0000	  
#define BLUE         0x001F  
#define RED          0xF800
#define MAGENTA      0xF81F
#define GREEN        0x07E0
#define CYAN         0x7FFF
#define YELLOW       0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define LGRAY 			 0XC618 //浅灰色

/*----------函数声明------------*/
/* 刷屏 */
void LCD_Clear(uint16_t Color);
void LCD_DrawPoint(uint16_t xsta, uint16_t ysta,uint16_t colour);
void LCD_Fill(uint8_t xsta, uint16_t ysta, uint8_t xend, uint16_t yend, uint16_t colour);
void LCD_DrawLine(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t colour);
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r,uint16_t colour);
void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t colour);

void LCD_WindowMax (unsigned int x,unsigned int y,unsigned int x_end,unsigned int y_end);
/*显示字符 包括中英文*/
void LCD_ShowString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor);
/*显示图片 gImage取模 bmp格式*/
void LCD_DrawPicture(u16 StartX,u16 StartY,u16 Xend,u16 Yend,u8 *pic);


void LCD_Configuration(void);
void LCD_Init(void);


#endif
