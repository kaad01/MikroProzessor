#include "beeper.h"

//=========================================================================
// Anschlussbelegung
//=========================================================================
//	BEEPER	PB8		TIM10C1 oder ...
//=========================================================================

static int ton_stop = 0;



//=========================================================================
void init_BEEPER(void)
//=========================================================================
{	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType =  GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}



//=========================================================================
void timer_tonfrequenz(unsigned short int frequenz_in_Hz)
//=========================================================================
{
	//=====================================================================
	// Variablen Berechnung aus der Frequenz
	//=====================================================================

	uint16_t PWM_Periode = 42;              // Periodendauer
	uint16_t PSC_Prescaler = 84;            // Prescaler
	uint16_t PWM_Tastverhaeltnis_OC1 = 5;  // Tastverhältnis TIM12C1 in %
	uint16_t PWM_Pulsbreite_OC1 = 0;

	// SystemCoreClock = 168000000 Hz
	// CK_INT = SystemCoreClock / 2 = 84000000 Hz
	// CK_CNT = 1000000 Hz entspricht einer Periodendauer 0.000001s
	// PSC_Prescaler = CK_INT / CK_CNT = 84000000Hz / 1000000Hz = 84
	PSC_Prescaler = 84;

	// PWM_Periode = PWM_Periodendauer / CK_CNT_Periodendauer
	// oder
	// PWM_Periode = CK_CNT / Frequenz
	PWM_Periode = (uint16_t) (2.0f*1000000.0f/frequenz_in_Hz);

	// PWM_Tastverhaeltnis_OC1 (1...100 in %)
	PWM_Pulsbreite_OC1 = (uint16_t) ( PWM_Periode * PWM_Tastverhaeltnis_OC1 / 100);

	//=====================================================================

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_TIM10);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);

	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_Prescaler = PSC_Prescaler -1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = PWM_Periode - 1; //ARR
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM10, &TIM_TimeBaseStructure);

	TIM_OCInitTypeDef  TIM_OCInitStructure;
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = PWM_Pulsbreite_OC1; //CCR
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM10, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM10, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM10, ENABLE);
}



//=========================================================================
void timer_tondauer( unsigned short int time_in_ms)
//=========================================================================
{
	// nur PreemptionPriority aktiv 0-15 ist möglich
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);			// nur PreemptionPriority

	// NVIC konfigurieren
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// 0-höchster Priorität
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 15;		 //	wird nicht Ausgewertet
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Taktsystem für TIM7 Freigeben
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	// Struktur anlegen
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// TIM7 in der Struktur konfigurieren
	TIM_TimeBaseStructure.TIM_Prescaler = 8400 - 1;      	// 100µs = 8400 * 1/84000000Hz
	TIM_TimeBaseStructure.TIM_Period = 10 * time_in_ms - 1;	// dauer in ms
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	// TIM7 Register aus der Struktur Schreiben
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

	// Counter auf 0 setzen
	TIM_SetCounter(TIM7, 0);

	// Interruptflag löschen
    TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

	// Interrupt erlauben
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

	// Timer starten durch Freigabe des Taktsignals
	// kann an anderer Stelle erfolgen
	//TIM_Cmd(TIM7, ENABLE);

	// wenn die Zeit abgelaufen ist wird die ISR aufgerufen
}


//=========================================================================
void beep( 	unsigned short int ton_frequenz,
			unsigned short int ton_dauer,
			unsigned short int ton_pause )
//=========================================================================
{
	timer_tonfrequenz(ton_frequenz);

	timer_tondauer(ton_dauer);

	// Taktsignal für den Timer (Ton) Freigeben
	TIM_Cmd(TIM10, ENABLE);

	// Taktsignal für den Timer (Dauer) Freigeben
	TIM_Cmd(TIM7, ENABLE);

	// warten bis Tondauer erreicht
	ton_stop = 0;
	while ( ton_stop != 1){;}

	if (ton_pause != 0)
	{
		timer_tondauer(ton_pause);
		TIM_Cmd(TIM7, ENABLE);
		ton_stop = 0;
		while ( ton_stop != 1){;}
	}

}



//=========================================================================
void beeper_sirene(void)
//=========================================================================
{	//2400-2850 1sec
	int i,j = 0;
	unsigned short int schritte = 100;
	unsigned short int gesamt_dauer = 10000;
	unsigned short int dauer = gesamt_dauer / schritte;

	unsigned short int start_frequenz = 200;
	unsigned short int end_frequenz = 1000;
	unsigned short int frequenz_schritte = (end_frequenz - start_frequenz) / schritte;

	unsigned short int frequenz = start_frequenz;

	for (j=0; j<10; j++){

		frequenz = start_frequenz;

	for (i=0; i<schritte; i++)
	{
		timer_tonfrequenz(frequenz);
		timer_tondauer(dauer);

		TIM_Cmd(TIM10, ENABLE);
		TIM_Cmd(TIM7, ENABLE);

		frequenz = frequenz + frequenz_schritte;

		ton_stop = 0;
		while ( ton_stop != 1){;}
	}

	}
}



//=========================================================================
void BEEPER_IRQHandler(void)
//=========================================================================
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
    	TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);

    	TIM_Cmd(TIM10, DISABLE);
        TIM_Cmd(TIM7, DISABLE);

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, DISABLE);
       	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, DISABLE);

       	init_BEEPER();

       	ton_stop = 1;
    }
}




