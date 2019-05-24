#include <stdint.h>

#include "util.h"
#include "mmio.h"


#define BW_REG_BASE 0x20000000L
#define WINDOW_SIZE (BW_REG_BASE)
#define MAX_XACTION(i) (BW_REG_BASE + (i+1) * 0x4)

#define NC_MAX 4
// 64 byte block
#define BLK_SZ 8
// L1 cache size * 2, L1 = 16k
#define WSS_MAX (16 * 1024 / BLK_SZ * 2)

volatile uint64_t values0[WSS_MAX];
volatile uint64_t values1[WSS_MAX];
volatile uint64_t values2[WSS_MAX];
volatile uint64_t values3[WSS_MAX];
uint64_t cycle1[NC_MAX];
uint64_t cycle2[NC_MAX];
int run_core_mask[NC_MAX] = {0, 0, 0, 1};
#define NC 1
#define PRINT_CORE 3

void thread_entry(int cid, int nc)
{
  volatile register uint64_t *values;
  register int itr;
  register int wss;

  while (run_core_mask[cid] == 0);
  if (cid == PRINT_CORE)
    printf("Start!\n");

  switch (cid)
  {
    case 0:
      values = values0;
      itr = 1;
      wss = 64 * BLK_SZ;
      break;
    case 1:
      values = values1;
      itr = 1;
      wss = 64 * BLK_SZ;
      break;
    case 2:
      values = values2;
      itr = 1;
      wss = 64 * BLK_SZ;
      break;
    case 3:
      values = values3;
      itr = 1;
      wss = 64 * BLK_SZ;
      break;
  }

  barrier(NC);
  reg_read32(WINDOW_SIZE);

  register uint64_t cycle1_reg = rdcycle();

  // This is the core of the benchmark
  for (int j = 0; j < itr; j++) {
    for (int i = 0; i < wss; i = i + BLK_SZ) {
      values[i];
    }
  }

  cycle2[cid] = rdcycle();
  reg_read32(WINDOW_SIZE);
  cycle1[cid] = cycle1_reg;

  barrier(NC);
  if (cid == PRINT_CORE) {
    for (int i = 0; i < NC_MAX; i++) {
      printf("C%d: cycle1: %lu, cycle2: %lu, diff: %lu\n", i, cycle1[i], cycle2[i], cycle2[i] - cycle1[i]);
    }
  }

  barrier(NC);
  exit(0);
}