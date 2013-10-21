#include "DMUtil.h"

#include "SkPicture.h"

namespace DM {

SkString UnderJoin(const char* a, const char* b) {
    SkString s;
    s.appendf("%s_%s", a, b);
    return s;
}

SkString Png(SkString s) {
    s.appendf(".png");
    return s;
}

bool MeetsExpectations(const skiagm::Expectations& expectations, const SkBitmap bitmap) {
    if (expectations.ignoreFailure() || expectations.empty()) {
        return true;
    }
    const skiagm::GmResultDigest digest(bitmap);
    return expectations.match(digest);
}

void RecordPicture(skiagm::GM* gm, SkPicture* picture) {
    SkCanvas* canvas = picture->beginRecording(SkScalarCeilToInt(gm->width()),
                                               SkScalarCeilToInt(gm->height()),
                                               0 /*flags*/);
    canvas->concat(gm->getInitialTransform());
    gm->draw(canvas);
    canvas->flush();
    picture->endRecording();
}

void SetupBitmap(const SkBitmap::Config config, skiagm::GM* gm, SkBitmap* bitmap) {
    bitmap->setConfig(config, SkScalarCeilToInt(gm->width()), SkScalarCeilToInt(gm->height()));
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
