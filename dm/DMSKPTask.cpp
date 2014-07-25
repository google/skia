#include "DMSKPTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPictureRecorder.h"

DEFINE_bool(skr, true, "Test that SKPs draw the same when re-recorded with SkRecord backend.");

namespace DM {

// Test that an SkPicture plays back the same when re-recorded into an
// SkPicture backed by SkRecord.
class SkrComparisonTask : public CpuTask {
public:
    SkrComparisonTask(const Task& parent, const SkPicture* picture, SkBitmap reference)
        : CpuTask(parent)
        , fPicture(SkRef(picture))
        , fReference(reference)
        , fName(UnderJoin(parent.name().c_str(), "skr")) {}

    virtual bool shouldSkip() const SK_OVERRIDE { return !FLAGS_skr; }
    virtual SkString name() const SK_OVERRIDE { return fName; }

    virtual void draw() SK_OVERRIDE {
        SkPictureRecorder recorder;
        fPicture->draw(recorder.EXPERIMENTAL_beginRecording(fPicture->width(), fPicture->height()));
        SkAutoTDelete<const SkPicture> skrPicture(recorder.endRecording());

        SkBitmap bitmap;
        AllocatePixels(kN32_SkColorType, fPicture->width(), fPicture->height(), &bitmap);
        DrawPicture(*skrPicture, &bitmap);

        if (!BitmapsEqual(fReference, bitmap)) {
            this->fail();
            this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
        }
    }

private:
    SkAutoTUnref<const SkPicture> fPicture;
    const SkBitmap fReference;
    const SkString fName;
};


SKPTask::SKPTask(Reporter* r, TaskRunner* tr, const SkPicture* pic, SkString filename)
    : CpuTask(r, tr), fPicture(SkRef(pic)), fName(FileToTaskName(filename)) {}

void SKPTask::draw() {
    SkBitmap bitmap;
    AllocatePixels(kN32_SkColorType, fPicture->width(), fPicture->height(), &bitmap);
    DrawPicture(*fPicture, &bitmap);

    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    this->spawnChild(SkNEW_ARGS(SkrComparisonTask, (*this, fPicture.get(), bitmap)));
}

}  // namespace DM
