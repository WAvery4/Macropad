// Bit-banding
#define PC7654 (*((volatile uint32_t *)0x400063C0))

// Switch matrix dimensions
#define ROW_COUNT 6
#define COLUMN_COUNT 4

/**
 * Initialize the switch matrix.
 */
void SwitchMatrix_Init(void);

/**
 * Cycle column output for PC7-PC4.
 */
void SwitchMatrix_CycleColumnOutput(void);
