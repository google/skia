#include "DMUtil.h"

#include "SkPicture.h"

namespace DM {

SkString UnderJoin(const char* a, const char* b) {
    SkString s;
    s.appendf("%s_%s", a, b);
    return s;
}

void RecordPicture(skiagm::GM* gm, SkPicture* picture, uint32_t recordFlags) {
    const SkISize size = gm->getISize();
    SkCanvas* canvas = picture->beginRecording(size.width(), size.height(), recordFlags);
    canvas->concat(gm->getInitialTransform());
    gm->draw(canvas);
    canvas->flush();
    picture->endRecording();
}

void SetupBitmap(const SkBitmap::Config config, skiagm::GM* gm, SkBitmap* bitmap) {
    const SkISize size = gm->getISize();
    bitmap->setConfig(config, size.width(), size.height());
    bitmap->allocPixels();
    bitmap->eraseColor(0x00000000);
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
