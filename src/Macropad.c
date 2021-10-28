#include <stdio.h>
#include <stdint.h>
#include "../inc/CortexM.h"
#include "../inc/PLL.h"
#include "SwitchMatrix.h"

int main(void)
{
  PLL_Init(Bus80MHz);
  DisableInterrupts();
  SwitchMatrix_Init();
  EnableInterrupts();

  while (1)
  {
    SwitchMatrix_CycleColumnOutput();
  }
}
