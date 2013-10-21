#ifndef DMSerializeTask_DEFINED
#define DMSerializeTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// Record a picture, serialize it, deserialize it, then draw it and compare to reference bitmap.

namespace DM {

class SerializeTask : public Task {

public:
    SerializeTask(const Task& parent,
                  skiagm::GM*,
                  SkBitmap reference);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return false; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    const SkString fName;
    SkAutoTDelete<skiagm::GM> fGM;
    const SkBitmap fReference;
};

}  // namespace DM

#endif  // DMSerializeTask_DEFINED
