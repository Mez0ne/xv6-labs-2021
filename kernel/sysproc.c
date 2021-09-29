#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"


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

// trace syscall by mask
uint64 sys_trace(void)
{
  int mask = 0;

  if (argint(0,&mask) < 0){
    return -1;
  }
  // printf("kernel_trace mask:%x\n", mask);
  myproc()->trace_mask = mask;

  return 0;
}

uint64 sys_sysinfo(void)
{
  uint64 uinfo = 0;
  uint64 free_cnt = 0;
  uint64 n_proc = 0;

  if(argaddr(0,&uinfo))
    return -1;

  pagetable_t pagetable = myproc()->pagetable;
  


  free_cnt = freecnt();
  // printf("Kernel free space: %d bytes\n",free_cnt);
  
  if(copyout(pagetable, (uint64)&((struct sysinfo*)uinfo)->freemem, (char*)&free_cnt, 8))
    return -1;

  n_proc = nproc();
  // printf("Kernel proc cnt: %d\n",n_proc);

  if(copyout(pagetable, (uint64)&((struct sysinfo*)uinfo)->nproc, (char*)&n_proc, 8))
    return -1;

  return 0;
}