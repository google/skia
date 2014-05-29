#ifndef DMTask_DEFINED
#define DMTask_DEFINED

#include "DMReporter.h"
#include "DMGpuSupport.h"
#include "SkRunnable.h"
#include "SkTime.h"

// DM will run() these tasks on one of two threadpools.
// Subclasses can call fail() to mark this task as failed, or make any number of spawnChild() calls
// to kick off dependent tasks.
//
// Tasks delete themselves when run.

namespace DM {

class TaskRunner;

class CpuTask;

class Task {
public:
    virtual bool shouldSkip() const = 0;
    virtual SkString name() const = 0;

    // Returns the number of parents above this task.
    // Top-level tasks return 0, their children 1, and so on.
    int depth() const { return fDepth; }

protected:
    Task(Reporter* reporter, TaskRunner* taskRunner);
    Task(const Task& parent);
    virtual ~Task();

    void start();
    void fail(const char* msg = NULL);
    void finish();

    void spawnChildNext(CpuTask* task);  // For now we don't allow GPU child tasks.

private:
    Reporter* fReporter;      // Unowned.
    TaskRunner* fTaskRunner;  // Unowned.
    int fDepth;
    SkMSec fStart;
};

class CpuTask : public Task, public SkRunnable {
public:
    CpuTask(Reporter* reporter, TaskRunner* taskRunner);
    CpuTask(const Task& parent);
    virtual ~CpuTask() {}

    void run() SK_OVERRIDE;
    virtual void draw() = 0;

    void spawnChild(CpuTask* task);
};

class GpuTask : public Task, public SkTRunnable<GrContextFactory> {
 public:
    GpuTask(Reporter* reporter, TaskRunner* taskRunner);
    virtual ~GpuTask() {}

    void run(GrContextFactory&) SK_OVERRIDE;
    virtual void draw(GrContextFactory*) = 0;

    void spawnChild(CpuTask* task);
};

}  // namespace DM

#endif  // DMTask_DEFINED
