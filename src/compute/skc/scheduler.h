/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 *
 */

#pragma once

//
// FIXME -- make the scheduler even more lightweight than it is.  Move
// to an idiom where the scheduled task brings its own state instead
// of relying on an internal table.  This will make it possible to
// reliably report the task's lifecycle and terminating state.
//

#include "types.h"

//
//
//

#define SKC_SCHEDULER_SCHEDULE(s,c,d) skc_scheduler_schedule(s,c,d,#c)

#ifndef NDEBUG

#include <stdio.h>

#define SKC_SCHEDULER_WAIT_WHILE(s,p)           \
  while (p) {                                   \
    fprintf(stderr,"WAITING ON: " #p "\n");     \
    skc_scheduler_wait(s);                      \
  }
#else
#define SKC_SCHEDULER_WAIT_WHILE(s,p)           \
  while (p) {                                   \
    skc_scheduler_wait(s);                      \
  }
#endif

//
//
//

#ifndef NDEBUG
#define SKC_CL_CB(s)  fprintf(stderr,"CB+ %s = %d\n",__func__,s)
#else
#include <stdio.h>
#define SKC_CL_CB(s)
#endif

//
//
//

#define SKC_SCHEDULER_COMMAND_INVALID  SKC_UINT_MAX

typedef skc_uint skc_scheduler_command_t;

typedef void (* skc_scheduler_command_pfn)(void * data);

//
//
//

struct skc_scheduler *
skc_scheduler_create(struct skc_runtime * const runtime, skc_uint const size);

void
skc_scheduler_dispose(struct skc_runtime   * const runtime,
                      struct skc_scheduler * const scheduler);


//
//
//

skc_scheduler_command_t
skc_scheduler_schedule(struct skc_scheduler    * const scheduler,
		       skc_scheduler_command_pfn const pfn,
		       void                    *       data,
                       char              const * const name);

//
//
//

skc_bool
skc_scheduler_yield(struct skc_scheduler * const scheduler);

void
skc_scheduler_wait(struct skc_scheduler * const scheduler);

void
skc_scheduler_wait_one(struct skc_scheduler * const scheduler);

//
// FIXME -- get rid of these
//

#if 0

skc_bool
skc_scheduler_wait_for(struct skc_scheduler  * const scheduler,
                       skc_scheduler_command_t const command);

void
skc_thread_sleep(skc_ulong const msecs);

#endif

//
//
//
