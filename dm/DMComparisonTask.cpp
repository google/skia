#include "DMComparisonTask.h"
#include "DMUtil.h"

namespace DM {

ComparisonTask::ComparisonTask(const Task& parent,
                               skiagm::Expectations expectations,
                               SkBitmap bitmap)
    : Task(parent)
    , fName(parent.name())  // Masquerade as parent so failures are attributed to it.
    , fExpectations(expectations)
    , fBitmap(bitmap)
    {}

void ComparisonTask::draw() {
    if (!MeetsExpectations(fExpectations, fBitmap)) {
        this->fail();
    }
}

}  // namespace DM
