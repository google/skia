/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCondVar_DEFINED
#define SkCondVar_DEFINED

#ifdef SK_USE_POSIX_THREADS
#include <pthread.h>
#elif defined(SK_BUILD_FOR_WIN32)
#include <windows.h>
#endif

/**
 * Condition variable for blocking access to shared data from other threads and
 * controlling which threads are awake.
 *
 * Currently only supported on platforms with posix threads and Windows Vista and
 * above.
 */
class SkCondVar {
public:
    SkCondVar();
    ~SkCondVar();

    /**
     * Lock a mutex. Must be done before calling the other functions on this object.
     */
    void lock();

    /**
     * Unlock the mutex.
     */
    void unlock();

    /**
     * Pause the calling thread. Will be awoken when signal() or broadcast() is called.
     * Must be called while lock() is held (but gives it up while waiting). Once awoken,
     * the calling thread will hold the lock once again.
     */
    void wait();

    /**
     * Wake one thread waiting on this condition. Must be called while lock()
     * is held.
     */
    void signal();

    /**
     * Wake all threads waiting on this condition. Must be called while lock()
     * is held.
     */
    void broadcast();

private:
#ifdef SK_USE_POSIX_THREADS
    pthread_mutex_t  fMutex;
    pthread_cond_t   fCond;
#elif defined(SK_BUILD_FOR_WIN32)
    CRITICAL_SECTION   fCriticalSection;
    CONDITION_VARIABLE fCondition;
#endif
};

#endif
