#include "DMReplayTask.h"
#include "DMWriteTask.h"
#include "DMUtil.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"

DEFINE_bool(replay, false, "If true, run picture replay tests.");

namespace DM {

ReplayTask::ReplayTask(const Task& parent,
                       skiagm::GM* gm,
                       SkBitmap reference)
    : Task(parent)
    , fName(UnderJoin(parent.name().c_str(), "replay"))
    , fGM(gm)
    , fReference(reference)
    {}

void ReplayTask::draw() {
    SkPicture recorded;
    RecordPicture(fGM.get(), &recorded);

    SkBitmap bitmap;
    SetupBitmap(fReference.config(), fGM.get(), &bitmap);
    DrawPicture(&recorded, &bitmap);
    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    }
}

bool ReplayTask::shouldSkip() const {
    return !FLAGS_replay || fGM->getFlags() & skiagm::GM::kSkipPicture_Flag;
}

}  // namespace DM
