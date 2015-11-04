/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOnce.h"
#include "SkRunnable.h"
#include "SkSemaphore.h"
#include "SkSpinlock.h"
#include "SkTDArray.h"
#include "SkTaskGroup.h"
#include "SkThreadUtils.h"

#if defined(SK_BUILD_FOR_WIN32)
    static void query_num_cores(int* num_cores) {
        SYSTEM_INFO sysinfo;
        GetNativeSystemInfo(&sysinfo);
        *num_cores = sysinfo.dwNumberOfProcessors;
    }
#else
    #include <unistd.h>
    static void query_num_cores(int* num_cores) {
        *num_cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif

// We cache sk_num_cores() so we only query the OS once.
SK_DECLARE_STATIC_ONCE(g_query_num_cores_once);
int sk_num_cores() {
    static int num_cores = 0;
    SkOnce(&g_query_num_cores_once, query_num_cores, &num_cores);
    SkASSERT(num_cores > 0);
    return num_cores;
}

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
                // We're stealing work opportunistically,
                // so we never call fWorkAvailable.wait(), which could sleep us if there's no work.
                // This means fWorkAvailable is only an upper bound on fWork.count().
                AutoLock lock(&gGlobal->fWorkLock);
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
        AutoLock(SkSpinlock* lock) : fLock(lock) { fLock->acquire(); }
        ~AutoLock() { fLock->release(); }
    private:
        SkSpinlock* fLock;
    };

    static void CallRunnable(void* arg) { static_cast<SkRunnable*>(arg)->run(); }

    struct Work {
        void (*fn)(void*);            // A function to call,
        void* arg;                    // its argument,
        SkAtomic<int32_t>* pending;   // then decrement pending afterwards.
    };

    explicit ThreadPool(int threads) {
        if (threads == -1) {
            threads = sk_num_cores();
        }
        for (int i = 0; i < threads; i++) {
            fThreads.push(new SkThread(&ThreadPool::Loop, this));
            fThreads.top()->start();
        }
    }

    ~ThreadPool() {
        SkASSERT(fWork.isEmpty());  // All SkTaskGroups should be destroyed by now.

        // Send a poison pill to each thread.
        SkAtomic<int> dummy(0);
        for (int i = 0; i < fThreads.count(); i++) {
            this->add(nullptr, nullptr, &dummy);
        }
        // Wait for them all to swallow the pill and die.
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
            AutoLock lock(&fWorkLock);
            fWork.push(work);
        }
        fWorkAvailable.signal(1);
    }

    void batch(void (*fn)(void*), void* arg, int N, size_t stride, SkAtomic<int32_t>* pending) {
        pending->fetch_add(+N, sk_memory_order_relaxed);  // No barrier needed.
        {
            AutoLock lock(&fWorkLock);
            Work* batch = fWork.append(N);
            for (int i = 0; i < N; i++) {
                Work work = { fn, (char*)arg + i*stride, pending };
                batch[i] = work;
            }
        }
        fWorkAvailable.signal(N);
    }

    static void Loop(void* arg) {
        ThreadPool* pool = (ThreadPool*)arg;
        Work work;
        while (true) {
            // Sleep until there's work available, and claim one unit of Work as we wake.
            pool->fWorkAvailable.wait();
            {
                AutoLock lock(&pool->fWorkLock);
                if (pool->fWork.isEmpty()) {
                    // Someone in Wait() stole our work (fWorkAvailable is an upper bound).
                    // Well, that's fine, back to sleep for us.
                    continue;
                }
                pool->fWork.pop(&work);
            }
            if (!work.fn) {
                return;  // Poison pill.  Time... to die.
            }
            work.fn(work.arg);
            work.pending->fetch_add(-1, sk_memory_order_release);  // Pairs with load in Wait().
        }
    }

    // fWorkLock must be held when reading or modifying fWork.
    SkSpinlock      fWorkLock;
    SkTDArray<Work> fWork;

    // A thread-safe upper bound for fWork.count().
    //
    // We'd have it be an exact count but for the loop in Wait():
    // we never want that to block, so it can't call fWorkAvailable.wait(),
    // and that's the only way to decrement fWorkAvailable.
    // So fWorkAvailable may overcount actual the work available.
    // We make do, but this means some worker threads may wake spuriously.
    SkSemaphore fWorkAvailable;

    // These are only changed in a single-threaded context.
    SkTDArray<SkThread*> fThreads;
    static ThreadPool* gGlobal;

    friend struct SkTaskGroup::Enabler;
    friend int ::sk_parallel_for_thread_count();
};
ThreadPool* ThreadPool::gGlobal = nullptr;

}  // namespace

SkTaskGroup::Enabler::Enabler(int threads) {
    SkASSERT(ThreadPool::gGlobal == nullptr);
    if (threads != 0) {
        ThreadPool::gGlobal = new ThreadPool(threads);
    }
}

SkTaskGroup::Enabler::~Enabler() { delete ThreadPool::gGlobal; }

SkTaskGroup::SkTaskGroup() : fPending(0) {}

void SkTaskGroup::wait()                            { ThreadPool::Wait(&fPending); }
void SkTaskGroup::add(SkRunnable* task)             { ThreadPool::Add(task, &fPending); }
void SkTaskGroup::add(void (*fn)(void*), void* arg) { ThreadPool::Add(fn, arg, &fPending); }
void SkTaskGroup::batch (void (*fn)(void*), void* args, int N, size_t stride) {
    ThreadPool::Batch(fn, args, N, stride, &fPending);
}

int sk_parallel_for_thread_count() {
    if (ThreadPool::gGlobal != nullptr) {
        return ThreadPool::gGlobal->fThreads.count();
    }
    return 0;
}
