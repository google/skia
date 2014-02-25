#include "DMReporter.h"

#include "SkCommandLineFlags.h"
#include "OverwriteLine.h"

DEFINE_bool(quiet, false, "If true, don't print status updates.");

namespace DM {

void Reporter::updateStatusLine() const {
    if (FLAGS_quiet) {
        return;
    }

    SkString status;
    status.printf("%s%d tasks left", kSkOverwriteLine, this->started() - this->finished());
    const int failed = this->failed();
    if (failed > 0) {
        status.appendf(", %d failed", failed);
    }
    SkDebugf(status.c_str());
}

int32_t Reporter::failed() const {
    SkAutoMutexAcquire reader(&fMutex);
    return fFailures.count();
}

void Reporter::fail(SkString name) {
    SkAutoMutexAcquire writer(&fMutex);
    fFailures.push_back(name);
}

void Reporter::getFailures(SkTArray<SkString>* failures) const {
    SkAutoMutexAcquire reader(&fMutex);
    *failures = fFailures;
}

}  // namespace DM
