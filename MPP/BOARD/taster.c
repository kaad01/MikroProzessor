#include "taster.h"


//=========================================================================
// Anschlussbelegung
//=========================================================================
//	Taster1 (On/Off) - PC8 	liefert 1 wenn nicht gedrückt
//	Taster2 (Boot)	 - PC5	liefert 0 wenn nicht gedrückt
//=========================================================================

//=========================================================================
void init_TASTER1(void)
//=========================================================================
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

	GPIO_Init(GPIOC, &GPIO_InitStructure);
}



//=========================================================================
void init_Taster1_IRQ8(void)
//=========================================================================
{

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);

	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;

	EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}



//=========================================================================
void TASTER1_IRQ(void)
//=========================================================================
{

	usart2_send("==>Taster 1 gedrückt\r\n");
	beep(1000,200,0);


	//	while(!(GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_8)))
	// {
	// ;
	// }
}


//=========================================================================
void init_TASTER2(void)
//=========================================================================
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

	GPIO_Init(GPIOC, &GPIO_InitStructure);

}



//=========================================================================
void init_Taster2_IRQ5(void)
//=========================================================================
{

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5);

	EXTI_InitTypeDef EXTI_InitStructure;

	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;

	EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

}



//=========================================================================
void TASTER2_IRQ(void)
//=========================================================================
{
	usart2_send("==>Taster 2 gedrückt\r\n");

	//	beep(4000,200,200);
//	beep(4000,200,200);
//	beep(4000,200,200);
//	set_autostart_in(0,0,0,30);
//	while (GPIO_ReadInputData(GPIOC) & 0x0020){;}
}


