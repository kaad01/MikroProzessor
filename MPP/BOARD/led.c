#include "led.h"


//=========================================================================
// Anschlussbelegung
//=========================================================================
//	LED_GR - PB2
//=========================================================================


//=========================================================================
void init_LED(void)
//=========================================================================
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType =  GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// Schaltet LED aus
	GPIO_ResetBits(GPIOB, GPIO_Pin_2);
}
