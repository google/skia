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
    : Task(parent)
    , fName(UnderJoin(parent.name().c_str(), "serialize"))
    , fGM(gm)
    , fReference(reference)
    {}

static SkData* trivial_bitmap_encoder(size_t* pixelRefOffset, const SkBitmap& bitmap) {
    if (NULL == bitmap.pixelRef()) {
        return NULL;
    }
    SkData* data = bitmap.pixelRef()->refEncodedData();
    *pixelRefOffset = bitmap.pixelRefOffset();
    return data;
}

void SerializeTask::draw() {
    SkPicture recorded;
    RecordPicture(fGM.get(), &recorded);

    SkDynamicMemoryWStream wStream;
    recorded.serialize(&wStream, &trivial_bitmap_encoder);
    SkAutoTUnref<SkStream> rStream(wStream.detachAsStream());
    SkAutoTUnref<SkPicture> reconstructed(SkPicture::CreateFromStream(rStream));

    SkBitmap bitmap;
    SetupBitmap(fReference.config(), fGM.get(), &bitmap);
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
