#include "DMTaskRunner.h"
#include "DMTask.h"

namespace DM {

TaskRunner::TaskRunner(int cputhreads, int gpuThreads)
    : fMain(cputhreads)
    , fGpu(gpuThreads)
    {}

void TaskRunner::add(Task* task) {
    if (task->usesGpu()) {
        fGpu.add(task);
    } else {
        fMain.add(task);
    }
}

void TaskRunner::wait() {
    // These wait calls block until the threadpool is done.  We don't allow
    // children to spawn new GPU tasks so we can wait for that first knowing
    // we'll never try to add to it later.  Same can't be said of fMain: fGpu
    // and fMain can both add tasks to fMain, so we have to wait for that last.
    fGpu.wait();
    fMain.wait();
}

}  // namespace DM
