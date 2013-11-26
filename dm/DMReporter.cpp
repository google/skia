#include "DMReporter.h"

#include "SkCommandLineFlags.h"

DEFINE_bool(quiet, false, "If true, don't print status updates.");

namespace DM {

void Reporter::updateStatusLine() const {
    if (FLAGS_quiet) {
        return;
    }

    SkString status;
    status.printf("\r\033[K%d tasks left", this->started() - this->finished());
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
