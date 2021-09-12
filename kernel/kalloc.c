// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  char name[16];
} kmem[NCPU];


void
kinit()
{
  for (int i = 0;i < NCPU;i++){
    snprintf(kmem[i].name,sizeof(kmem[i].name),"kmem%d",i);
    initlock(&(kmem[NCPU].lock),kmem[i].name);
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  push_off();
  int ncpu = cpuid();

  acquire(&kmem[ncpu].lock);
  r->next = kmem[ncpu].freelist;
  kmem[ncpu].freelist = r;
  release(&kmem[ncpu].lock);
  pop_off();
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int ncpu = cpuid();

  acquire(&kmem[ncpu].lock);  // 这边先获得锁
  r = kmem[ncpu].freelist;
  if(r) { // fast path :如果还有freelist,就直接拿来
    kmem[ncpu].freelist = r->next;  
  } 
  release(&kmem[ncpu].lock);  //　这里就释放了这个锁

  if (!r) { // slow path:如果没有freelist,那么steal
    // steal other cpu's freelist
    for (int i = 0; i < NCPU; i++) {
      if (i == ncpu) continue;
      acquire(&kmem[i].lock);
      r = kmem[i].freelist;     // 偷到另一个kmem freelist
      if (r) {  // 如果偷到了
        kmem[i].freelist = r->next; //更新被偷的freelist
        release(&kmem[i].lock); 
        break;
      }
      release(&kmem[i].lock);
    }
  }
  pop_off();
  
  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}