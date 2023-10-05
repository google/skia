/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/GpuToolUtils.h"
#include "tools/ToolUtils.h"

#include <cstddef>
#include <iterator>

static sk_sp<SkImage> make_image(SkCanvas* destCanvas) {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(64, 64));
    auto tmpCanvas = surf->getCanvas();

    tmpCanvas->drawColor(SK_ColorRED);
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint pts[] = { { 0, 0 }, { 64, 64 } };
    const SkColor colors[] = { SK_ColorWHITE, SK_ColorBLUE };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp));
    tmpCanvas->drawCircle(32, 32, 32, paint);

    return ToolUtils::MakeTextureImage(destCanvas, surf->makeImageSnapshot());
}

class DrawBitmapRect2 : public skiagm::GM {
    bool fUseIRect;
public:
    DrawBitmapRect2(bool useIRect) : fUseIRect(useIRect) {
    }

protected:
    SkString getName() const override {
        SkString str;
        str.printf("bitmaprect_%s", fUseIRect ? "i" : "s");
        return str;
    }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(0xFFCCCCCC);

        const SkIRect src[] = {
            { 0, 0, 32, 32 },
            { 0, 0, 80, 80 },
            { 32, 32, 96, 96 },
            { -32, -32, 32, 32, }
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        auto sampling = SkSamplingOptions();

        auto image = make_image(canvas);

        SkRect dstR = { 0, 200, 128, 380 };

        canvas->translate(16, 40);
        for (size_t i = 0; i < std::size(src); i++) {
            SkRect srcR;
            srcR.set(src[i]);

            canvas->drawImage(image, 0, 0, sampling, &paint);
            if (!fUseIRect) {
                canvas->drawImageRect(image.get(), srcR, dstR, sampling, &paint,
                                      SkCanvas::kStrict_SrcRectConstraint);
            } else {
                canvas->drawImageRect(image.get(), SkRect::Make(src[i]), dstR, sampling, &paint,
                                      SkCanvas::kStrict_SrcRectConstraint);
            }

            canvas->drawRect(dstR, paint);
            canvas->drawRect(srcR, paint);

            canvas->translate(160, 0);
        }
    }

private:
    using INHERITED = skiagm::GM;
};

//////////////////////////////////////////////////////////////////////////////

static void make_3x3_bitmap(SkBitmap* bitmap) {
    const int xSize = 3;
    const int ySize = 3;

    const SkColor textureData[xSize][ySize] = {
        { SK_ColorRED,    SK_ColorWHITE, SK_ColorBLUE },
        { SK_ColorGREEN,  SK_ColorBLACK, SK_ColorCYAN },
        { SK_ColorYELLOW, SK_ColorGRAY,  SK_ColorMAGENTA }
    };

    bitmap->allocN32Pixels(xSize, ySize, true);
    SkCanvas canvas(*bitmap);
    SkPaint paint;

    for (int y = 0; y < ySize; y++) {
        for (int x = 0; x < xSize; x++) {
            paint.setColor(textureData[x][y]);
            canvas.drawIRect(SkIRect::MakeXYWH(x, y, 1, 1), paint);
        }
    }
}

// This GM attempts to make visible any issues drawBitmapRect may have
// with partial source rects. In this case the eight pixels on the border
// should be half the width/height of the central pixel, i.e.:
//                         __|____|__
//                           |    |
//                         __|____|__
//                           |    |
class DrawBitmapRect3 : public skiagm::GM {
public:
    DrawBitmapRect3() {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    SkString getName() const override {
        SkString str;
        str.printf("3x3bitmaprect");
        return str;
    }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {

        SkBitmap bitmap;
        make_3x3_bitmap(&bitmap);

        SkRect srcR = { 0.5f, 0.5f, 2.5f, 2.5f };
        SkRect dstR = { 100, 100, 300, 200 };

        canvas->drawImageRect(ToolUtils::MakeTextureImage(canvas, bitmap.asImage()),
                              srcR, dstR, SkSamplingOptions(),
                              nullptr, SkCanvas::kStrict_SrcRectConstraint);
    }

private:
    using INHERITED = skiagm::GM;
};

//////////////////////////////////////////////////////////////////////////////
static sk_sp<SkImage> make_big_bitmap(SkCanvas* canvas) {

    constexpr int gXSize = 4096;
    constexpr int gYSize = 4096;
    constexpr int gBorderWidth = 10;

    SkBitmap bitmap;
    bitmap.allocN32Pixels(gXSize, gYSize);
    for (int y = 0; y < gYSize; ++y) {
        for (int x = 0; x < gXSize; ++x) {
            if (x <= gBorderWidth || x >= gXSize-gBorderWidth ||
                y <= gBorderWidth || y >= gYSize-gBorderWidth) {
                *bitmap.getAddr32(x, y) = SkPreMultiplyColor(0x88FFFFFF);
            } else {
                *bitmap.getAddr32(x, y) = SkPreMultiplyColor(0x88FF0000);
            }
        }
    }
    bitmap.setImmutable();
    return ToolUtils::MakeTextureImage(canvas, bitmap.asImage());
}

// This GM attempts to reveal any issues we may have when the GPU has to
// break up a large texture in order to draw it. The XOR transfer mode will
// create stripes in the image if there is imprecision in the destination
// tile placement.
class DrawBitmapRect4 : public skiagm::GM {
    bool fUseIRect;
    sk_sp<SkImage> fBigImage;

public:
    DrawBitmapRect4(bool useIRect) : fUseIRect(useIRect) {
        this->setBGColor(0x88444444);
    }

protected:
    SkString getName() const override {
        SkString str;
        str.printf("bigbitmaprect_%s", fUseIRect ? "i" : "s");
        return str;
    }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        if (!fBigImage) {
            fBigImage = make_big_bitmap(canvas);
        }

        SkPaint paint;
        paint.setAlpha(128);
        paint.setBlendMode(SkBlendMode::kXor);
        SkSamplingOptions sampling;

        SkRect srcR1 = { 0.0f, 0.0f, 4096.0f, 2040.0f };
        SkRect dstR1 = { 10.1f, 10.1f, 629.9f, 400.9f };

        SkRect srcR2 = { 4085.0f, 10.0f, 4087.0f, 12.0f };
        SkRect dstR2 = { 10, 410, 30, 430 };

        if (!fUseIRect) {
            canvas->drawImageRect(fBigImage, srcR1, dstR1, sampling, &paint,
                                   SkCanvas::kStrict_SrcRectConstraint);
            canvas->drawImageRect(fBigImage, srcR2, dstR2, sampling, &paint,
                                   SkCanvas::kStrict_SrcRectConstraint);
        } else {
            canvas->drawImageRect(fBigImage, SkRect::Make(srcR1.roundOut()), dstR1, sampling,
                                  &paint, SkCanvas::kStrict_SrcRectConstraint);
            canvas->drawImageRect(fBigImage, SkRect::Make(srcR2.roundOut()), dstR2, sampling,
                                  &paint, SkCanvas::kStrict_SrcRectConstraint);
        }
    }

private:
    using INHERITED = skiagm::GM;
};

class BitmapRectRounding : public skiagm::GM {
    SkBitmap fBM;

public:
    BitmapRectRounding() {}

protected:
    SkString getName() const override {
        SkString str;
        str.printf("bitmaprect_rounding");
        return str;
    }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onOnceBeforeDraw() override {
        fBM.allocN32Pixels(10, 10);
        fBM.eraseColor(SK_ColorBLUE);
    }

    // This choice of coordinates and matrix land the bottom edge of the clip (and bitmap dst)
    // at exactly 1/2 pixel boundary. However, drawBitmapRect may lose precision along the way.
    // If it does, we may see a red-line at the bottom, instead of the bitmap exactly matching
    // the clip (in which case we should see all blue).
    // The correct image should be all blue.
    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorRED);

        const SkRect r = SkRect::MakeXYWH(1, 1, 110, 114);
        canvas->scale(0.9f, 0.9f);

        // the drawRect shows the same problem as clipRect(r) followed by drawcolor(red)
        canvas->drawRect(r, paint);
        canvas->drawImageRect(fBM.asImage(), r, SkSamplingOptions());
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new BitmapRectRounding; )

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new DrawBitmapRect2(false); )
DEF_GM( return new DrawBitmapRect2(true); )
DEF_GM( return new DrawBitmapRect3(); )

#ifndef SK_BUILD_FOR_ANDROID
DEF_GM( return new DrawBitmapRect4(false); )
DEF_GM( return new DrawBitmapRect4(true); )
#endif
