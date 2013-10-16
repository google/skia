#ifndef DMReplayTask_DEFINED
#define DMReplayTask_DEFINED

#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"
#include "gm_expectations.h"

// Records a GM through an SkPicture, draws it, and compares against the reference checksum.

namespace DM {

class ReplayTask : public Task {

public:
    ReplayTask(const char* name,
               const Task& parent,
               skiagm::GM*,
               skiagm::GmResultDigest reference,
               SkBitmap::Config);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    SkAutoTDelete<skiagm::GM> fGM;
    const skiagm::GmResultDigest fReference;
    const SkBitmap::Config fConfig;
};

}  // namespace DM

#endif  // DMReplayTask_DEFINED
