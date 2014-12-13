#ifndef DMImageTask_DEFINED
#define DMImageTask_DEFINED

#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkData.h"
#include "SkString.h"

// Decode an image into its natural bitmap, perhaps decoding random subsets.

namespace DM {

class ImageTask : public CpuTask {
public:
    ImageTask(Reporter*, TaskRunner*, const SkData*, SkString name, int subsets = 0);

    void draw() SK_OVERRIDE;
    bool shouldSkip() const SK_OVERRIDE { return false; }
    SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTUnref<const SkData> fEncoded;
    const SkString fName;
    int fSubsets;
};

}  // namespace DM

#endif // DMImageTask_DEFINED
