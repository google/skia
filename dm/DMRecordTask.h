#ifndef DMRecordTask_DEFINED
#define DMRecordTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkPicture.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// Records a GM or SKP through an SkRecord, draws it, and compares against the reference bitmap.

namespace DM {

class RecordTask : public CpuTask {

public:
    enum Mode {
        kNoOptimize_Mode,
        kOptimize_Mode,
    };
    RecordTask(const Task& parent, skiagm::GM*, SkBitmap reference, Mode);
    RecordTask(const Task& parent, SkPicture*,  SkBitmap reference, Mode);

    virtual void draw() SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    bool fOptimize;
    const SkString fName;
    SkAutoTUnref<SkPicture> fPicture;
    SkAutoTDelete<skiagm::GM> fGM;
    const SkBitmap fReference;
};

}  // namespace DM

#endif  // DMRecordTask_DEFINED
