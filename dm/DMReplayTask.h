#ifndef DMReplayTask_DEFINED
#define DMReplayTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// Records a GM through an SkPicture, draws it, and compares against the reference bitmap.

namespace DM {

class ReplayTask : public Task {

public:
    ReplayTask(const Task& parent,  // ReplayTask must be a child task.  Pass its parent here.
               skiagm::GM*,         // GM to run through a picture.  Takes ownership.
               SkBitmap reference,  // Bitmap to compare picture replay results to.
               bool useRTree);      // Record with an RTree?

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    SkAutoTDelete<skiagm::GM> fGM;
    const SkBitmap fReference;
    const bool fUseRTree;
};

}  // namespace DM

#endif  // DMReplayTask_DEFINED
