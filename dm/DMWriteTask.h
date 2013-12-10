#ifndef DMWriteTask_DEFINED
#define DMWriteTask_DEFINED

#include "DMExpectations.h"
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

    // Reads image files WriteTask wrote under root and compares them with bitmap.
    class Expectations : public DM::Expectations {
    public:
        explicit Expectations(const char* root) : fRoot(root) {}

        bool check(const Task& task, SkBitmap bitmap) const SK_OVERRIDE;
    private:
        const char* fRoot;
    };

private:
    SkTArray<SkString> fSuffixes;
    SkString fGmName;
    const SkBitmap fBitmap;

    void makeDirOrFail(SkString dir);
};

}  // namespace DM

#endif  // DMWriteTask_DEFINED
