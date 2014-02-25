#ifndef DMTaskRunner_DEFINED
#define DMTaskRunner_DEFINED

#include "SkThreadPool.h"
#include "SkTypes.h"

// TaskRunner runs Tasks on one of two threadpools depending on the Task's usesGpu() method.
// This lets us drive the GPU with a small number of threads (e.g. 2 or 4 can be faster than 1)
// while not swamping it with requests from the full fleet of threads that CPU-bound tasks run on.

namespace DM {

class Task;

class TaskRunner : SkNoncopyable {
public:
    TaskRunner(int cputhreads, int gpuThreads);

    void add(Task* task);
    void wait();

private:
    SkThreadPool fMain, fGpu;
};

}  // namespace DM

#endif  // DMTaskRunner_DEFINED
