/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"

static void make_bitmap(SkBitmap* bitmap) {
    bitmap->allocN32Pixels(64, 64);

    SkCanvas canvas(*bitmap);

    canvas.drawColor(SK_ColorRED);
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint pts[] = { { 0, 0 }, { 64, 64 } };
    const SkColor colors[] = { SK_ColorWHITE, SK_ColorBLUE };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, 2,
                                                 SkShader::kClamp_TileMode));
    canvas.drawCircle(32, 32, 32, paint);
}

class DrawBitmapRect2 : public skiagm::GM {
    bool fUseIRect;
public:
    DrawBitmapRect2(bool useIRect) : fUseIRect(useIRect) {
    }

protected:
    SkString onShortName() override {
        SkString str;
        str.printf("bitmaprect_%s", fUseIRect ? "i" : "s");
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->drawColor(sk_tool_utils::color_to_565(0xFFCCCCCC));

        const SkIRect src[] = {
            { 0, 0, 32, 32 },
            { 0, 0, 80, 80 },
            { 32, 32, 96, 96 },
            { -32, -32, 32, 32, }
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);

        SkBitmap bitmap;
        make_bitmap(&bitmap);

        SkRect dstR = { 0, 200, 128, 380 };

        canvas->translate(16, 40);
        for (size_t i = 0; i < SK_ARRAY_COUNT(src); i++) {
            SkRect srcR;
            srcR.set(src[i]);

            canvas->drawBitmap(bitmap, 0, 0, &paint);
            if (!fUseIRect) {
                canvas->drawBitmapRect(bitmap, srcR, dstR, &paint,
                                       SkCanvas::kStrict_SrcRectConstraint);
            } else {
                canvas->drawBitmapRect(bitmap, src[i], dstR, &paint);
            }

            canvas->drawRect(dstR, paint);
            canvas->drawRect(srcR, paint);

            canvas->translate(160, 0);
        }
    }

private:
    typedef skiagm::GM INHERITED;
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
    SkString onShortName() override {
        SkString str;
        str.printf("3x3bitmaprect");
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {

        SkBitmap bitmap;
        make_3x3_bitmap(&bitmap);

        SkRect srcR = { 0.5f, 0.5f, 2.5f, 2.5f };
        SkRect dstR = { 100, 100, 300, 200 };

        canvas->drawBitmapRect(bitmap, srcR, dstR, nullptr, SkCanvas::kStrict_SrcRectConstraint);
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
static void make_big_bitmap(SkBitmap* bitmap) {

    constexpr int gXSize = 4096;
    constexpr int gYSize = 4096;
    constexpr int gBorderWidth = 10;

    bitmap->allocN32Pixels(gXSize, gYSize);
    for (int y = 0; y < gYSize; ++y) {
        for (int x = 0; x < gXSize; ++x) {
            if (x <= gBorderWidth || x >= gXSize-gBorderWidth ||
                y <= gBorderWidth || y >= gYSize-gBorderWidth) {
                *bitmap->getAddr32(x, y) = SkPreMultiplyColor(0x88FFFFFF);
            } else {
                *bitmap->getAddr32(x, y) = SkPreMultiplyColor(0x88FF0000);
            }
        }
    }
}

// This GM attempts to reveal any issues we may have when the GPU has to
// break up a large texture in order to draw it. The XOR transfer mode will
// create stripes in the image if there is imprecision in the destination
// tile placement.
class DrawBitmapRect4 : public skiagm::GM {
    bool fUseIRect;
    SkBitmap fBigBitmap;

public:
    DrawBitmapRect4(bool useIRect) : fUseIRect(useIRect) {
        this->setBGColor(0x88444444);
    }

protected:
    SkString onShortName() override {
        SkString str;
        str.printf("bigbitmaprect_%s", fUseIRect ? "i" : "s");
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onOnceBeforeDraw() override {
        make_big_bitmap(&fBigBitmap);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAlpha(128);
        paint.setBlendMode(SkBlendMode::kXor);

        SkRect srcR1 = { 0.0f, 0.0f, 4096.0f, 2040.0f };
        SkRect dstR1 = { 10.1f, 10.1f, 629.9f, 400.9f };

        SkRect srcR2 = { 4085.0f, 10.0f, 4087.0f, 12.0f };
        SkRect dstR2 = { 10, 410, 30, 430 };

        if (!fUseIRect) {
            canvas->drawBitmapRect(fBigBitmap, srcR1, dstR1, &paint,
                                   SkCanvas::kStrict_SrcRectConstraint);
            canvas->drawBitmapRect(fBigBitmap, srcR2, dstR2, &paint,
                                   SkCanvas::kStrict_SrcRectConstraint);
        } else {
            canvas->drawBitmapRect(fBigBitmap, srcR1.roundOut(), dstR1, &paint);
            canvas->drawBitmapRect(fBigBitmap, srcR2.roundOut(), dstR2, &paint);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

class BitmapRectRounding : public skiagm::GM {
    SkBitmap fBM;

public:
    BitmapRectRounding() {}

protected:
    SkString onShortName() override {
        SkString str;
        str.printf("bitmaprect_rounding");
        return str;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

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
        canvas->drawBitmapRect(fBM, r, nullptr);
    }

private:
    typedef skiagm::GM INHERITED;
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
