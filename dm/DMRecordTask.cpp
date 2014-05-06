#include "DMRecordTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"
#include "SkRecord.h"
#include "SkRecordDraw.h"
#include "SkRecordOpts.h"
#include "SkRecorder.h"

DEFINE_bool(skr, true, "If true, run SKR tests.");

namespace DM {

RecordTask::RecordTask(const Task& parent, skiagm::GM* gm, SkBitmap reference, bool optimize)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), optimize ? "skr" : "skr-noopt"))
    , fGM(gm)
    , fReference(reference)
    , fOptimize(optimize)
    {}

void RecordTask::draw() {
    // Record the GM into an SkRecord.
    SkRecord record;
    SkRecorder recorder(SkRecorder::kWriteOnly_Mode, &record,
                        fReference.width(), fReference.height());
    recorder.concat(fGM->getInitialTransform());
    fGM->draw(&recorder);

    if (fOptimize) {
        SkRecordOptimize(&record);
    }

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
