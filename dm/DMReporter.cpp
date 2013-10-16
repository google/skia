#include "DMReporter.h"

namespace DM {

void Reporter::updateStatusLine() const {
    SkDebugf("\r\033[K%d / %d, %d failed", this->finished(), this->started(), this->failed());
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
