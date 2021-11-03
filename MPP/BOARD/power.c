#include "power.h"

//=========================================================================
// Anschlussbelegung LTC2950 - ON/OFF-Logik
//=========================================================================
//	/KILL	PB0		0-Reset 	1-normal
//=========================================================================


//=========================================================================
void init_POWER_ON(void)
//=========================================================================
{
	// Initialisierung der /KILL-Leitung des LTC2959
	// PB0 Leitung als OpenDrain mit Zustand 1

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType =  GPIO_OType_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_0);
}


//=========================================================================
void set_POWER_OFF(void)
//=========================================================================
{
	// /KILL-Leitung auf 0 ziehen um Spannungsregler auszuschalten

	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}


//=========================================================================
void read_VBAT_RTC(void)
//=========================================================================
{
	char buf[40];
	uint16_t messwert=0;
	static float i = 0.0;
	unsigned long us = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , ENABLE);

	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Resolution =	ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_StructInit(&ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_Vbat, 1, ADC_SampleTime_3Cycles);

	ADC_Cmd(ADC1, ENABLE);
	ADC_VBATCmd(ENABLE);

	us = 500;
	us *= 21;
	while(us--) { __NOP(); }

	ADC_SoftwareStartConv(ADC1);
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)==RESET);

	messwert=ADC_GetConversionValue(ADC1);

	i = 2.0f * (float) messwert * (3.3f / 4096.0f) ;

	sprintf(buf,"=> VBAT = %6.3f V\r\n",i);
	usart2_send(buf);

	ADC_Cmd(ADC1, DISABLE);
	ADC_VBATCmd(DISABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 , DISABLE);
}

