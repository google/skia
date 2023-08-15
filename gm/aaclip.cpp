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
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "src/core/SkCanvasPriv.h"
#include "tools/ToolUtils.h"

/** Draw a 2px border around the target, then red behind the target;
    set the clip to match the target, then draw >> the target in blue.
*/

static void draw(SkCanvas* canvas, SkRect& target, int x, int y) {
    SkPaint borderPaint;
    borderPaint.setColor(SkColorSetRGB(0x0, 0xDD, 0x0));
    borderPaint.setAntiAlias(true);
    SkPaint backgroundPaint;
    backgroundPaint.setColor(SkColorSetRGB(0xDD, 0x0, 0x0));
    backgroundPaint.setAntiAlias(true);
    SkPaint foregroundPaint;
    foregroundPaint.setColor(SkColorSetRGB(0x0, 0x0, 0xDD));
    foregroundPaint.setAntiAlias(true);

    canvas->save();
    canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
    target.inset(SkIntToScalar(-2), SkIntToScalar(-2));
    canvas->drawRect(target, borderPaint);
    target.inset(SkIntToScalar(2), SkIntToScalar(2));
    canvas->drawRect(target, backgroundPaint);
    canvas->clipRect(target, true);
    target.inset(SkIntToScalar(-4), SkIntToScalar(-4));
    canvas->drawRect(target, foregroundPaint);
    canvas->restore();
}

static void draw_square(SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_column(SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(1 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_bar(SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 1 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_rect_tests(SkCanvas* canvas) {
    draw_square(canvas, 10, 10);
    draw_column(canvas, 30, 10);
    draw_bar(canvas, 10, 30);
}

/**
   Test a set of clipping problems discovered while writing blitAntiRect,
   and test all the code paths through the clipping blitters.
   Each region should show as a blue center surrounded by a 2px green
   border, with no red.
*/
DEF_SIMPLE_GM(aaclip, canvas, 240, 120) {
        // Initial pixel-boundary-aligned draw
        draw_rect_tests(canvas);

        // Repeat 4x with .2, .4, .6, .8 px offsets
        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);
}

/////////////////////////////////////////////////////////////////////////

#ifdef SK_BUILD_FOR_MAC

#include "include/utils/mac/SkCGUtils.h"

static std::unique_ptr<SkCanvas> make_canvas(const SkBitmap& bm) {
    return SkCanvas::MakeRasterDirect(bm.info(), bm.getPixels(), bm.rowBytes());
}

static void test_image(SkCanvas* canvas, const SkImageInfo& info) {
    SkBitmap bm;
    bm.allocPixels(info);

    if (info.isOpaque()) {
        bm.eraseColor(SK_ColorGREEN);
    } else {
        bm.eraseColor(0);
    }

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);
    make_canvas(bm)->drawCircle(50, 50, 49, paint);
    canvas->drawImage(bm.asImage(), 10, 10);

    CGImageRef image = SkCreateCGImageRefWithColorspace(bm, nullptr);

    SkBitmap bm2;
    SkCreateBitmapFromCGImage(&bm2, image);
    canvas->drawImage(bm2.asImage(), 10, 120);
    canvas->drawImage(SkMakeImageFromCGImage(image), 10, 120 + bm2.height() + 10);

    CGImageRelease(image);
}

DEF_SIMPLE_GM(cgimage, canvas, 800, 250) {
        const struct {
            SkColorType fCT;
            SkAlphaType fAT;
        } rec[] = {
            { kRGB_565_SkColorType, kOpaque_SkAlphaType },

            { kRGBA_8888_SkColorType, kPremul_SkAlphaType },
            { kRGBA_8888_SkColorType, kUnpremul_SkAlphaType },
            { kRGBA_8888_SkColorType, kOpaque_SkAlphaType },

            { kBGRA_8888_SkColorType, kPremul_SkAlphaType },
            { kBGRA_8888_SkColorType, kUnpremul_SkAlphaType },
            { kBGRA_8888_SkColorType, kOpaque_SkAlphaType },
        };

        for (size_t i = 0; i < std::size(rec); ++i) {
            SkImageInfo info = SkImageInfo::Make(100, 100, rec[i].fCT, rec[i].fAT);
            test_image(canvas, info);
            canvas->translate(info.width() + 10, 0);
        }
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

// https://bug.skia.org/3716
class ClipCubicGM : public skiagm::GM {
    const SkScalar W = 100;
    const SkScalar H = 240;

    SkPath fVPath, fHPath;
public:
    ClipCubicGM() {
        fVPath = SkPathBuilder().moveTo(W, 0)
                                .cubicTo(W, H-10, 0, 10, 0, H)
                                .detach();

        SkMatrix pivot;
        pivot.setRotate(90, W/2, H/2);
        fHPath = fVPath.makeTransform(pivot);
    }

protected:
    SkString getName() const override { return SkString("clipcubic"); }

    SkISize getISize() override { return SkISize::Make(400, 410); }

    void doDraw(SkCanvas* canvas, const SkPath& path) {
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setColor(0xFFCCCCCC);
        canvas->drawPath(path, paint);

        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, paint);
    }

    void drawAndClip(SkCanvas* canvas, const SkPath& path, SkScalar dx, SkScalar dy) {
        SkAutoCanvasRestore acr(canvas, true);

        SkRect r = SkRect::MakeXYWH(0, H/4, W, H/2);
        SkPaint paint;
        paint.setColor(ToolUtils::color_to_565(0xFF8888FF));

        canvas->drawRect(r, paint);
        this->doDraw(canvas, path);

        canvas->translate(dx, dy);

        canvas->drawRect(r, paint);
        canvas->clipRect(r);
        this->doDraw(canvas, path);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(80, 10);
        this->drawAndClip(canvas, fVPath, 200, 0);
        canvas->translate(0, 200);
        this->drawAndClip(canvas, fHPath, 200, 0);
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM(return new ClipCubicGM;)
