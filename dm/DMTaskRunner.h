#ifndef DMTaskRunner_DEFINED
#define DMTaskRunner_DEFINED

#include "GrContextFactory.h"
#include "SkThreadPool.h"
#include "SkTypes.h"

// TaskRunner runs Tasks on one of two threadpools depending on the Task's usesGpu() method.  This
// lets us drive the GPU from a single thread while parallelizing CPU-bound work.

namespace DM {

class Task;

class TaskRunner : SkNoncopyable {
public:
    explicit TaskRunner(int cputhreads);

    void add(Task* task);
    void wait();

    // This can only be safely called from a GPU task's draw() method.
    GrContextFactory* getGrContextFactory() const { return fGrContextFactory; }

private:
    SkThreadPool fMain, fGpu;
    GrContextFactory* fGrContextFactory;  // Created and destroyed on fGpu threadpool.
};

}  // namespace DM

#endif  // DMTaskRunner_DEFINED
