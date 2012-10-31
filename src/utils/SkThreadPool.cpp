/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkThreadPool.h"
#include "SkRunnable.h"
#include "SkThreadUtils.h"

SkThreadPool::SkThreadPool(const int count)
: fDone(false) {
    // Create count threads, all running SkThreadPool::Loop.
    for (int i = 0; i < count; i++) {
        SkThread* thread = SkNEW_ARGS(SkThread, (&SkThreadPool::Loop, this));
        *fThreads.append() = thread;
        thread->start();
    }
}

SkThreadPool::~SkThreadPool() {
    fDone = true;
    fReady.lock();
    fReady.broadcast();
    fReady.unlock();

    // Wait for all threads to stop.
    for (int i = 0; i < fThreads.count(); i++) {
        fThreads[i]->join();
        SkDELETE(fThreads[i]);
    }
}

/*static*/ void SkThreadPool::Loop(void* arg) {
    // The SkThreadPool passes itself as arg to each thread as they're created.
    SkThreadPool* pool = static_cast<SkThreadPool*>(arg);

    while (true) {
        // We have to be holding the lock to read the queue and to call wait.
        pool->fReady.lock();
        while(pool->fQueue.isEmpty()) {
            // Is it time to die?
            if (pool->fDone) {
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
        pool->fReady.unlock();

        // OK, now really do the work.
        r->fRunnable->run();
        SkDELETE(r);
    }

    SkASSERT(false); // Unreachable.  The only exit happens when pool->fDone.
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
    LinkedRunnable* linkedRunnable = SkNEW(LinkedRunnable);
    linkedRunnable->fRunnable = r;
    fQueue.addToHead(linkedRunnable);
    fReady.signal();
    fReady.unlock();
}
