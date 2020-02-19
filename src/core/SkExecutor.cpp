/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkExecutor.h"
#include "include/private/SkMutex.h"
#include "include/private/SkSemaphore.h"
#include "include/private/SkSpinlock.h"
#include "include/private/SkTArray.h"
#include <deque>
#include <thread>

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

uint32_t SkExecutor::CreateUniqueID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidUniqueID);
    return id;
}

SkExecutor::SkExecutor() : fID(CreateUniqueID()) {
}

SkExecutor::~SkExecutor() {}

// The default default SkExecutor is an SkTrivialExecutor, which just runs the work right away.
class SkTrivialExecutor final : public SkExecutor {
    const char* name() const override { return "trivial"; }
    int numThreads() const override { return 0; }
    void print() const override { SkDebugf("no threads\n"); }

    void add(std::function<void(void)> work) override {
        work();
    }
};

static SkExecutor* gDefaultExecutor = nullptr;

void SetDefaultTrivialExecutor() {
    static SkTrivialExecutor *gTrivial = new SkTrivialExecutor();
    gDefaultExecutor = gTrivial;
}
SkExecutor& SkExecutor::GetDefault() {
    if (!gDefaultExecutor) {
        SetDefaultTrivialExecutor();
    }
    return *gDefaultExecutor;
}
void SkExecutor::SetDefault(SkExecutor* executor) {
    if (executor) {
        gDefaultExecutor = executor;
    } else {
        SetDefaultTrivialExecutor();
    }
}

// We'll always push_back() new work, but pop from the front of deques or the back of SkTArray.
static inline std::function<void(void)> pop(std::deque<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->front());
    list->pop_front();
    return fn;
}
static inline std::function<void(void)> pop(SkTArray<std::function<void(void)>>* list) {
    std::function<void(void)> fn = std::move(list->back());
    list->pop_back();
    return fn;
}

// An SkThreadPool is an executor that runs work on a fixed pool of OS threads.
template <typename WorkList>
class SkThreadPool final : public SkExecutor {
public:
    const char* fName;

    const char* name() const override { return fName; }
    int numThreads() const override { return fThreads.size(); }
    void print() const override {
        SkDebugf("threads %d: ", fThreads.count());
        for (int i = 0; i < fThreads.count(); i++) {
            SkDebugf("%d ", fThreads[i].get_id());
        }
        SkDebugf("\n");
    }

    explicit SkThreadPool(int threads, const char* name) : fName(name) {
        for (int i = 0; i < threads; i++) {
            fThreads.emplace_back(&Loop, this);
        }
    }

    ~SkThreadPool() override {
        // Signal each thread that it's time to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            this->add(nullptr);
        }
        // Wait for each thread to shut down.
        for (int i = 0; i < fThreads.count(); i++) {
            fThreads[i].join();
        }
    }

    virtual void add(std::function<void(void)> work) override {
        // Add some work to our pile of work to do.
        {
            SkAutoMutexExclusive lock(fWorkLock);
            fWork.emplace_back(std::move(work));
        }
        // Tell the Loop() threads to pick it up.
        fWorkAvailable.signal(1);
    }

    virtual void borrow() override {
        // If there is work waiting, do it.
//        if (fWorkAvailable.try_wait()) {
//            SkAssertResult(this->do_work());
//        }
    }

private:
    // This method should be called only when fWorkAvailable indicates there's work to do.
    bool do_work() {
        std::function<void(void)> work;
        {
            SkAutoMutexExclusive lock(fWorkLock);
            SkASSERT(!fWork.empty());        // TODO: if (fWork.empty()) { return true; } ?
            work = pop(&fWork);
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

    SkTArray<std::thread> fThreads;
    WorkList              fWork;
    Lock                  fWorkLock;
    SkSemaphore           fWorkAvailable;
};

std::unique_ptr<SkExecutor> SkExecutor::MakeFIFOThreadPool(int threads) {
    using WorkList = std::deque<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(threads > 0 ? threads : num_cores(), "FIFO");
}
std::unique_ptr<SkExecutor> SkExecutor::MakeLIFOThreadPool(int threads) {
    using WorkList = SkTArray<std::function<void(void)>>;
    return std::make_unique<SkThreadPool<WorkList>>(threads > 0 ? threads : num_cores(), "LIFO");
}
