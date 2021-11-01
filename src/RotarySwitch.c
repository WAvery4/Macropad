#define PORT_F_PRIORITY 5
#define TIMER1A_PRIORITY 3

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "RotarySwitch.h"

/**
 * Arm interrupts for PF2-PF0.
 */
static void ArmPortF(void)
{
    GPIO_PORTF_ICR_R = 0x07;                                           // Clear flags
    GPIO_PORTF_IM_R |= 0x07;                                           // Arm interrupt
    NVIC_PRI7_R = (NVIC_PRI7_R & ~0xE00000) | (PORT_F_PRIORITY << 21); // Set priority (bits 23-21)
    NVIC_EN0_R = 1 << 30;                                              // Enable IRQ 30 in NVIC
}

/**
 * Arm Timer1A for debouncing.
 * 
 * @param period the duration of the timer
 */
static void Timer1A_Arm(uint32_t period)
{
    SYSCTL_RCGCTIMER_R |= 0x02;                                          // 0) activate Timer1
    TIMER1_CTL_R &= ~0x01;                                               // 1) disable Timer1A during setup
    TIMER1_CFG_R = 0x00;                                                 // 2) configure for 32-bit mode
    TIMER1_TAMR_R = 0x01;                                                // 3) 1-shot mode
    TIMER1_TAILR_R = period - 1;                                         // 4) reload value
    TIMER1_TAPR_R = 0;                                                   // 5) bus clock resolution
    TIMER1_ICR_R = 0x01;                                                 // 6) clear Timer1A timeout flag
    TIMER1_IMR_R = 0x01;                                                 // 7) arm timeout interrupt
    NVIC_PRI5_R = (NVIC_PRI5_R & 0xFFFF00FF) | (TIMER1A_PRIORITY << 13); // 8) set priority
    NVIC_EN0_R = 1 << 21;                                                // 9) enable IRQ 21 in NVIC
    TIMER1_CTL_R |= 0x01;                                                // 10) enable Timer1A
}

/** 
 * Handle Port F interaction.
 * CW: PF0
 * CCW: PF1
 * Click: PF2
 */
void GPIOPortF_Handler(void)
{
    GPIO_PORTF_IM_R &= ~0x07; // disarm interrupt on PF0-2
    uint32_t triggeredPort = GPIO_PORTF_RIS_R & 0x07;

    if (triggeredPort == 1)
    {
        /* TODO: Increase volume */
    }
    else if (triggeredPort == 2)
    {
        /* TODO: Decrease volume */
    }
    else if (triggeredPort == 4)
    {
        /* TODO: Toggle volume mute */
    }

    Timer1A_Arm(800000);
}

/**
 * Handle re-arming port F.
 */
void Timer1A_Handler(void)
{
    TIMER1_ICR_R = TIMER_ICR_TATOCINT; // acknowledge TIMER1A timeout

    ArmPortF();
}

void RotarySwitch_Init(void)
{
    SYSCTL_RCGCGPIO_R |= 0x00000020; // activate clock for Port F

    while ((SYSCTL_PRGPIO_R & 0x20) == 0)
    {
        // Wait for activation
    }

    GPIO_PORTF_LOCK_R = 0x4C4F434B;   // Unlock GPIO for port F
    GPIO_PORTF_DIR_R &= ~0x07;        // Set as input
    GPIO_PORTF_AFSEL_R &= ~0x07;      // Disable alternate function
    GPIO_PORTF_DEN_R |= 0x07;         // Enable digital I/O
    GPIO_PORTF_PCTL_R &= ~0x00000FFF; // Configure as GPIO
    GPIO_PORTF_AMSEL_R &= ~0x07;      // Disable analog functionality
    GPIO_PORTF_PUR_R |= 0x07;         // Enable pull-up resistors
    GPIO_PORTF_IS_R &= ~0x07;         // Set as edge-sensitive
    GPIO_PORTF_IBE_R |= 0x07;         // Set trigger to both edges

    ArmPortF();
}
