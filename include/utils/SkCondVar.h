/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCondVar_DEFINED
#define SkCondVar_DEFINED

#include <pthread.h>

class SkCondVar {
public:
    SkCondVar();
    ~SkCondVar();

    void lock();
    void unlock();

    /**
     * Pause the calling thread. Will be awoken when signal() is called.
     * Must be called while lock() is held (but gives it up while waiting).
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
    // FIXME: Make this independent of pthreads.
    pthread_mutex_t  fMutex;
    pthread_cond_t   fCond;
};

#endif
