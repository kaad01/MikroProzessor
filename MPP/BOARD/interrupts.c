#include "interrupts.h"

void hard_fault_handler_c(unsigned int * hardfault_args);

//=========================================================================
void NMI_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
void HardFault_Handler(void)
{
	asm ("TST LR, #4");
	asm ("ITE EQ");
	asm ("MRSEQ R0, MSP");
	asm ("MRSNE R0, PSP");
	asm ("B hard_fault_handler_c");
}

//=========================================================================
void MemManage_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
void BusFault_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
void UsageFault_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
void SVC_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
void DebugMon_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
//void PendSV_Handler(void){
//}

//=========================================================================
void SysTick_Handler(void)
{
	static unsigned long stc_led = 0;
	static unsigned long stc0 = 0;
	static unsigned long stc1 = 0;
	static unsigned long stc2 = 0;
	stc_led++;
	stc0++;
	stc1++;
	stc2++;

	//======================================================================
	// DW1000 Timeout
	systickcounter += 1;

	if ( stc0 >= 20 )
		{
			uwbranging_tick();
			stc0 = 0;
		}



	//======================================================================
	//	CoOS_SysTick_Handler alle 10ms in CoOs arch.c aufrufen
	// nur Einkommentieren wenn CoOS genutzt wird
	CoOS_SysTick_Handler();



	//======================================================================
	// CC3100 alle 50ms Sockets aktualisieren
	if (stc2 >= 5)
		{
			stc2 = 0;
			if ( (IS_CONNECTED(WiFi_Status)) && (IS_IP_ACQUIRED(WiFi_Status)) && (!Stop_CC3100_select) && (!mqtt_run) )
			{
			CC3100_select(); // nur aktiv wenn mit AP verbunden
			}
			else
			{
			_SlNonOsMainLoopTask();
			}
		}

	//======================================================================
	// SD-Card
	sd_card_SysTick_Handler();

	//======================================================================
	// MQTT
	MQTT_SysTickHandler();

	//======================================================================
	// LED zyklisch schalten
	if ( stc_led >= 500 )
		{
			LED_GR_TOGGLE;
			stc_led = 0;
		}
}


//=========================================================================
void WWDG_IRQHandler(void)
{

	unsigned char value_watchdog_counter = 0x7F;
    WWDG_ClearFlag();
    WWDG_DeInit();
	WWDG_SetCounter(value_watchdog_counter);
    WWDG_ClearFlag();
    usart2_send("WWDG_IRQn\r\n");
}


//=========================================================================
void EXTI0_IRQHandler(void)
{
	//===== CC3100
	if(EXTI_GetITStatus(EXTI_Line0) == SET)
		{
			EXTI_ClearFlag(EXTI_Line0);
			EXTI_ClearITPendingBit(EXTI_Line0);
			WLAN_intHandler();	// ISR fuer CC3100 Interrupt
		}
}


//=========================================================================
void EXTI1_IRQHandler(void)
{
	//==== DW1000
	EXTI->PR = EXTI_Line1;		// Setze den Interrupt zurück
	dw1000_irqactive = 1;		// IRQ aktiv merken
	dw1000_handleInterrupt();	// ISR fuer DW1000 Interrupt
	dw1000_irqactive = 0;		// IRQ nicht mehr aktiv merken
}


//=========================================================================
void EXTI2_IRQHandler(void)
{
	//===== nicht belegt
	if(EXTI_GetITStatus(EXTI_Line2) == SET)
		{
			EXTI_ClearFlag(EXTI_Line2);
			EXTI_ClearITPendingBit(EXTI_Line2);
			// nicht belegt
		}
}


//=========================================================================
void EXTI3_IRQHandler(void)
{
	//===== nicht belegt
	if(EXTI_GetITStatus(EXTI_Line3) == SET)
		{
			EXTI_ClearFlag(EXTI_Line3);
			EXTI_ClearITPendingBit(EXTI_Line3);
			// nicht belegt
		}
}


//=========================================================================
void EXTI4_IRQHandler(void)
{	//===== MPU-9250
	if(EXTI_GetITStatus(EXTI_Line4) == SET)
		{
			EXTI_ClearFlag(EXTI_Line4);
			EXTI_ClearITPendingBit(EXTI_Line4);
			// ISR fuer MPU-9250 nichts hinterlegt
		}
}


//=========================================================================
void EXTI9_5_IRQHandler(void)
{
	//===== Taster2
	if (EXTI_GetITStatus(EXTI_Line5) == SET)
		{
			EXTI_ClearFlag(EXTI_Line5);
			EXTI_ClearITPendingBit(EXTI_Line5);
			TASTER2_IRQ();	// ISR fuer Taste 2
		}
	//===== nicht belegt
	if (EXTI_GetITStatus(EXTI_Line6) == SET)
		{
			EXTI_ClearFlag(EXTI_Line6);
			EXTI_ClearITPendingBit(EXTI_Line6);
			// nicht belegt
		}
	//===== nicht belegt
	if (EXTI_GetITStatus(EXTI_Line7) == SET)
		{
			EXTI_ClearFlag(EXTI_Line7);
			EXTI_ClearITPendingBit(EXTI_Line7);
			// nicht belegt
		}
	//===== Taster 1
	if (EXTI_GetITStatus(EXTI_Line8) == SET)
		{
			EXTI_ClearFlag(EXTI_Line8);
			EXTI_ClearITPendingBit(EXTI_Line8);
			TASTER1_IRQ();	// ISR fuer Taste 1
		}
	//===== nicht belegt
	if (EXTI_GetITStatus(EXTI_Line9) == SET)
		{
			EXTI_ClearFlag(EXTI_Line9);
			EXTI_ClearITPendingBit(EXTI_Line9);
			// nicht belegt
		}
}


//=========================================================================
void EXTI15_10_IRQHandler(void)
{
	//===== nicht belegt
	if(EXTI_GetITStatus(EXTI_Line10) == SET)
		{
			EXTI_ClearFlag(EXTI_Line10);
			EXTI_ClearITPendingBit(EXTI_Line10);
			// nicht belegt
		}
	//===== nicht belegt
	if(EXTI_GetITStatus(EXTI_Line11) == SET)
		{
			EXTI_ClearFlag(EXTI_Line11);
			EXTI_ClearITPendingBit(EXTI_Line11);
			// nicht belegt
		}
	//===== nicht belegt
	if (EXTI_GetITStatus(EXTI_Line12) == SET)
		{
			EXTI_ClearFlag(EXTI_Line12);
			EXTI_ClearITPendingBit(EXTI_Line12);
			// nicht belegt
		}
	//===== nicht belegt
	if(EXTI_GetITStatus(EXTI_Line13) == SET)
		{
			EXTI_ClearFlag(EXTI_Line13);
			EXTI_ClearITPendingBit(EXTI_Line13);
			// nicht belegt
		}
	//===== nicht belegt
	if(EXTI_GetITStatus(EXTI_Line14) == SET)
		{
			EXTI_ClearFlag(EXTI_Line14);
			EXTI_ClearITPendingBit(EXTI_Line14);
			// nicht belegt
		}
	//===== nicht belegt
	if(EXTI_GetITStatus(EXTI_Line15) == SET)
		{
			EXTI_ClearFlag(EXTI_Line15);
			EXTI_ClearITPendingBit(EXTI_Line15);
			// nicht belegt
		}

}


//=========================================================================
void RTC_Alarm_IRQHandler(void)
{
	//===== Time Stamp interrupt
	if(RTC_GetITStatus(RTC_IT_TS) != RESET)
		{
			RTC_ClearITPendingBit(RTC_IT_TS);
			EXTI_ClearITPendingBit(EXTI_Line21);
			// nicht belegt
		}
	//=====	WakeUp Timer interrupt
	if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
		{
			RTC_ClearITPendingBit(RTC_IT_WUT);
			EXTI_ClearITPendingBit(EXTI_Line22);
			// nicht belegt
		}
	//===== RTC_IT_ALRB: Alarm B interrupt
	if(RTC_GetITStatus(RTC_IT_ALRB) != RESET)
		{
			RTC_ClearITPendingBit(RTC_IT_ALRB);
			EXTI_ClearITPendingBit(EXTI_Line17);
			// nicht belegt
		}
	//===== RTC_IT_ALRA: Alarm A interrupt
	if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
		{
			RTC_ClearITPendingBit(RTC_IT_ALRA);
			EXTI_ClearITPendingBit(EXTI_Line17);
			//	if (RTC_Alarm_CallBack[0] != NULL)
			//	{
			//	RTC_Alarm_CallBack[0]();
			//	wait_uSek(3000000);
			//	}
		}
	//===== RTC_IT_TAMP1: Tamper 1 event interrupt
	if(RTC_GetITStatus(RTC_IT_TAMP1) != RESET)
		{
			RTC_ClearITPendingBit(RTC_IT_TAMP1);
			EXTI_ClearITPendingBit(EXTI_Line21);
			// nicht belegt
		}
}


//=========================================================================
void ADC_IRQHandler(void)
{
	//===== ADC EOC interrupt
	if(ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
		{
			ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
			// ... Code für Ende Wandlung
		}
	//===== ADC AWD interrupt
	if(ADC_GetITStatus(ADC1, ADC_IT_AWD) == SET)
		{
			ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
			// ... Code für analogen Watchdog
		}
}


//=========================================================================
void USART2_IRQHandler(void)
{
	//===== USART2
	//UART2_IRQHandler();
	//USART2_IRQ();
    //usart2_send("USART2_IRQn\r\n");
}


//=========================================================================
void UART5_IRQHandler(void)
{
	//===== USART5
	if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
		{
			USART_SendData(USART2, (char)USART_ReceiveData(UART5));
		}
}


//=========================================================================
void USART6_IRQHandler(void)
{
	//===== USART6
	CC3100_uart6_receive_IRQ();
}

//=========================================================================
void DMA2_Stream6_IRQHandler(void)
{
	//===== DMA2_Stream6
	//DMA2_Stream6_IRQ();
}

//=========================================================================
void RTC_WKUP_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
		{
			RTC_ClearITPendingBit(RTC_IT_WUT);
			EXTI_ClearITPendingBit(EXTI_Line22);
		}
}


//=========================================================================
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_CC2) == SET)
		{
			TIM_ClearITPendingBit(TIM1, TIM_IT_CC2);
		}
}


//=========================================================================
void TIM7_IRQHandler(void)
{
	BEEPER_IRQHandler();
}

//=========================================================================
void DMA2_Stream2_IRQHandler(void)
{
	//====DW1000 RXD stream der SPI1
	if (DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2))
	{
	DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);
	}
}

//=========================================================================
void DMA2_Stream3_IRQHandler(void)
{
	//====DW1000 TXD stream der SPI1
	if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3))
	{
	DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);
	}
}




//=========================================================================
void TIM6_DAC_IRQHandler()
{
	if((TIM6->SR) & (TIM_SR_UIF != 0) )
		{

		}
	TIM6->SR &= ~TIM_SR_UIF;
}


//=========================================================================
// From Joseph Yiu, minor edits by FVH
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler in file
void hard_fault_handler_c(unsigned int * hardfault_args)
{
	char out[256];
	unsigned int stacked_r0;
	unsigned int stacked_r1;
	unsigned int stacked_r2;
	unsigned int stacked_r3;
	unsigned int stacked_r12;
	unsigned int stacked_lr;
	unsigned int stacked_pc;
	unsigned int stacked_psr;

	stacked_r0 = ((unsigned long) hardfault_args[0]);
	stacked_r1 = ((unsigned long) hardfault_args[1]);
	stacked_r2 = ((unsigned long) hardfault_args[2]);
	stacked_r3 = ((unsigned long) hardfault_args[3]);

	stacked_r12 = ((unsigned long) hardfault_args[4]);
	stacked_lr = ((unsigned long) hardfault_args[5]);
	stacked_pc = ((unsigned long) hardfault_args[6]);
	stacked_psr = ((unsigned long) hardfault_args[7]);

	uart_send("\r\n[Hard fault handler - all numbers in hex]\r\n");
	sprintf(out, "R0  = %x\r\n", stacked_r0);
	uart_send(out);
	sprintf(out, "R1  = %x\r\n", stacked_r1);
	uart_send(out);
	sprintf(out, "R2  = %x\r\n", stacked_r2);
	uart_send(out);
	sprintf(out, "R3  = %x\r\n", stacked_r3);
	uart_send(out);
	sprintf(out, "R12 = %x\r\n", stacked_r12);
	uart_send(out);
	sprintf(out, "LR [R14] = %x  subroutine call return address\r\n", stacked_lr);
	uart_send(out);
	sprintf(out, "PC [R15] = %x  program counter\r\n", stacked_pc);
	uart_send(out);
	sprintf(out, "PSR  = %x\r\n", stacked_psr);
	uart_send(out);
	sprintf(out, "BFAR = %lx\r\n", (*((volatile unsigned long *) (0xE000ED38))));
	uart_send(out);
	sprintf(out, "CFSR = %lx\r\n", (*((volatile unsigned long *) (0xE000ED28))));
	uart_send(out);
	sprintf(out, "HFSR = %lx\r\n", (*((volatile unsigned long *) (0xE000ED2C))));
	uart_send(out);
	sprintf(out, "DFSR = %lx\r\n", (*((volatile unsigned long *) (0xE000ED30))));
	uart_send(out);
	sprintf(out, "AFSR = %lx\r\n", (*((volatile unsigned long *) (0xE000ED3C))));
	uart_send(out);
	sprintf(out, "SCB_SHCSR = %x\r\n", (unsigned int) SCB->SHCSR);
	uart_send(out);

	if (SCB->HFSR & SCB_HFSR_DEBUGEVT_Msk) {
		uart_send("##This is a DEBUG FAULT##\r\n");

	} else if (SCB->HFSR & SCB_HFSR_FORCED_Msk) {
		uart_send("##This is a FORCED FAULT##\r\n");

//		if (SCB->CFSR & (0x1 << SCB_CFSR_USGFAULTSR_Msk)) {
//			uart_send("undefined instruction\r\n");
//
//		} else if (SCB->CFSR & (0x2 << SCB_CFSR_USGFAULTSR_Pos)) {
//			uart_send("instruction makes illegal use of the EPSR\r\n");
//
//		} else if (SCB->CFSR & (0x4 << SCB_CFSR_USGFAULTSR_Pos)) {
//			uart_send("Invalid PC load UsageFault, caused by an invalid PC load by EXC_RETURN\r\n");
//
//		} else if (SCB->CFSR & (0x8 << SCB_CFSR_USGFAULTSR_Pos)) {
//			uart_send("The processor does not support coprocessor instructions\r\n");
//
//		} else if (SCB->CFSR & (0x100 << SCB_CFSR_USGFAULTSR_Pos)) {
//			uart_send("Unaligned access\r\n");
//
//		} else if (SCB->CFSR & (0x200 << SCB_CFSR_USGFAULTSR_Pos)) {
//			uart_send("Divide by zero\r\n");
//
//		}
	} else if (SCB->HFSR & SCB_HFSR_VECTTBL_Pos) {
		sprintf(out,"##This is a BUS FAULT##\r\n");
		uart_send(out);
	}
	uart_send("HARDFAULT HANDLER !!!!\r\n");
	while (1) {;}
}

