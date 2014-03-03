#include "DMReporter.h"

#include "SkCommandLineFlags.h"
#include "OverwriteLine.h"

DEFINE_bool2(quiet, q, false, "If true, don't print status updates.");
DEFINE_bool2(verbose, v, false, "If true, print status updates one-per-line.");

namespace DM {

void Reporter::finish(SkString name, SkMSec timeMs) {
    sk_atomic_inc(&fFinished);

    if (FLAGS_quiet) {
        return;
    }

    SkString status;
    status.printf("%s%d tasks left",
                  FLAGS_verbose ? "\n" : kSkOverwriteLine,
                  this->started() - this->finished());
    const int failed = this->failed();
    if (failed > 0) {
        status.appendf(", %d failed", failed);
    }
    if (FLAGS_verbose) {
        status.appendf("\t%5dms %s", timeMs, name.c_str());
    }
    SkDebugf("%s", status.c_str());
}

int32_t Reporter::failed() const {
    SkAutoMutexAcquire reader(&fMutex);
    return fFailures.count();
}

void Reporter::fail(SkString msg) {
    SkAutoMutexAcquire writer(&fMutex);
    fFailures.push_back(msg);
}

void Reporter::getFailures(SkTArray<SkString>* failures) const {
    SkAutoMutexAcquire reader(&fMutex);
    *failures = fFailures;
}

}  // namespace DM
