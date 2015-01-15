#ifndef DMQuiltTask_DEFINED
#define DMQuiltTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// Records a GM through an SkPicture, draws it in tiles, and compares against the reference bitmap.

namespace DM {

class QuiltTask : public CpuTask {
public:
    enum BBH {
        kNone_BBH,
        kRTree_BBH,
    };

    QuiltTask(const Task& parent,  // QuiltTask must be a child task.  Pass its parent here.
              skiagm::GM*,         // GM to run through a picture.  Takes ownership.
              SkBitmap reference,  // Bitmap to compare picture replay results to.
              BBH);

    void draw() SK_OVERRIDE;
    bool shouldSkip() const SK_OVERRIDE;
    SkString name() const SK_OVERRIDE { return fName; }

private:
    const BBH fBBH;
    const SkString fName;
    SkAutoTDelete<skiagm::GM> fGM;
    const SkBitmap fReference;
};

}  // namespace DM

#endif  // DMReplayTask_DEFINED
