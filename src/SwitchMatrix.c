#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/Timer0A.h"
#include "Macro.h"
#include "SwitchMatrix.h"

static int8_t LastRowIndexSelected;
static int8_t LastColumnIndexSelected;
static uint8_t ColumnIndex;

/**
 * Arm interrupts for PB5-PB0.
 */
static void GPIOArm(void)
{
    GPIO_PORTB_ICR_R = 0x3F;                        // Clear flags
    GPIO_PORTB_IM_R |= 0x3F;                        // Arm interrupts
    NVIC_PRI0_R = (NVIC_PRI0_R & ~0xE000) | 0xA000; // Set priority to 5 (bits 15-13)
    NVIC_EN0_R = 0x02;                              // Enable interrupt 1 in NVIC
}

/**
 * Read high pin from port B and update row and column indices.
 */
static void UpdateRowAndColumnIndices(void)
{
    uint32_t portBReadings[] = {PB0, PB1, PB2, PB3, PB4, PB5};

    for (uint8_t i = 0; i < ROW_COUNT; i++)
    {
        if (portBReadings[i] != 0)
        {
            LastRowIndexSelected = i;
            LastColumnIndexSelected = ColumnIndex;

            return;
        }
    }

    LastRowIndexSelected = -1;
    LastColumnIndexSelected = -1;
}

/**
 * Handle switch debounce.
 */
static void Debounce(void)
{
    TIMER0_IMR_R = 0x00; // Disarm interrupt

    UpdateRowAndColumnIndices();
    GPIOArm();
}

/**
 * Initialize PB5-PB0 as row input.
 */
static void InitRows(void)
{
    LastRowIndexSelected = -1;
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
    GPIO_PORTB_PUR_R |= 0x3F;         // Enable pull-up resistors
    GPIO_PORTB_IS_R &= ~0x3F;         // Set as edge-sensitive
    GPIO_PORTB_IBE_R |= 0x3F;         // Set trigger to both edges

    GPIOArm();
    Timer0A_Init(Debounce, 800000, 3);
    UpdateRowAndColumnIndices();
}

/**
 * Initialize PC7-PC4 as column output.
 */
static void InitColumns(void)
{
    LastColumnIndexSelected = -1;
    ColumnIndex = 0;
    SYSCTL_RCGCGPIO_R |= 0x04; // Activate clock for Port C

    while ((SYSCTL_PRGPIO_R & 0x04) == 0)
    {
        // Wait for activation
    }

    GPIO_PORTC_DIR_R != 0xF0;         // Set as output
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

    // TODO is this allowed within an ISR?
    if (LastRowIndexSelected >= 0 && LastColumnIndexSelected >= 0)
    {
        // Handle switch press
        Macro selectedMacro = Macro_Keybindings[LastRowIndexSelected][LastColumnIndexSelected];

        Macro_Execute(selectedMacro);
    }

    TIMER0_IMR_R = 0x01; // Arm interrupt
}

void SwitchMatrix_Init(void)
{
    InitRows();
    InitColumns();
    Macro_Init();
}

void SwitchMatrix_CycleColumnOutput(void)
{
    volatile uint32_t *portCAddresses[] = {&PB4, &PB5, &PB6, &PB7};

    for (uint8_t i = 0; i < COLUMN_COUNT; i++)
    {
        // Set all columns low
        *portCAddresses[i] = 0x00;
    }

    ColumnIndex = (ColumnIndex + 1) % COLUMN_COUNT;

    for (uint8_t i = 0; i < COLUMN_COUNT; i++)
    {
        if (i == ColumnIndex)
        {
            // Set next column high
            *portCAddresses[i] = 1 << (i + COLUMN_COUNT);
        }
    }
}
