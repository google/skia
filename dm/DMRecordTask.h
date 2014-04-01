#ifndef DMRecordTask_DEFINED
#define DMRecordTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// Records a GM through an SkRecord, draws it, and compares against the reference bitmap.

namespace DM {

class RecordTask : public CpuTask {

public:
    RecordTask(const Task& parent, skiagm::GM*, SkBitmap reference);

    virtual void draw() SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    SkAutoTDelete<skiagm::GM> fGM;
    const SkBitmap fReference;
};

}  // namespace DM

#endif  // DMRecordTask_DEFINED
