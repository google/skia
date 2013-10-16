#include "DMReplayTask.h"
#include "DMUtil.h"

#include "SkPicture.h"

namespace DM {

ReplayTask::ReplayTask(const char* suffix,
                       const Task& parent,
                       skiagm::GM* gm,
                       skiagm::GmResultDigest reference,
                       SkBitmap::Config config)
    : Task(parent)
    , fName(underJoin(parent.name().c_str(), suffix))
    , fGM(gm)
    , fReference(reference)
    , fConfig(config)
    {}

void ReplayTask::draw() {
    SkPicture picture;
    SkCanvas* canvas = picture.beginRecording(fGM->width(), fGM->height(), 0 /*flags*/);

    canvas->concat(fGM->getInitialTransform());
    fGM->draw(canvas);
    canvas->flush();

    picture.endRecording();

    SkBitmap bitmap;
    bitmap.setConfig(fConfig, fGM->width(), fGM->height());
    bitmap.allocPixels();
    bitmap.eraseColor(0x00000000);

    SkCanvas replay(bitmap);
    replay.drawPicture(picture);
    replay.flush();

    const skiagm::GmResultDigest replayDigest(bitmap);
    if (!replayDigest.equals(fReference)) {
        this->fail();
    }
}

bool ReplayTask::shouldSkip() const {
    return fGM->getFlags() & skiagm::GM::kGPUOnly_Flag ||
           fGM->getFlags() & skiagm::GM::kSkipPicture_Flag;
}

}  // namespace
