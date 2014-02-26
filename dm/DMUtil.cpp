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

static void setup_bitmap(SkColorType ct, int width, int height, SkBitmap* bitmap) {
    bitmap->allocPixels(SkImageInfo::Make(width, height, ct, kPremul_SkAlphaType));
    bitmap->eraseColor(0x00000000);
}

void SetupBitmap(const SkColorType ct, skiagm::GM* gm, SkBitmap* bitmap) {
    setup_bitmap(ct, gm->getISize().width(), gm->getISize().height(), bitmap);
}

void SetupBitmap(const SkColorType ct, SkBenchmark* bench, SkBitmap* bitmap) {
    setup_bitmap(ct, bench->getSize().x(), bench->getSize().y(), bitmap);
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
