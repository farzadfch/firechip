#include <stdint.h>

#include "util.h"
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

#define NC_MAX 4
// 64 byte blocks
#define BLK_SZ 8
// L1 cache size * 4, L1 = 16k
#define WSS_MAX (16 * 1024 / BLK_SZ * 4)

#define NC 4
#define PRINT_CORE 0
int run_core_mask[NC_MAX] = {1, 1, 1, 1};


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
  if (cid == PRINT_CORE) {
    printf("Start\n");
    reg_write32(ENABLE_PERF, 1 << 1);
    asm volatile ("fence");
  }

  /*switch (cid)
  {
    case 0:
      bw_write(values0, 1, WSS_MAX);
      //bw_read_l1(values3, 1);
      break;
    case 1:
      bw_read(values1, 1, WSS_MAX);
      break;
    case 2:
      bw_read(values2, 1, WSS_MAX);
      break;
    case 3:
      bw_read(values3, 1, WSS_MAX);
      break;
  }*/

  /*barrier(NC);
  if (cid == PRINT_CORE)
    printf("Init\n");
  */

  /*barrier(NC);
  reg_read32(WINDOW_SIZE);
  asm volatile ("fence");
  */

  switch (cid)
  {
    case 0:
      bw_write(values0, 1, WSS_MAX);
      cycle1_reg = rdcycle();
      bw_write(values0, 1, WSS_MAX);
      //bw_read_l1(values3, 100);
      cycle2_reg = rdcycle();
      break;
    case 1:
      bw_read(values1, 10000, WSS_MAX);
      break;
    case 2:
      bw_read(values2, 10000, WSS_MAX);
      break;
    case 3:
      bw_read(values3, 10000, WSS_MAX);
      break;
  }

  reg_write32(ENABLE_PERF, 0);
  //reg_read32(WINDOW_SIZE);
  cycle1[cid] = cycle1_reg;
  cycle2[cid] = cycle2_reg;

  //barrier(NC);
  if (cid == PRINT_CORE) {
    for (int i = 0; i < NC_MAX; i++) {
      printf("C%d: cy1:%lu cy2:%lu d:%lu\n", i, cycle1[i], cycle2[i], cycle2[i] - cycle1[i]);
    }
    exit(0);
  }

  while(1);
}
