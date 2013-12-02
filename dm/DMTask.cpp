#include "DMTask.h"

#include "DMTaskRunner.h"
#include "DMUtil.h"
#include "SkBitmap.h"
#include "SkCommandLineFlags.h"

namespace DM {

Task::Task(Reporter* reporter, TaskRunner* taskRunner)
    : fReporter(reporter), fTaskRunner(taskRunner), fDepth(0) {
    fReporter->start();
}

Task::Task(const Task& parent)
    : INHERITED(parent)
    , fReporter(parent.fReporter)
    , fTaskRunner(parent.fTaskRunner)
    , fDepth(parent.depth()+1) {
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
