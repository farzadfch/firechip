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

uint8_t *dummy_ptr = 0x90000000;

#define START_DELAY 2000
#define START_DELAY_OFFSET 2000
uint64_t sync_cycle;
uint64_t end_cycle[N];

#define NB 4
#define MASTER_CORE 3

#define STRIDE_SMALL 64
#define ACCESS_MEM_SMALL(start_addr, end_addr) do {\
  for (int i = start_addr; i < end_addr; i += STRIDE_SMALL) \
    values[i] = 0xaa; \
} while (0)

// Typically used for thrashing LLC
// 0x40000, so it will wrap the LLC sets
#define STRIDE_BIG 0x40000
#define ACCESS_MEM_BIG(start_itr, end_itr, start_addr, end_addr) do {\
  for (int j = start_itr * STRIDE_BIG; j < end_itr * STRIDE_BIG; j += STRIDE_BIG) \
    ACCESS_MEM_SMALL(j + start_addr, j + end_addr); \
} while (0)

#define ITR_BIG_SUB 1
#define ITR_BIG_CR (ITR_BIG_SUB + 1)

void thread_entry(int cid, int nc)
{
  volatile uint8_t *values = dummy_ptr;

  if (cid == MASTER_CORE) {
    reg_write32(WINDOW_SIZE, 19-1);
    reg_write32(MAX(0), 1);
    //reg_write32(MAX(1), 1);
    //reg_write32(MAX(2), 1);
    //reg_write32(MAX(3), 1);
    reg_write32(MAX_WR(0), 1);
    reg_write32(DOMAIN_ID(0), 0);
    reg_write32(DOMAIN_ID(1), 0);
    reg_write32(DOMAIN_ID(2), 0);
    reg_write32(DOMAIN_ID(3), 0);
    //reg_write32(DOMAIN_ID(1), 1);
    //reg_write32(DOMAIN_ID(2), 2);
    //reg_write32(DOMAIN_ID(3), 3);
    reg_write32(ENABLE_MASTERS, 0xf);
    reg_write32(BW_SETTINGS, 1);
    //reg_write32(BW_SETTINGS, 3);
    //reg_write32(ENABLE_BW, 1);
    reg_write32(ENABLE_PRINT_LATENCY, 1);
    asm volatile ("fence");
  } 
/*  else {
    while(1);
  }
*/
/*  switch (cid)
  {
    case 0:
      ACCESS_MEM_BIG(0, 8, 0, 0x10000);
      break;
    case 1:
      ACCESS_MEM_BIG(0, 8, 0x10000, 0x20000);
      break;
    case 2:
      ACCESS_MEM_BIG(0, 8, 0x20000, 0x30000);
      break;
    case 3:
      ACCESS_MEM_SMALL(0x30000, 0x40000);
      break;
  }
*/

  switch (cid)
  {
    case 0:
      ACCESS_MEM_SMALL(0, 0x10000);
      break;
    case 1:
      ACCESS_MEM_SMALL(0x10000, 0x20000);
      break;
    case 2:
      ACCESS_MEM_SMALL(0x20000, 0x30000);
      break;
    case 3:
      ACCESS_MEM_SMALL(0x30000, 0x40000);
      break;
  }
 
  barrier(NB);

  // wait for START_DELAY cycles
  int delay_offest = 0;
  if (cid == MASTER_CORE) {
    sync_cycle = rdcycle();
    delay_offest = START_DELAY_OFFSET;
    //reg_write32(ENABLE_BW, 1);
  }
  barrier(NB);
  while (rdcycle() < sync_cycle + START_DELAY + delay_offest); 

  switch (cid)
  {
    case 0:
      for (int i = 0; i < ITR_BIG_CR; i++)
        ACCESS_MEM_SMALL(0, 0x10000);
      break;
    case 1:
      for (int i = 0; i < ITR_BIG_CR; i++)
        ACCESS_MEM_SMALL(0x10000, 0x20000);
      break;
    case 2:
      for (int i = 0; i < ITR_BIG_CR; i++)
        ACCESS_MEM_SMALL(0x20000, 0x30000);
      break;
    case 3:
      for (int i = 0; i < ITR_BIG_SUB; i++)
        ACCESS_MEM_SMALL(0x30000, 0x40000);
      break;
  }

/*  switch (cid)
  {
    case 0:
      ACCESS_MEM_BIG(8, 9, 0, 0x10000);
      break;
    case 1:
      ACCESS_MEM_BIG(8, 9, 0x10000, 0x20000);
      break;
    case 2:
      ACCESS_MEM_BIG(8, 9, 0x20000, 0x30000);
      break;
    case 3:
      for (int i = 0; i < 10; i++)
        ACCESS_MEM_SMALL(0x30000, 0x40000);
      break;
  }
*/
/*
  for (int j = 0; j < 32 * 0x40000; j += 0x40000)
    for (int i = j + 0x30000; i < j + 0x40000; i += STRIDE_SMALL)
      values[i] = 0xaa;
*/
  
  end_cycle[cid] = rdcycle();

  if (cid == MASTER_CORE) {
    reg_write32(ENABLE_PRINT_LATENCY, 0);
    reg_write32(ENABLE_BW, 0);
    asm volatile ("fence");
  }

  barrier(NB);

  if (cid == MASTER_CORE) {
    for (int i = 0; i < N; i++) {
      uint64_t start_cycle = sync_cycle + START_DELAY + (i == 3 ? START_DELAY_OFFSET : 0);
      if (end_cycle[i])
      	printf("C%d: s %lu e %lu d %lu\n", i, start_cycle, end_cycle[i], end_cycle[i] - start_cycle);
    }
  }

  barrier(NB);
  exit(0);
}

