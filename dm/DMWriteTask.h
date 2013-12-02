#ifndef DMWriteTask_DEFINED
#define DMWriteTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"

// Writes a bitmap to a file.

namespace DM {

class WriteTask : public Task {

public:
    WriteTask(const Task& parent, SkBitmap bitmap);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE;

private:
    SkString fConfig;
    SkString fGmName;
    const SkBitmap fBitmap;
};

}  // namespace DM

#endif  // DMWriteTask_DEFINED
