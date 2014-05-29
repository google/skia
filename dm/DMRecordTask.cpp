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

RecordTask::RecordTask(const Task& parent, skiagm::GM* gm, SkBitmap reference, Mode mode)
    : CpuTask(parent)
    , fOptimize(mode == kOptimize_Mode)
    , fName(UnderJoin(parent.name().c_str(), fOptimize ? "skr" : "skr-noopt"))
    , fGM(gm)
    , fReference(reference)
    {}

RecordTask::RecordTask(const Task& parent, SkPicture* pic, SkBitmap reference, Mode mode)
    : CpuTask(parent)
    , fOptimize(mode == kOptimize_Mode)
    , fName(UnderJoin(parent.name().c_str(), fOptimize ? "skr" : "skr-noopt"))
    , fPicture(SkRef(pic))
    , fReference(reference)
    {}

void RecordTask::draw() {
    // Record into an SkRecord.
    SkRecord record;
    SkRecorder recorder(&record, fReference.width(), fReference.height());

    if (fGM.get()) {
        recorder.concat(fGM->getInitialTransform());
        fGM->draw(&recorder);
    } else {
        fPicture->draw(&recorder);
    }


    if (fOptimize) {
        SkRecordOptimize(&record);
    }

    // Draw the SkRecord back into a bitmap.
    SkBitmap bitmap;
    AllocatePixels(fReference, &bitmap);
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
