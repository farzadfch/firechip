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
#define ENABLE_PERF (BW_REG_BASE + (4+3*N)*0x4)
#define PERF_PERIOD (BW_REG_BASE + (5+3*N)*0x4)
#define ENABLE_PRINT_LATENCY (BW_REG_BASE + (6+3*N)*0x4)

#define WSS_MAX (16 * 1024 / 8 * 4)
volatile uint64_t values0[WSS_MAX];
volatile uint64_t values1[WSS_MAX];
volatile uint64_t values2[WSS_MAX];
volatile uint64_t values3[WSS_MAX];

uint64_t cycle1[N];
uint64_t cycle2[N];

#define MASTER_CORE 3

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

  if (cid == MASTER_CORE) {
    reg_write32(WINDOW_SIZE, 18-1);
    reg_write32(MAX(0), 1);
    //reg_write32(MAX_WR(0), 213);
    reg_write32(DOMAIN_ID(0), 0);
    reg_write32(DOMAIN_ID(1), 0);
    reg_write32(DOMAIN_ID(2), 0);
    reg_write32(DOMAIN_ID(3), 0);
    reg_write32(ENABLE_MASTERS, 0xf);
    reg_write32(BW_SETTINGS, 1);
    //reg_write32(PERF_PERIOD, 1000-1);
    //reg_write32(ENABLE_PERF, 1);
    asm volatile ("fence");
  }

  barrier(N);

  if (cid == MASTER_CORE) {
    reg_write32(ENABLE_BW, 1);
    reg_write32(ENABLE_PRINT_LATENCY, 1);
    asm volatile ("fence");
  }

  // warm up the cache
  for (int i = 0; i < WSS_MAX; i = i + 8)
    values[i];

  uint64_t cycle1_reg = rdcycle();

  for (int j = 0; j < 1; j++)
    for (int i = 0; i < WSS_MAX; i = i + 8)
      values[i];

  uint64_t cycle2_reg = rdcycle();

  if (cid == MASTER_CORE) {
    reg_write32(ENABLE_PRINT_LATENCY, 0);
    reg_write32(ENABLE_BW, 0);
    asm volatile ("fence");
  }

  cycle1[cid] = cycle1_reg;
  cycle2[cid] = cycle2_reg;

  barrier(N);

  if (cid == MASTER_CORE) {
    for (int i = 0; i < N; i++) {
      printf("C%d: cycle1: %lu, cycle2: %lu, diff: %lu\n", i, cycle1[i], cycle2[i], cycle2[i] - cycle1[i]);
    }
  }

  barrier(N);
  exit(0);
}

