#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkCommonFlags.h"

namespace DM {

Task::Task(Reporter* reporter, TaskRunner* taskRunner)
    : fReporter(reporter)
    , fTaskRunner(taskRunner)
    , fDepth(0) {
    fReporter->taskCreated();
}

Task::Task(const Task& parent)
    : fReporter(parent.fReporter)
    , fTaskRunner(parent.fTaskRunner)
    , fDepth(parent.depth() + 1) {
    fReporter->taskCreated();
}

Task::~Task() {
    fReporter->taskDestroyed();
}

void Task::fail(const char* msg) {
    SkString failure(this->name());
    if (msg) {
        failure.appendf(": %s", msg);
    }
    fReporter->fail(failure);
}

void Task::start() {
    fStart = SkTime::GetMSecs();
}

void Task::finish() {
    fReporter->printStatus(this->name(), SkTime::GetMSecs() - fStart);
}

void Task::reallySpawnChild(CpuTask* task) {
    fTaskRunner->add(task);
}

CpuTask::CpuTask(Reporter* reporter, TaskRunner* taskRunner) : Task(reporter, taskRunner) {}
CpuTask::CpuTask(const Task& parent) : Task(parent) {}

void CpuTask::run() {
    // If the task says skip, or if we're starting a top-level CPU task and we don't want to, skip.
    const bool skip = this->shouldSkip() || (this->depth() == 0 && !FLAGS_cpu);
    if (!skip) {
        this->start();
        if (!FLAGS_dryRun) this->draw();
        this->finish();
    }
    SkDELETE(this);
}

void CpuTask::spawnChild(CpuTask* task) {
    // Run children serially on this (CPU) thread.  This tends to save RAM and is usually no slower.
    // Calling reallySpawnChild() is nearly equivalent, but it'd pointlessly contend on the
    // threadpool; reallySpawnChild() is most useful when you want to change threadpools.
    task->run();
}

GpuTask::GpuTask(Reporter* reporter, TaskRunner* taskRunner) : Task(reporter, taskRunner) {}

void GpuTask::run(GrContextFactory* factory) {
    // If the task says skip, or if we're starting a top-level GPU task and we don't want to, skip.
    const bool skip = this->shouldSkip() || (this->depth() == 0 && !FLAGS_gpu);
    if (!skip) {
        this->start();
        if (!FLAGS_dryRun) this->draw(factory);
        this->finish();
        if (FLAGS_abandonGpuContext) {
            factory->abandonContexts();
        }
        if (FLAGS_resetGpuContext || FLAGS_abandonGpuContext) {
            factory->destroyContexts();
        }
    }
    SkDELETE(this);
}

void GpuTask::spawnChild(CpuTask* task) {
    // Spawn a new task so it runs on the CPU threadpool instead of the GPU one we're on now.
    // It goes on the front of the queue to minimize the time we must hold reference bitmaps in RAM.
    this->reallySpawnChild(task);
}

}  // namespace DM
