#define PORT_B_PRIORITY 5
#define TIMER0A_PRIORITY 3

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "Macro.h"
#include "SwitchMatrix.h"

extern long StartCritical(void);
extern void EndCritical(long sr);

static uint8_t ColumnIndex;

/**
 * Arm interrupts for PB5-PB0.
 */
static void ArmPortB(void)
{
    GPIO_PORTB_ICR_R = 0x3F;                                         // Clear flags
    GPIO_PORTB_IM_R |= 0x3F;                                         // Arm interrupts
    NVIC_PRI0_R = (NVIC_PRI0_R & ~0xE000) | (PORT_B_PRIORITY << 13); // Set priority (bits 15-13)
    NVIC_EN0_R = 1 << 1;                                             // Enable IRQ 1 in NVIC
}

/**
 * Arm Timer0A for debouncing.
 * 
 * @param period the duration of the timer
 */
static void Timer0A_Arm(uint32_t period)
{
    SYSCTL_RCGCTIMER_R |= 0x01;                                          // 0) activate Timer0
    TIMER0_CTL_R &= ~0x01;                                               // 1) disable Timer0A during setup
    TIMER0_CFG_R = 0x00;                                                 // 2) configure for 32-bit timer mode
    TIMER0_TAMR_R = 0x01;                                                // 3) 1-shot mode
    TIMER0_TAILR_R = period - 1;                                         // 4) reload value
    TIMER0_TAPR_R = 0;                                                   // 5) 12.5ns Timer0A
    TIMER0_ICR_R = 0x01;                                                 // 6) clear Timer0A timeout flag
    TIMER0_IMR_R |= 0x01;                                                // 7) arm timeout interrupt
    NVIC_PRI4_R = (NVIC_PRI4_R & 0x00FFFFFF) | (TIMER0A_PRIORITY << 29); // 8) set priority
    NVIC_EN0_R = 1 << 19;                                                // 9) enable IRQ 19 in NVIC
    TIMER0_CTL_R |= 0x01;                                                // 10) enable Timer0A
}

/**
 * Initialize PB5-PB0 as row input.
 */
static void InitRows(void)
{
    SYSCTL_RCGCGPIO_R |= 0x02; // Activate clock for Port B

    while ((SYSCTL_PRGPIO_R & 0x02) == 0)
    {
        // Wait for activation
    }

    GPIO_PORTB_DIR_R &= ~0x3F;        // Set as input
    GPIO_PORTB_AFSEL_R &= ~0x3f;      // Disable alternate function
    GPIO_PORTB_DEN_R |= 0x3F;         // Enable digital I/O
    GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // Configure as GPIO
    GPIO_PORTB_AMSEL_R &= ~0x3F;      // Disable analog functionality
    GPIO_PORTB_PDR_R |= 0x3F;         // Enable pull-down resistors
    GPIO_PORTB_IS_R &= ~0x3F;         // Set as edge-sensitive
    GPIO_PORTB_IBE_R |= 0x3F;         // Set trigger to both edges

    ArmPortB();
}

/**
 * Initialize PC7-PC4 as column output.
 */
static void InitColumns(void)
{
    ColumnIndex = 0;
    SYSCTL_RCGCGPIO_R |= 0x04; // Activate clock for Port C

    while ((SYSCTL_PRGPIO_R & 0x04) == 0)
    {
        // Wait for activation
    }

    GPIO_PORTC_LOCK_R = 0x4C4F434B;   // Unlock GPIO for port C
    GPIO_PORTC_DIR_R |= 0xF0;         // Set as output
    GPIO_PORTC_AFSEL_R &= ~0xF0;      // Disable alternate function
    GPIO_PORTC_DEN_R |= 0xF0;         // Enable digital I/O
    GPIO_PORTC_PCTL_R &= ~0xFFFF0000; // Configure as GPIO
    GPIO_PORTC_AMSEL_R &= ~0xF0;      // Disable analog functionality

    SwitchMatrix_CycleColumnOutput();
}

/**
 * Handle port B interaction.
 */
void GPIOPortB_Handler(void)
{
    GPIO_PORTB_IM_R &= ~0x3F; // Disarm interrupts
    uint32_t triggeredPort = GPIO_PORTB_RIS_R & 0x3F;

    for (uint8_t i = 0; i < ROW_COUNT; i++)
    {
        if (triggeredPort >> i == 1)
        {
            // Handle switch press
            Macro selectedMacro = Macro_Keybindings[i][ColumnIndex];

            Macro_Execute(selectedMacro);
        }
    }

    Timer0A_Arm(800000);
}

/**
 * Handle re-arming port B.
 */
void Timer0A_Handler(void)
{
    TIMER0_ICR_R = TIMER_ICR_TATOCINT; // acknowledge timer0A timeout

    ArmPortB();
}

void SwitchMatrix_Init(void)
{
    InitRows();
    InitColumns();
    Macro_Init();
}

void SwitchMatrix_CycleColumnOutput(void)
{
    long sr = StartCritical();
    ColumnIndex = (ColumnIndex + 1) % COLUMN_COUNT;
    PC7654 = 1 << (ColumnIndex + COLUMN_COUNT);

    EndCritical(sr);
}
