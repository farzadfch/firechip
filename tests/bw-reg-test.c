#include <stdint.h>
#include "mmio.h"

#define N 1
#define BW_REG_BASE 0x20000000L
#define ENABLE_BW (BW_REG_BASE)
#define BW_SETTINGS (BW_REG_BASE + 1*0x4)
#define WINDOW_SIZE (BW_REG_BASE + 2*0x4)
#define MAX(i) (BW_REG_BASE + (3+i)*0x4)
#define ENABLE_MASTERS (BW_REG_BASE + (3+N)*0x4)
#define DOMAIN_ID(i) (BW_REG_BASE + (4+N+i)*0x4)


#define WSS_MAX (16 * 1024 / 8 * 4)
volatile uint64_t values[WSS_MAX];

int main(void)
{
  reg_write32(WINDOW_SIZE, 400-1);
  reg_write32(MAX(0), 1);
  reg_write32(DOMAIN_ID(0), 0);
  reg_write32(ENABLE_MASTERS, 1 << 0);
  reg_write32(BW_SETTINGS, 3);
  reg_write32(ENABLE_BW, 1);
  asm volatile ("fence");

  for (int i = 0; i < WSS_MAX / 8; i = i + 8)
    values[i] = 0xaaaaaaaaaaaaaaaa;

  /*for (int j = 0; j < 1; j++)
    for (int i = 0; i < 50 * 8; i = i + 8) {
      values[i] = 0xaa;
    }
  */
  return 0;
}
