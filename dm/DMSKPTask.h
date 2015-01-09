#ifndef DMSKPTask_DEFINED
#define DMSKPTask_DEFINED

#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkTemplates.h"

// Draws an SKP to a raster canvas, then compares it with some other modes.

namespace DM {

class SKPTask : public CpuTask {
public:
    SKPTask(Reporter*, TaskRunner*, const SkPicture*, SkString name);

    void draw() SK_OVERRIDE;
    bool shouldSkip() const SK_OVERRIDE { return false; }
    SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTUnref<const SkPicture> fPicture;
    const SkString fName;
};

}  // namespace DM

#endif // DMSKPTask_DEFINED
