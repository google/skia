#include "SkExecutor.h"
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
    static void thread_yield() {
        (void)SwitchToThread();
    }
#else
    #include <sched.h>
    #include <unistd.h>
    static int num_cores() {
        return (int)sysconf(_SC_NPROCESSORS_ONLN);
    }
    static void thread_yield() {
        (void)sched_yield();
    }
#endif

SkExecutor::~SkExecutor() {}

// There is always a default SkExecutor available by calling SkExecutor::GetDefault().
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

    ~SkThreadPool() {
        for (int i = 0; i < fThreads.count(); i++) {
            this->add(nullptr);
        }
        for (int i = 0; i < fThreads.count(); i++) {
            fThreads[i]->join();
        }
    }

    virtual void add(std::function<void(void)> work) override {
        {
            SkAutoExclusive lock(fWorkLock);
            fWork.emplace_back(std::move(work));
        }
        fWorkAvailable.signal(1);
    }

    virtual void yield() override {
        thread_yield();
    }

    virtual void borrow() override {
        if (fWorkAvailable.try_wait()) {
            SkAssertResult(this->do_work());
        }
    }

private:
    bool do_work() {
        std::function<void(void)> work;
        {
            SkAutoExclusive lock(fWorkLock);
            SkASSERT(!fWork.empty());
            work = std::move(fWork.back());
            fWork.pop_back();
        }
        if (work) {
            work();
            return true;
        }
        return false;
    }

    static void Loop(void* ctx) {
        auto pool = (SkThreadPool*)ctx;
        do {
            pool->fWorkAvailable.wait();
        } while (pool->do_work());
    }

    SkTArray<std::unique_ptr<SkThread>> fThreads;
    SkTArray<std::function<void(void)>> fWork;
    SkSpinlock                          fWorkLock;
    SkSemaphore                         fWorkAvailable;
};

SkExecutor* SkExecutor::NewThreadPool(int threads) {
    return new SkThreadPool(threads > 0 ? threads : num_cores());
}
