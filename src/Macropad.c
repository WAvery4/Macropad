#include <stdio.h>
#include <stdint.h>
#include "../inc/CortexM.h"
#include "../inc/PLL.h"

int main(void)
{
  PLL_Init(Bus80MHz);
  DisableInterrupts();
  EnableInterrupts();

  while (1)
  {
    WaitForInterrupt();
  }
}
