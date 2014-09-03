#ifndef DMTaskRunner_DEFINED
#define DMTaskRunner_DEFINED

#include "DMGpuSupport.h"
#include "SkTDArray.h"
#include "SkTaskGroup.h"
#include "SkTypes.h"

// TaskRunner runs Tasks on one of two threadpools depending on the need for a GrContextFactory.
// We fix the number of GPU threads to 1, but go nuts with CPU threads.

namespace DM {

class CpuTask;
class GpuTask;

class TaskRunner : SkNoncopyable {
public:
    // 0 -> one thread per core
    explicit TaskRunner(int cpuThreads);

    void add(CpuTask* task);
    void add(GpuTask* task);
    void wait();

private:
    SkTaskGroup fCpuWork;
    SkTDArray<GpuTask*> fGpuWork;
};

}  // namespace DM

#endif  // DMTaskRunner_DEFINED
