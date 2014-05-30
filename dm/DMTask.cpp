#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkCommandLineFlags.h"

DEFINE_bool(cpu, true, "Master switch for running CPU-bound work.");
DEFINE_bool(gpu, true, "Master switch for running GPU-bound work.");

DECLARE_bool(dryRun);

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

void Task::spawnChildNext(CpuTask* task) {
    fTaskRunner->addNext(task);
}

CpuTask::CpuTask(Reporter* reporter, TaskRunner* taskRunner) : Task(reporter, taskRunner) {}
CpuTask::CpuTask(const Task& parent) : Task(parent) {}

void CpuTask::run() {
    if (FLAGS_cpu && !this->shouldSkip()) {
        this->start();
        if (!FLAGS_dryRun) this->draw();
        this->finish();
    }
    SkDELETE(this);
}

void CpuTask::spawnChild(CpuTask* task) {
    // Run children serially on this (CPU) thread.  This tends to save RAM and is usually no slower.
    // Calling spawnChildNext() is nearly equivalent, but it'd pointlessly contend on the
    // threadpool; spawnChildNext() is most useful when you want to change threadpools.
    task->run();
}

GpuTask::GpuTask(Reporter* reporter, TaskRunner* taskRunner) : Task(reporter, taskRunner) {}

void GpuTask::run(GrContextFactory& factory) {
    if (FLAGS_gpu && !this->shouldSkip()) {
        this->start();
        if (!FLAGS_dryRun) this->draw(&factory);
        this->finish();
    }
    SkDELETE(this);
}

void GpuTask::spawnChild(CpuTask* task) {
    // Really spawn a new task so it runs on the CPU threadpool instead of the GPU one we're on now.
    // It goes on the front of the queue to minimize the time we must hold reference bitmaps in RAM.
    this->spawnChildNext(task);
}

}  // namespace DM
