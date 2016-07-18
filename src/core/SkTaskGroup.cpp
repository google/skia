/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLeanWindows.h"
#include "SkOnce.h"
#include "SkSemaphore.h"
#include "SkSpinlock.h"
#include "SkTArray.h"
#include "SkTDArray.h"
#include "SkTaskGroup.h"
#include "SkThreadUtils.h"

#if defined(SK_BUILD_FOR_WIN32)
    static void query_num_cores(int* cores) {
        SYSTEM_INFO sysinfo;
        GetNativeSystemInfo(&sysinfo);
        *cores = sysinfo.dwNumberOfProcessors;
    }
#else
    #include <unistd.h>
    static void query_num_cores(int* cores) {
        *cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif

static int num_cores() {
    // We cache num_cores() so we only query the OS once.
    static int cores = 0;
    static SkOnce once;
    once(query_num_cores, &cores);
    SkASSERT(cores > 0);
    return cores;
}

namespace {

class ThreadPool : SkNoncopyable {
public:
    static void Add(std::function<void(void)> fn, SkAtomic<int32_t>* pending) {
        if (!gGlobal) {
            return fn();
        }
        gGlobal->add(fn, pending);
    }

    static void Batch(int N, std::function<void(int)> fn, SkAtomic<int32_t>* pending) {
        if (!gGlobal) {
            for (int i = 0; i < N; i++) { fn(i); }
            return;
        }
        gGlobal->batch(N, fn, pending);
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
                if (gGlobal->fWork.empty()) {
                    // Someone has picked up all the work (including ours).  How nice of them!
                    // (They may still be working on it, so we can't assert *pending == 0 here.)
                    continue;
                }
                work = gGlobal->fWork.back();
                gGlobal->fWork.pop_back();
            }
            // This Work isn't necessarily part of our SkTaskGroup of interest, but that's fine.
            // We threads gotta stick together.  We're always making forward progress.
            work.fn();
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

    struct Work {
        std::function<void(void)> fn; // A function to call
        SkAtomic<int32_t>* pending;   // then decrement pending afterwards.
    };

    explicit ThreadPool(int threads) {
        if (threads == -1) {
            threads = num_cores();
        }
        for (int i = 0; i < threads; i++) {
            fThreads.push(new SkThread(&ThreadPool::Loop, this));
            fThreads.top()->start();
        }
    }

    ~ThreadPool() {
        SkASSERT(fWork.empty());  // All SkTaskGroups should be destroyed by now.

        // Send a poison pill to each thread.
        SkAtomic<int> dummy(0);
        for (int i = 0; i < fThreads.count(); i++) {
            this->add(nullptr, &dummy);
        }
        // Wait for them all to swallow the pill and die.
        for (int i = 0; i < fThreads.count(); i++) {
            fThreads[i]->join();
        }
        SkASSERT(fWork.empty());  // Can't hurt to double check.
        fThreads.deleteAll();
    }

    void add(std::function<void(void)> fn, SkAtomic<int32_t>* pending) {
        Work work = { fn, pending };
        pending->fetch_add(+1, sk_memory_order_relaxed);  // No barrier needed.
        {
            AutoLock lock(&fWorkLock);
            fWork.push_back(work);
        }
        fWorkAvailable.signal(1);
    }

    void batch(int N, std::function<void(int)> fn, SkAtomic<int32_t>* pending) {
        pending->fetch_add(+N, sk_memory_order_relaxed);  // No barrier needed.
        {
            AutoLock lock(&fWorkLock);
            for (int i = 0; i < N; i++) {
                Work work = { [i, fn]() { fn(i); }, pending };
                fWork.push_back(work);
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
                if (pool->fWork.empty()) {
                    // Someone in Wait() stole our work (fWorkAvailable is an upper bound).
                    // Well, that's fine, back to sleep for us.
                    continue;
                }
                work = pool->fWork.back();
                pool->fWork.pop_back();
            }
            if (!work.fn) {
                return;  // Poison pill.  Time... to die.
            }
            work.fn();
            work.pending->fetch_add(-1, sk_memory_order_release);  // Pairs with load in Wait().
        }
    }

    // fWorkLock must be held when reading or modifying fWork.
    SkSpinlock      fWorkLock;
    SkTArray<Work>  fWork;

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
void SkTaskGroup::add(std::function<void(void)> fn) { ThreadPool::Add(fn, &fPending); }
void SkTaskGroup::batch(int N, std::function<void(int)> fn) {
    ThreadPool::Batch(N, fn, &fPending);
}
