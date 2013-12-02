#ifndef DMComparisonTask_DEFINED
#define DMComparisonTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "gm_expectations.h"

namespace DM {

// We use ComparisonTask to move CPU-bound comparison work of GpuTasks back to
// the main thread pool, where we probably have more threads available.

class ComparisonTask : public Task {
public:
    ComparisonTask(const Task& parent, skiagm::Expectations, SkBitmap);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE { return false; }
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    const skiagm::Expectations fExpectations;
    const SkBitmap fBitmap;
};

}  // namespace DM

#endif  // DMComparisonTask_DEFINED
