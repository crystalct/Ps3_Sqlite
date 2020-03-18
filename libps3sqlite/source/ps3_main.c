#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <dirent.h>
#include <assert.h>
#include <fcntl.h>

#undef EMERGENCY_EXIT_THREAD

#include <netinet/in.h>
#include <net/net.h>
#include <net/netctl.h>


#include <sysmodule/sysmodule.h>
//#include <psl1ght/lv2.h>
//#include <psl1ght/lv2/spu.h>
//#include <lv2/process.h>
#include <lv2/system.h>
#include <sys/memory.h>
//#include <sys/file.h>
//#include <rtc.h>
#include "ps3.h"
//#include "threads.h"
#include "atomic.h"
#include "arch.h"
#include "main.h"
#include "fileaccess.h"

static uint64_t ticks_per_us;

static uint64_t mftb(void)
{
  uint64_t ret;
  asm volatile ("1: mftb %[tmp];       "
		"   cmpwi 7, %[tmp], 0;"
		"   beq-  7, 1b;       "
		: [tmp] "=r" (ret):: "cr7");
  return ret;
}

int64_t
arch_get_ts(void)
{
  return mftb() / ticks_per_us;
}


void
arch_get_random_bytes(void *ptr, size_t size)
{
  /*uint8_t tmp[0x10];
  while(size > 0) {
    size_t copy = MIN(size, sizeof(tmp));
    Lv2Syscall2(984, (u64)&tmp[0], sizeof(tmp));
    memcpy(ptr, tmp, copy);
    ptr += copy;
    size -= copy;*/
	sysGetRandomNumber(ptr, (u64) size);
  
}

void
hfree(void *ptr, size_t size)
{
  //Lv2Syscall1(349, (uint64_t)ptr);
  sysMemoryFree((uint64_t) ptr);
}

void *
halloc(size_t size)
{
#define ROUND_UP(p, round) ((p + (round) - 1) & ~((round) - 1))

  size_t allocsize = ROUND_UP(size, 64*1024);
  //u32 taddr;
  sys_mem_addr_t *taddr;
  taddr = NULL;
  //sysMemoryAllocate(size_t size,u64 flags,sys_mem_addr_t *alloc_addr)
  sysMemoryAllocate(allocsize,0x200, taddr);
  if(taddr == NULL)
  //if(Lv2Syscall3(348, allocsize, 0x200, (u64)&taddr))
    //panic("halloc(%d) failed", (int)size);
	printf("halloc(%d) failed\n", (int)size);
  return (void *)(uint64_t)taddr;
}
