#include "DMUtil.h"

#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"

DEFINE_string(matrix, "1 0 0 0 1 0 0 0 1",
              "Matrix to apply to the canvas before drawing.");

namespace DM {

void CanvasPreflight(SkCanvas* canvas) {
    if (FLAGS_matrix.count() == 9) {
        SkMatrix m;
        for (int i = 0; i < 9; i++) {
            m[i] = (SkScalar)atof(FLAGS_matrix[i]);
        }
        canvas->concat(m);
    }
}

SkString UnderJoin(const char* a, const char* b) {
    SkString s;
    s.appendf("%s_%s", a, b);
    return s;
}

SkString FileToTaskName(SkString filename) {
    for (size_t i = 0; i < filename.size(); i++) {
        if ('_' == filename[i]) { filename[i] = '-'; }
        if ('.' == filename[i]) { filename[i] = '_'; }
    }
    return filename;
}

SkPicture* RecordPicture(skiagm::GM* gm, SkBBHFactory* factory) {
    const SkScalar w = SkIntToScalar(gm->getISize().width()),
                   h = SkIntToScalar(gm->getISize().height());
    SkPictureRecorder recorder;

    SkCanvas* canvas = recorder.beginRecording(w, h, factory);
    CanvasPreflight(canvas);
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

void DrawPicture(const SkPicture& picture, SkBitmap* bitmap) {
    SkASSERT(bitmap != NULL);
    SkCanvas canvas(*bitmap);
    canvas.drawPicture(&picture);
    canvas.flush();
}

static void unpack_565(uint16_t pixel, unsigned* r, unsigned* g, unsigned* b) {
    *r = SkGetPackedR16(pixel);
    *g = SkGetPackedG16(pixel);
    *b = SkGetPackedB16(pixel);
}

// Returns |a-b|.
static unsigned abs_diff(unsigned a, unsigned b) {
    return a > b ? a - b : b - a;
}

unsigned MaxComponentDifference(const SkBitmap& a, const SkBitmap& b) {
    if (a.info() != b.info()) {
        SkFAIL("Can't compare bitmaps of different shapes.");
    }

    unsigned max = 0;

    const SkAutoLockPixels lockA(a), lockB(b);
    if (a.info().colorType() == kRGB_565_SkColorType) {
        // 565 is special/annoying because its 3 components straddle 2 bytes.
        const uint16_t* aPixels = (const uint16_t*)a.getPixels();
        const uint16_t* bPixels = (const uint16_t*)b.getPixels();
        for (size_t i = 0; i < a.getSize() / 2; i++) {
            unsigned ar, ag, ab,
                     br, bg, bb;
            unpack_565(aPixels[i], &ar, &ag, &ab);
            unpack_565(bPixels[i], &br, &bg, &bb);
            max = SkTMax(max, abs_diff(ar, br));
            max = SkTMax(max, abs_diff(ag, bg));
            max = SkTMax(max, abs_diff(ab, bb));
        }
    } else {
        // Everything else we produce is byte aligned, so max component diff == max byte diff.
        const uint8_t* aBytes = (const uint8_t*)a.getPixels();
        const uint8_t* bBytes = (const uint8_t*)b.getPixels();
        for (size_t i = 0; i < a.getSize(); i++) {
            max = SkTMax(max, abs_diff(aBytes[i], bBytes[i]));
        }
    }

    return max;
}

bool BitmapsEqual(const SkBitmap& a, const SkBitmap& b) {
    if (a.info() != b.info()) {
        return false;
    }
    const SkAutoLockPixels lockA(a), lockB(b);
    return 0 == memcmp(a.getPixels(), b.getPixels(), a.getSize());
}

}  // namespace DM
