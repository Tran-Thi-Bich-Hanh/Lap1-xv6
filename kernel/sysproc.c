#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "kernel/ptree.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
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

uint64
sys_ptree(void)
{
  uint64 buf_addr;
  int max;

  // Gọi riêng lẻ vì argaddr / argint có kiểu trả về là void trong phiên bản xv6 này
  argaddr(0, &buf_addr);
  argint(1, &max);

  if(max <= 0)
    return -1;

  // Cấp phát một trang bộ nhớ tạm trong kernel để chứa danh sách tiến trình
  struct ptreeinfo *infop = (struct ptreeinfo *)kalloc();
  if(infop == 0)
    return -1;

  int count = 0;
  struct proc *p;

  extern struct proc proc[NPROC];

  // Duyệt bảng tiến trình
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state != UNUSED) {
      if(count < max) {
        infop[count].pid = p->pid;
        infop[count].ppid = p->parent ? p->parent->pid : 0;
        infop[count].state = p->state;
        infop[count].memsize = p->sz;
        safestrcpy(infop[count].name, p->name, sizeof(infop[count].name));
        count++;
      }
    }
    release(&p->lock);
  }

  // Sao chép dữ liệu ra user space bằng copyout
  struct proc *my_p = myproc();
  if(copyout(my_p->pagetable, buf_addr, (char *)infop, count * sizeof(struct ptreeinfo)) < 0) {
    kfree((void *)infop);
    return -1;
  }

  kfree((void *)infop);
  return count;
}

uint64
sys_trace(void)
{
  int mask;
  argint(0, &mask);
  myproc()->mask = mask;
  return 0;
}