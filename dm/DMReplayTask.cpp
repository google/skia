#include "DMReplayTask.h"
#include "DMUtil.h"

#include "SkPicture.h"

namespace DM {

ReplayTask::ReplayTask(const char* suffix,
                       const Task& parent,
                       skiagm::GM* gm,
                       SkBitmap reference)
    : Task(parent)
    , fName(underJoin(parent.name().c_str(), suffix))
    , fGM(gm)
    , fReference(reference)
    {}

void ReplayTask::draw() {
    SkPicture picture;
    SkCanvas* canvas = picture.beginRecording(SkScalarCeilToInt(fGM->width()),
                                              SkScalarCeilToInt(fGM->height()),
                                              0 /*flags*/);

    canvas->concat(fGM->getInitialTransform());
    fGM->draw(canvas);
    canvas->flush();

    picture.endRecording();

    SkBitmap bitmap;
    bitmap.setConfig(fReference.config(),
                     SkScalarCeilToInt(fGM->width()),
                     SkScalarCeilToInt(fGM->height()));
    bitmap.allocPixels();
    bitmap.eraseColor(0x00000000);

    SkCanvas replay(bitmap);
    replay.drawPicture(picture);
    replay.flush();

    const SkAutoLockPixels mine(bitmap), theirs(fReference);
    if (bitmap.getSize() != fReference.getSize() ||
        0 != memcmp(bitmap.getPixels(), fReference.getPixels(), bitmap.getSize()))
    {
        this->fail();
    }
}

bool ReplayTask::shouldSkip() const {
    return fGM->getFlags() & skiagm::GM::kGPUOnly_Flag ||
           fGM->getFlags() & skiagm::GM::kSkipPicture_Flag;
}

}  // namespace
