#include "DMTaskRunner.h"
#include "DMTask.h"

namespace DM {

void TaskRunner::add(CpuTask* task) { fCpuWork.add(task);  }
void TaskRunner::add(GpuTask* task) { fGpuWork.push(task); }

void TaskRunner::wait() {
    GrContextFactory factory;
    for (int i = 0; i < fGpuWork.count(); i++) {
        fGpuWork[i]->run(&factory);
    }
    fCpuWork.wait();
}

}  // namespace DM
