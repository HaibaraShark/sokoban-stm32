/*****************************************************
** Descriptions:        ��Ҫ��3.2������SSD1298���ĳ�ʼ�����ã��Լ�����API�����ı�д
**																		 
*****************************************************************/ 

#include "font.h"
#include "stdio.h"
#include "delay.h"
#include "stdlib.h"
#include "NUAA107_32_Driver_IO16.h"

/* Private variable ---------------------------------------------------------*/
uint16_t DeviceCode; //LCD��ID�ű���


/*********************************************************************************
��������void LCD_GPIO_Configuration(void)
���ܣ����ò���ʼ��LCD���ݺͿ��ƶ˿�
�����������
�����������
*********************************************************************************/
void LCD_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*������Ӧʱ�� TFT�õ�PC��PD��PE�˿�*/
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD 
							|RCC_APB2Periph_GPIOE, ENABLE);
							  
	/*����LCD������������Ϊ�������*/
	/*GPIOE*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* LCD_CS */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* LCD_RS  LCD_WR  LCD_RD*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/*********************************************************************************
��������void LCD_WR_REG(uint16_t LCD_Reg)
���ܣ�д��LCD�Ŀ�������
���������Ҫд�������uint16_t LCD_Reg
�����������
**********************************************************************************/
void LCD_WR_REG(uint16_t LCD_Reg)
{
	
	LCD_RD(1);					/*��ʧ��*/
	LCD_CS(0);					/*Ƭѡ*/
	LCD_RS(0);					/*0��ʾ����*/
	DataToWrite(LCD_Reg);
	LCD_WR(0);				
	LCD_WR(1);					/*������д��*/
	LCD_RS(1);					/*1��ʾ����*/	
}
					  
/*********************************************************************************
��������void LCD_WR_DATA(uint16_t LCD_Reg)
���ܣ�д��LCD������
���������Ҫд�������uint16_t LCD_Data
�����������
*********************************************************************************/
void LCD_WR_DATA(uint16_t LCD_Data)	
{	
	LCD_RD(1);					/*��ʧ��*/
	LCD_CS(0);
	LCD_RS(1);					/*1��ʾ����*/
	DataToWrite(LCD_Data);
	LCD_WR(0);
	LCD_WR(1);					/*������д��*/
	LCD_CS(1);
} 


/*********************************************************************************
��������uint16_t LCD_ReadReg(uint16_t LCD_Reg)
���ܣ���ȡLCD������
���������uint16_t LCD_Reg Ҫ��ȡ�ļĴ���
���������temp ��ȡ������
*********************************************************************************/
uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	uint16_t temp;
	
	GPIO_InitTypeDef GPIO_InitStructure;
	LCD_WR_REG(LCD_Reg);

	/* ����Ϊ����ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);	

	GPIO_ResetBits(GPIOC,LCD_CS_PIN); //����Ƭѡ��CS					
	GPIO_SetBits(GPIOD,LCD_RS_PIN); //��ȡ��������
	GPIO_ResetBits(GPIOD,LCD_RD_PIN); //���Ͷ�ȡ�����ţ�׼����ȡ����
	GPIO_SetBits(GPIOD,LCD_RD_PIN); //���߶�ȡ�����ţ���ȡ����
	temp=GPIO_ReadInputData(GPIOE);

	GPIO_SetBits(GPIOC,LCD_CS_PIN); //����Ƭѡ��CS

	/* ����Ϊ���ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	return temp;				    	 	
}

/*********************************************************************************
��������void LCD_WriteReg(uint16_t LCD_Reg ,uint16_t LCD_RegValue)
���ܣ���ָ���Ĵ���д��ָ������
���������uint16_t LCD_Reg       �Ĵ�����ַ��
					uint16_t LCD_RegValue  Ҫд�������
�����������
*********************************************************************************/
void LCD_WriteReg(uint16_t LCD_Reg ,uint16_t LCD_RegValue)
{	
	LCD_RD(1);					/*��ʧ��*/
	LCD_CS(0);					/*Ƭѡ*/
	LCD_RS(0);					/*0��ʾ����*/
	DataToWrite(LCD_Reg);
	LCD_WR(0);				
	LCD_WR(1);					/*������д��*/

	LCD_RS(1);					/*1��ʾ����*/
	DataToWrite(LCD_RegValue);
	LCD_WR(0);
	LCD_WR(1);					/*������д��*/
	LCD_CS(1);	
}

/*********************************************************************************
	*���ƣ�void LCD_WriteRAM_Prepare(void)
	*��������
	*���أ���
	*���ܣ�
	*��ע����ʼдGRAM
*********************************************************************************/
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(0x22);
}

/*********************************************************************************
	*���ƣ�void LCD_WriteRAM(u16 RGB_Code)
	*������u16 RGB_Code��д����ɫ����ֵ
	*���أ���
	*���ܣ�LCDдGRAM
	*��ע��
*********************************************************************************/
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code);    /*дʮ��λGRAM*/
}

/*********************************************************************************
	*���ƣ�void LCD_SetCursor(u16 Xpos, u16 Ypos)
	*������Xpos ������
				 Ypos ������
	*���أ���
	*���ܣ����ù��λ��
*********************************************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	
 	if(DeviceCode==0x8999||DeviceCode==0x9919)
	{
		LCD_WriteReg(0x004E, Xpos);
		LCD_WriteReg(0x004F, Ypos);
	}
	else 
	{
		LCD_WriteReg(0x0020, Xpos);
		LCD_WriteReg(0x0021, Ypos);
	}
}

/*********************************************************************************
	*���ƣ�void_LCD_Clear(uint16_t Colour)
	*������Colour 16λ��ɫ����
	*���أ���
	*���ܣ���ָ����ɫˢ��
*********************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index=0;
	LCD_WindowMax (0, 0, 240, 320);
	LCD_SetCursor(0x00,0x0000);      /*���ù��λ��  */
	LCD_WriteRAM_Prepare();          /*��ʼд��GRAM	 */
				 
	for(index=0;index<76800;index++)
	{
		LCD_WR_DATA(Color);		   /*д������ */ 
	}	
}

/*********************************************************************************
	*���ƣ�void LCD_DrawPoint(void)
	*������xsta X��ʼ���� 0~239
		   ysta	Y��ʼ���� 0~319
	*���أ���
	*���ܣ�POINT_COLORָ�������ɫ

*********************************************************************************/
void LCD_DrawPoint(uint16_t xsta, uint16_t ysta,uint16_t colour)
{
	LCD_WindowMax (xsta, ysta, 240, 320);
	LCD_SetCursor(xsta,ysta);  /*���ù��λ��  */
	LCD_WR_REG(0x0022);           /*��ʼд��GRAM */
	LCD_WR_DATA(colour); 
	/*�ָ�����*/
	LCD_WindowMax (0, 0, 240, 320); 
}

/*********************************************************************************					
	*���ƣ�void LCD_WindowMax()
	*������
	*���أ���
	*���ܣ����ô���
	*��ע��
*********************************************************************************/
void LCD_WindowMax (unsigned int x,unsigned int y,unsigned int x_end,unsigned int y_end) 
{
	
	if(DeviceCode==0x8999)
	{
		LCD_WriteReg(0x44,x|((x_end-1)<<8));
		LCD_WriteReg(0x45,y);
		LCD_WriteReg(0x46,y_end-1);
	}
	else
	{
		LCD_WriteReg(0x50, x);                      /* Horizontal GRAM Start Address      */
		LCD_WriteReg(0x51, x_end-1);               /* Horizontal GRAM End   Address (-1) */
		LCD_WriteReg(0x52, y);                      /* Vertical   GRAM Start Address      */
		LCD_WriteReg(0x53, y_end-1);                /* Vertical   GRAM End   Address (-1) */
	}
}

/*********************************************************************************
	*���ƣ�void LCD_Fill(uint8_t xsta, uint16_t ysta, uint8_t xend, uint16_t yend, uint16_t colour)
	*������xsta ��ʼX����
		   ysta ��ʼY����
		   xend ����X����
		   yend ����Y����
		   color �������ɫ
	*���أ���
	*���ܣ���ָ�������������ָ����ɫ�������С(xend-xsta)*(yend-ysta)
	*��ע������������һ�����ص�	
*********************************************************************************/
void LCD_Fill(uint8_t xsta, uint16_t ysta, uint8_t xend, uint16_t yend, uint16_t colour)
{                    
    u32 n;

	/*���ô���	*/	
	LCD_WindowMax (xsta, ysta, xend, yend); 
	LCD_SetCursor(xsta,ysta);        /*���ù��λ�� */ 
	LCD_WriteRAM_Prepare();         /*��ʼд��GRAM*/	 	   	   
	n=(u32)(yend-ysta+1)*(xend-xsta+1);    
	while(n--){LCD_WR_DATA(colour);} /*��ʾ��������ɫ*/
	 
	/*�ָ�����*/
	LCD_WindowMax (0, 0, 240, 320); 
}  

/*********************************************************************************
	*���ƣ�void LCD_DrawLine(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend)
	*������xsta X��ʼ����
	   	   ysta Y��ʼ����
		   xend X�յ�����
		   yend Y�յ�����	
	*���أ���
	*���ܣ�ָ������(����)������
	*��ע����Ҫ������ɫ����
*********************************************************************************/ 
void LCD_DrawLine(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t colour)
{
    u16 x, y, t;
	if((xsta==xend)&&(ysta==yend))LCD_DrawPoint(xsta, ysta,colour);
	else if(abs(yend-ysta)>abs(xend-xsta))/*б�ʴ���1 */ 
	{
		if(ysta>yend) 
		{
			t=ysta;
			ysta=yend;
			yend=t; 
			t=xsta;
			xsta=xend;
			xend=t; 
		}
		for(y=ysta;y<yend;y++)            /*��y��Ϊ��׼*/ 
		{
			x=(u32)(y-ysta)*(xend-xsta)/(yend-ysta)+xsta;
			LCD_DrawPoint(x, y,colour);  
		}
	}
	else     /*б��С�ڵ���1 */
	{
		if(xsta>xend)
		{
			t=ysta;
			ysta=yend;
			yend=t;
			t=xsta;
			xsta=xend;
			xend=t;
		}   
		for(x=xsta;x<=xend;x++)  /*��x��Ϊ��׼*/ 
		{
			y =(u32)(x-xsta)*(yend-ysta)/(xend-xsta)+ysta;
			LCD_DrawPoint(x,y,colour); 
		}
	} 
} 

/*********************************************************************************
	*���ƣ�void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r)
	*������x0 ���ĵ������
	       y0 ���ĵ�������
		   r  �뾶
	*���أ���
	*���ܣ���ָ��λ�û�һ��ָ����С��Բ
	*��ע��������ɫ�������Ƿ�����
*********************************************************************************/
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r,uint16_t colour)
{
	int a,b;
	int di;
	a=0;b=r;	  									 
	di=3-(r<<1);             /*�ж��¸���λ�õı�־*/
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,colour);             //3           
		LCD_DrawPoint(x0+b,y0-a,colour);             //0           
		LCD_DrawPoint(x0-a,y0+b,colour);             //1       
		LCD_DrawPoint(x0-b,y0-a,colour);             //7           
		LCD_DrawPoint(x0-a,y0-b,colour);             //2             
		LCD_DrawPoint(x0+b,y0+a,colour);             //4               
		LCD_DrawPoint(x0+a,y0-b,colour);             //5
		LCD_DrawPoint(x0+a,y0+b,colour);             //6 
		LCD_DrawPoint(x0-b,y0+a,colour);             
		a++;

		/*ʹ��Bresenham�㷨��Բ*/     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 
		LCD_DrawPoint(x0+a,y0+b,colour);
	}
} 

/*********************************************************************************
	*���ƣ�void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend)
	*������xsta X��ʼ����
	       ysta Y��ʼ����
		   xend X��������
		   yend Y��������
	*���أ���
	*���ܣ���ָ�����򻭾���
	*��ע��

*********************************************************************************/
void LCD_DrawRectangle(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,uint16_t colour)
{
	LCD_DrawLine(xsta,ysta,xend,ysta,colour);
	LCD_DrawLine(xsta,ysta,xsta,yend,colour);
	LCD_DrawLine(xsta,yend,xend,yend,colour);
	LCD_DrawLine(xend,ysta,xend,yend,colour);
} 

/*********************************************************************************
	*���ƣ�void LCD_ShowChar(u8 x, u16 y, u8 num, u8 size, u16 PenColor, u16 BackColor)
	*������x��y      ��ʼ���꣨x:0~234 y:0~308��
		   num       �ַ�ASCII��ֵ
		   size      �ַ���С��ʹ��Ĭ��8*16
		   PenColor  ������ɫ
		   BackColor ���屳����ɫ
	*���ܣ�
	*��ע��ע����Ļ��С
*********************************************************************************/
void LCD_ShowChar(u8 x, u16 y, u8 num, u8 size, u16 PenColor, u16 BackColor)
{       
#define MAX_CHAR_POSX 232
#define MAX_CHAR_POSY 304 
    u8 temp;
    u8 pos,t;
    if(x>MAX_CHAR_POSX||y>MAX_CHAR_POSY)return;		    
	LCD_WindowMax(x,y,x+size/2,y+size);	   /*���ô���	*/										
	LCD_SetCursor(x, y);                  /*���ù��λ�� */
  
	LCD_WriteRAM_Prepare();               /*��ʼд��GRAM  */   
	num=num-' ';                         /*�õ�ƫ�ƺ��ֵ */
	for(pos=0;pos<size;pos++)
	{
		if(size==12)
			temp=asc2_1206[num][pos];/*����1206����*/
		else 
			temp=asc2_1608[num][pos];		 /*����1608����	*/
		for(t=0;t<size/2;t++)
	    {                 
	        if(temp&0x01)			   /*�ӵ�λ��ʼ*/
			{
				LCD_WR_DATA(PenColor);  /*��������ɫ һ����*/
			}
			else 
				LCD_WR_DATA(BackColor);	   /*��������ɫ һ����*/     
	        temp>>=1; 
	    }
	}			
	LCD_WindowMax(0x0000,0x0000,240,320);	/*�ָ������С*/	 
} 

  
/*********************************************************************************
	���ƣ�void LCD_ShowCharString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t PenColor, uint16_t BackColor)
	������x��y      ��ʼ����
	      p         ָ���ַ�����ʼ��ַ
		  PenColor  �ַ���ɫ
		  BackColor ������ɫ
	���ܣ�
	��ע����16���壬���Ե��� �˺������ܵ�������
*********************************************************************************/
void LCD_ShowCharString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t PenColor, uint16_t BackColor)
{   
	uint8_t size = 16;     /*---�ַ���СĬ��16*8---*/
	 
    if(x>MAX_CHAR_POSX){x=0;y+=size;}			         /*����X��������С��λ������*/
    if(y>MAX_CHAR_POSY){y=x=0;LCD_Clear(WHITE);}	 /*����Y��������С��λ���ص�ԭ�㣬��������*/
    LCD_ShowChar(x, y, *p, size, PenColor, BackColor);			   /*0��ʾ�ǵ��ӷ�ʽ*/
}

/*********************************************************************************
	*����: u16 findHzIndex(u8 *hz)
	*������hz
	*���ܣ��������ִ洢���ڴ��ַ
	*��ע��
*********************************************************************************/
u16 findHzIndex(u8 *hz)                            /* ���Զ��庺�ֿ��ڲ�����Ҫ��ʾ */
                                                      /* �ĺ��ֵ�λ�� */
{
	u16 i=0;
	FNT_GB16 *ptGb16 = (FNT_GB16 *)GBHZ_16;		  /*ptGb16ָ��GBHZ_16*/
	while(ptGb16[i].Index[0] > 0x80)
	{
	    if ((*hz == ptGb16[i].Index[0]) && (*(hz+1) == ptGb16[i].Index[1])) /*��������λ����ʾ��ַ��*/
		{
	        return i;
	    }
	    i++;
	    if(i > (sizeof((FNT_GB16 *)GBHZ_16) / sizeof(FNT_GB16) - 1))  /* �����±�Լ�� */
	    {
		    break;
	    }
	}
	return 0;
}

/*********************************************************************************
	*���ƣ�void WriteOneHz(uint16_t x0, uint16_t y0, uint8_t *pucMsk, uint16_t PenColor, uint16_t BackColor)
	*������x0,y0     ��ʼ����
		   *pucMsk   ָ��
		   PenColor	 �ַ���ɫ
		   BackColor ������ɫ
	*���ܣ�
	*��ע���˺������ܵ�����Ϊ�����ַ���ʾ											  
*********************************************************************************/					
void WriteOneHz(u16 x0, u16 y0, u8 *pucMsk, u16 PenColor, u16 BackColor)
{
    u16 i,j;
    u16 mod[16];                                      /* ��ǰ��ģ 16*16 */
    u16 *pusMsk;                                      /* ��ǰ�ֿ��ַ  */
    u16 y;

	u16 size = 16;       /*����Ĭ�ϴ�С16*16*/

    pusMsk = (u16 *)pucMsk;


    for(i=0; i<16; i++)                                    /* ���浱ǰ���ֵ���ʽ��ģ       */
    {
        mod[i] = *pusMsk;                                /* ȡ�õ�ǰ��ģ�����ֶ������   */
        mod[i] = ((mod[i] & 0xff00) >> 8) | ((mod[i] & 0x00ff) << 8);/* ��ģ�����ߵ��ֽ�*/
		pusMsk = pusMsk+1;
    }
    y = y0;
	LCD_WindowMax(x0,y0,x0+size,y0+size);	 	/*���ô���*/
	LCD_SetCursor(x0,y0);                       /*���ù��λ�� */ 
	LCD_WriteRAM_Prepare();                     /*��ʼд��GRAM*/  
    for(i=0; i<16; i++)                                    /* 16��   */
    {                                              
        for(j=0; j<16; j++)                                /* 16��   */
        {
		    if((mod[i] << j) & 0x8000)       /* ��ʾ��i�� ��16���� */
            { 
			    LCD_WriteRAM(PenColor);
            } 
			else 
			{
                LCD_WriteRAM(BackColor);      /* �ö���ʽ����д�հ׵������*/
			}
        }
        y++;
    }
	LCD_WindowMax(0x0000,0x0000,240,320);  	/*�ָ������С*/
}

/*********************************************************************************
	*���ƣ�void LCD_ShowHzString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor)
	*������x0��y0    ��ʼ����
		   pcStr     ָ��
		   PenColor  ������ɫ
		   BackColor ���屳��
	*���ܣ���ʾ�����ַ���
	*��ע������������ܵ�������	       
*********************************************************************************/
void LCD_ShowHzString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor)
{
#define MAX_HZ_POSX 224	 //224+16=240
#define MAX_HZ_POSY 304  //304+16=320
	u16 usIndex;
	u8 size = 16; 
	FNT_GB16 *ptGb16 = 0;    
    ptGb16 = (FNT_GB16 *)GBHZ_16; 

	if(x0>MAX_HZ_POSX){x0=0;y0+=size;}			         /*����X��������С��λ������*/
    if(y0>MAX_HZ_POSY){y0=x0=0;LCD_Clear(WHITE);}	   /*����Y��������С��λ���ص�ԭ�㣬��������*/

	usIndex = findHzIndex(pcStr);
	WriteOneHz(x0, y0, (u8 *)&(ptGb16[usIndex].Msk[0]),  PenColor, BackColor); /* ��ʾ�ַ� */
}

/*********************************************************************************
	*���ƣ�void LCD_ShowString(u16 x0, u16 y0, u8 *pcstr, u16 PenColor, u16 BackColor)
	*������x0 y0     ��ʼ����
		   pcstr     �ַ���ָ��
		   PenColor  ������ɫ
		   BackColor ���屳��ɫ
	*���ܣ������ַ��ͺ�����ʾ������ʵ���ַ�����ʾ
	*��ע��	
*********************************************************************************/
void LCD_ShowString(u16 x0, u16 y0, u8 *pcStr, u16 PenColor, u16 BackColor)
{
	while(*pcStr!='\0')
	{
	 	if(*pcStr>0x80) /*��ʾ����*/
		{
			LCD_ShowHzString(x0, y0, pcStr, PenColor, BackColor);
			pcStr += 2;
			x0 += 16;	
		}
		else           /*��ʾ�ַ�*/
		{
			LCD_ShowCharString(x0, y0, pcStr, PenColor, BackColor);	
			pcStr +=1;
			x0+= 8;
		}
	
	}	

}

/****************************************************************************
* ��    �ƣ�u16 ili9320_BGRtoRGB(u16 Color)
* ��    �ܣ�RRRRRGGGGGGBBBBB ��Ϊ BBBBBGGGGGGRRRRR ��ʽ
* ��ڲ�����Color      BRG ��ɫֵ
* ���ڲ�����RGB ��ɫֵ
* ˵    �����ڲ���������
* ���÷�����
****************************************************************************/
u16 LCD_RGBtoBGR(u16 Color)
{						   
  u16  r, g, b, bgr;

  b = (Color>>0)  & 0x1f;	/* ��ȡB    */
  g = (Color>>5)  & 0x3f;	/* �м���λ */
  r = (Color>>11) & 0x1f;	/* ��ȡR    */
  
  bgr =  (b<<11) + (g<<5) + (r<<0);

  return( bgr );
}

/*********************************************************************************
* ��    �ƣ�void LCD_DrawPicture(u16 StartX,u16 StartY,u16 EndX,u16 EndY,u16 *pic)
* ��    �ܣ���ָ�����귶Χ��ʾһ��ͼƬ
* ��ڲ�����StartX     ����ʼ����
*           StartY     ����ʼ����
*           EndX       �н�������
*           EndY       �н�������
            pic        ͼƬͷָ��
* ���ڲ�������
* ˵    ����ͼƬȡģ��ʽΪˮƽɨ�裬16λ��ɫģʽ
* ���÷�����LCD_DrawPicture(0,0,100,100,(u16*)demo);
*********************************************************************************/
void LCD_DrawPicture(u16 StartX,u16 StartY,u16 Xend,u16 Yend,u8 *pic)
{
	static	u16 i=0,j=0;
	u16 *bitmap = (u16 *)pic;
	/*����ͼƬ��ʾ���ڴ�С*/
	LCD_WindowMax(StartX, StartY, Xend, Yend);	
	LCD_SetCursor(StartX,StartY);
	LCD_WriteRAM_Prepare();
	for(j=0; j<Yend-StartY; j++)
	{
		for(i=0; i<Xend-StartX; i++) LCD_WriteRAM(*bitmap++); 	
	}
	/*�ָ�����*/
	LCD_WindowMax(0, 0, 240, 320);
}



/*********************************************************************************
	*���ƣ�void LCD_Init(void)
	*���룺��
	*���أ���
	*���ܣ�LCD��ʼ��
	*˵����
*********************************************************************************/
void LCD_Init(void)
{
	LCD_Configuration();

	Delay_10ms(10); /* delay 50 ms */ 
	Delay_10ms(10); /* delay 50 ms */
	DeviceCode = LCD_ReadReg(0x0000); /*��ȡ����ID��*/
	Delay_10ms(10); /* delay 50 ms */
	if(DeviceCode==0x8999)	   /*��Ӧ������ICΪSSD1298*/
	{
		/*-----   Start Initial Sequence ------*/
		LCD_WriteReg(0x00, 0x0001); /*�����ڲ�����*/
		LCD_WriteReg(0x01, 0x3B3F); /*����������� */
		LCD_WriteReg(0x02, 0x0600); /* set 1 line inversion	*/
		/*-------- Power control setup --------*/
		LCD_WriteReg(0x0C, 0x0007); /* Adjust VCIX2 output voltage */
		LCD_WriteReg(0x0D, 0x0006); /* Set amplitude magnification of VLCD63 */
		LCD_WriteReg(0x0E, 0x3200); /* Set alternating amplitude of VCOM */
		LCD_WriteReg(0x1E, 0x00BB); /* Set VcomH voltage */
		LCD_WriteReg(0x03, 0x6A64); /* Step-up factor/cycle setting  */
		/*-------- RAM position control --------*/
		LCD_WriteReg(0x0F, 0x0000); /* Gate scan position start at G0 */
		LCD_WriteReg(0x44, 0xEF00); /* Horizontal RAM address position */
		LCD_WriteReg(0x45, 0x0000); /* Vertical RAM address start position*/
		LCD_WriteReg(0x46, 0x013F); /* Vertical RAM address end position */
		/* ------ Adjust the Gamma Curve -------*/
		LCD_WriteReg(0x30, 0x0000);
		LCD_WriteReg(0x31, 0x0706);
		LCD_WriteReg(0x32, 0x0206);
		LCD_WriteReg(0x33, 0x0300);
		LCD_WriteReg(0x34, 0x0002);
		LCD_WriteReg(0x35, 0x0000);
		LCD_WriteReg(0x36, 0x0707);
		LCD_WriteReg(0x37, 0x0200);
		LCD_WriteReg(0x3A, 0x0908);
		LCD_WriteReg(0x3B, 0x0F0D);
		/*--------- Special command -----------*/
		LCD_WriteReg(0x28, 0x0006); /* Enable test command	*/
		LCD_WriteReg(0x2F, 0x12EB); /* RAM speed tuning	 */
		LCD_WriteReg(0x26, 0x7000); /* Internal Bandgap strength */
		LCD_WriteReg(0x20, 0xB0E3); /* Internal Vcom strength */
		LCD_WriteReg(0x27, 0x0044); /* Internal Vcomh/VcomL timing */
		LCD_WriteReg(0x2E, 0x7E45); /* VCOM charge sharing time */ 
		/*--------- Turn On display ------------*/
		LCD_WriteReg(0x10, 0x0000); /* Sleep mode off */
		Delay_10ms(3);              /* Wait 30mS  */
		LCD_WriteReg(0x11, 0x6870); /* Entry mode setup. 262K type B, take care on the data bus with 16it only */
		LCD_WriteReg(0x07, 0x0033); /* Display ON	*/
	}
else if(DeviceCode==0x9325||DeviceCode==0x9328)
	{
				LCD_WriteReg(0x00e7,0x0010);      
        LCD_WriteReg(0x0000,0x0001);  			//start internal osc
        LCD_WriteReg(0x0001,0x0100);     
        LCD_WriteReg(0x0002,0x0700); 				//power on sequence 
		//���� 
	  LCD_WriteReg(0x0003,(1<<12)|(1<<7)|(1<<5)|(1<<4)|(0<<3) ); 	//65K 
		
        LCD_WriteReg(0x0004,0x0000);                                   
        LCD_WriteReg(0x0008,0x0207);	           
        LCD_WriteReg(0x0009,0x0000);         
        LCD_WriteReg(0x000a,0x0000); 				//display setting         
        LCD_WriteReg(0x000c,0x0001);				//display setting          
        LCD_WriteReg(0x000d,0x0000); 				//0f3c          
        LCD_WriteReg(0x000f,0x0000);
//Power On sequence //
        LCD_WriteReg(0x0010,0x0000);   
        LCD_WriteReg(0x0011,0x0007);
        LCD_WriteReg(0x0012,0x0000);                                                                 
        LCD_WriteReg(0x0013,0x0000);                 
//        for(i=50000;i>0;i--);
//		for(i=50000;i>0;i--);
        LCD_WriteReg(0x0010,0x1590);   
        LCD_WriteReg(0x0011,0x0227);
//        for(i=50000;i>0;i--);
//		for(i=50000;i>0;i--);
        LCD_WriteReg(0x0012,0x009c);                  
//        for(i=50000;i>0;i--);
//		for(i=50000;i>0;i--);
        LCD_WriteReg(0x0013,0x1900);   
        LCD_WriteReg(0x0029,0x0023);
        LCD_WriteReg(0x002b,0x000e);
//        for(i=50000;i>0;i--);
//		for(i=50000;i>0;i--);
        LCD_WriteReg(0x0020,0x0000);                                                            
        LCD_WriteReg(0x0021,0x0000);           
///////////////////////////////////////////////////////      
//        for(i=50000;i>0;i--);
//		for(i=50000;i>0;i--);
				LCD_WriteReg(0x0030,0x0007); 
				LCD_WriteReg(0x0031,0x0707);   
        LCD_WriteReg(0x0032,0x0006);
        LCD_WriteReg(0x0035,0x0704);
        LCD_WriteReg(0x0036,0x1f04); 
        LCD_WriteReg(0x0037,0x0004);
        LCD_WriteReg(0x0038,0x0000);        
        LCD_WriteReg(0x0039,0x0706);     
        LCD_WriteReg(0x003c,0x0701);
        LCD_WriteReg(0x003d,0x000f);
//        for(i=50000;i>0;i--);
//		for(i=50000;i>0;i--);

        LCD_WriteReg(0x0050,0x0000);        
        LCD_WriteReg(0x0051,0x00ef);   
        LCD_WriteReg(0x0052,0x0000);     
        LCD_WriteReg(0x0053,0x013f);

        LCD_WriteReg(0x0060,0xa700);        
        LCD_WriteReg(0x0061,0x0001); 
        LCD_WriteReg(0x006a,0x0000);
        LCD_WriteReg(0x0080,0x0000);
        LCD_WriteReg(0x0081,0x0000);
        LCD_WriteReg(0x0082,0x0000);
        LCD_WriteReg(0x0083,0x0000);
        LCD_WriteReg(0x0084,0x0000);
        LCD_WriteReg(0x0085,0x0000);
      
        LCD_WriteReg(0x0090,0x0010);     
        LCD_WriteReg(0x0092,0x0000);  
        LCD_WriteReg(0x0093,0x0003);
        LCD_WriteReg(0x0095,0x0110);
        LCD_WriteReg(0x0097,0x0000);        
        LCD_WriteReg(0x0098,0x0000);  
         //display on sequence     
        LCD_WriteReg(0x0007,0x0133);
    
        LCD_WriteReg(0x0020,0x0000);                                                            
        LCD_WriteReg(0x0021,0x0000);
	}		
	Delay_10ms(5);	     /*��ʱ50ms*/
	LCD_Clear(BLACK);
}


