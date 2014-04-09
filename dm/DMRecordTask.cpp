#include "DMRecordTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"
#include "SkRecordDraw.h"
#include "SkRecorder.h"

DEFINE_bool(skr, false, "If true, run SKR tests.");

namespace DM {

RecordTask::RecordTask(const Task& parent, skiagm::GM* gm, SkBitmap reference)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), "skr"))
    , fGM(gm)
    , fReference(reference)
    {}

void RecordTask::draw() {
    // Record the GM into an SkRecord.
    SkRecord record;
    SkRecorder canvas(SkRecorder::kWriteOnly_Mode, &record,
                      fReference.width(), fReference.height());
    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);

    // Draw the SkRecord back into a bitmap.
    SkBitmap bitmap;
    SetupBitmap(fReference.colorType(), fGM.get(), &bitmap);
    SkCanvas target(bitmap);

    SkRecordDraw(record, &target);

    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    }
}

bool RecordTask::shouldSkip() const {
    return !FLAGS_skr;
}

}  // namespace DM
