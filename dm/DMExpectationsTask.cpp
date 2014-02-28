#include "DMExpectationsTask.h"
#include "DMUtil.h"

namespace DM {

ExpectationsTask::ExpectationsTask(const Task& parent,
                                   const Expectations& expectations,
                                   SkBitmap bitmap)
    : CpuTask(parent)
    , fName(parent.name())  // Masquerade as parent so failures are attributed to it.
    , fExpectations(expectations)
    , fBitmap(bitmap)
    {}

void ExpectationsTask::draw() {
    if (!fExpectations.check(*this, fBitmap)) {
        this->fail();
    }
}

}  // namespace DM
