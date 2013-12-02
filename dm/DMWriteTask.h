#ifndef DMWriteTask_DEFINED
#define DMWriteTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTArray.h"

// Writes a bitmap to a file.

namespace DM {

class WriteTask : public Task {

public:
    WriteTask(const Task& parent,  // WriteTask must be a child Task.  Pass its parent here.
              SkBitmap bitmap);    // Bitmap to write.

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE;

private:
    SkTArray<SkString> fSuffixes;
    SkString fGmName;
    const SkBitmap fBitmap;

    void makeDirOrFail(SkString dir);
};

}  // namespace DM

#endif  // DMWriteTask_DEFINED
