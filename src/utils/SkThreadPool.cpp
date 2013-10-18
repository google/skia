/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRunnable.h"
#include "SkThreadPool.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_ANDROID)
#include <unistd.h>
#endif

// Returns the number of cores on this machine.
static int num_cores() {
#if defined(SK_BUILD_FOR_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_ANDROID)
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
    return 1;
#endif
}

SkThreadPool::SkThreadPool(int count)
: fState(kRunning_State), fBusyThreads(0) {
    if (count < 0) count = num_cores();
    // Create count threads, all running SkThreadPool::Loop.
    for (int i = 0; i < count; i++) {
        SkThread* thread = SkNEW_ARGS(SkThread, (&SkThreadPool::Loop, this));
        *fThreads.append() = thread;
        thread->start();
    }
}

SkThreadPool::~SkThreadPool() {
    if (kRunning_State == fState) {
        this->wait();
    }
}

void SkThreadPool::wait() {
    fReady.lock();
    fState = kWaiting_State;
    fReady.broadcast();
    fReady.unlock();

    // Wait for all threads to stop.
    for (int i = 0; i < fThreads.count(); i++) {
        fThreads[i]->join();
        SkDELETE(fThreads[i]);
    }
    SkASSERT(fQueue.isEmpty());
}

/*static*/ void SkThreadPool::Loop(void* arg) {
    // The SkThreadPool passes itself as arg to each thread as they're created.
    SkThreadPool* pool = static_cast<SkThreadPool*>(arg);

    while (true) {
        // We have to be holding the lock to read the queue and to call wait.
        pool->fReady.lock();
        while(pool->fQueue.isEmpty()) {
            // Does the client want to stop and are all the threads ready to stop?
            // If so, we move into the halting state, and whack all the threads so they notice.
            if (kWaiting_State == pool->fState && pool->fBusyThreads == 0) {
                pool->fState = kHalting_State;
                pool->fReady.broadcast();
            }
            // Any time we find ourselves in the halting state, it's quitting time.
            if (kHalting_State == pool->fState) {
                pool->fReady.unlock();
                return;
            }
            // wait yields the lock while waiting, but will have it again when awoken.
            pool->fReady.wait();
        }
        // We've got the lock back here, no matter if we ran wait or not.

        // The queue is not empty, so we have something to run.  Claim it.
        LinkedRunnable* r = pool->fQueue.tail();

        pool->fQueue.remove(r);

        // Having claimed our SkRunnable, we now give up the lock while we run it.
        // Otherwise, we'd only ever do work on one thread at a time, which rather
        // defeats the point of this code.
        pool->fBusyThreads++;
        pool->fReady.unlock();

        // OK, now really do the work.
        r->fRunnable->run();
        SkDELETE(r);

        // Let everyone know we're not busy.
        pool->fReady.lock();
        pool->fBusyThreads--;
        pool->fReady.unlock();
    }

    SkASSERT(false); // Unreachable.  The only exit happens when pool->fState is kHalting_State.
}

void SkThreadPool::add(SkRunnable* r) {
    if (NULL == r) {
        return;
    }

    // If we don't have any threads, obligingly just run the thing now.
    if (fThreads.isEmpty()) {
        return r->run();
    }

    // We have some threads.  Queue it up!
    fReady.lock();
    SkASSERT(fState != kHalting_State);  // Shouldn't be able to add work when we're halting.
    LinkedRunnable* linkedRunnable = SkNEW(LinkedRunnable);
    linkedRunnable->fRunnable = r;
    fQueue.addToHead(linkedRunnable);
    fReady.signal();
    fReady.unlock();
}
