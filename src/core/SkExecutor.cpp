/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkSemaphore.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "src/base/SkNoDestructor.h"

#include <deque>
#include <thread>
#include <utility>

using namespace skia_private;

#if defined(SK_BUILD_FOR_WIN)
    #include "src/base/SkLeanWindows.h"
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
public:
    void add(std::function<void(void)> work, int /* workList */) override {
        work();
    }
    void add(std::function<void(void)> work) override {
        this->add(std::move(work), /* workList= */ 0);
    }
    int discardAllPendingWork() override { return 0;}
};

static SkExecutor& trivial_executor() {
    static SkNoDestructor<SkTrivialExecutor> executor;
    return *executor;
}

static SkExecutor* gDefaultExecutor = nullptr;

SkExecutor& SkExecutor::GetDefault() {
    if (gDefaultExecutor) {
        return *gDefaultExecutor;
    }
    return trivial_executor();
}

void SkExecutor::SetDefault(SkExecutor* executor) {
    gDefaultExecutor = executor;
}

// We'll always push_back() new work, but pop from the front of deques or the back of SkTArray.
static inline std::function<void(void)> pop(std::deque<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->front());
    list->pop_front();
    return fn;
}
static inline std::function<void(void)> pop(TArray<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->back());
    list->pop_back();
    return fn;
}

// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
template <typename WorkList>
class SkThreadPool final : public SkExecutor {
public:
    explicit SkThreadPool(int numWorkLists, int threads, bool allowBorrowing)
            : fNumWorkLists(numWorkLists < 1 ? 1 : numWorkLists)
            , fAllowBorrowing(allowBorrowing) {

        fWorkLists = std::make_unique<WorkList[]>(fNumWorkLists);

        for (int i = 0; i < threads; i++) {
            fThreads.emplace_back(&Loop, this);
        }
    }

    ~SkThreadPool() override {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            // Add the notification to the highest priority list
            this->add(nullptr, /* workList= */ 0);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            fThreads[i].join();
        }
    }

    void add(std::function<void(void)> work, int workList) override {
        workList = SkTPin(workList, 0, fNumWorkLists-1);

        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);

            fWorkLists[workList].emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    void add(std::function<void(void)> work) override {
        this->add(std::move(work), /* workList= */ 0);
    }

    int discardAllPendingWork() override {
        SkAutoMutexExclusive lock(fWorkLock);

        int numDiscarded = 0;
        for (int i = 0; i < fNumWorkLists; ++i) {
            numDiscarded += fWorkLists[i].size();
            fWorkLists[i].clear();
        }

        return numDiscarded;
    }

    void borrow() override {
        // If there is work waiting and we're allowed to borrow work, do it.
        if (fAllowBorrowing && fWorkAvailable.try_wait()) {
            SkAssertResult(this->do_work());
        }
    }

private:
    // This method should usually be called only when fWorkAvailable indicates there's work to do.
    bool do_work() {
        std::function<void(void)> work;
        bool workAvailable = false;
        {
            SkAutoMutexExclusive lock(fWorkLock);

            for (int i = 0; i < fNumWorkLists; ++i) {
                if (!fWorkLists[i].empty()) {
                    workAvailable = true;
                    work = pop(&fWorkLists[i]);
                    break;
                }
            }
        }

        if (!workAvailable) {
            // Because we can discard work asynchronous to Loop() we can sometimes get in this
            // method with no work to do
            return true;
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

    TArray<std::thread>         fThreads;
    const int                   fNumWorkLists; // guaranteed >= 1
    std::unique_ptr<WorkList[]> fWorkLists SK_GUARDED_BY(fWorkLock);
    Lock                        fWorkLock;
    SkSemaphore                 fWorkAvailable;
    const bool                  fAllowBorrowing;
};

std::unique_ptr<SkExecutor> SkExecutor::MakeFIFOThreadPool(int threads, bool allowBorrowing) {
    using WorkList = std::deque<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(/* numWorkLists= */ 1,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}
std::unique_ptr<SkExecutor> SkExecutor::MakeLIFOThreadPool(int threads, bool allowBorrowing) {
    using WorkList = TArray<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(/* numWorkLists= */ 1,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}

std::unique_ptr<SkExecutor> SkExecutor::MakeMultiListFIFOThreadPool(int numWorkLists,
                                                                    int threads,
                                                                    bool allowBorrowing) {
    using WorkList = std::deque<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(numWorkLists,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}
std::unique_ptr<SkExecutor> SkExecutor::MakeMultiListLIFOThreadPool(int numWorkLists,
                                                                    int threads,
                                                                    bool allowBorrowing) {
    using WorkList = TArray<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(numWorkLists,
                                                    threads > 0 ? threads : num_cores(),
                                                    allowBorrowing);
}
