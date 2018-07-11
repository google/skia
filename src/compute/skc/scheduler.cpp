/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

//
// There is either no or limited support for C11 atomics in the C
// compilers we're targeting so, for now, implement this in C++11
//

//
// FIXME -- get rid of the extent and have supplicants bring their own
// task structure with a known base structure
//

extern "C" {

#include "scheduler.h"
#include "runtime_cl_12.h" // FIXME -- all allocations are extent structures

}

//
// SUPER-IMPORTANT:
//
// THIS LOW-LEVEL SCHEDULER ASSUMES THERE ARE A MAXIMUM NUMBER OF
// PIPELINE GRIDS ACTIVE AT ANY TIME
//
// THIS IS A SAFE INVARIANT BECAUSE WE CONTROL THE VARIOUS POOL SIZE
// KNOBS AT CONTEXT CONFIGURATION TIME
//
// TRANSLATION: DO NOT LARD UP THIS IMPLEMENTATION WITH WELL-INTENDED
// BUT ENTIRELY UNNECESSARY LOGIC
//

#include <mutex>
#include <condition_variable>

//
// GRID STATES
//

typedef enum skc_scheduler_command_state {

  SKC_SCHEDULER_COMMAND_STATE_READY,
  SKC_SCHEDULER_COMMAND_STATE_WAITING,
  SKC_SCHEDULER_COMMAND_STATE_EXECUTING,
  SKC_SCHEDULER_COMMAND_STATE_COMPLETED,

  SKC_SCHEDULER_COMMAND_STATE_COUNT

} skc_scheduler_command_state;

//
//
//

struct skc_scheduler_command
{
  void *                      data;
  skc_scheduler_command_pfn   pfn;
  skc_scheduler_command_state state;
  char const *                name;
};

#if 0
struct skc_scheduler_command
{
  union {
    struct scheduler             * scheduler;
    struct skc_scheduler_command * next;
  };
  skc_scheduler_command_pfn        pfn;
};
#endif

//
//
//

struct skc_scheduler
{
  //
  // FIXME -- consider adding a backpointer to the runtime or other
  // critical state
  //

  struct skc_scheduler_command * extent;

  struct {
    std::mutex                   mutex;

    skc_ushort                 * indices;
    skc_uint                     rem;
  } available;

  struct {
    std::mutex                   mutex;
    std::condition_variable      condvar;

    skc_ushort                 * indices;
    skc_uint                     size;
    skc_uint                     head;
    skc_uint                     tail;
  } waiting;
};

//
//
//

#if 1
#define SKC_SCHEDULER_EXECUTE(sc) \
  sc->pfn(sc->data)
#else
#define SKC_SCHEDULER_EXECUTE(sc)               \
  fprintf(stderr,"EXECUTE+ %s\n",sc->name);     \
  sc->pfn(sc->data);                            \
  fprintf(stderr,"EXECUTE- %s\n",sc->name);
#endif

//
//
//

struct skc_scheduler *
skc_scheduler_create(struct skc_runtime * const runtime, skc_uint const size)
{
  // allocate scheduler
  struct skc_scheduler * scheduler = (struct skc_scheduler*)
    skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*scheduler));

  // execute various std:atomic constructors
  new (scheduler) skc_scheduler();

  // initialize members
  scheduler->extent            = (struct skc_scheduler_command*)
    skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,(sizeof(*scheduler->extent) * size));

  scheduler->available.indices = (skc_ushort*)
    skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*scheduler->available.indices) * size);

  scheduler->available.rem     = size;

  scheduler->waiting.indices   = (skc_ushort*)
    skc_runtime_host_perm_alloc(runtime,SKC_MEM_FLAGS_READ_WRITE,sizeof(*scheduler->available.indices) * (size + 1));

  scheduler->waiting.size      = size + 1; // the ring has an extra slot
  scheduler->waiting.head      = 0;
  scheduler->waiting.tail      = 0;

  for (skc_uint ii=0; ii<size; ii++)
    scheduler->available.indices[ii] = ii;

  return scheduler;
}

void
skc_scheduler_dispose(struct skc_runtime   * const runtime,
                      struct skc_scheduler * const scheduler)
{
  // free members
  skc_runtime_host_perm_free(runtime,scheduler->waiting.indices);
  skc_runtime_host_perm_free(runtime,scheduler->available.indices);
  skc_runtime_host_perm_free(runtime,scheduler->extent);

  // execute various std:atomic destructors
  scheduler->~skc_scheduler();

  // free struct
  skc_runtime_host_perm_free(runtime,scheduler);
}

//
//
//

static
skc_scheduler_command_t
skc_scheduler_acquire(struct skc_scheduler * const scheduler,
                      skc_scheduler_command_pfn    pfn,
                      void                       * data,
                      char const                 * name)
{
  skc_scheduler_command_t command = SKC_SCHEDULER_COMMAND_INVALID;

  {
    // mutex lock
    std::lock_guard<std::mutex> lock(scheduler->available.mutex);

    // get first available index
    if (scheduler->available.rem > 0)
      command = scheduler->available.indices[--scheduler->available.rem];

    // mutex unlock
  }

  if (command != SKC_SCHEDULER_COMMAND_INVALID)
    {
      // initialize command
      struct skc_scheduler_command * const sc = scheduler->extent + command;

      sc->pfn   = pfn;
      sc->data  = data;
      sc->name  = name;
      sc->state = SKC_SCHEDULER_COMMAND_STATE_READY;
    }

  // return command handle
  return command;
}

//
//
//

static
void
skc_scheduler_release(struct skc_scheduler  * const scheduler,
                      skc_scheduler_command_t const command)
{
  // mutex lock
  std::lock_guard<std::mutex> lock(scheduler->available.mutex);

  // get first available index
  scheduler->available.indices[scheduler->available.rem++] = command;

  // mutex unlock
}

//
//
//

static
void
skc_scheduler_append(struct skc_scheduler  * const scheduler,
                     skc_scheduler_command_t const command)
{
  scheduler->extent[command].state = SKC_SCHEDULER_COMMAND_STATE_WAITING;

  {
    // mutex unique lock (locks on construction)
    std::unique_lock<std::mutex> lock(scheduler->waiting.mutex);

    // note that we guarantee there is always room to store the command

    // append index to ring
    scheduler->waiting.indices[scheduler->waiting.tail] = command;

    // update last
    if (++scheduler->waiting.tail == scheduler->waiting.size)
      scheduler->waiting.tail = 0;

    // mutex unlock
  }

  // signal condvar
  scheduler->waiting.condvar.notify_one();
}

//
//
//

skc_scheduler_command_t
skc_scheduler_schedule(struct skc_scheduler    * const scheduler,
                       skc_scheduler_command_pfn const pfn,
                       void                    *       data,
                       char              const * const name)
{
  while (true)
    {
      skc_scheduler_command_t const command = skc_scheduler_acquire(scheduler,pfn,data,name);

      if (command != SKC_SCHEDULER_COMMAND_INVALID)
        {
          skc_scheduler_append(scheduler,command);

          return command;
        }
      else
        {
          skc_scheduler_wait(scheduler);
        }
    }
}

//
// try to pop but don't wait
//

static
void
skc_scheduler_pop(struct skc_scheduler    * const scheduler,
                  skc_scheduler_command_t * const command)
{
  *command = SKC_SCHEDULER_COMMAND_INVALID;

  // mutex lock
  std::unique_lock<std::mutex> lock(scheduler->waiting.mutex);

  if (scheduler->waiting.head != scheduler->waiting.tail)
    {
      // get index at first
      *command = scheduler->waiting.indices[scheduler->waiting.head];

      // update first
      if (++scheduler->waiting.head == scheduler->waiting.size)
        scheduler->waiting.head = 0;
    }

  // mutex unlock
}

static
void
skc_scheduler_pop_wait(struct skc_scheduler    * const scheduler,
                       skc_scheduler_command_t * const command)
{
  // mutex unique lock -- locks on construction
  std::unique_lock<std::mutex> lock(scheduler->waiting.mutex);

  // wait for command
  scheduler->waiting.condvar.wait(lock,[scheduler] {
      return scheduler->waiting.head != scheduler->waiting.tail;
    });

  // get index at first
  *command = scheduler->waiting.indices[scheduler->waiting.head];

  // update first
  if (++scheduler->waiting.head == scheduler->waiting.size)
    scheduler->waiting.head = 0;

  // mutex unlock
}

//
//
//

static
void
skc_scheduler_command_execute(struct skc_scheduler_command * const sc)
{
  sc->state = SKC_SCHEDULER_COMMAND_STATE_EXECUTING;

  SKC_SCHEDULER_EXECUTE(sc);

  sc->state = SKC_SCHEDULER_COMMAND_STATE_COMPLETED;
}

static
void
skc_scheduler_execute(struct skc_scheduler  * const scheduler,
                      skc_scheduler_command_t const command)

{
  // execute
  skc_scheduler_command_execute(scheduler->extent + command);

  // release
  skc_scheduler_release(scheduler,command);
}

//
// drain the scheduler
//

skc_bool
skc_scheduler_yield(struct skc_scheduler * const scheduler) // wait for 0 or more completed grids
{
  // fprintf(stderr,"YIELD+\n");

  while (true)
    {
      skc_scheduler_command_t command;

      skc_scheduler_pop(scheduler,&command);

      if (command == SKC_SCHEDULER_COMMAND_INVALID) {
        // fprintf(stderr,"YIELD!\n");
        return false;
      }

      // otherwise execute the completion record
      skc_scheduler_execute(scheduler,command);
    }

  // fprintf(stderr,"YIELD-\n");

  return true;
}

//
// wait for at least one grid to complete
//

void
skc_scheduler_wait(struct skc_scheduler * const scheduler)
{
  // fprintf(stderr,"WAIT+\n");

  skc_scheduler_command_t command;

  // wait for a completion record
  skc_scheduler_pop_wait(scheduler,&command);

  // execute the completion record
  skc_scheduler_execute(scheduler,command);

  // process remaining
  skc_scheduler_yield(scheduler);

  // fprintf(stderr,"WAIT-\n");
}

//
// wait for one grid to complete
//

void
skc_scheduler_wait_one(struct skc_scheduler * const scheduler)
{
  // fprintf(stderr,"WAIT1+\n");

  skc_scheduler_command_t command;

  // wait for a completion record
  skc_scheduler_pop_wait(scheduler,&command);

  // execute the completion record
  skc_scheduler_execute(scheduler,command);

  // fprintf(stderr,"WAIT1-\n");
}

//
//
//

#if 0

//
// wait for a specific grid to complete
//
//   true  : success
//   false : command wasn't started
//
// FIXME -- get rid of this idiom
//

skc_bool
skc_scheduler_wait_for(struct skc_scheduler  * const scheduler,
                       skc_scheduler_command_t const command)
{
  struct skc_scheduler_command * const sc = scheduler->extent + command;

  // command not started
  if (sc->state == SKC_SCHEDULER_COMMAND_STATE_READY)
    return false; // SKC_ERR_COMMAND_NOT_STARTED;

  // command is already complete
  if (sc->state == SKC_SCHEDULER_COMMAND_STATE_COMPLETED)
    return true; // SKC_ERR_SUCCESS;

  // force wip grids to start
  // skc_grid_force(grid_wait_for);

  // otherwise, wait!
  while (true)
    {
      skc_scheduler_command_t next;

      // wait for a completion record
      skc_scheduler_pop_wait(scheduler,&next);

      // execute the completion record
      skc_scheduler_execute(scheduler,next);

      // return if this was a match
      if (next == command)
        return true; // SKC_ERR_SUCCESS;
    }
}

//
//
//

void
skc_thread_sleep(skc_ulong const msecs)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(msecs));
}

#endif

//
//
//
