#include "Macro.h"

Macro Macro_Keybindings[ROW_COUNT][COLUMN_COUNT];

void Macro_Init(void)
{
    for (uint8_t i = 0; i < ROW_COUNT; i++)
    {
        for (uint8_t j = 0; j < COLUMN_COUNT; j++)
        {
            // TODO load macros from SD card
            Macro_Keybindings[i][j].name = "Test";

            for (int k = 0; k < MAX_KEY_COMBO_COUNT; k++)
            {
                Macro_Keybindings[i][j].asciiCodes[k] = 0;
            }
        }
    }
}

void Macro_Execute(Macro macro)
{
    // TODO send keyboard combination to computer through USB
}
