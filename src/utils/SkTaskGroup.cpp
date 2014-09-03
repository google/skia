#include "SkTaskGroup.h"

#include "SkCondVar.h"
#include "SkLazyPtr.h"
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

static int gThreadCount = 0;

class ThreadPool : SkNoncopyable {
public:
    static void Add(SkRunnable* task, int32_t* pending) {
        Global()->add(task, pending);
    }

    static void Wait(int32_t* pending) {
        while (sk_acquire_load(pending) > 0) {  // Pairs with sk_atomic_dec here or in Loop.
            // Lend a hand until our SkTaskGroup of interest is done.
            ThreadPool* pool = Global();
            Work work;
            {
                AutoLock lock(&pool->fReady);
                if (pool->fWork.isEmpty()) {
                    // Someone has picked up all the work (including ours).  How nice of them!
                    // (They may still be working on it, so we can't assert *pending == 0 here.)
                    continue;
                }
                pool->fWork.pop(&work);
            }
            // This Work isn't necessarily part of our SkTaskGroup of interest, but that's fine.
            // We threads gotta stick together.  We're always making forward progress.
            work.task->run();
            sk_atomic_dec(work.pending);  // Release pairs with the sk_acquire_load() just above.
        }
    }

private:
    struct AutoLock {
        AutoLock(SkCondVar* c) : fC(c) { fC->lock(); }
        ~AutoLock() { fC->unlock(); }
    private:
        SkCondVar* fC;
    };

    struct Work {
        SkRunnable* task;  // A task to ->run(),
        int32_t* pending;  // then sk_atomic_dec(pending) afterwards.
    };

    static ThreadPool* Create() { return SkNEW(ThreadPool); }
    static void Destroy(ThreadPool* p) { SkDELETE(p); }
    static ThreadPool* Global() {
        SK_DECLARE_STATIC_LAZY_PTR(ThreadPool, global, Create, Destroy);
        return global.get();
    }

    ThreadPool() : fDraining(false) {
        const int threads = gThreadCount ? gThreadCount : num_cores();
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

    void add(SkRunnable* task, int32_t* pending) {
        Work work = { task, pending };
        sk_atomic_inc(pending);  // No barrier needed.
        {
            AutoLock lock(&fReady);
            fWork.push(work);
            fReady.signal();
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
            work.task->run();
            sk_atomic_dec(work.pending);  // Release pairs with sk_acquire_load() in Wait().
        }
    }

    SkTDArray<Work>      fWork;
    SkTDArray<SkThread*> fThreads;
    SkCondVar            fReady;
    bool                 fDraining;
};

}  // namespace

void SkTaskGroup::SetThreadCount(int n) { gThreadCount = n; }

SkTaskGroup::SkTaskGroup() : fPending(0) {}

void SkTaskGroup::add(SkRunnable* task) { ThreadPool::Add(task, &fPending); }
void SkTaskGroup::wait()                { ThreadPool::Wait(&fPending); }
