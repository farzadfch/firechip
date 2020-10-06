#include <stdint.h>
#include "mmio.h"

#define N 4
#define BW_REG_BASE 0x20000000L
#define ENABLE_BW (BW_REG_BASE)
#define BW_SETTINGS (BW_REG_BASE + 1*0x4)
#define WINDOW_SIZE (BW_REG_BASE + 2*0x4)
#define MAX(i) (BW_REG_BASE + (3+i)*0x4)
#define MAX_WR(i) (BW_REG_BASE + (3+N+i)*0x4)
#define ENABLE_MASTERS (BW_REG_BASE + (3+2*N)*0x4)
#define DOMAIN_ID(i) (BW_REG_BASE + (4+2*N+i)*0x4)
#define ENABLE_PERF (BW_REG_BASE + (4+3*N)*0x4)
#define PERF_PERIOD (BW_REG_BASE + (5+3*N)*0x4)
#define ENABLE_PRINT_LATENCY (BW_REG_BASE + (6+3*N)*0x4)

#define WSS_MAX (16 * 1024 / 8 * 4)
volatile uint64_t values[WSS_MAX];

int main(void)
{
  reg_write32(WINDOW_SIZE, 9-1);
  reg_write32(MAX(0), 1);
  reg_write32(MAX_WR(0), 1);
  reg_write32(DOMAIN_ID(0), 0);
  reg_write32(ENABLE_MASTERS, 1 << 0);
  reg_write32(BW_SETTINGS, 3);
  reg_write32(ENABLE_BW, 1);
  //reg_write32(PERF_PERIOD, 1000-1);
  //reg_write32(ENABLE_PERF, 1);
  reg_write32(ENABLE_PRINT_LATENCY, 1);
  asm volatile ("fence");

  for (int j = 0; j < 2; j++)
    for (int i = 0; i < WSS_MAX; i = i + 8) {
      values[i];
      //values[i] = 0xaaaaaaaaaaaaaaaa;
    }

  reg_write32(ENABLE_PRINT_LATENCY, 0);

  return 0;
}
