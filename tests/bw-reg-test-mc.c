#include <stdint.h>
#include "mmio.h"
#include "util.h"

#define N 4
#define BW_REG_BASE 0x20000000L
#define ENABLE_BW (BW_REG_BASE)
#define BW_SETTINGS (BW_REG_BASE + 1*0x4)
#define WINDOW_SIZE (BW_REG_BASE + 2*0x4)
#define MAX(i) (BW_REG_BASE + (3+i)*0x4)
#define MAX_WR(i) (BW_REG_BASE + (3+N+i)*0x4)
#define ENABLE_MASTERS (BW_REG_BASE + (3+2*N)*0x4)
#define DOMAIN_ID(i) (BW_REG_BASE + (4+2*N+i)*0x4)
#define PERF (BW_REG_BASE + (4+3*N)*0x4)

#define WSS_MAX (16 * 1024 / 8 * 4)
volatile uint64_t values0[WSS_MAX];
volatile uint64_t values1[WSS_MAX];
volatile uint64_t values2[WSS_MAX];
volatile uint64_t values3[WSS_MAX];

#define SETTINGS_CORE 0

void thread_entry(int cid, int nc)
{
  volatile register uint64_t *values;

  switch (cid)
  {
    case 0:
      values = values0;
      break;
    case 1:
      values = values1;
      break;
    case 2:
      values = values2;
      break;
    case 3:
      values = values3;
      break;
  }

  if (cid == SETTINGS_CORE) {
    //reg_write32(PERF, 1 << 24 | 2130-1);
    reg_write32(WINDOW_SIZE, 213-1);
    reg_write32(MAX(0), 1);
    reg_write32(MAX_WR(0), 213);
    reg_write32(DOMAIN_ID(0), 0);
    reg_write32(DOMAIN_ID(1), 0);
    reg_write32(DOMAIN_ID(2), 0);
    reg_write32(DOMAIN_ID(3), 0);
    reg_write32(ENABLE_MASTERS, 0xf);
    reg_write32(BW_SETTINGS, 3);
    //reg_write32(ENABLE_BW, 1);
    asm volatile ("fence");
  }

  barrier(N);

  for (int j = 0; j < 2; j++)
    for (int i = 0; i < WSS_MAX; i = i + 8)
      values[i];

  barrier(N);
  exit(0);
}

