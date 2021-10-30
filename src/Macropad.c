/**
 * Filename: Macropad.c
 * Authors: William Avery (add names here)
 * Last Modified: 10/29/2021
 */

#include <stdio.h>
#include <stdint.h>
#include "../inc/CortexM.h"
#include "../inc/PLL.h"
#include "../inc/ff.h"
#include "../inc/diskio.h"
#include "../inc/ST7735_SDC.h"
#include "SwitchMatrix.h"
#include "RotarySwitch.h"
#include "Macro.h"

/**
 * Choose the test loop to run
 * 0 - Main Control Loop
 * 1 - SD Card Test
 * 2 - Macro.c Test
 */
#define __MAIN__ 2

#if __MAIN__ == 0
int main(void)
{
  PLL_Init(Bus80MHz);
  DisableInterrupts();
  SwitchMatrix_Init();
  RotarySwitch_Init();
  EnableInterrupts();

  while (1)
  {
    SwitchMatrix_CycleColumnOutput();
  }
}

#elif __MAIN__ == 1
static FATFS g_sFatFs;
FIL Handle_t,Handle2;
FRESULT MountFresult_t;
FRESULT Fresult_t;
unsigned char buffer[512];
const char inFilename_t[] = "test.txt";   // 8 characters or fewer
const char outFilename[] = "out.txt";   // 8 characters or fewer

int main(void) {
  UINT successfulreads, successfulwrites;
  uint8_t c, x, y;
  
  PLL_Init(Bus80MHz);
  DisableInterrupts();
  ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(0); // set screen to black
  EnableInterrupts();
  
  MountFresult_t = f_mount(&g_sFatFs, "", 0); // mount the filesystem
  if(MountFresult_t){
    ST7735_DrawString(0, 0, "f_mount error", ST7735_Color565(0, 0, 255));
    while(1){};
  }
  Fresult_t = f_open(&Handle_t, inFilename_t, FA_READ); // open test.txt
  if(Fresult_t == FR_OK){
    ST7735_DrawString(0, 0, "Opened ", ST7735_Color565(0, 255, 0));
    ST7735_DrawString(7, 0, (char *)inFilename_t, ST7735_Color565(0, 255, 0));
    // get a character in 'c' and the number of successful reads in 'successfulreads'
    Fresult_t = f_read(&Handle_t, &c, 1, &successfulreads);
    x = 0;                              // start in the first column
    y = 10;                             // start in the second row
    while((Fresult_t == FR_OK) && (successfulreads == 1) && (y <= 130)){
      if(c == '\n'){
        x = 0;                          // go to the first column (this seems implied)
        y = y + 10;                     // go to the next row
      } else if(c == '\r'){
        x = 0;                          // go to the first column
      } else{                           // the character is printable, so print it
        ST7735_DrawChar(x, y, c, ST7735_Color565(255, 255, 255), 0, 1);
        x = x + 6;                      // go to the next column
        if(x > 122){                    // reached the right edge of the screen
          x = 0;                        // go to the first column
          y = y + 10;                   // go to the next row
        }
      }
      // get the next character in 'c'
      Fresult_t = f_read(&Handle_t, &c, 1, &successfulreads);
    }
    // close the file
    Fresult_t = f_close(&Handle_t);
  } else{
    // print the error code
    ST7735_DrawString(0, 0, "Error          (  )", ST7735_Color565(255, 0, 0));
    ST7735_DrawString(6, 0, (char *)inFilename_t, ST7735_Color565(255, 0, 0));
    ST7735_SetCursor(16, 0);
    ST7735_SetTextColor(ST7735_Color565(255, 0, 0));
    ST7735_OutUDec((uint32_t)Fresult_t);
  }

  // open out.txt
  // Options:
  // FA_CREATE_NEW    - Creates a new file, only if it does not already exist.  If file already exists, the function fails.
  // FA_CREATE_ALWAYS - Creates a new file, always.  If file already exists, it is over-written.
  // FA_OPEN_ALWAYS   - Opens a file, always.  If file does not exist, the function creates a file.
  // FA_OPEN_EXISTING - Opens a file, only if it exists.  If the file does not exist, the function fails.
  Fresult_t = f_open(&Handle_t, outFilename, FA_WRITE|FA_OPEN_ALWAYS);
  if(Fresult_t == FR_OK){
    ST7735_DrawString(0, 14, "Opened ", ST7735_Color565(0, 255, 0));
    ST7735_DrawString(7, 14, (char *)outFilename, ST7735_Color565(0, 255, 0));
    // jump to the end of the file
    Fresult_t = f_lseek(&Handle_t, Handle_t.fsize);
    // write a message and get the number of successful writes in 'successfulwrites'
    Fresult_t = f_write(&Handle_t, "Another successful write.\r\n", 27, &successfulwrites);
    if(Fresult_t == FR_OK){
      // print the number of successful writes
      // expect: third parameter of f_write()
      ST7735_DrawString(0, 15, "Writes:    @", ST7735_Color565(0, 255, 0));
      ST7735_SetCursor(8, 15);
      ST7735_SetTextColor(ST7735_Color565(255, 255, 255));
      ST7735_OutUDec((uint32_t)successfulwrites);
      ST7735_SetCursor(13, 15);
      // print the byte offset from the start of the file where the writes started
      // expect: (third parameter of f_write())*(number of times this program has been run before)
      ST7735_OutUDec((uint32_t)(Handle_t.fptr - successfulwrites));
    } else{
      // print the error code
      ST7735_DrawString(0, 15, "f_write() error (  )", ST7735_Color565(255, 0, 0));
      ST7735_SetCursor(17, 15);
      ST7735_SetTextColor(ST7735_Color565(255, 0, 0));
      ST7735_OutUDec((uint32_t)Fresult_t);
    }
    // close the file
    Fresult_t = f_close(&Handle_t);
  } else{
    // print the error code
    ST7735_DrawString(0, 14, "Error          (  )", ST7735_Color565(255, 0, 0));
    ST7735_DrawString(6, 14, (char *)outFilename, ST7735_Color565(255, 0, 0));
    ST7735_SetCursor(16, 14);
    ST7735_SetTextColor(ST7735_Color565(255, 0, 0));
    ST7735_OutUDec((uint32_t)Fresult_t);
  }
  while(1){};
}

#elif __MAIN__ == 2


int main(void) {
  PLL_Init(Bus80MHz);
  DisableInterrupts();
  ST7735_InitR(INITR_REDTAB);
  ST7735_FillScreen(0); // set screen to black
  EnableInterrupts();
  Macro_Init();
  while(1){};
}
#endif


