#ifndef DMTaskRunner_DEFINED
#define DMTaskRunner_DEFINED

#include "DMGpuSupport.h"
#include "SkTDArray.h"
#include "SkTaskGroup.h"
#include "SkTypes.h"

namespace DM {

class CpuTask;
class GpuTask;

class TaskRunner : SkNoncopyable {
public:
    TaskRunner() {}

    void add(CpuTask* task);
    void add(GpuTask* task);
    void wait();

private:
    SkTaskGroup fCpuWork;
    SkTDArray<GpuTask*> fGpuWork;
};

}  // namespace DM

#endif  // DMTaskRunner_DEFINED
