/**
 * Filename: Macro.c
 * Authors: William Avery (add names here)
 * Last Modified: 10/29/2021
 */
#include "Macro.h"
#include "../inc/diskio.h"
#include "../inc/ff.h"
#include "../inc/ST7735_SDC.h"

Macro Macro_Keybindings[ROW_COUNT][COLUMN_COUNT];
static FATFS g_sFatFs;
FIL Handle;
FRESULT MountFresult;
FRESULT Fresult;
const char inFilename[] = "mac.txt";   // 8 characters or fewer

/**
 * @brief Reads and binds the macros to the appropriate keys on the Macropad.
 * Input: MicroSD card in the ST7735 with a 'MAC.txt' file
 *        
 *         --- MAC.TXT FORMAT ---
 *        - 24 Lines
 *        - Line Format: MACRO_NAME NUM_KEYS KEYS
 *        - MACRO_NAME: 3 characters long
 *        - NUM_KEYS: [1,5] (integer)
 *        - KEYS: [1,5] (characters)
 */
void Macro_Init(void)
{
    UINT successfulReads;
    uint8_t ch, x, y;
  
    MountFresult = f_mount(&g_sFatFs, "", 0); // mount the filesystem
    if(MountFresult){
        ST7735_DrawString(122, 148, "f_mount error", ST7735_Color565(0, 0, 255));
        while(1){};
    }
    Fresult = f_open(&Handle, inFilename, FA_READ); // open mac.txt
    x = 122, y = 130;
    if(Fresult == FR_OK) {
        for (uint8_t i = 0; i < ROW_COUNT; i++)
        {
            for (uint8_t j = 0; j < COLUMN_COUNT; j++)
            {
                char* name;
                uint8_t nameIdx = 0;
                // read in the name of the macro
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                while((Fresult == FR_OK) && (successfulReads == 1) && (ch != 0x20)) {
                    ST7735_DrawChar(x, y, ch, ST7735_Color565(255, 255, 255), 0, 1);
                    name[nameIdx] = ch;
                    nameIdx++;
                    // go to the next column
                    x -= 6;
                    // read the next character into 'ch'
                    Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                }
                
                Macro_Keybindings[i][j].name = name;

                // read in how many keys are in the macro
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                uint8_t numKeys = ch - 0x30;
                
                // read extra space character
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                x -= 6; // add space between macro name and macro keys

                for (int k = 0; k < numKeys; k++)
                { 
                    // read in the k-th key
                    Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                    Macro_Keybindings[i][j].asciiCodes[k] = ch;
                }
                
                // read return and new line characters
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                Fresult = f_read(&Handle, &ch, 1, &successfulReads);
                x -= 12;
            }
            x = 122;
            y -= 20;
        }
    }
    else {
        ST7735_DrawString(122, 148, "f_open error", ST7735_Color565(0, 0, 255));
        while(1){};
    }
}

void Macro_Execute(Macro macro)
{
    // TODO send keyboard combination to computer through USB
}
