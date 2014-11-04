#ifndef DMWriteTask_DEFINED
#define DMWriteTask_DEFINED

#include "DMTask.h"
#include "SkBitmap.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"


// Writes a bitmap to a file.

namespace DM {

class WriteTask : public CpuTask {

public:
    WriteTask(const Task& parent,      // WriteTask must be a child task.
              const char* sourceType,  // E.g. "GM", "SKP".  For humans.
              SkBitmap bitmap);        // Bitmap to encode to PNG and write to disk.

    // Takes ownership of SkStreamAsset
    WriteTask(const Task& parent,      // WriteTask must be a child task.
              const char* sourceType,  // E.g. "GM", "SKP".  For humans.
              SkStreamAsset* data,     // Pre-encoded data to write to disk.
              const char* ext);        // File extension.

    virtual void draw() SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE;

private:
    SkTArray<SkString> fSuffixes;
    const SkString fBaseName;
    const SkString fSourceType;
    const SkBitmap fBitmap;
    SkAutoTDelete<SkStreamAsset> fData;
    const char* fExtension;

    void makeDirOrFail(SkString dir);
};

}  // namespace DM

#endif  // DMWriteTask_DEFINED
