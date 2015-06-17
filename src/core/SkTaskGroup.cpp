/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTaskGroup.h"

#include "SkCondVar.h"
#include "SkRunnable.h"
#include "SkTDArray.h"
#include "SkThread.h"
#include "SkThreadUtils.h"

#if defined(SK_BUILD_FOR_WIN32)
    static inline int num_cores() {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
    }
#else
    #include <unistd.h>
    static inline int num_cores() {
        return (int) sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif

namespace {

class ThreadPool : SkNoncopyable {
public:
    static void Add(SkRunnable* task, SkAtomic<int32_t>* pending) {
        if (!gGlobal) {  // If we have no threads, run synchronously.
            return task->run();
        }
        gGlobal->add(&CallRunnable, task, pending);
    }

    static void Add(void (*fn)(void*), void* arg, SkAtomic<int32_t>* pending) {
        if (!gGlobal) {
            return fn(arg);
        }
        gGlobal->add(fn, arg, pending);
    }

    static void Batch(void (*fn)(void*), void* args, int N, size_t stride,
                      SkAtomic<int32_t>* pending) {
        if (!gGlobal) {
            for (int i = 0; i < N; i++) { fn((char*)args + i*stride); }
            return;
        }
        gGlobal->batch(fn, args, N, stride, pending);
    }

    static void Wait(SkAtomic<int32_t>* pending) {
        if (!gGlobal) {  // If we have no threads, the work must already be done.
            SkASSERT(pending->load(sk_memory_order_relaxed) == 0);
            return;
        }
        // Acquire pairs with decrement release here or in Loop.
        while (pending->load(sk_memory_order_acquire) > 0) {
            // Lend a hand until our SkTaskGroup of interest is done.
            Work work;
            {
                AutoLock lock(&gGlobal->fReady);
                if (gGlobal->fWork.isEmpty()) {
                    // Someone has picked up all the work (including ours).  How nice of them!
                    // (They may still be working on it, so we can't assert *pending == 0 here.)
                    continue;
                }
                gGlobal->fWork.pop(&work);
            }
            // This Work isn't necessarily part of our SkTaskGroup of interest, but that's fine.
            // We threads gotta stick together.  We're always making forward progress.
            work.fn(work.arg);
            work.pending->fetch_add(-1, sk_memory_order_release);  // Pairs with load above.
        }
    }

private:
    struct AutoLock {
        AutoLock(SkCondVar* c) : fC(c) { fC->lock(); }
        ~AutoLock() { fC->unlock(); }
    private:
        SkCondVar* fC;
    };

    static void CallRunnable(void* arg) { static_cast<SkRunnable*>(arg)->run(); }

    struct Work {
        void (*fn)(void*);            // A function to call,
        void* arg;                    // its argument,
        SkAtomic<int32_t>* pending;   // then decrement pending afterwards.
    };

    explicit ThreadPool(int threads) : fDraining(false) {
        if (threads == -1) {
            threads = num_cores();
        }
        for (int i = 0; i < threads; i++) {
            fThreads.push(SkNEW_ARGS(SkThread, (&ThreadPool::Loop, this)));
            fThreads.top()->start();
        }
    }

    ~ThreadPool() {
        SkASSERT(fWork.isEmpty());  // All SkTaskGroups should be destroyed by now.
        {
            AutoLock lock(&fReady);
            fDraining = true;
            fReady.broadcast();
        }
        for (int i = 0; i < fThreads.count(); i++) {
            fThreads[i]->join();
        }
        SkASSERT(fWork.isEmpty());  // Can't hurt to double check.
        fThreads.deleteAll();
    }

    void add(void (*fn)(void*), void* arg, SkAtomic<int32_t>* pending) {
        Work work = { fn, arg, pending };
        pending->fetch_add(+1, sk_memory_order_relaxed);  // No barrier needed.
        {
            AutoLock lock(&fReady);
            fWork.push(work);
            fReady.signal();
        }
    }

    void batch(void (*fn)(void*), void* arg, int N, size_t stride, SkAtomic<int32_t>* pending) {
        pending->fetch_add(+N, sk_memory_order_relaxed);  // No barrier needed.
        {
            AutoLock lock(&fReady);
            Work* batch = fWork.append(N);
            for (int i = 0; i < N; i++) {
                Work work = { fn, (char*)arg + i*stride, pending };
                batch[i] = work;
            }
            fReady.broadcast();
        }
    }

    static void Loop(void* arg) {
        ThreadPool* pool = (ThreadPool*)arg;
        Work work;
        while (true) {
            {
                AutoLock lock(&pool->fReady);
                while (pool->fWork.isEmpty()) {
                    if (pool->fDraining) {
                        return;
                    }
                    pool->fReady.wait();
                }
                pool->fWork.pop(&work);
            }
            work.fn(work.arg);
            work.pending->fetch_add(-1, sk_memory_order_release);  // Pairs with load in Wait().
        }
    }

    SkTDArray<Work>      fWork;
    SkTDArray<SkThread*> fThreads;
    SkCondVar            fReady;
    bool                 fDraining;

    static ThreadPool* gGlobal;
    friend struct SkTaskGroup::Enabler;
};
ThreadPool* ThreadPool::gGlobal = NULL;

}  // namespace

SkTaskGroup::Enabler::Enabler(int threads) {
    SkASSERT(ThreadPool::gGlobal == NULL);
    if (threads != 0 && SkCondVar::Supported()) {
        ThreadPool::gGlobal = SkNEW_ARGS(ThreadPool, (threads));
    }
}

SkTaskGroup::Enabler::~Enabler() {
    SkDELETE(ThreadPool::gGlobal);
}

SkTaskGroup::SkTaskGroup() : fPending(0) {}

void SkTaskGroup::wait()                            { ThreadPool::Wait(&fPending); }
void SkTaskGroup::add(SkRunnable* task)             { ThreadPool::Add(task, &fPending); }
void SkTaskGroup::add(void (*fn)(void*), void* arg) { ThreadPool::Add(fn, arg, &fPending); }
void SkTaskGroup::batch (void (*fn)(void*), void* args, int N, size_t stride) {
    ThreadPool::Batch(fn, args, N, stride, &fPending);
}

