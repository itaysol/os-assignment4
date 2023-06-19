#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"



struct {
  struct spinlock lock;
#define BUF_SIZE sizeof(uint8)
  char buff[BUF_SIZE];
  uint8 seed;
} random;

int write(int user_src, uint64 src, int n)
{
    if (n==1){
        char a;
        if(either_copyin(&a, user_src, src, 1) == -1)
            return -1;
        acquire(&random.lock);
        random.seed = a;
        release(&random.lock);
        return 1;
    }
    return -1;
}
int read(int user_dst, uint64 dst, int n)
{
  printf("inside_read\n");
  uint8 char_buffer;
  char buff;
  int i=0;
  acquire(&random.lock);
  for (i=0; i < n; i++){
    char_buffer = lfsr_char(random.seed);
    random.seed = char_buffer;
    buff = char_buffer;
    if (either_copyout(user_dst, dst ,&buff, 1)==-1){
      break;
    }
    dst++;
    }
    release(&random.lock);
    return i;
}
void
randominit(void)
{
  random.seed=0x2A;
  initlock(&random.lock,"random");
  devsw[RANDOM].write=write;
  devsw[RANDOM].read=read;
}
uint8 lfsr_char(uint8 lfsr){
    uint8 bit;
    bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 0x01;
    lfsr = (lfsr >> 1) | (bit << 7);
    return lfsr;
}