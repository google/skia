/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkThreadPool_DEFINED
#define SkThreadPool_DEFINED

#include "SkCondVar.h"
#include "SkRunnable.h"
#include "SkTDArray.h"
#include "SkTInternalLList.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_ANDROID)
#    include <unistd.h>
#endif

// Returns the number of cores on this machine.
static inline int num_cores() {
#if defined(SK_BUILD_FOR_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_ANDROID)
    return (int) sysconf(_SC_NPROCESSORS_ONLN);
#else
    return 1;
#endif
}

template <typename T>
class SkTThreadPool {
public:
    /**
     * Create a threadpool with count threads, or one thread per core if kThreadPerCore.
     */
    static const int kThreadPerCore = -1;
    explicit SkTThreadPool(int count);
    ~SkTThreadPool();

    /**
     * Queues up an SkRunnable to run when a thread is available, or synchronously if count is 0.
     * Does not take ownership. NULL is a safe no-op.  If T is not void, the runnable will be passed
     * a reference to a T on the thread's local stack.
     */
    void add(SkTRunnable<T>*);

    /**
     * Same as add, but adds the runnable as the very next to run rather than enqueueing it.
     */
    void addNext(SkTRunnable<T>*);

    /**
     * Block until all added SkRunnables have completed.  Once called, calling add() is undefined.
     */
    void wait();

 private:
    struct LinkedRunnable {
        SkTRunnable<T>* fRunnable;  // Unowned.
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(LinkedRunnable);
    };

    enum State {
        kRunning_State,  // Normal case.  We've been constructed and no one has called wait().
        kWaiting_State,  // wait has been called, but there still might be work to do or being done.
        kHalting_State,  // There's no work to do and no thread is busy.  All threads can shut down.
    };

    void addSomewhere(SkTRunnable<T>* r,
                      void (SkTInternalLList<LinkedRunnable>::*)(LinkedRunnable*));

    SkTInternalLList<LinkedRunnable> fQueue;
    SkCondVar                        fReady;
    SkTDArray<SkThread*>             fThreads;
    State                            fState;
    int                              fBusyThreads;

    static void Loop(void*);  // Static because we pass in this.
};

template <typename T>
SkTThreadPool<T>::SkTThreadPool(int count) : fState(kRunning_State), fBusyThreads(0) {
    if (count < 0) {
        count = num_cores();
    }
    // Create count threads, all running SkTThreadPool::Loop.
    for (int i = 0; i < count; i++) {
        SkThread* thread = SkNEW_ARGS(SkThread, (&SkTThreadPool::Loop, this));
        *fThreads.append() = thread;
        thread->start();
    }
}

template <typename T>
SkTThreadPool<T>::~SkTThreadPool() {
    if (kRunning_State == fState) {
        this->wait();
    }
}

namespace SkThreadPoolPrivate {

template <typename T>
struct ThreadLocal {
    void run(SkTRunnable<T>* r) { r->run(data); }
    T data;
};

template <>
struct ThreadLocal<void> {
    void run(SkTRunnable<void>* r) { r->run(); }
};

}  // namespace SkThreadPoolPrivate

template <typename T>
void SkTThreadPool<T>::addSomewhere(SkTRunnable<T>* r,
                                    void (SkTInternalLList<LinkedRunnable>::* f)(LinkedRunnable*)) {
    if (r == NULL) {
        return;
    }

    if (fThreads.isEmpty()) {
        SkThreadPoolPrivate::ThreadLocal<T> threadLocal;
        threadLocal.run(r);
        return;
    }

    LinkedRunnable* linkedRunnable = SkNEW(LinkedRunnable);
    linkedRunnable->fRunnable = r;
    fReady.lock();
    SkASSERT(fState != kHalting_State);  // Shouldn't be able to add work when we're halting.
    (fQueue.*f)(linkedRunnable);
    fReady.signal();
    fReady.unlock();
}

template <typename T>
void SkTThreadPool<T>::add(SkTRunnable<T>* r) {
    this->addSomewhere(r, &SkTInternalLList<LinkedRunnable>::addToTail);
}

template <typename T>
void SkTThreadPool<T>::addNext(SkTRunnable<T>* r) {
    this->addSomewhere(r, &SkTInternalLList<LinkedRunnable>::addToHead);
}


template <typename T>
void SkTThreadPool<T>::wait() {
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

template <typename T>
/*static*/ void SkTThreadPool<T>::Loop(void* arg) {
    // The SkTThreadPool passes itself as arg to each thread as they're created.
    SkTThreadPool<T>* pool = static_cast<SkTThreadPool<T>*>(arg);
    SkThreadPoolPrivate::ThreadLocal<T> threadLocal;

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
        LinkedRunnable* r = pool->fQueue.head();

        pool->fQueue.remove(r);

        // Having claimed our SkRunnable, we now give up the lock while we run it.
        // Otherwise, we'd only ever do work on one thread at a time, which rather
        // defeats the point of this code.
        pool->fBusyThreads++;
        pool->fReady.unlock();

        // OK, now really do the work.
        threadLocal.run(r->fRunnable);
        SkDELETE(r);

        // Let everyone know we're not busy.
        pool->fReady.lock();
        pool->fBusyThreads--;
        pool->fReady.unlock();
    }

    SkASSERT(false); // Unreachable.  The only exit happens when pool->fState is kHalting_State.
}

typedef SkTThreadPool<void> SkThreadPool;

#endif
