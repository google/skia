/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkShader.h"

namespace skiagm {

// This GM draws a 3x3 grid (with the center element excluded) of rectangles
// filled with a bitmap shader. The bitmap shader is transformed so that the
// pattern cell is at the center (excluded) region.
//
// In Repeat and Mirror mode, this tests that the bitmap shader still draws
// even though the pattern cell is outside the clip.
//
// In Clamp mode, this tests that the clamp is handled properly. For PDF,
// (and possibly other exported formats) this also "tests" that the image itself
// is not stored (well, you'll need to open it up with an external tool to
// verify that).

static SkBitmap create_bitmap() {
    SkBitmap bmp;
    bmp.allocN32Pixels(2, 2);
    uint32_t* pixels = reinterpret_cast<uint32_t*>(bmp.getPixels());
    pixels[0] = SkPreMultiplyColor(SK_ColorRED);
    pixels[1] = SkPreMultiplyColor(SK_ColorGREEN);
    pixels[2] = SkPreMultiplyColor(SK_ColorBLACK);
    pixels[3] = SkPreMultiplyColor(SK_ColorBLUE);

    return bmp;
}

static const SkScalar RECT_SIZE = 64;
static const SkScalar SLIDE_SIZE = 300;

class ClippedBitmapShadersGM : public GM {
public:
    ClippedBitmapShadersGM(SkShader::TileMode mode, bool hq=false)
    : fMode(mode), fHQ(hq) {
    }

protected:
    SkShader::TileMode fMode;
    bool fHQ;

    virtual SkString onShortName() {
        SkString descriptor;
        switch (fMode) {
            case SkShader::kRepeat_TileMode:
                descriptor = "tile";
            break;
            case SkShader::kMirror_TileMode:
                descriptor = "mirror";
            break;
            case SkShader::kClamp_TileMode:
                descriptor = "clamp";
            break;
            default:
                SkASSERT(false);
        }
        descriptor.prepend("clipped-bitmap-shaders-");
        if (fHQ) {
            descriptor.append("-hq");
        }
        return descriptor;
    }

    virtual SkISize onISize() {
        return SkISize::Make(300, 300);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkBitmap bmp = create_bitmap();
        SkMatrix s;
        s.reset();
        s.setScale(8, 8);
        s.postTranslate(SLIDE_SIZE / 2, SLIDE_SIZE / 2);
        SkPaint paint;
        paint.setShader(SkShader::MakeBitmapShader(bmp, fMode, fMode, &s));

        if (fHQ) {
            paint.setFilterQuality(kHigh_SkFilterQuality);
        }

        SkScalar margin = (SLIDE_SIZE / 3 - RECT_SIZE) / 2;
        for (int i = 0; i < 3; i++) {
            SkScalar yOrigin = SLIDE_SIZE / 3 * i + margin;
            for (int j = 0; j < 3; j++) {
                SkScalar xOrigin = SLIDE_SIZE / 3 * j + margin;
                if (i == 1 && j == 1) {
                    continue;   // skip center element
                }
                SkRect rect = SkRect::MakeXYWH(xOrigin, yOrigin,
                                               RECT_SIZE, RECT_SIZE);
                canvas->save();
                canvas->clipRect(rect);
                canvas->drawRect(rect, paint);
                canvas->restore();
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ClippedBitmapShadersGM(SkShader::kRepeat_TileMode); )
DEF_GM( return new ClippedBitmapShadersGM(SkShader::kMirror_TileMode); )
DEF_GM( return new ClippedBitmapShadersGM(SkShader::kClamp_TileMode); )

DEF_GM( return new ClippedBitmapShadersGM(SkShader::kRepeat_TileMode, true); )
DEF_GM( return new ClippedBitmapShadersGM(SkShader::kMirror_TileMode, true); )
DEF_GM( return new ClippedBitmapShadersGM(SkShader::kClamp_TileMode, true); )


}
