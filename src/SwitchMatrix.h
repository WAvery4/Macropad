// Bit-specific addresses
#define PB0 (*((volatile uint32_t *)0x40005004))
#define PB1 (*((volatile uint32_t *)0x40005008))
#define PB2 (*((volatile uint32_t *)0x40005010))
#define PB3 (*((volatile uint32_t *)0x40005020))
#define PB4 (*((volatile uint32_t *)0x40005040))
#define PB5 (*((volatile uint32_t *)0x40005080))
#define PB6 (*((volatile uint32_t *)0x40005100))
#define PB7 (*((volatile uint32_t *)0x40005200))

#define PC4 (*((volatile uint32_t *)0x40006040))
#define PC5 (*((volatile uint32_t *)0x40006080))
#define PC6 (*((volatile uint32_t *)0x40006100))
#define PC7 (*((volatile uint32_t *)0x40006200))

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
