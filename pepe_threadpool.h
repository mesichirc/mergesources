#ifndef PEPE_THREAD_POOL
#define PEPE_THREAD_POOL

#include "pepe_core.h"
#include <pthread.h>
#include <stdlib.h>
typedef struct Pepe_ThreadPool Pepe_ThreadPool;
typedef void (*Pepe_ThreadPoolTaskFunc)(Pepe_ThreadPool *pool, void * userdata);
typedef struct Pepe_SemaphoreImpl Pepe_Semaphore;
typedef struct Pepe_ThreadPoolTask Pepe_ThreadPoolTask;
typedef struct Pepe_TicketMutex Pepe_TicketMutex;
typedef struct Pepe_Thread Pepe_Thread;

void Pepe_TicketMutex_Begin(Pepe_TicketMutex *mutex);
void Pepe_TicketMutex_End(Pepe_TicketMutex *mutex);

struct Pepe_Thread {
	pthread_t thread;
};

struct Pepe_ThreadPoolTask {
  void                    *userdata;
  Pepe_ThreadPoolTaskFunc func;
  Pepe_ThreadPoolTask     *next;
};

struct Pepe_TicketMutex {
  u64 volatile Ticket;
  char _padding[64];
  u64 volatile Serving;
};

#define PEPE_THREAD_POOL_STOP 0
#define PEPE_THREAD_POOL_RUNNING 1
#define PEPE_THREAD_POOL_INVALID 2

struct Pepe_SemaphoreImpl {
  int               value;
  int               PipeFds[2];
  Pepe_TicketMutex  mutex;
};


struct Pepe_ThreadPool {
	Pepe_ThreadPoolTask   *tasksFreeList;
	Pepe_Thread 					*threads;
	Pepe_Semaphore				semaphore;	
	Pepe_TicketMutex			tasksMutex;
	Pepe_Arena						arena;
	u32 volatile				  state;
	u32 								  threadCount;
	Pepe_ThreadPoolTask   *firstTask;
	Pepe_ThreadPoolTask   *lastTask;
};

typedef void * (*Pepe_ThreadFunc)(void *arg);

int
Pepe_Semaphore_Init(Pepe_Semaphore *sem, int value)
{
  sem->value = value;
  sem->mutex.Serving = 0;
  sem->mutex.Ticket = 0;
  return pipe(sem->PipeFds); 
}

int
Pepe_Semaphore_Post(Pepe_Semaphore *sem)
{
  int result;
  Pepe_TicketMutex_Begin(&sem->mutex);

  result = write(sem->PipeFds[1], "x", 1);
  sem->value++;

  Pepe_TicketMutex_End(&sem->mutex);
  return result;
}

bool
Pepe_Semaphore_Wait(Pepe_Semaphore *sem)
{
  char buf;
  int result;
  for (;;) {
    Pepe_TicketMutex_Begin(&sem->mutex);
    if (sem->value > 0) {
      sem->value--;
      Pepe_TicketMutex_End(&sem->mutex);
      return true;
    }
    Pepe_TicketMutex_End(&sem->mutex);

    result = read(sem->PipeFds[0], &buf, 1);
    if (result != 1) {
      return false;
    } 
    return true;
  } 
}

bool
Pepe_Thread_Init(Pepe_Thread *thread, Pepe_ThreadFunc func, void *arg)
{
  return pthread_create(&thread->thread, nil, func, arg) == 0;
}

void
Pepe_Thread_Join(Pepe_Thread *thread)
{
  pthread_join(thread->thread, nil);
}

u64
Pepe_AtomicAddU64(u64 volatile *i, u64 count)
{
	return __atomic_fetch_add(i, count, __ATOMIC_SEQ_CST);
}

u32
Pepe_AtomicLoadU32(volatile u32 *ptr)
{
	return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

void 
Pepe_AtomicStoreU32(volatile u32 *ptr, u32 value)
{
	__atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
}

void
Pepe_TicketMutex_Begin(Pepe_TicketMutex *mutex)
{
	u64 Ticket;
	Ticket = Pepe_AtomicAddU64(&mutex->Ticket, 1);
	while (Ticket != __atomic_load_n(&mutex->Serving, __ATOMIC_SEQ_CST));
}

void
Pepe_TicketMutex_End(Pepe_TicketMutex *mutex)
{
	Pepe_AtomicAddU64(&mutex->Serving, 1);
}


void *
Pepe_ThreadPool_WorkerFunc(void *data)
{
  Pepe_ThreadPool *pool;
  Pepe_ThreadPoolTask *task;
  Pepe_ThreadPoolTaskFunc func;
  void *userdata;

  pool = (Pepe_ThreadPool *)data;
  while (Pepe_AtomicLoadU32(&pool->state) == PEPE_THREAD_POOL_RUNNING) {
    Pepe_Semaphore_Wait(&pool->semaphore);
    Pepe_TicketMutex_Begin(&pool->tasksMutex);

    task = pool->firstTask;
    printf("worker started\n");
    if (task != nil) {
      printf("task with %lu\n", (uptr)task);
      userdata = task->userdata;
      func = task->func;
      task->next = pool->tasksFreeList;
      pool->tasksFreeList = task;
      pool->firstTask = task->next;
      if (pool->firstTask == nil) {
        pool->lastTask = nil;
      }
      task->func = nil;
    }
    Pepe_TicketMutex_End(&pool->tasksMutex);
    if (func != nil) {
      printf("func found with %lu\n", (uptr)task);
      func(pool, userdata);
    } else {
      printf("func not found with %lu\n", (uptr)task); 
    }
  }

  return nil;
}

#define PEPE_THREAD_POOL_TASKS_COUNT 256

u64
Pepe_ThreadPool_RequiredMemory(u32 workers)
{
  return sizeof(Pepe_ThreadPoolTask) * PEPE_THREAD_POOL_TASKS_COUNT + sizeof(Pepe_Thread) * workers;
}

void
Pepe_ThreadPool_Init(Pepe_ThreadPool *pool, u32 workers, void *buffer, u32 bufferSize)
{
  u32 i, freeListCapacity;
  Pepe_ThreadPoolTask *freeTask;

  assert(pool);
  assert(workers > 0);

  
  Pepe_ArenaInit(&pool->arena, Pepe_SliceInit(buffer, bufferSize));

  pool->state = PEPE_THREAD_POOL_RUNNING;
  pool->threadCount = workers;
  memset(&pool->tasksMutex, 0, sizeof(Pepe_TicketMutex));

  freeListCapacity = PEPE_THREAD_POOL_TASKS_COUNT;
  pool->tasksFreeList = (Pepe_ThreadPoolTask *)Pepe_ArenaAllocAlign(&pool->arena, sizeof(Pepe_ThreadPoolTask) * (u64)freeListCapacity, PEPE_DEFAULT_ALIGNMENT);
  assert(pool->tasksFreeList);


  freeTask = pool->tasksFreeList;
  for (i = 0; i < freeListCapacity - 1; i++) {
    freeTask += 1;
    freeTask->func = nil;
    freeTask->userdata = nil;
    freeTask->next = freeTask + 1;
  }
  freeTask += 1;
  freeTask->func = nil;
  freeTask->next = nil;
  freeTask->userdata = nil;

  Pepe_Semaphore_Init(&pool->semaphore, 0);
  pool->firstTask = nil;
  pool->lastTask = nil;

  pool->threads = (Pepe_Thread *)Pepe_ArenaAllocAlign(&pool->arena, sizeof(Pepe_Thread), PEPE_DEFAULT_ALIGNMENT);
  if (!pool->threads) {
    pool->state = PEPE_THREAD_POOL_INVALID;
    return;
  }

  for (i = 0; i < workers; i++) {
    if (!Pepe_Thread_Init(pool->threads + i, Pepe_ThreadPool_WorkerFunc, (void *)pool)) {
      pool->state = PEPE_THREAD_POOL_INVALID;
      return;
    }
  }
}

void
Pepe_ThreadPool_Destroy(Pepe_ThreadPool *pool)
{
  u32 i;
  Pepe_AtomicStoreU32(&pool->state, PEPE_THREAD_POOL_STOP);
  Pepe_TicketMutex_Begin(&pool->tasksMutex);
  pool->firstTask = nil;
  pool->lastTask = nil;
  Pepe_TicketMutex_End(&pool->tasksMutex);
  // NOTE(sichirc): we need iterate 2 times, because of
  // first time we unlock all workers to complete execution
  // second time we join threads
  for (i = 0; i < pool->threadCount; i++) {
    Pepe_Semaphore_Post(&pool->semaphore);
  }

  for (i = 0; i < pool->threadCount; i++) {
    Pepe_Thread_Join(pool->threads + i);
  }
}


void
Pepe_ThreadPool_ScheduleWork(Pepe_ThreadPool *pool, Pepe_ThreadPoolTaskFunc func, void *userdata)
{
  Pepe_ThreadPoolTask *freeTask;
  assert(pool);
  assert(func);

  freeTask = nil;
  
  while (freeTask == nil) {
    Pepe_TicketMutex_Begin(&pool->tasksMutex);
    freeTask = pool->tasksFreeList;
    printf("schedule worker with number %d \n", freeTask == nil);
    if (freeTask != nil) {
      freeTask->func = func;
      freeTask->userdata = userdata;
      if (pool->firstTask == nil) {
        pool->firstTask = freeTask;
        pool->lastTask = freeTask;
      } else if (pool->firstTask != nil && pool->lastTask == pool->firstTask) {
        pool->firstTask->next = freeTask;
        pool->lastTask = freeTask;
      } else {
        pool->lastTask->next = freeTask;
      }

      pool->tasksFreeList = freeTask->next; 
    }
    Pepe_TicketMutex_End(&pool->tasksMutex);
  }
  assert(Pepe_Semaphore_Post(&pool->semaphore));
}

#endif
