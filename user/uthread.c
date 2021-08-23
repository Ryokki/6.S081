#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/* Possible states of a thread: */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  4

struct thread_context{
  uint64     ra;
  uint64     sp;
  uint64     fp;  // frame pointer:记录当前栈顶. (s0)
  uint64     s1;
  uint64     s2;
  uint64     s3;
  uint64     s4;
  uint64     s5;
  uint64     s6;
  uint64     s7;
  uint64     s8;
  uint64     s9;
  uint64     s10;
  uint64     s11;
};

struct thread { // thread私有信息:栈和状态
  char       stack[STACK_SIZE]; /* the thread's stack */
  int        state;             /* FREE, RUNNING, RUNNABLE */
  struct thread_context context;

};
struct thread all_thread[MAX_THREAD]; // 4个线程的数组
struct thread *current_thread;  // 当前在执行的线程
extern void thread_switch(uint64, uint64);  // 线程调度函数
              
void 
thread_init(void) // 初始化线程
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule().  it needs a stack so that the first thread_switch() can
  // save thread 0's state.  thread_schedule() won't run the main thread ever
  // again, because its state is set to RUNNING, and thread_schedule() selects
  // a RUNNABLE thread.
  current_thread = &all_thread[0]; 
  current_thread->state = RUNNING;
  // 第一个线程是main函数,这个将current_thread变成all_thread[0]
}

void 
thread_schedule(void) // 线程调度
{
  // printf("scheduler\n");
  struct thread *t, *next_thread;

  /* Find another runnable thread. */
  next_thread = 0;
  t = current_thread + 1; // 下一个线程
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD)
      t = all_thread; // 如果当前已经是最后一个线程了,那么t=&all_thread[0]
    if(t->state == RUNNABLE) {
      next_thread = t;  // 如果t的状态是RUNNABLE的话,那么就找到了, nexe_thread就是准备运行的线程
      break;
    }
    t = t + 1;  
  }

  if (next_thread == 0) { // 如果没有RUNNABLE的..
    printf("thread_schedule: no runnable threads\n");
    exit(-1);
  }

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    /* YOUR CODE HERE
     * Invoke thread_switch to switch from t to next_thread:
     * thread_switch(??, ??);
     */
    // printf("%d -> %d\n",t-all_thread,current_thread-all_thread);
    thread_switch((uint64)&t->context,(uint64)&current_thread->context);  // 仅仅是恢复到下一个context,
    // printf("after switch\n");
    // 保存
  } else
    next_thread = 0;
  // printf("scheduler end\n");
}

void 
thread_create(void (*func)()) // 创建线程,指定这个函数
{
  struct thread *t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;
  }
  // 这里是假定一定能有FREE的t吗.
  t->state = RUNNABLE;
  // YOUR CODE HERE
  t->context.sp  = (uint64)&(t->stack[STACK_SIZE-1]);
  t->context.fp  = (uint64)&(t->stack[STACK_SIZE-1]);
  t->context.ra  = (uint64)(func);
}

void 
thread_yield(void)
{
  // printf("thread_yield\n");
  // printf("state:%d,%d,%d,%d\n",all_thread[0].state,all_thread[1].state,all_thread[2].state,all_thread[3].state);
  current_thread->state = RUNNABLE; //改成runnable,然后重新调度.
  thread_schedule();
}

volatile int a_started, b_started, c_started;
volatile int a_n, b_n, c_n;

void 
thread_a(void)
{
  int i;
  printf("thread_a started\n");
  a_started = 1;
  while(b_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_a %d\n", i);
    a_n += 1;
    thread_yield();
  }
  printf("thread_a: exit after %d\n", a_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_b(void)
{
  int i;
  printf("thread_b started\n");
  b_started = 1;
  while(a_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_b %d\n", i);
    b_n += 1;
    thread_yield();
  }
  printf("thread_b: exit after %d\n", b_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_c(void)
{
  int i;
  printf("thread_c started\n");
  c_started = 1;
  while(a_started == 0 || b_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_c %d\n", i);
    c_n += 1;
    thread_yield();
  }
  printf("thread_c: exit after %d\n", c_n);

  current_thread->state = FREE;
  thread_schedule();
}

int 
main(int argc, char *argv[]) 
{
  a_started = b_started = c_started = 0;
  a_n = b_n = c_n = 0;
  thread_init();    // 初始化:指定thread[0]为current_thread
  thread_create(thread_a);
  thread_create(thread_b);
  thread_create(thread_c);
  thread_schedule();
  exit(0);
}
