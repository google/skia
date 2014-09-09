#include "DMSerializeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkPixelRef.h"

DEFINE_bool(serialize,     true, "If true, run picture serialization tests via SkPictureData.");
DEFINE_bool(serialize_skr, true, "If true, run picture serialization tests via SkRecord.");

static const char* kSuffixes[] = { "serialize", "serialize_skr" };
static const bool* kEnabled[]  = { &FLAGS_serialize, &FLAGS_serialize_skr };

namespace DM {

SerializeTask::SerializeTask(const Task& parent,
                             skiagm::GM* gm,
                             SkBitmap reference,
                             SerializeTask::Mode mode)
    : CpuTask(parent)
    , fMode(mode)
    , fName(UnderJoin(parent.name().c_str(), kSuffixes[mode]))
    , fGM(gm)
    , fReference(reference)
    {}

void SerializeTask::draw() {
    SkAutoTUnref<SkPicture> recorded(
        RecordPicture(fGM.get(), NULL/*no BBH*/, kSkRecord_Mode == fMode));

    SkDynamicMemoryWStream wStream;
    recorded->serialize(&wStream, NULL);
    SkAutoTUnref<SkStream> rStream(wStream.detachAsStream());
    SkAutoTUnref<SkPicture> reconstructed(SkPicture::CreateFromStream(rStream));

    SkBitmap bitmap;
    AllocatePixels(fReference, &bitmap);
    DrawPicture(*reconstructed, &bitmap);
    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "GM", bitmap)));
    }
}

bool SerializeTask::shouldSkip() const {
    if (fGM->getFlags() & skiagm::GM::kSkipPicture_Flag) {
        return true;
    }
    return !*kEnabled[fMode];
}

}  // namespace DM
