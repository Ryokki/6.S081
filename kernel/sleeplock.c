// Sleeping locks

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"

void
initsleeplock(struct sleeplock *lk, char *name)
{
  initlock(&lk->lk, "sleep lock"); 
  lk->name = name;
  lk->locked = 0;
  lk->pid = 0;
}

void
acquiresleep(struct sleeplock *lk)
{
  acquire(&lk->lk); //获得自旋锁
  while (lk->locked) {  //如果已经被锁了，那么sleep
    sleep(lk, &lk->lk);
  }
  // 此时获得这把睡眠锁
  lk->locked = 1;   // 上锁
  lk->pid = myproc()->pid;  //修改这把锁的pid
  release(&lk->lk); //释放自旋锁
}

void
releasesleep(struct sleeplock *lk)
{
  acquire(&lk->lk); //获得自旋锁
  lk->locked = 0; //locked置0
  lk->pid = 0;  //pid置0
  wakeup(lk); //唤醒这把睡眠锁
  release(&lk->lk); //释放自旋锁
}

int
holdingsleep(struct sleeplock *lk)
{
  int r;
  
  acquire(&lk->lk);
  r = lk->locked && (lk->pid == myproc()->pid);
  release(&lk->lk);
  return r;
}



