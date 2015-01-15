#ifndef DMPipeTask_DEFINED
#define DMPipeTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// Sends a GM through a pipe, draws it, and compares against the reference bitmap.

namespace DM {

class PipeTask : public CpuTask {

public:
    enum Mode {
        kInProcess_Mode,
        kCrossProcess_Mode,
        kSharedAddress_Mode,
    };

    PipeTask(const Task& parent,        // PipeTask must be a child task.  Pass its parent here.
             skiagm::GM*,               // GM to run through a pipe.  Takes ownership.
             SkBitmap reference,        // Bitmap to compare pipe results to.
             Mode);

    void draw() SK_OVERRIDE;
    bool shouldSkip() const SK_OVERRIDE;
    SkString name() const SK_OVERRIDE { return fName; }

private:
    const uint32_t fFlags;
    const SkString fName;
    SkAutoTDelete<skiagm::GM> fGM;
    const SkBitmap fReference;
};

}  // namespace DM

#endif  // DMPipeTask_DEFINED
