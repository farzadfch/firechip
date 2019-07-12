#include <stdint.h>
#include "mmio.h"

#define N 4
#define BW_REG_BASE 0x20000000L
#define ENABLE_BW (BW_REG_BASE)
#define WINDOW_SIZE (BW_REG_BASE + 0x4)
#define MAX(i) (BW_REG_BASE + (2+i)*0x4)
#define ENABLE_MASTERS (BW_REG_BASE + (2+N)*0x4)
#define DOMAIN_ID(i) (BW_REG_BASE + (3+N+i)*0x4)


#define WSS_MAX (16 * 1024 / 8 * 2)
volatile uint64_t values[WSS_MAX];

int main(void)
{
  reg_write32(WINDOW_SIZE, 200-1);
  reg_write32(MAX(0), 2);
  reg_write32(DOMAIN_ID(0), 0);
  reg_write32(ENABLE_MASTERS, 1 << 0);
  reg_write32(ENABLE_BW, 1 | 1 << 1);
  asm volatile ("fence");

  for (int i = 0; i < WSS_MAX / 8; i = i + 8)
    values[i];

  for (int j = 0; j < 1; j++)
    for (int i = 0; i < WSS_MAX /*50 * 8*/; i = i + 8) {
      values[i] = 0xaa;
    }
  return 0;
}
