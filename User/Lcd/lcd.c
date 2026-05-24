#include "stdio.h"
#include "hw_config.h"
#include "font.h"
#include "string.h"
//#include "delay.h"
/*****************************************************/
#include "pic.h"

#include "lcd.h"
#include "stdlib.h"
//#include "usart.h"
#include "delay.h"	 
/***********************************************************************************************
//////////////////////////////////////////////////////////////////////////////////	 
//����Ӳ����ARM Cortex-M3:STM32F107VCT6,NUAA_CS_107ʵ�鿪����,25MHz���壬��Ƶ72MHZ
//9341-TFTҺ������ for STM32 IOģ��
//�Ա���վ��http://qdtech.taobao.com
//=======================================Һ���������߽���==========================================//
	STM32F107VCT6 PE0~PE15���Һ����DB0~DB15,��������ΪDB0��PB0,..DB15��PE15.   
//=======================================Һ���������߽���==========================================//
	LCD_CS	��PC6	//Ƭѡ�ź�
	LCD_RS	��PD13	//�Ĵ���/����ѡ���ź�
	LCD_WR	��PD14	//д�ź�
	LCD_RD	��PD15	//���ź�
	LCD_RST	��PC5	//��λ�źţ�����
	LCD_LED	��PB14	//��������ź�(�ߵ�ƽ����)
//=========================================������������=========================================//
//��ʹ�ô�������ģ�鱾��������������ɲ�����
	MO(MISO)	��PC11	//SPI�������
	MI(MOSI)	��PC12	//SPI��������
	PEN		��PC5	//�������ж��ź�
	TCS		��PC8	//����ICƬѡ
	CLK		��PC10	//SPI����ʱ��
**************************************************************************************************/	

	   
//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;

//������ɫ,������ɫ
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 


//******************************************************************
//��������  LCD_WR_REG
//���ܣ�    ��Һ��������д��д16λָ��
//���������Reg:��д���ָ��ֵ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_WR_REG(u16 data)
{ 
	LCD_RS_CLR;
	LCD_CS_CLR;
	DATAOUT(data);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}

//******************************************************************
//��������  LCD_WR_DATA
//���ܣ�    ��Һ��������д��д16λ����
//���������Data:��д�������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_WR_DATA(u16 data)
{
	LCD_RS_SET;
	LCD_CS_CLR;
	DATAOUT(data);
	LCD_WR_CLR;
	LCD_WR_SET;
	LCD_CS_SET;
}
//******************************************************************
//��������  LCD_DrawPoint_16Bit
//���ܣ�    8λ���������д��һ��16λ����
//���������(x,y):�������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_DrawPoint_16Bit(u16 color)
{
#if LCD_USE8BIT_MODEL==1
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;//д��ַ  	 
	DATAOUT(color);	
	LCD_WR_CLR;
	LCD_WR_SET;	
	DATAOUT(color<<8);	
	LCD_WR_CLR;
	LCD_WR_SET;	 
	LCD_CS_SET;
#else
	LCD_WR_DATA(color); 
#endif


}

//******************************************************************
//��������  LCD_WriteReg
//���ܣ�    д�Ĵ�������
//���������LCD_Reg:�Ĵ�����ַ
//			LCD_RegValue:Ҫд�������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
	 
//******************************************************************
//��������  LCD_WriteRAM_Prepare
//���ܣ�    ��ʼдGRAM
//			�ڸ�Һ��������RGB����ǰ��Ӧ�÷���дGRAMָ��
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}	 

//******************************************************************
//��������  LCD_DrawPoint
//���ܣ�    ��ָ��λ��д��һ�����ص�����
//���������(x,y):�������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);//���ù��λ�� 
#if LCD_USE8BIT_MODEL==1
	LCD_CS_CLR;
	LCD_RD_SET;
	LCD_RS_SET;//д��ַ  	 
	DATAOUT(POINT_COLOR);	
	LCD_WR_CLR;
	LCD_WR_SET;	
	DATAOUT(POINT_COLOR<<8);	
	LCD_WR_CLR;
	LCD_WR_SET;	 
	LCD_CS_SET;
#else
	LCD_WR_DATA(POINT_COLOR); 
#endif


}

//******************************************************************
//��������  LCD_Clear
//���ܣ�    LCDȫ�������������
//���������Color:Ҫ���������ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_Clear(u16 Color)
{
	u32 index=0;      
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);	
#if LCD_USE8BIT_MODEL==1
	LCD_RS_SET;//д���� 
	LCD_CS_CLR;	   
	for(index=0;index<lcddev.width*lcddev.height;index++)
	{
		DATAOUT(Color);	
		LCD_WR_CLR;
		LCD_WR_SET;	
		
		DATAOUT(Color<<8);	
		LCD_WR_CLR;
		LCD_WR_SET;	   
	}
	LCD_CS_SET;	
#else //16λģʽ
	for(index=0;index<lcddev.width*lcddev.height;index++)
	{
		LCD_WR_DATA(Color);		  
	}
#endif
	
} 

//******************************************************************
//��������  LCD_GPIOInit
//���ܣ�    Һ����IO��ʼ����Һ����ʼ��ǰҪ���ô˺���
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//**************9341****************************************************
void LCD_GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE); 
//	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;	   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	GPIO_SetBits(GPIOC,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	   
	GPIO_Init(GPIOD, &GPIO_InitStructure); 
	GPIO_SetBits(GPIOD,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;	   
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	GPIO_SetBits(GPIOB,GPIO_Pin_14); //��������

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;	//  
	GPIO_Init(GPIOE, &GPIO_InitStructure); //GPIOE
	GPIO_SetBits(GPIOE,GPIO_Pin_All);	 

}


 	 
//******************************************************************
//��������  LCD_Init
//���ܣ�    LCD��ʼ��
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_Init(void)
{  
	LCD_GPIOInit();
	/************* Start Initial Sequence **********/		
	LCD_WR_REG(0xCF);  
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x81); //C1
	LCD_WR_DATA(0X30); 
	LCD_WR_REG(0xED);  
	LCD_WR_DATA(0x64); 
	LCD_WR_DATA(0x03); 
	LCD_WR_DATA(0X12); 
	LCD_WR_DATA(0X81); 
	LCD_WR_REG(0xE8);  
	LCD_WR_DATA(0x85); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x78); //7A
	LCD_WR_REG(0xCB);  
	LCD_WR_DATA(0x39); 
	LCD_WR_DATA(0x2C); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x34); 
	LCD_WR_DATA(0x02); 
	LCD_WR_REG(0xF7);  
	LCD_WR_DATA(0x20); 
	LCD_WR_REG(0xEA);  
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0xC0);    //Power control 
	LCD_WR_DATA(0x1B);   //VRH[5:0] 
	LCD_WR_REG(0xC1);    //Power control 
	LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0] 
	LCD_WR_REG(0xC5);    //VCM control 
	LCD_WR_DATA(0x30); 	 //3F
	LCD_WR_DATA(0x30); 	 //3C
	LCD_WR_REG(0xC7);    //VCM control2 
	LCD_WR_DATA(0XB7); 
	LCD_WR_REG(0x36);    // Memory Access Control 
	LCD_WR_DATA(0x48); 
	LCD_WR_REG(0x3A);   
	LCD_WR_DATA(0x55); 
	LCD_WR_REG(0xB1);   
	LCD_WR_DATA(0x00);   
	LCD_WR_DATA(0x1A); 
	LCD_WR_REG(0xB6);    // Display Function Control 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0xA2); 
	LCD_WR_REG(0xF2);    // 3Gamma Function Disable 
	LCD_WR_DATA(0x00); 
	LCD_WR_REG(0x26);    //Gamma curve selected 
	LCD_WR_DATA(0x01); 
	LCD_WR_REG(0xE0);    //Set Gamma 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x2A); 
	LCD_WR_DATA(0x28); 
	LCD_WR_DATA(0x08); 
	LCD_WR_DATA(0x0E); 
	LCD_WR_DATA(0x08); 
	LCD_WR_DATA(0x54); 
	LCD_WR_DATA(0XA9); 
	LCD_WR_DATA(0x43); 
	LCD_WR_DATA(0x0A); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x00); 		 
	LCD_WR_REG(0XE1);    //Set Gamma 
	LCD_WR_DATA(0x00); 
	LCD_WR_DATA(0x15); 
	LCD_WR_DATA(0x17); 
	LCD_WR_DATA(0x07); 
	LCD_WR_DATA(0x11); 
	LCD_WR_DATA(0x06); 
	LCD_WR_DATA(0x2B); 
	LCD_WR_DATA(0x56); 
	LCD_WR_DATA(0x3C); 
	LCD_WR_DATA(0x05); 
	LCD_WR_DATA(0x10); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_DATA(0x3F); 
	LCD_WR_DATA(0x3F); 
	LCD_WR_DATA(0x0F); 
	LCD_WR_REG(0x2B); 
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x01);
	LCD_WR_DATA(0x3f);
	LCD_WR_REG(0x2A); 
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xef);	 
	LCD_WR_REG(0x11); //Exit Sleep
	Delay_ms(120);
	LCD_WR_REG(0x29); //display on	
	LCD_SetParam();//����LCD����	 
	LCD_LED=1;//��������	 
	//LCD_Clear(WHITE);
}
  		  
/*************************************************
��������LCD_SetWindows
���ܣ�����lcd��ʾ���ڣ��ڴ�����д�������Զ�����
��ڲ�����xy�����յ�
����ֵ����
*************************************************/
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{	
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar);		
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar);		
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd);	

	LCD_WriteRAM_Prepare();	//��ʼд��GRAM				
}   

/*************************************************
��������LCD_SetCursor
���ܣ����ù��λ��
��ڲ�����xy����
����ֵ����
*************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(Xpos>>8);
	LCD_WR_DATA(0x00FF&Xpos);		

	
	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(Ypos>>8);
	LCD_WR_DATA(0x00FF&Ypos);		

	LCD_WriteRAM_Prepare();	//��ʼд��GRAM	
} 

//����LCD����
//������к�����ģʽ�л�
void LCD_SetParam(void)
{ 	
	lcddev.wramcmd=0x2C;
//#if USE_HORIZONTAL==1	//ʹ�ú���	
if (USE_HORIZONTAL)
{  
	lcddev.dir=1;//����
	lcddev.width=320;
	lcddev.height=240;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;			
	LCD_WriteReg(0x36,0x6C);
}
//#else//����
else 
{
	lcddev.dir=0;//����				 	 		
	lcddev.width=240;
	lcddev.height=320;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;	
	LCD_WriteReg(0x36,0xC9);
}
//#endif
}	/*����9341��*/

//**************************************************************************************************/	
//******************************************************************
//��������  GUI_DrawPoint
//���ܣ�    GUI���һ����
//���������x:���λ��x����
//        	y:���λ��y����
//			color:Ҫ������ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void GUI_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_SetCursor(x,y);//���ù��λ�� 
	LCD_DrawPoint_16Bit(color); 
}

//******************************************************************
//��������  LCD_Fill
//���ܣ�    ��ָ�������������ɫ
//���������sx:ָ������ʼ��x����
//        	sy:ָ������ʼ��y����
//			ex:ָ�����������x����
//			ey:ָ�����������y����
//        	color:Ҫ������ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{  	

	u16 i,j;			
	u16 width=ex-sx+1; 		//�õ����Ŀ���
	u16 height=ey-sy+1;		//�߶�
	LCD_SetWindows(sx,sy,ex,ey);//������ʾ����

#if LCD_USE8BIT_MODEL==1
	LCD_RS_SET;//д���� 
	LCD_CS_CLR;	   
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		{
			DATAOUT(color);	
			LCD_WR_CLR;
			LCD_WR_SET;	
			
			DATAOUT(color<<8);	
			LCD_WR_CLR;
			LCD_WR_SET;	
		}   
	}
	LCD_CS_SET;	
#else //16λģʽ
	for(i=0;i<height;i++)
	{
		for(j=0;j<width;j++)
		LCD_WR_DATA(color);	//д������ 	 
	}
#endif
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ���������Ϊȫ��
}

//******************************************************************
//��������  LCD_DrawLine
//���ܣ�    GUI����
//���������x1,y1:�������
//        	x2,y2:�յ����� 
//����ֵ��  ��
//�޸ļ�¼����
//****************************************************************** 
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		LCD_DrawPoint(uRow,uCol);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
} 

//******************************************************************
//��������  LCD_DrawRectangle
//���ܣ�    GUI������(�����)
//���������(x1,y1),(x2,y2):���εĶԽ�����
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_DrawLine(x1,y1,x2,y1);
	LCD_DrawLine(x1,y1,x1,y2);
	LCD_DrawLine(x1,y2,x2,y2);
	LCD_DrawLine(x2,y1,x2,y2);
}  

//******************************************************************
//��������  LCD_DrawFillRectangle
//���ܣ�    GUI������(���)
//���������(x1,y1),(x2,y2):���εĶԽ�����
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************   
void LCD_DrawFillRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
	LCD_Fill(x1,y1,x2,y2,POINT_COLOR);

}
 
//******************************************************************
//��������  _draw_circle_8
//���ܣ�    8�Գ��Ի�Բ�㷨(�ڲ�����)
//���������(xc,yc) :Բ��������
// 			(x,y):��������Բ�ĵ�����
//         	c:������ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
void _draw_circle_8(int xc, int yc, int x, int y, u16 c)
{
	GUI_DrawPoint(xc + x, yc + y, c);

	GUI_DrawPoint(xc - x, yc + y, c);

	GUI_DrawPoint(xc + x, yc - y, c);

	GUI_DrawPoint(xc - x, yc - y, c);

	GUI_DrawPoint(xc + y, yc + x, c);

	GUI_DrawPoint(xc - y, yc + x, c);

	GUI_DrawPoint(xc + y, yc - x, c);

	GUI_DrawPoint(xc - y, yc - x, c);
}

//******************************************************************
//��������  gui_circle
//���ܣ�    ��ָ��λ�û�һ��ָ����С��Բ(���)
//���������(xc,yc) :Բ��������
//         	c:������ɫ
//		 	r:Բ�뾶
//		 	fill:����жϱ�־��1-��䣬0-�����
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
void gui_circle(int xc, int yc,u16 c,int r, int fill)
{
	int x = 0, y = r, yi, d;

	d = 3 - 2 * r;


	if (fill) 
	{
		// �����䣨��ʵ��Բ��
		while (x <= y) {
			for (yi = x; yi <= y; yi++)
				_draw_circle_8(xc, yc, x, yi, c);

			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	} else 
	{
		// �������䣨������Բ��
		while (x <= y) {
			_draw_circle_8(xc, yc, x, y, c);
			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
}

void Draw_Circle(u16 x0, u16 y0, u16 fc, u8 r)
{
	gui_circle(x0, y0, fc, r, 0);
}

//******************************************************************
//��������  LCD_ShowChar
//���ܣ�    ��ʾ����Ӣ���ַ�
//���������(x,y):�ַ���ʾλ����ʼ����
//        	fc:ǰ�û�����ɫ
//			bc:������ɫ
//			num:��ֵ��0-94��
//			size:�����С��
//			mode:ģʽ  0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
void LCD_ShowChar(u16 x,u16 y,u16 fc, u16 bc, u8 num,u8 size,u8 mode)
{  
    u8 temp;
    u8 pos,t;
	u16 colortemp=POINT_COLOR;      
		   
	num=num-' ';//�õ�ƫ�ƺ��ֵ
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);//���õ���������ʾ����
	if(!mode) //�ǵ��ӷ�ʽ
	{
		
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//����1206����
			else temp=asc2_1608[num][pos];		 //����1608����
			for(t=0;t<size/2;t++)
		    {                 
		        if(temp&0x01)LCD_DrawPoint_16Bit(fc); 
				else LCD_DrawPoint_16Bit(bc); 
				temp>>=1; 
				
		    }
			
		}	
	}else//���ӷ�ʽ
	{
		for(pos=0;pos<size;pos++)
		{
			if(size==12)temp=asc2_1206[num][pos];//����1206����
			else temp=asc2_1608[num][pos];		 //����1608����
			for(t=0;t<size/2;t++)
		    {   
				POINT_COLOR=fc;              
		        if(temp&0x01)LCD_DrawPoint(x+t,y+pos);//��һ����    
		        temp>>=1; 
		    }
		}
	}
	POINT_COLOR=colortemp;	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��    	   	 	  
} 
//******************************************************************
//��������  LCD_ShowString
//���ܣ�    ��ʾӢ���ַ���
//���������x,y :�������	 
//			size:�����С
//			*p:�ַ�����ʼ��ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  	  
void LCD_ShowString(u16 x,u16 y,u8 size,u8 *p,u8 mode)
{         
    while((*p<='~')&&(*p>=' '))//�ж��ǲ��ǷǷ��ַ�!
    {   
		if(x>(lcddev.width-1)||y>(lcddev.height-1)) 
		return;     
        LCD_ShowChar(x,y,POINT_COLOR,BACK_COLOR,*p,size,mode);
        x+=size/2;
        p++;
    }  
} 

//******************************************************************
//��������  mypow
//���ܣ�    ��m��n�η�(gui�ڲ�����)
//���������m:����
//	        n:��
//����ֵ��  m��n�η�
//�޸ļ�¼����
//******************************************************************  
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

//******************************************************************
//��������  LCD_ShowNum
//���ܣ�    ��ʾ�������ֱ���ֵ
//���������x,y :�������	 
//			len :ָ����ʾ���ֵ�λ��
//			size:�����С(12,16)
//			color:��ɫ
//			num:��ֵ(0~4294967295)
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  			 
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,' ',size,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+(size/2)*t,y,POINT_COLOR,BACK_COLOR,temp+'0',size,0); 
	}
} 

//******************************************************************
//��������  GUI_DrawFont16
//���ܣ�    ��ʾ����16X16��������
//���������x,y :�������
//			fc:ǰ�û�����ɫ
//			bc:������ɫ	 
//			s:�ַ�����ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void GUI_DrawFont16(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//�Զ�ͳ�ƺ�����Ŀ
	
			
	for (k=0;k<HZnum;k++) 
	{
	  if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
	  { 	LCD_SetWindows(x,y,x+16-1,y+16-1);
		    for(i=0;i<16*2;i++)
		    {
				for(j=0;j<8;j++)
		    	{	
					if(!mode) //�ǵ��ӷ�ʽ
					{
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
						else LCD_DrawPoint_16Bit(bc);
					}
					else
					{
						POINT_COLOR=fc;
						if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
						x++;
						if((x-x0)==16)
						{
							x=x0;
							y++;
							break;
						}
					}

				}
				
			}
			
			
		}				  	
		continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
	}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��  
} 

//******************************************************************
//��������  GUI_DrawFont24
//���ܣ�    ��ʾ����24X24��������
//���������x,y :�������
//			fc:ǰ�û�����ɫ
//			bc:������ɫ	 
//			s:�ַ�����ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void GUI_DrawFont24(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//�Զ�ͳ�ƺ�����Ŀ
		
			for (k=0;k<HZnum;k++) 
			{
			  if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+24-1,y+24-1);
				    for(i=0;i<24*3;i++)
				    {
							for(j=0;j<8;j++)
							{
								if(!mode) //�ǵ��ӷ�ʽ
								{
									if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
									else LCD_DrawPoint_16Bit(bc);
								}
							else
							{
								POINT_COLOR=fc;
								if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
								x++;
								if((x-x0)==24)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
			}

	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��  
}

//******************************************************************
//��������  GUI_DrawFont32
//���ܣ�    ��ʾ����32X32��������
//���������x,y :�������
//			fc:ǰ�û�����ɫ
//			bc:������ɫ	 
//			s:�ַ�����ַ
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//�޸ļ�¼����
//****************************************************************** 
void GUI_DrawFont32(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//�Զ�ͳ�ƺ�����Ŀ
	for (k=0;k<HZnum;k++) 
			{
			  if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+32-1,y+32-1);
				    for(i=0;i<32*4;i++)
				    {
						for(j=0;j<8;j++)
				    	{
							if(!mode) //�ǵ��ӷ�ʽ
							{
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
								else LCD_DrawPoint_16Bit(bc);
							}
							else
							{
								POINT_COLOR=fc;
								if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
								x++;
								if((x-x0)==32)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
			}
	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��  
} 
void GUI_DrawFont40(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u8 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont40)/sizeof(typFNT_GB40);	//�Զ�ͳ�ƺ�����Ŀ
	for (k=0;k<HZnum;k++) 
			{
			  if ((tfont40[k].Index[0]==*(s))&&(tfont40[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+40-1,y+40-1);
				    for(i=0;i<40*5;i++)
				    {
						for(j=0;j<8;j++)
				    	{
							if(!mode) //�ǵ��ӷ�ʽ
							{
								if(tfont40[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
								else LCD_DrawPoint_16Bit(bc);
							}
							else
							{
								POINT_COLOR=fc;
								if(tfont40[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
								x++;
								if((x-x0)==40)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
			}
	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��  
} 
void GUI_DrawFont64(u16 x, u16 y, u16 fc, u16 bc, u8 *s,u8 mode)
{
	u16 i,j;
	u16 k;
	u16 HZnum;
	u16 x0=x;
	HZnum=sizeof(tfont64)/sizeof(typFNT_GB64);	//�Զ�ͳ�ƺ�����Ŀ
	for (k=0;k<HZnum;k++) 
			{
			  if ((tfont64[k].Index[0]==*(s))&&(tfont64[k].Index[1]==*(s+1)))
			  { 	LCD_SetWindows(x,y,x+64-1,y+64-1);
				    for(i=0;i<64*8;i++)
				    {
						for(j=0;j<8;j++)
				    	{
							if(!mode) //�ǵ��ӷ�ʽ
							{
								if(tfont64[k].Msk[i]&(0x80>>j))	LCD_DrawPoint_16Bit(fc);
								else LCD_DrawPoint_16Bit(bc);
							}
							else
							{
								POINT_COLOR=fc;
								if(tfont64[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//��һ����
								x++;
								if((x-x0)==64)
								{
									x=x0;
									y++;
									break;
								}
							}
						}
					}
					
					
				}				  	
				continue;  //���ҵ���Ӧ�����ֿ������˳�����ֹ��������ظ�ȡģ����Ӱ��
			}
	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ�����Ϊȫ��  
} 


//******************************************************************
//��������  Show_Str
//���ܣ�    ��ʾһ���ַ���,������Ӣ����ʾ
//���������x,y :�������
// 			fc:ǰ�û�����ɫ
//			bc:������ɫ
//			str :�ַ���	 
//			size:�����С,��Ϊ16*16��24*24��32*32,64*64���ִ�С
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************    	   		   
void Show_Str(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode)
{					
	u16 x0=x;							  	  
  	u8 bHz=0;     //�ַ��������� 
    while(*str!=0)//����δ����
    { 
        if(!bHz)
        {
			if(x>(lcddev.width-size/2)||y>(lcddev.height-size)) 
			return; 
	        if(*str>0x80)bHz=1;//���� 
	        else              //�ַ�
	        {          
		        if(*str==0x0D)//���з���
		        {         
		            y+=size;
					x=x0;
		            str++; 
		        }  
		        else
				{
					if(size>16)//�ֿ���û�м���12X24 16X32��Ӣ������,��8X16����
					{  
					LCD_ShowChar(x,y,fc,bc,*str,16,mode);
					x+=8; //�ַ�,Ϊȫ�ֵ�һ�� 
					}
					else
					{
					LCD_ShowChar(x,y,fc,bc,*str,size,mode);
					x+=size/2; //�ַ�,Ϊȫ�ֵ�һ�� 
					}
				} 
				str++; 
		        
	        }
        }else//���� 
        {   
			if(x>(lcddev.width-size)||y>(lcddev.height-size)) 
			return;  
            bHz=0;//�к��ֿ�    
			if(size==64)
			GUI_DrawFont64(x,y,fc,bc,str,mode);	 	
			else if(size==40)
			GUI_DrawFont40(x,y,fc,bc,str,mode);	 	
			else if(size==32)
			GUI_DrawFont32(x,y,fc,bc,str,mode);	 	
			else if(size==24)
			GUI_DrawFont24(x,y,fc,bc,str,mode);	
			else
			GUI_DrawFont16(x,y,fc,bc,str,mode);
				
	        str+=2; 
	        x+=size;//��һ������ƫ��	    
        }						 
    }   
}

//******************************************************************
//��������  Gui_StrCenter
//���ܣ�    ������ʾһ���ַ���,������Ӣ����ʾ
//���������x,y :�������
// 			fc:ǰ�û�����ɫ
//			bc:������ɫ
//			str :�ַ���	 
//			size:�����С
//			mode:ģʽ	0,���ģʽ;1,����ģʽ
//						ֻ�������ģʽ�£�������ɫ�ŷ�������			            
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************   
void Gui_StrCenter(u16 x, u16 y, u16 fc, u16 bc, u8 *str,u8 size,u8 mode)
{
	u16 len=strlen((const char *)str);
	u16 x1=(lcddev.width-len*8)/2;
	Show_Str(x+x1,y,fc,bc,str,size,mode);
} 
 
//******************************************************************
//��������  Gui_Drawbmp16
//���ܣ�    ��ʾһ��16λBMPͼ��
//���������x,y :�������
// 			*p :ͼ��������ʼ��ַ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************  
#define xL 40
#define yL 40

void Gui_Drawbmp16(u16 x,u16 y,const unsigned char *p) //��ʾxL*yL ͼƬ
{
  	int i; 
	unsigned char picH,picL; 
	LCD_SetWindows(x,y,x+xL-1,y+yL-1);//��������
    for(i=0;i<xL*yL;i++)
	{	
	 	picL=*(p+i*2);	//���ݵ�λ��ǰ
		picH=*(p+i*2+1);				
		LCD_DrawPoint_16Bit(picH<<8|picL);  						
	}	
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);//�ָ���ʾ����Ϊȫ��	

}
/**************************************************************************************************/	
//////////////////////////////////////////////////////////////////////////////////
//========================variable==========================//
u16 ColorTab[5]={BRED,YELLOW,RED,GREEN,BLUE};//������ɫ����
//=====================end of variable======================//

//******************************************************************
//��������  DrawTestPage
//���ܣ�    ���Ʋ��Խ���
//���������str :�ַ���ָ��
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
void DrawTestPage(u8 *str)
{
//���ƹ̶���up
LCD_Fill(0,0,lcddev.width,20,BLUE);
//���ƹ̶���down
LCD_Fill(0,lcddev.height-20,lcddev.width,lcddev.height,BLUE);
POINT_COLOR=WHITE;
Gui_StrCenter(0,2,WHITE,BLUE,str,16,1);//������ʾ
Gui_StrCenter(0,lcddev.height-18,WHITE,BLUE,"�Ϻ� NUAA CS ��Ȩ����",16,1);//������ʾ
//���Ʋ�������
LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLACK);
}

//******************************************************************
//��������  main_test
//���ܣ�    ���Ʋ��Գ���������
//�����������
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
void main_test(void)
{
	DrawTestPage("3.2' TFT LCD ���Գ���");
	
	Gui_StrCenter(0,40,BRED,YELLOW,"�Ϻ� NUAA_CM3_107 KIT",16,1);//������ʾ
	//Gui_StrCenter(0,60,RED,BLUE,"Ƕ��ʽϵͳ����ʵ�鿪����",16,1);//������ʾ
	if (USE_HORIZONTAL==1)	Show_Str(45,65,GREEN,BLACK,"Ƕ��ʽϵͳʵ�鿪����",24,1);
	else					Show_Str(10,65,GREEN,BLACK,"Ƕ��ʽϵͳʵ�鿪����",24,1);
//Gui_StrCenter(0,46,RED,BLUE,"Ƕ��ʽϵͳԭ����Ӧ������ʵ���",16,1);//������ʾ
	Gui_StrCenter(0,100,RED,BLUE,"TFT���Գ���",16,1);//������ʾ	
	Gui_StrCenter(0,120,YELLOW,BLUE,"3.2' ILI9341 240X320",16,1);//������ʾ
}

//******************************************************************
//��������  Test_Color
//���ܣ�    ��ɫ�����ԣ���������ɫ����ɫ����ɫ����ɫ����ɫ
//�����������
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
void Test_Color(void)  
{
	DrawTestPage("����1:��ɫ������");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"White",16,1);Delay_ms(500);
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLACK);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"Black",16,1);Delay_ms(500);
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,RED);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"Red",16,1); Delay_ms(500);
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,GREEN);
	Show_Str(lcddev.width-50,30,BLUE,YELLOW,"Green",16,1);Delay_ms(500);
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,BLUE);
	Show_Str(lcddev.width-50,30,WHITE,YELLOW,"Blue",16,1);Delay_ms(500);

}

//******************************************************************
//��������  Test_FillRec
//���ܣ�    ���ο���ʾ�������ԣ�������ʾ�ۺ�ɫ����ɫ����ɫ����ɫ����ɫ���ο�
//       	��ʱ1500��������ΰ��շۺ�ɫ����ɫ����ɫ����ɫ����ɫ�����ο� 
//�����������
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
void Test_FillRec(void)
{
	u8 i=0;
	DrawTestPage("����2:GUI����������");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++) 
	{
		LCD_DrawRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60); 
		POINT_COLOR=ColorTab[i];
	}
	Delay_ms(1000);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	{
		LCD_DrawFillRectangle(lcddev.width/2-80+(i*15),lcddev.height/2-80+(i*15),lcddev.width/2-80+(i*15)+60,lcddev.height/2-80+(i*15)+60); 
		POINT_COLOR=ColorTab[i];
	}
}

//******************************************************************
//��������  Test_FillRec
//���ܣ�    Բ�ο���ʾ�������ԣ�������ʾ�ۺ�ɫ����ɫ����ɫ����ɫ����ɫԲ�ο�
//       	��ʱ1500��������ΰ��շۺ�ɫ����ɫ����ɫ����ɫ����ɫ���Բ�ο� 
//�����������
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
void Test_Circle(void)
{
	u8 i=0;
	DrawTestPage("����3:GUI��Բ������");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	for (i=0; i<5; i++)  
		gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,0);
	Delay_ms(1000);	
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE); 
	for (i=0; i<5; i++) 
	  	gui_circle(lcddev.width/2-80+(i*25),lcddev.height/2-50+(i*25),ColorTab[i],30,1);
}

//******************************************************************
//��������  English_Font_test
//���ܣ�    Ӣ����ʾ���� 
//�����������
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
void English_Font_test(void)
{
	DrawTestPage("����4:Ӣ����ʾ����");
	POINT_COLOR=RED;
	BACK_COLOR=BLUE;
	LCD_ShowString(10,30,12,"6X12:abcdefghijklmnopqrstuvwxyz0123456789",1);
	POINT_COLOR=WHITE;
	LCD_ShowString(10,45,12,"6X12:ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",1);
	POINT_COLOR=BLUE;
	LCD_ShowString(10,60,12,"6X12:~!@#$%^&*()_+{}:<>?/|-+.",1);
	POINT_COLOR=YELLOW;
	LCD_ShowString(10,80,16,"8X16:abcdefghijklmnopqrstuvwxyz0123456789",1);
	POINT_COLOR=GREEN;
	LCD_ShowString(10,100,16,"8X16:ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",1);
	POINT_COLOR=RED;
	LCD_ShowString(10,120,16,"8X16:~!@#$%^&*()_+{}:<>?/|-+.",1); 
}

//******************************************************************
//��������  Chinese_Font_test
//���ܣ�    ������ʾ����
//�����������
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
#if 0
void Chinese_Font_test(void)
{
	DrawTestPage("����5:������ʾ����");
	Show_Str(10,30,YELLOW,BLACK,"162310313:ʯ����",16,0);
	Show_Str(10,50,RED,BLACK,"16X16:NUAA_CM3_107 Kit.",16,0);
	Show_Str(10,70,GREEN,BLACK,"24X24:Ƕ��ʽϵͳʵ�鿪����",24,0);
	Show_Str(10,100,WHITE,BLACK,"32X32:�������",32,1);
	//Show_Str(10,150,MAGENTA,BLACK,"40X40:Ƕ��ʽϵͳ",40,0);
	Show_Str(10,130,MAGENTA,BLACK,"64X64:",16,0);
	Show_Str(0,150,MAGENTA,BLACK,"Ƕ��ʽϵͳ",64,0);
	Show_Str(30,100,RED,YELLOW,"162310313ʯ����",16,0);
}
#endif

//******************************************************************
//��������  Pic_test
//���ܣ�    ͼƬ��ʾ���ԣ�������ʾ����40X40 QQͼ��
//�����������
//����ֵ��  ��
//�޸ļ�¼��
//******************************************************************
#define picx0  20
#define picy0  50


void Pic_test(void)
{
	DrawTestPage("����6:ͼƬ��ʾ����");
	LCD_Fill(0,20,lcddev.width,lcddev.height-20,WHITE);
	Gui_Drawbmp16(picx0,picy0,gImage_qq);
	Show_Str(picx0+12,picy0+45,BLUE,YELLOW,"QQ1",16,1);
	Gui_Drawbmp16(picx0+60,picy0,gImage_qq);
	Show_Str(picx0+60+12,picy0+45,BLUE,YELLOW,"QQ2",16,1);
	Gui_Drawbmp16(picx0+60*2,picy0,gImage_qq);
	Show_Str(picx0+60*2+12,picy0+45,BLUE,YELLOW,"QQ3",16,1);
	Gui_Drawbmp16(picx0+60*3,picy0,gImage_qq);
	Show_Str(picx0+60*3+12,picy0+45,BLUE,YELLOW,"QQ4",16,1);
	Gui_Drawbmp16(picx0+60*4,picy0,gImage_qq);
	Show_Str(picx0+60*4+12,picy0+45,BLUE,YELLOW,"QQ5",16,1);
	
	Gui_Drawbmp16(picx0,picy0+70,gImage_qq);
	Show_Str(picx0+12,picy0+70+45,BLUE,YELLOW,"QQ6",16,1);
	Gui_Drawbmp16(picx0+60,picy0+70,gImage_qq);
	Show_Str(picx0+60+12,picy0+70+45,BLUE,YELLOW,"QQ7",16,1);
	Gui_Drawbmp16(picx0+60*2,picy0+70,gImage_qq);
	Show_Str(picx0+60*2+12,picy0+70+45,BLUE,YELLOW,"QQ8",16,1);
	Gui_Drawbmp16(picx0+60*3,picy0+70,gImage_qq);
	Show_Str(picx0+60*3+12,picy0+70+45,BLUE,YELLOW,"QQ9",16,1);
	Gui_Drawbmp16(picx0+60*4,picy0+70,gImage_qq);
	Show_Str(picx0+60*4+12,picy0+70+45,BLUE,YELLOW,"QQ10",16,1);
	
}







