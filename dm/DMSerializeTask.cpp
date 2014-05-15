#include "DMSerializeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkPixelRef.h"

DEFINE_bool(serialize, true, "If true, run picture serialization tests.");

namespace DM {

SerializeTask::SerializeTask(const Task& parent,
                             skiagm::GM* gm,
                             SkBitmap reference)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), "serialize"))
    , fGM(gm)
    , fReference(reference)
    {}

void SerializeTask::draw() {
    SkAutoTUnref<SkPicture> recorded(RecordPicture(fGM.get()));

    SkDynamicMemoryWStream wStream;
    recorded->serialize(&wStream, NULL);
    SkAutoTUnref<SkStream> rStream(wStream.detachAsStream());
    SkAutoTUnref<SkPicture> reconstructed(SkPicture::CreateFromStream(rStream));

    SkBitmap bitmap;
    AllocatePixels(fReference, &bitmap);
    DrawPicture(reconstructed, &bitmap);
    if (!BitmapsEqual(bitmap, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
    }
}

bool SerializeTask::shouldSkip() const {
    return !FLAGS_serialize || fGM->getFlags() & skiagm::GM::kSkipPicture_Flag;
}

}  // namespace DM
