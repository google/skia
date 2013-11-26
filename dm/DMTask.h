#ifndef DMTask_DEFINED
#define DMTask_DEFINED

#include "DMReporter.h"
#include "SkRunnable.h"
#include "SkThreadPool.h"

// DM will run() these tasks on one of two threadpools, depending on the result
// of usesGpu().  The subclasses can call fail() to mark this task as failed,
// or make any number of spawnChild() calls to kick off dependent tasks.
//
// Task deletes itself when run.

namespace DM {

class TaskRunner;

class Task : public SkRunnable {
public:
    Task(Reporter* reporter, TaskRunner* taskRunner);
    Task(const Task& parent);
    virtual ~Task();

    void run() SK_OVERRIDE;

    virtual void draw() = 0;
    virtual bool usesGpu() const = 0;
    virtual bool shouldSkip() const = 0;
    virtual SkString name() const = 0;

    // Returns the number of parents above this task.
    // Top-level tasks return 0, their children 1, and so on.
    int depth() const { return fDepth; }

protected:
    void spawnChild(Task* task);
    void fail();

private:
    // Both unowned.
    Reporter* fReporter;
    TaskRunner* fTaskRunner;
    int fDepth;

    typedef SkRunnable INHERITED;
};

}  // namespace DM

#endif  // DMTask_DEFINED
