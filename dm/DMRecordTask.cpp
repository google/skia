#include "DMRecordTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"
#include "SkRecording.h"

DEFINE_bool(skr, true, "If true, run SKR tests.");

namespace DM {

RecordTask::RecordTask(const Task& parent, skiagm::GM* gm, SkBitmap reference)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), "skr"))
    , fGM(gm)
    , fReference(reference)
    {}

void RecordTask::draw() {
    // Record the GM into an SkRecord.
    EXPERIMENTAL::SkRecording recording(fReference.width(), fReference.height());
    recording.canvas()->concat(fGM->getInitialTransform());
    fGM->draw(recording.canvas());
    SkAutoTDelete<const EXPERIMENTAL::SkPlayback> playback(recording.releasePlayback());

    // Draw the SkRecord back into a bitmap.
    SkBitmap bitmap;
    SetupBitmap(fReference.colorType(), fGM.get(), &bitmap);
    SkCanvas target(bitmap);
    playback->draw(&target);

    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    }
}

bool RecordTask::shouldSkip() const {
    return !FLAGS_skr;
}

}  // namespace DM
