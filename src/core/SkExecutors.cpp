/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkExecutors.h"

#include "include/private/SkMutex.h"
#include "include/private/SkSemaphore.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTPin.h"
#include "src/core/SkNoDestructor.h"

#include <deque>
#include <thread>
#include <utility>

using namespace skia_private;

#if defined(SK_BUILD_FOR_WIN)
    #include "src/core/SkLeanWindows.h"
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

namespace {

// We'll always push_back() new work, but pop from the front of deques or the back of SkTArray.
template <typename WorkType>
WorkType pop(std::deque<WorkType>* list) {
    WorkType work = std::move(list->front());
    list->pop_front();
    return work;
}
template <typename WorkType>
WorkType pop(TArray<WorkType>* list) {
    WorkType work = std::move(list->back());
    list->pop_back();
    return work;
}

template <typename T> struct WorkTraits;

template <>
struct WorkTraits<std::function<void()>> {
    static void Execute(const std::function<void()>& fn) { fn(); }
    static bool IsValid(const std::function<void()>& fn) { return static_cast<bool>(fn); }
    static std::function<void()> Null() { return nullptr; }
};

template <>
struct WorkTraits<SkExecutorCallbackWork> {
    static void Execute(SkExecutorCallbackWork w) { w.fCallback(w.fContext); }
    static bool IsValid(SkExecutorCallbackWork w) { return w.fCallback != nullptr; }
    static SkExecutorCallbackWork Null() { return {nullptr, nullptr}; }
};

// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
template <typename WorkList, typename ExecutorType>
class SkThreadPool final : public ExecutorType {
public:
    using ActualWorkTraits = WorkTraits<typename ExecutorType::WorkType>;

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
            this->add(ActualWorkTraits::Null(), /* workList= */ 0);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.size(); i++) {
            fThreads[i].join();
        }
    }

    void add(typename ExecutorType::WorkType work, int workList) override {
        workList = SkTPin(workList, 0, fNumWorkLists-1);

        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);

            fWorkLists[workList].emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    void add(typename ExecutorType::WorkType work) override {
        this->add(std::move(work), /* workList= */ 0);
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
        typename ExecutorType::WorkType work;
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

        if (!ActualWorkTraits::IsValid(work)) {
            return false;  // This is Loop()'s signal to shut down.
        }

        ActualWorkTraits::Execute(work);
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

// The default, default SkExecutor is an SkTrivialExecutor, which just runs the work right away.
template <typename ExecutorType>
class SkTrivialExecutor final : public ExecutorType {
public:
    using ActualWorkTraits = WorkTraits<typename ExecutorType::WorkType>;

    void add(typename ExecutorType::WorkType work, int /* workList */) override {
        ActualWorkTraits::Execute(work);
    }
    void add(typename ExecutorType::WorkType work) override {
        ActualWorkTraits::Execute(work);
    }
};

template <typename ExecutorType>
ExecutorType& trivial_executor() {
    static SkNoDestructor<SkTrivialExecutor<ExecutorType>> executor;
    return *executor;
}

} // anonymous namespace

template <typename ExecutorType>
ExecutorType* SkExecutors<ExecutorType>::gDefaultExecutor = nullptr;

template <typename ExecutorType>
std::unique_ptr<ExecutorType> SkExecutors<ExecutorType>::MakeFIFOThreadPool(int threads,
                                                                            bool allowBorrowing) {
    using WorkList = std::deque<typename ExecutorType::WorkType>;
    return std::make_unique<SkThreadPool<WorkList, ExecutorType>>(
            /* numWorkLists= */ 1,
            threads > 0 ? threads : num_cores(),
            allowBorrowing);
}

template <typename ExecutorType>
std::unique_ptr<ExecutorType> SkExecutors<ExecutorType>::MakeLIFOThreadPool(int threads,
                                                                            bool allowBorrowing) {
    using WorkList = TArray<typename ExecutorType::WorkType>;
    return std::make_unique<SkThreadPool<WorkList, ExecutorType>>(
            /* numWorkLists= */ 1,
            threads > 0 ? threads : num_cores(),
            allowBorrowing);
}

template <typename ExecutorType>
std::unique_ptr<ExecutorType> SkExecutors<ExecutorType>::MakeMultiListFIFOThreadPool(
        int numWorkLists,
        int threads,
        bool allowBorrowing) {
    using WorkList = std::deque<typename ExecutorType::WorkType>;
    return std::make_unique<SkThreadPool<WorkList, ExecutorType>>(
            numWorkLists,
            threads > 0 ? threads : num_cores(),
            allowBorrowing);
}

template <typename ExecutorType>
std::unique_ptr<ExecutorType> SkExecutors<ExecutorType>::MakeMultiListLIFOThreadPool(
            int numWorkLists,
            int threads,
            bool allowBorrowing) {
    using WorkList = TArray<typename ExecutorType::WorkType>;
    return std::make_unique<SkThreadPool<WorkList, ExecutorType>>(
            numWorkLists,
            threads > 0 ? threads : num_cores(),
            allowBorrowing);
}

template <typename ExecutorType>
ExecutorType& SkExecutors<ExecutorType>::GetDefault() {
    if (gDefaultExecutor) {
        return *gDefaultExecutor;
    }
    return trivial_executor<ExecutorType>();
}

template <typename ExecutorType>
void SkExecutors<ExecutorType>::SetDefault(ExecutorType* executor) {
    gDefaultExecutor = executor;
}

template <typename ExecutorType>
SkExecutors<ExecutorType>::Enabler::Enabler(int threads) {
    if (threads) {
        fThreadPool = MakeLIFOThreadPool(threads);
        SetDefault(fThreadPool.get());
    }
}

template <typename ExecutorType>
SkExecutors<ExecutorType>::Enabler::~Enabler() {
    if (fThreadPool) {
        SetDefault(nullptr);
    }
}

template class SkExecutors<SkFunctionalExecutor>;
template class SkExecutors<SkCallbackExecutor>;

