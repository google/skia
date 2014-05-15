#include "DMReplayTask.h"
#include "DMWriteTask.h"
#include "DMUtil.h"

#include "SkBBHFactory.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"

DEFINE_bool(replay, true, "If true, run picture replay tests.");
DEFINE_bool(rtree,  true, "If true, run picture replay tests with an rtree.");

namespace DM {

ReplayTask::ReplayTask(const Task& parent,
                       skiagm::GM* gm,
                       SkBitmap reference,
                       Mode mode)
    : CpuTask(parent)
    , fUseRTree(mode == kRTree_Mode)
    , fName(UnderJoin(parent.name().c_str(), fUseRTree ? "rtree" : "replay"))
    , fGM(gm)
    , fReference(reference)
    {}

void ReplayTask::draw() {
    SkAutoTDelete<SkBBHFactory> factory;
    if (fUseRTree) {
        factory.reset(SkNEW(SkRTreeFactory));
    }
    SkAutoTUnref<SkPicture> recorded(RecordPicture(fGM.get(), 0, factory.get()));

    SkBitmap bitmap;
    AllocatePixels(fReference, &bitmap);
    DrawPicture(recorded, &bitmap);
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
