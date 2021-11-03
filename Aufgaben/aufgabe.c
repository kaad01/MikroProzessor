#include "aufgabe.h"
#include "stm32f4xx_gpio.h"



void aufgabe_A01_1_1(void)
{
	int i = 0;
	char out[20] = {0};

	while(1)
	{
		i++;
		sprintf(out,"i=%d\r\n",i);
		usart2_send(out);
		if ( i>9){ i=0;}
		wait_mSek(500);
	}
}

// Aufgabe A01-01.3
void init_leds(void)
{
	// Reset des GPIO Ports
	GPIO_DeInit(GPIOC);

	// Struct anlegen
	GPIO_InitTypeDef GPIO_InitStructure;

	// Taktquelle für Peripherie
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	// Struct Initialisierung --> Input ohne PushPull
	GPIO_StructInit(&GPIO_InitStructure);

	// Funktionalität für PIN_2 (Grüne LED) festlegen:
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;

	// Speed of Rise and Fall Time (Slew Rate)
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

	// PIN set as output
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;

	// Spannung bleibt???
	GPIO_InitStructure.GPIO_PuPd  =  GPIO_PuPd_UP;

	//
	GPIO_InitStructure.GPIO_OType =  GPIO_OType_PP;

	// Initialisierung der Portleitung
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// Schaltet LED aus
	GR_LED_OFF;
}

// A01-01.4
void LED_Blinken(void)
{
	GR_LED_ON;
	int x = 100000000;
	while(x--){
		delay_ms(2000000);
		//wait_uSek(20000000000);
		GR_LED_TOGGLE;
	}
}

//A01-02.2
void init_taste_1(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;

	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;       //GPIO Input Mode
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;   //Medium speed
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //PushPull
	GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_NOPULL;       //PullUp

	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void init_taste_2(void){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;

	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN;       //GPIO Input Mode
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;   //Slow speed
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //PushPull
	GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_NOPULL;       //PullUp

	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void taster_druck(void){
	int i = 0;
	while(1){
		if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)) {
			if(i==1)
			{
				GR_LED_ON;
				i= 0;
				wait_mSek(500);
			}
			else {
				i++;
				wait_mSek(500);
			}
		}
		if (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)) {
			GR_LED_OFF;
		}
	}
}
