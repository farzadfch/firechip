#include <stdint.h>
#include "mmio.h"

#define BW_REG_BASE 0x20000000L
#define WINDOW_SIZE (BW_REG_BASE)
#define MAX_XACTION(i) (BW_REG_BASE + (i+1)*0x4)


#define N (16 * 1024 / 8 * 2)
volatile uint64_t values[N];

int main(void)
{
  reg_write32(WINDOW_SIZE, 500-1);
  reg_write32(MAX_XACTION(0), 1);
  asm volatile ("fence");

  for (int j = 0; j < 1; j++)
    for (int i = 0; i < 50 * 8; i = i + 8) {
      values[i];
    }
  return 0;
}
