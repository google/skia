#ifndef DMExpectationsTask_DEFINED
#define DMExpectationsTask_DEFINED

#include "DMExpectations.h"
#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"

namespace DM {

// ExpectationsTask compares an SkBitmap against some Expectations.
// Moving this off the GPU threadpool is a nice (~30%) runtime win.
class ExpectationsTask : public Task {
public:
    ExpectationsTask(const Task& parent, const Expectations&, SkBitmap);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE { return false; }
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    const Expectations& fExpectations;
    const SkBitmap fBitmap;
};

}  // namespace DM

#endif  // DMExpectationsTask_DEFINED
