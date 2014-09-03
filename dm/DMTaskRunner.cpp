#include "DMTaskRunner.h"
#include "DMTask.h"

namespace DM {

TaskRunner::TaskRunner(int cpuThreads, int gpuThreads) : fCpu(cpuThreads), fGpu(gpuThreads) {}

void TaskRunner::add(CpuTask* task) { fCpu.add(task); }
void TaskRunner::addNext(CpuTask* task) { fCpu.addNext(task); }
void TaskRunner::add(GpuTask* task) { fGpu.add(task); }

void TaskRunner::wait() {
    // These wait calls block until each threadpool is done.  We don't allow
    // spawning new child GPU tasks, so we can wait for that first knowing
    // we'll never try to add to it later.  Same can't be said of the CPU pool:
    // both CPU and GPU tasks can spawn off new CPU work, so we wait for that last.
    fGpu.wait();
    fCpu.wait();
}

}  // namespace DM
