/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkExecutor.h"
#include "SkMakeUnique.h"
#include "SkMutex.h"
#include "SkSemaphore.h"
#include "SkSpinlock.h"
#include "SkTArray.h"
#include "SkThreadUtils.h"

#if defined(_MSC_VER)
    #include <windows.h>
    static int num_cores() {
        SYSTEM_INFO sysinfo;
        GetNativeSystemInfo(&sysinfo);
        return (int)sysinfo.dwNumberOfProcessors;
    }
#else
    #include <unistd.h>
    static int num_cores() {
        return (int)sysconf(_SC_NPROCESSORS_ONLN);
    }
#endif

SkExecutor::~SkExecutor() {}

// The default default SkExecutor is an SkTrivialExecutor, which just runs the work right away.
class SkTrivialExecutor final : public SkExecutor {
    void add(std::function<void(void)> work) override {
        work();
    }
};

static SkTrivialExecutor gTrivial;
static SkExecutor* gDefaultExecutor = &gTrivial;

SkExecutor& SkExecutor::GetDefault() {
    return *gDefaultExecutor;
}
void SkExecutor::SetDefault(SkExecutor* executor) {
    gDefaultExecutor = executor ? executor : &gTrivial;
}

// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
class SkThreadPool final : public SkExecutor {
public:
    explicit SkThreadPool(int threads) {
        for (int i = 0; i < threads; i++) {
            fThreads.emplace_back(new SkThread(&Loop, this));
            fThreads.back()->start();
        }
    }

    ~SkThreadPool() override {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            this->add(nullptr);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            fThreads[i]->join();
        }
    }

    virtual void add(std::function<void(void)> work) override {
        // Add some work to our pile of work to do.
        {
            SkAutoExclusive lock(fWorkLock);
            fWork.emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    virtual void borrow() override {
        // If there is work waiting, do it.
        if (fWorkAvailable.try_wait()) {
            SkAssertResult(this->do_work());
        }
    }

private:
    // This method should be called only when fWorkAvailable indicates there's work to do.
    bool do_work() {
        std::function<void(void)> work;
        {
            SkAutoExclusive lock(fWorkLock);
            SkASSERT(!fWork.empty());        // TODO: if (fWork.empty()) { return true; } ?
            work = std::move(fWork.back());
            fWork.pop_back();
        }

        if (!work) {
            return false;  // This is Loop()'s signal to shut down.
        }

        work();
        return true;
    }

    static void Loop(void* ctx) {
        auto pool = (SkThreadPool*)ctx;
        do {
            pool->fWorkAvailable.wait();
        } while (pool->do_work());
    }

    // Both SkMutex and SkSpinlock can work here.
    using Lock = SkMutex;

    SkTArray<std::unique_ptr<SkThread>> fThreads;
    SkTArray<std::function<void(void)>> fWork;
    Lock                                fWorkLock;
    SkSemaphore                         fWorkAvailable;
};

std::unique_ptr<SkExecutor> SkExecutor::MakeThreadPool(int threads) {
    return skstd::make_unique<SkThreadPool>(threads > 0 ? threads : num_cores());
}
