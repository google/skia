#include "DMChecksumTask.h"
#include "DMUtil.h"

namespace DM {

ChecksumTask::ChecksumTask(const Task& parent,
                           skiagm::Expectations expectations,
                           SkBitmap bitmap)
    : Task(parent)
    , fName(parent.name())  // Masquerade as parent so failures are attributed to it.
    , fExpectations(expectations)
    , fBitmap(bitmap)
    {}

void ChecksumTask::draw() {
    if (fExpectations.ignoreFailure() || fExpectations.empty()) {
        return;
    }

    const skiagm::GmResultDigest digest(fBitmap);
    if (!fExpectations.match(digest)) {
        this->fail();
    }
}

}  // namespace DM
