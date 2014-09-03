#ifndef DMTaskRunner_DEFINED
#define DMTaskRunner_DEFINED

#include "DMGpuSupport.h"
#include "SkThreadPool.h"
#include "SkTypes.h"

// TaskRunner runs Tasks on one of two threadpools depending on the need for a GrContextFactory.
// It's typically a good idea to run fewer GPU threads than CPU threads (go nuts with those).

namespace DM {

class CpuTask;
class GpuTask;

class TaskRunner : SkNoncopyable {
public:
    explicit TaskRunner(int cpuThreads, int gpuThreads);

    void add(CpuTask* task);
    void addNext(CpuTask* task);
    void add(GpuTask* task);
    void wait();

private:
    SkTThreadPool<void> fCpu;
    SkTThreadPool<GrContextFactory> fGpu;
};

}  // namespace DM

#endif  // DMTaskRunner_DEFINED
