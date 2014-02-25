#include "DMTaskRunner.h"
#include "DMTask.h"

namespace DM {


TaskRunner::TaskRunner(int cputhreads)
    : fMain(cputhreads)
    , fGpu(1) {
    // Enqueue a task on the GPU thread to create a GrContextFactory.
    struct Create : public SkRunnable {
        Create(GrContextFactory** ptr) : fPtr(ptr) {}
        void run() SK_OVERRIDE {
            *fPtr = SkNEW(GrContextFactory);
            delete this;
        }
        GrContextFactory** fPtr;
    };
    fGpu.add(SkNEW_ARGS(Create, (&fGrContextFactory)));
}

void TaskRunner::add(Task* task) {
    if (task->usesGpu()) {
        fGpu.add(task);
    } else {
        fMain.add(task);
    }
}

void TaskRunner::wait() {
    // Enqueue a task on the GPU thread to destroy the GrContextFactory.
    struct Delete : public SkRunnable {
        Delete(GrContextFactory* ptr) : fPtr(ptr) {}
        void run() SK_OVERRIDE {
            delete fPtr;
            delete this;
        }
        GrContextFactory* fPtr;
    };
    fGpu.add(SkNEW_ARGS(Delete, (fGrContextFactory)));

    // These wait calls block until the threadpool is done.  We don't allow
    // children to spawn new GPU tasks so we can wait for that first knowing
    // we'll never try to add to it later.  Same can't be said of fMain: fGpu
    // and fMain can both add tasks to fMain, so we have to wait for that last.
    fGpu.wait();
    fMain.wait();
}

}  // namespace DM
