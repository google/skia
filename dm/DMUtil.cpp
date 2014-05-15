#include "DMUtil.h"

#include "SkPicture.h"
#include "SkPictureRecorder.h"

namespace DM {

SkString UnderJoin(const char* a, const char* b) {
    SkString s;
    s.appendf("%s_%s", a, b);
    return s;
}

SkPicture* RecordPicture(skiagm::GM* gm, uint32_t recordFlags, SkBBHFactory* factory) {
    const SkISize size = gm->getISize();
    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(size.width(), size.height(), factory, recordFlags);
    canvas->concat(gm->getInitialTransform());
    gm->draw(canvas);
    canvas->flush();
    return recorder.endRecording();
}

void AllocatePixels(SkColorType ct, int width, int height, SkBitmap* bitmap) {
    bitmap->allocPixels(SkImageInfo::Make(width, height, ct, kPremul_SkAlphaType));
    bitmap->eraseColor(0x00000000);
}

void AllocatePixels(const SkBitmap& reference, SkBitmap* bitmap) {
    AllocatePixels(reference.colorType(), reference.width(), reference.height(), bitmap);
}

void DrawPicture(SkPicture* picture, SkBitmap* bitmap) {
    SkASSERT(picture != NULL);
    SkASSERT(bitmap != NULL);
    SkCanvas canvas(*bitmap);
    canvas.drawPicture(*picture);
    canvas.flush();
}

bool BitmapsEqual(const SkBitmap& a, const SkBitmap& b) {
    const SkAutoLockPixels lockA(a), lockB(b);
    return a.getSize() == b.getSize() && 0 == memcmp(a.getPixels(), b.getPixels(), b.getSize());
}

}  // namespace DM
