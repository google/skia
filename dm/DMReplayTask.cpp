#include "DMReplayTask.h"
#include "DMWriteTask.h"
#include "DMUtil.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"

DEFINE_bool(replay, true, "If true, run picture replay tests.");
DEFINE_bool(rtree,  true, "If true, run picture replay tests with an rtree.");

namespace DM {

ReplayTask::ReplayTask(const Task& parent,
                       skiagm::GM* gm,
                       SkBitmap reference,
                       bool useRTree)
    : Task(parent)
    , fName(UnderJoin(parent.name().c_str(), useRTree ? "rtree" : "replay"))
    , fGM(gm)
    , fReference(reference)
    , fUseRTree(useRTree)
    {}

void ReplayTask::draw() {
    SkPicture recorded;
    const uint32_t flags = fUseRTree ? SkPicture::kOptimizeForClippedPlayback_RecordingFlag : 0;
    RecordPicture(fGM.get(), &recorded, flags);

    SkBitmap bitmap;
    SetupBitmap(fReference.config(), fGM.get(), &bitmap);
    DrawPicture(&recorded, &bitmap);
    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    }
}

bool ReplayTask::shouldSkip() const {
    if (fGM->getFlags() & skiagm::GM::kSkipPicture_Flag) {
        return true;
    }

    if (FLAGS_rtree && fUseRTree) {
        return false;
    }
    if (FLAGS_replay && !fUseRTree) {
        return false;
    }
    return true;
}

}  // namespace DM
