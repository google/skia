#include "DMTask.h"

#include "DMTaskRunner.h"
#include "DMUtil.h"
#include "SkBitmap.h"
#include "SkCommandLineFlags.h"

namespace DM {

Task::Task(Reporter* reporter, TaskRunner* taskRunner)
    : fReporter(reporter), fTaskRunner(taskRunner) {
    fReporter->start();
}

Task::Task(const Task& that) : fReporter(that.fReporter), fTaskRunner(that.fTaskRunner) {
    fReporter->start();
}

Task::~Task() {}

void Task::run() {
    if (!this->shouldSkip()) {
        this->draw();
    }
    fReporter->finish();
    fReporter->updateStatusLine();
    delete this;
}

void Task::spawnChild(Task* task) {
    if (!task->usesGpu()) {
        fTaskRunner->add(task);
    } else {
        SkDEBUGFAIL("Sorry, we can't spawn GPU tasks. :(  See comment in TaskRunner::wait().");
    }
}

void Task::fail() {
    fReporter->fail(this->name());
}

}  // namespace DM
