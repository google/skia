#ifndef DMWriteTask_DEFINED
#define DMWriteTask_DEFINED

#include "DMExpectations.h"
#include "DMTask.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTArray.h"


// Writes a bitmap to a file.

namespace DM {

class WriteTask : public CpuTask {

public:
    WriteTask(const Task& parent,  // WriteTask must be a child task.
              SkBitmap bitmap);    // Bitmap to encode to PNG and write to disk.

    WriteTask(const Task& parent,   // WriteTask must be a child task.
              SkData *data,         // Pre-encoded data to write to disk.
              const char* ext);     // File extension.

    virtual void draw() SK_OVERRIDE;
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
    const SkString fGmName;
    const SkBitmap fBitmap;
    SkAutoTUnref<SkData> fData;
    const char* fExtension;

    void makeDirOrFail(SkString dir);
};

}  // namespace DM

#endif  // DMWriteTask_DEFINED
