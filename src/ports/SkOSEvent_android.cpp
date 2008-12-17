/* libs/graphics/ports/SkOSEvent_android.cpp
**
** Copyright 2006, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkEvent.h"
#include "utils/threads.h"
#include <stdio.h>

using namespace android;

Mutex gEventQMutex;
Condition gEventQCondition;

void SkEvent::SignalNonEmptyQueue()
{
    gEventQCondition.broadcast();
}

///////////////////////////////////////////////////////////////////

#ifdef FMS_ARCH_ANDROID_ARM

// don't have pthreads.h, and therefore no timedwait(), so we punt for the demo

void SkEvent::SignalQueueTimer(SkMSec delay)
{
}

void SkEvent_start_timer_thread()
{
}

void SkEvent_stop_timer_thread()
{
}

#else

#include <pthread.h>
#include <errno.h>

static pthread_t        gTimerThread;
static pthread_mutex_t  gTimerMutex;
static pthread_cond_t   gTimerCond;
static timespec         gTimeSpec;

static void* timer_event_thread_proc(void*)
{
    for (;;)
    {
        int status;
        
        pthread_mutex_lock(&gTimerMutex);

        timespec spec = gTimeSpec;
        // mark our global to be zero
        // so we don't call timedwait again on a stale value
        gTimeSpec.tv_sec = 0;
        gTimeSpec.tv_nsec = 0;

        if (spec.tv_sec == 0 && spec.tv_nsec == 0)
            status = pthread_cond_wait(&gTimerCond, &gTimerMutex);
        else
            status = pthread_cond_timedwait(&gTimerCond, &gTimerMutex, &spec);
        
        if (status == 0)    // someone signaled us with a new time
        {
            pthread_mutex_unlock(&gTimerMutex);
        }
        else
        {
            SkASSERT(status == ETIMEDOUT);  // no need to unlock the mutex (its unlocked)
            // this is the payoff. Signal the event queue to wake up
            // and also check the delay-queue
            gEventQCondition.broadcast();
        }
    }
    return 0;
}

#define kThousand   (1000)
#define kMillion    (kThousand * kThousand)
#define kBillion    (kThousand * kThousand * kThousand)

void SkEvent::SignalQueueTimer(SkMSec delay)
{
    pthread_mutex_lock(&gTimerMutex);

    if (delay)
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        // normalize tv
        if (tv.tv_usec >= kMillion)
        {
            tv.tv_sec += tv.tv_usec / kMillion;
            tv.tv_usec %= kMillion;
        }

        // add tv + delay, scale each up to land on nanoseconds
        gTimeSpec.tv_nsec   = (tv.tv_usec + (delay % kThousand) * kThousand) * kThousand;
        gTimeSpec.tv_sec    = (tv.tv_sec + (delay / kThousand) * kThousand) * kThousand;
        
        // check for overflow in nsec
        if ((unsigned long)gTimeSpec.tv_nsec >= kBillion)
        {
            gTimeSpec.tv_nsec -= kBillion;
            gTimeSpec.tv_sec += 1;
            SkASSERT((unsigned long)gTimeSpec.tv_nsec < kBillion);
        }

    //  printf("SignalQueueTimer(%d) timespec(%d %d)\n", delay, gTimeSpec.tv_sec, gTimeSpec.tv_nsec);
    }
    else    // cancel the timer
    {
        gTimeSpec.tv_nsec = 0;
        gTimeSpec.tv_sec = 0;
    }

    pthread_mutex_unlock(&gTimerMutex);
    pthread_cond_signal(&gTimerCond);
}

void SkEvent_start_timer_thread()
{
    int             status;
    pthread_attr_t  attr;
    
    status = pthread_attr_init(&attr);
    SkASSERT(status == 0);
    status = pthread_create(&gTimerThread, &attr, timer_event_thread_proc, 0);
    SkASSERT(status == 0);
}

void SkEvent_stop_timer_thread()
{
    int status = pthread_cancel(gTimerThread);
    SkASSERT(status == 0);
}

#endif
