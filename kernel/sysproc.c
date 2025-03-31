#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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


#ifdef LAB_PGTBL
int 
sys_pgaccess(void) 
{
    struct proc *p = myproc();
    uint64 start_va;
    int num_pages;
    uint64 result_buffer;
    uint64 mask = 0;
    
    // Lấy các tham số từ user space
    argaddr(0, &start_va);
    argint(1, &num_pages);
    argaddr(2, &result_buffer);

    // Kiểm tra số lượng trang hợp lệ (1-64)
    if (num_pages > 64)
        return -1;
    //Duyệt qua và kiểm tra PTE
    for (int i = 0; i < num_pages; i++) {
        uint64 va = start_va + i * PGSIZE;
        pte_t *pte = walk(p->pagetable, va, 0);  // Không cấp phát trang mới
        if (pte && (*pte & PTE_V)) {  // PTE hợp lệ
            if (*pte & PTE_A) {
                mask |= (1L << i);     // Đặt bit tương ứng
                *pte &= ~PTE_A;        // Xóa bit truy cập
            }
        }
    }

    // Gửi kết quả về user space
    if (copyout(p->pagetable, result_buffer, (char *)&mask, sizeof(mask)) < 0)
        return -1;

    return 0;

}
#endif

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

