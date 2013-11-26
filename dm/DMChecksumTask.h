#ifndef DMChecksumTask_DEFINED
#define DMChecksumTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "gm_expectations.h"

namespace DM {

// ChecksumTask compares an SkBitmap against some Expectations.
// Moving this off the GPU threadpool is a nice (~30%) runtime win.
class ChecksumTask : public Task {
public:
    ChecksumTask(const Task& parent, skiagm::Expectations, SkBitmap);

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

#endif  // DMChecksumTask_DEFINED
