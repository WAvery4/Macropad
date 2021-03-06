#define MAX_KEY_COMBO_COUNT 5

#include <stdint.h>
#include <stdbool.h>
#include "SwitchMatrix.h"

typedef struct Macro
{
    char name[4];
    uint8_t modifiers[MAX_KEY_COMBO_COUNT];
    uint8_t asciiCodes[MAX_KEY_COMBO_COUNT];
    uint8_t numKeys;
} Macro;

extern Macro Macro_Keybindings[ROW_COUNT][COLUMN_COUNT];

/**
 * Initialize the list of keybindings.
 */
void Macro_Init(void);

/**
 * Execute the specified macro.
 * 
 * @param macro the macro to send through USB
 */
void Macro_Execute(Macro macro, bool isMedia);
