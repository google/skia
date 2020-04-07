#include "gm/gm.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "tools/Resources.h"

static bool getResourceAsBitmap(const char* image, SkBitmap* dst) {
    auto data = GetResourceAsData(image);
    auto codec = SkAndroidCodec::MakeFromData(std::move(data));
    if (!codec) return false;

    auto info = codec->getInfo();
    info = info.makeColorSpace(codec->computeOutputColorSpace(info.colorType()));
    dst->allocPixels(info);
    return codec->getPixels(info, dst->getPixels(), dst->rowBytes()) == SkCodec::kSuccess;
}

// Create a scaled version mimicking android.graphics.Bitmap#createScaledBitmap
static SkBitmap createScaledBitmap(const SkBitmap& orig) {
    int neww = orig.width() / 4;
    int newh = orig.height() / 4;

    SkMatrix m = SkMatrix::MakeScale((float) neww / orig.width(),
                                     (float) newh / orig.height());

    SkIRect srcR = orig.info().bounds();
    SkRect dstR = SkRect::MakeWH(orig.width(), orig.height());
    SkRect deviceR = m.mapRect(dstR);

    neww = SkScalarRoundToInt(deviceR.width());
    newh = SkScalarRoundToInt(deviceR.height());

    SkBitmap scaled;
    scaled.allocPixels(orig.info().makeWH(neww,newh));
    SkCanvas canvas(scaled);
    canvas.concat(m);
    SkPaint paint;
    paint.setFilterQuality(kLow_SkFilterQuality);
    canvas.drawBitmapRect(orig, srcR, dstR, &paint);
    return scaled;
}

DEF_SIMPLE_GM(b153410582, canvas, 1000, 1000) {
    canvas->scale (.1f, .1f);
    const char* images[] = {
        "images/EPSON  Gray - Gamma 2.2.jpg",
        "images/Generic Gray Gamma 2.2 Profile.jpg",
        "images/Happy-Test-Screen_Generic Gray Gamma 2.2 Profile.jpg",
        "images/Test-Screen_GIMP built-in D65 Grayscale with sRGB TRC.jpg",
    };

    for (const char* image : images) {
        SkBitmap orig;
        if (!getResourceAsBitmap(image, &orig)) {
            SkDebugf("failed to decode %s\n", image);
            continue;
        }

        canvas->drawBitmap(orig, 0, 0);

        SkBitmap scaled = createScaledBitmap(orig);
        canvas->drawBitmap(scaled, orig.width(), 0);
        canvas->translate(0, orig.height());
    }
}
