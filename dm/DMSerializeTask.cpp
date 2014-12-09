#include "DMSerializeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkPixelRef.h"

DEFINE_bool(serialize, true, "If true, run picture serialization tests via SkPictureData.");

namespace DM {

SerializeTask::SerializeTask(const Task& parent, skiagm::GM* gm, SkBitmap reference)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), "serialize"))
    , fGM(gm)
    , fReference(reference)
    {}

void SerializeTask::draw() {
    SkAutoTUnref<SkPicture> recorded(RecordPicture(fGM.get(), NULL/*no BBH*/));

    SkDynamicMemoryWStream wStream;
    recorded->serialize(&wStream);
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
    return !FLAGS_serialize;
}

}  // namespace DM
