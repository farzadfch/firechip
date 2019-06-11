#include <stdint.h>

#include "util.h"
#include "mmio.h"


#define BW_REG_BASE 0x20000000L
#define WINDOW_SIZE (BW_REG_BASE)
#define MAX_XACTION(i) (BW_REG_BASE + (i+1) * 0x4)

#define NC_MAX 4
// 64 byte blocks
#define BLK_SZ 8
// L1 cache size * 4, L1 = 16k
#define WSS_MAX (16 * 1024 / BLK_SZ * 4)

int run_core_mask[NC_MAX] = {0, 0, 0, 1};
#define NC 1
#define PRINT_CORE 3

volatile uint64_t values0[WSS_MAX];
volatile uint64_t values1[WSS_MAX];
volatile uint64_t values2[WSS_MAX];
volatile uint64_t values3[WSS_MAX];
uint64_t cycle1[NC_MAX];
uint64_t cycle2[NC_MAX];

void bw_read(volatile uint64_t *values, int itr, int wss)
{
  for (int j = 0; j < itr; j++) {
    for (int i = 0; i < wss; i = i + 2 * BLK_SZ) {
      values[i];
      values[i + BLK_SZ];
      //values[i + 2 * BLK_SZ];
      //values[i + 3 * BLK_SZ];
    }
  }
}

void bw_read_l1(volatile uint64_t *values, int itr)
{
  for (int i = 0; i < itr; i++) {
    values[0];
    values[1];
    values[2];
    values[3];
    values[4];
    values[5];
    values[6];
    values[7];
    values[8];
    values[9];
  }
}

void bw_write(volatile uint64_t *values, int itr, int wss)
{
  for (int j = 0; j < itr; j++) {
    for (int i = 0; i < wss; i = i + 2 * BLK_SZ) {
      values[i] = 0xaa;
      values[i + BLK_SZ] = 0xaa;
      //values[i + 2 * BLK_SZ] = 0xaa;
      //values[i + 3 * BLK_SZ] = 0xaa;
    }
  }
}

void thread_entry(int cid, int nc)
{
  register uint64_t cycle1_reg;
  register uint64_t cycle2_reg;

  while (run_core_mask[cid] == 0);
  if (cid == PRINT_CORE)
    printf("Start\n");

  switch (cid)
  {
    case 0:
      bw_read(values0, 1, WSS_MAX);
      break;
    case 1:
      bw_read(values1, 1, WSS_MAX);
      break;
    case 2:
      bw_read(values2, 1, WSS_MAX);
      break;
    case 3:
      bw_write(values3, 1, WSS_MAX);
      //bw_read_l1(values3, 1);
      break;
  }

  /*barrier(NC);
  if (cid == PRINT_CORE)
    printf("Init\n");
  */

  barrier(NC);
  reg_read32(WINDOW_SIZE);
  asm volatile ("fence");

  switch (cid)
  {
    case 0:
      bw_read(values0, 10000, WSS_MAX);
      break;
    case 1:
      bw_read(values1, 10000, WSS_MAX);
      break;
    case 2:
      bw_read(values2, 10000, WSS_MAX);
      break;
    case 3:
      cycle1_reg = rdcycle();
      bw_write(values3, 1, WSS_MAX);
      //bw_read_l1(values3, 100);
      cycle2_reg = rdcycle();
      break;
  }

  reg_read32(WINDOW_SIZE);
  cycle1[cid] = cycle1_reg;
  cycle2[cid] = cycle2_reg;

  //barrier(NC);
  if (cid == PRINT_CORE) {
    for (int i = 3; i < NC_MAX; i++) {
      printf("C%d: cy1:%lu cy2:%lu d:%lu\n", i, cycle1[i], cycle2[i], cycle2[i] - cycle1[i]);
    }
    exit(0);
  }

  while(1);
}
