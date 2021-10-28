#include "../inc/tm4c123gh6pm.h"
#include "../inc/Timer0A.h"
#include "RotarySwitch.h"

/**
 * Arm 10 ms one shot timer for debouncing.
 */
void Timer0_Arm(void){
	TIMER0_CTL_R = 0x00000000;    // 1) disable TIMER0A during setup
	TIMER0_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
	TIMER0_TAMR_R = 0x0000001;    // 3) 1-SHOT mode
	TIMER0_TAILR_R = 800000;      // 4) 10ms reload value
	TIMER0_TAPR_R = 0;            // 5) bus clock resolution
	TIMER0_ICR_R = 0x00000001;    // 6) clear TIMER0A timeout flag
	TIMER0_IMR_R = 0x00000001;    // 7) arm timeout interrupt
	NVIC_PRI4_R = (NVIC_PRI4_R&0x00FFFFFF)|0x80000000; // 8) priority 4
	NVIC_EN0_R = 1<<19;           // 9) enable IRQ 19 in NVIC
	TIMER0_CTL_R = 0x00000001;    // 10) enable TIMER0A
}

/**
 * Arm interrupts for PF0-2
 */
void RotarySwitch_Arm(void) {
	GPIO_PORTF_ICR_R = 0x07;      // clear flag 0-2
	GPIO_PORTF_IM_R |= 0x07;      // arm interrupt on PF0-2
	NVIC_PRI0_R = (NVIC_PRI0_R & ~0xE000) | 0xA000; // Set priority to 5 (bits 15-13)
	
	NVIC_EN0_R = 0x00000004;      // enable interrupt 2 in NVIC
}

void RotarySwitch_Init(void) {
	SYSCTL_RCGCGPIO_R     |= 0x00000020;      // activate clock for Port F
	while((SYSCTL_PRGPIO_R & 0x20)==0){};     // allow time for clock to stabilize
    
	GPIO_PORTF_LOCK_R     = 0x4C4F434B;       // unlock GPIO Port F
	GPIO_PORTF_CR_R       = 0x07;             // allow changes to PF0-2
  
	GPIO_PORTF_AMSEL_R    = 0x00;             // disable analog on PF
	GPIO_PORTF_PCTL_R     = 0x00000000;       // PCTL GPIO on PF0-2
	GPIO_PORTF_DIR_R      = 0x07;             // PF0-2 output
  GPIO_PORTF_AFSEL_R    = 0x00;             // disable alt funct on PF0-7
  GPIO_PORTF_PUR_R      = 0x07;             // enable pull-up on PF0-2
  GPIO_PORTF_DEN_R      = 0x07;             // enable digital I/O on PF0-2
		
	SYSCTL_RCGCTIMER_R |= 0x01;   // activate TIMER0 to use for debouncing	
		
	RotarySwitch_Arm();	
}

/** 
 * Handle Port F interaction.
 * CW: PF0
 * CCW: PF1
 * Click: PF2
 */
void GPIOPortF_Handler(void) {
	GPIO_PORTF_IM_R &= ~0x07;     // disarm interrupt on PF0-2
	
	uint32_t triggeredPort = GPIO_PORTF_RIS_R & 0x7;
	if (triggeredPort == 1) {
		/* TODO: Increase volume */
	} else if (triggeredPort == 2) {
		/* TODO: Decrease volume */
	} else if (triggeredPort == 4) {
		/* TODO: Toggle volume mute */
	}
	
	Timer0_Arm();
}