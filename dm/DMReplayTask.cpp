#include "DMReplayTask.h"
#include "DMWriteTask.h"
#include "DMUtil.h"

#include "SkBBHFactory.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"

DEFINE_bool(replay, true, "If true, run picture replay tests.");
DEFINE_bool(rtree,  true, "If true, run picture replay tests with an rtree.");
DEFINE_bool(skr,    true, "If true, run picture replay tests with SkRecord backend.");

static const char* kSuffixes[] = { "replay", "rtree", "skr" };
static const bool* kEnabled[]  = { &FLAGS_replay, &FLAGS_rtree, &FLAGS_skr };

namespace DM {

ReplayTask::ReplayTask(const Task& parent,
                       skiagm::GM* gm,
                       SkBitmap reference,
                       Mode mode)
    : CpuTask(parent)
    , fMode(mode)
    , fName(UnderJoin(parent.name().c_str(), kSuffixes[mode]))
    , fGM(gm)
    , fReference(reference)
    {}

void ReplayTask::draw() {
    SkAutoTDelete<SkBBHFactory> factory;
    if (kRTree_Mode == fMode) {
        factory.reset(SkNEW(SkRTreeFactory));
    }
    SkAutoTUnref<SkPicture> recorded(
            RecordPicture(fGM.get(), factory.get(), kSkRecord_Mode == fMode));

    SkBitmap bitmap;
    AllocatePixels(fReference, &bitmap);
    DrawPicture(*recorded, &bitmap);
    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    }
}

bool ReplayTask::shouldSkip() const {
    if (fGM->getFlags() & skiagm::GM::kSkipPicture_Flag) {
        return true;
    }
    return !*kEnabled[fMode];
}

}  // namespace DM
