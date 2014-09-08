#ifndef DMWriteTask_DEFINED
#define DMWriteTask_DEFINED

#include "DMExpectations.h"
#include "DMTask.h"
#include "SkBitmap.h"
#include "SkJSONCPP.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"


// Writes a bitmap to a file.

namespace DM {

class WriteTask : public CpuTask {

public:
    WriteTask(const Task& parent,  // WriteTask must be a child task.
              SkBitmap bitmap);    // Bitmap to encode to PNG and write to disk.

    // Takes ownership of SkStreamAsset
    WriteTask(const Task& parent,   // WriteTask must be a child task.
              SkStreamAsset* data,  // Pre-encoded data to write to disk.
              const char* ext);     // File extension.

    virtual void draw() SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE;

    // Reads JSON file WriteTask wrote under root and compares the bitmap with checksums inside.
    class Expectations : public DM::Expectations {
    public:
        static Expectations* Create(const char*);
        bool check(const Task& task, SkBitmap bitmap) const SK_OVERRIDE;
    private:
        Expectations() {}
        Json::Value fJson;
    };

    static void DumpJson();

private:
    SkTArray<SkString> fSuffixes;
    const SkString fFullName;
    const SkString fBaseName;
    const SkBitmap fBitmap;
    SkAutoTDelete<SkStreamAsset> fData;
    const char* fExtension;

    void makeDirOrFail(SkString dir);
};

}  // namespace DM

#endif  // DMWriteTask_DEFINED
