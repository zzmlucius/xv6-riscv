#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  struct proc *p = myproc();
  int n;

  argint(0, &n);

  uint64 oldsz = p->sz;

  if(n < 0) {
    if (growproc(n) == -1) {
      return -1;
    }
  }

  else if (p->sz + n >= USYSCALL) {
    printk("sys_sbrk : Grow out of range");
    return -1;
  }

  else p->sz += n;
  // if (growproc(n) < 0)
  //   return -1;

  return oldsz;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
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

  argint(0, &pid);
  return kkill(pid);
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

uint64
sys_trace(void) // sys_trace implementation
{
  // 只有当前进程会访问 p->mask,所以不用lock
  int mask;
  
  // RISC-V ABI 规定 一开始user.h/trace()第一个参数放在a0寄存器
  argint(0, &mask); 
  
  struct proc *p = myproc();
  p->mask = mask;
  return 0;
}

uint64 
sys_info(void) { // 遍历freelist和proc[NPROC]数组
  struct sysinfo kif;
  uint64 ifp;
  
  argaddr(0, &ifp); // 拿过来的用户态的地址
  
  kif.freemem  = freemem();
  kif.nproc    = nproc();

  if(copyout(myproc() -> pagetable, ifp, (char*)&kif, sizeof(kif)) < 0)
    return -1;

  return 0;
}

uint64
sys_shutdown(void) {
  volatile uint32* sdreg = (uint32*)SHUTDOWN_VA;
  *sdreg = (uint32)SBIT;
  return 0;
}

uint64
sys_pgaccess(void) {
  struct proc *p = myproc();
  uint64 va;
  int npages;
  uint64 ua;

  argaddr(0, &va);
  argint(1, &npages);
  argaddr(2, &ua);

  uint buf = 0; // 存掩码

  if(npages > 32 || npages < 0)
    return -1;

  if(va % PGSIZE)
    va -= va % PGSIZE; // 页对齐

  if(va + npages * PGSIZE > MAXVA) 
    return -1;
  
  for(int i = 0;i < npages;i++) {
    if(va < MAXVA) {           // 检查合法性
      uint64 *pte = 0;
      if((pte = walk(p->pagetable, va, 0)) == 0) {
        va += PGSIZE;
        continue;
      }

      if(*pte & PTE_V) { // PTE_A位 = 1 记录并消除标记
        if(*pte & PTE_A) {
          buf += (1U << i);
          *pte = *pte & (~PTE_A);
        }
      }

      va += PGSIZE;
    }

    else
      return -1;
  }

  // 将buf中的结果传到用户
  if(copyout(myproc() -> pagetable, ua, (char*)(&buf), sizeof(buf)) < 0)
    return -1;
  
  return 0;
}

uint64
sys_sigalarm(void) {
  struct proc *p = myproc();

  argint(0, &p->interval);
  argaddr(1, &p->ttrhandler);
  
  p->timeintrs = 0;

  return 0;
}

uint64
sys_sigreturn(void) {
  struct proc *p = myproc();
  memmove(p->trapframe, p->ttr_trapframe, sizeof(struct trapframe));
  p->handling = 0;
  return p->trapframe->a0; // 接下来会trampoline/userret 会直接trampoline的a0, 不能修改
}
