#include "delay.h"

void Delay_Init(void)
{
    /* SysTick 在 main() 中初始化, 此处无需操作 */
}

/*********************************************
*�������ƣ�void Delay_ms(void)
*
*��ڲ�����N  ms
*
*���ڲ�������
*
*����˵��������ʱN ms
**********************************************/
void Delay_ms(uint16_t N)
{
	uint32_t i;
	for (i=0;i<(8000*N);i++);
}



/*---LCD��ʱ���� 10MS*nCount-----*/
void Delay_10ms(uint32_t nCount)
{
	volatile int i;	 	
	for (i=0;i<nCount*100;i++);
}
