#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 va;
  argaddr(0, &va);
  
  int num = 0;
  argint(1,&num);

  if(num == 0)
    return 0;

  if(num > 0x400*8) // number too big
    return -1; 
  
  char buffer[0x400]={};
  memset(buffer, 0, 0x400);

  // user address to a buffer to store the results into a bitmask
  // bitmask: one bit per page 
  // the first page corresponds to the least significant bit
  uint64 output;
  argaddr(2, &output);

  // printf("pgaccess: %p, cnt:%d, dst:%p, cnt:%d\n",va, num, output, ((num-1) / 8)+1);
  pte_t * ppte = 0;


  pagetable_t pagetable = myproc()->pagetable;

  // vmprint(pagetable);

  for (int i = 0; i < num; i++)
  {
    ppte = walk(pagetable, va, 0);
    int accessed = ((*ppte) & PTE_A) != 0;

    // printf("va:%p, pte:%p, a:%d\n", va, *ppte, accessed);

    // write to bitmask
    int n = i/8;
    int pos = i%8;
    buffer[n] |= (accessed << pos);

    // clear PTE_A
    if(accessed)
      *ppte &=  ~PTE_A;

    va += 0x1000;
  }

  // printf("Result: \n");
  // for (int i = 0; i < (num-1)/8 + 1; i++)
  // {
  //   printf("%x:",buffer[i]);
  // }
  // printf("\n");
  
  if(copyout(pagetable, output, buffer, ((num-1) / 8)+1)){
    printf("pgaccess: copyout failed\n");
    return -1;
  }
  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
