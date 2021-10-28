#include <stdio.h>
#include <stdint.h>
#include "../inc/CortexM.h"
#include "../inc/PLL.h"
#include "SwitchMatrix.h"
#include "RotarySwitch.h"

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
