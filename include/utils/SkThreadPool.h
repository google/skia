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

class SkThread;

class SkThreadPool {

public:
    /**
     * Create a threadpool with count threads, or one thread per core if kThreadPerCore.
     */
    static const int kThreadPerCore = -1;
    explicit SkThreadPool(int count);
    ~SkThreadPool();

    /**
     * Queues up an SkRunnable to run when a thread is available, or immediately if
     * count is 0.  NULL is a safe no-op.  Does not take ownership.
     */
    void add(SkRunnable*);

    /**
     * Block until all added SkRunnables have completed.  Once called, calling add() is undefined.
     */
    void wait();

 private:
    struct LinkedRunnable {
        // Unowned pointer.
        SkRunnable* fRunnable;

    private:
        SK_DECLARE_INTERNAL_LLIST_INTERFACE(LinkedRunnable);
    };

    SkTInternalLList<LinkedRunnable>    fQueue;
    SkCondVar                           fReady;
    SkTDArray<SkThread*>                fThreads;
    bool                                fDone;

    static void Loop(void*);  // Static because we pass in this.
};

#endif
