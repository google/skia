#include "DMTask.h"
#include "DMTaskRunner.h"

namespace DM {

Task::Task(Reporter* reporter, TaskRunner* taskRunner)
    : fReporter(reporter)
    , fTaskRunner(taskRunner)
    , fDepth(0) {
    fReporter->start();
}

Task::Task(const Task& parent)
    : fReporter(parent.fReporter)
    , fTaskRunner(parent.fTaskRunner)
    , fDepth(parent.depth() + 1) {
    fReporter->start();
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
    fReporter->finish(this->name(), SkTime::GetMSecs() - fStart);
}

void Task::spawnChild(CpuTask* task) {
    fTaskRunner->add(task);
}

CpuTask::CpuTask(Reporter* reporter, TaskRunner* taskRunner) : Task(reporter, taskRunner) {}
CpuTask::CpuTask(const Task& parent) : Task(parent) {}

void CpuTask::run() {
    this->start();
    if (!this->shouldSkip()) {
        this->draw();
    }
    this->finish();
    SkDELETE(this);
}

GpuTask::GpuTask(Reporter* reporter, TaskRunner* taskRunner) : Task(reporter, taskRunner) {}

void GpuTask::run(GrContextFactory& factory) {
    this->start();
    if (!this->shouldSkip()) {
        this->draw(&factory);
    }
    this->finish();
    SkDELETE(this);
}



}  // namespace DM
