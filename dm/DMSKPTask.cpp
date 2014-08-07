#include "DMSKPTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPictureRecorder.h"

DEFINE_bool(skr, true, "Test that SKPs draw the same when re-recorded with SkRecord backend.");
DEFINE_int32(skpMaxWidth,  1000, "Max SKPTask viewport width.");
DEFINE_int32(skpMaxHeight, 1000, "Max SKPTask viewport height.");

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
        AllocatePixels(kN32_SkColorType, fReference.width(), fReference.height(), &bitmap);
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
    const int width  = SkTMin(fPicture->width(),  FLAGS_skpMaxWidth),
              height = SkTMin(fPicture->height(), FLAGS_skpMaxHeight);
    SkBitmap bitmap;
    AllocatePixels(kN32_SkColorType, width, height, &bitmap);
    DrawPicture(*fPicture, &bitmap);

    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    this->spawnChild(SkNEW_ARGS(SkrComparisonTask, (*this, fPicture.get(), bitmap)));
}

}  // namespace DM
