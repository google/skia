/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkMakeUnique.h"

static void do_draw(SkCanvas* canvas, const SkRect& r) {
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setColor(0x800000FF);
    canvas->drawRect(r, paint);
}

/**
 *  Exercise kDontClipToLayer_Legacy_SaveLayerFlag flag, which does not limit the clip to the
 *  layer's bounds. Thus when a draw occurs, it can (depending on "where" it is) draw into the layer
 *  and/or draw onto the surrounding portions of the canvas, or both.
 *
 *  This GM has a 100x100 rectangle (r), which its going to draw. However first it creates a layer
 *  with this flag covering 1/2 of the rectangle (upper half). Then it draws the rect in SRC mode.
 *
 *  The portion of the draw that intersects the layer should see the SRC draw, apply it to the layer
 *  and then during restore, it will SRC_OVER that layer onto the canvas (SRC_OVER since the layer
 *  has no paint, so it gets the default xfermode during restore).
 *
 *  Note: when we talk about drawing directly into the "canvas", in fact we are drawing into an
 *        "outer" layer we created (filled with red). This is a testing detail, so that our final
 *        output image is itself opaque, otherwise we make it harder to view the GM as a PNG.
 *
 *  The portion of the draw below the layer draws directly into the canvas. Since it is in SRC mode,
 *  it will write 0x80 to the canvas' alpha, overwriting the "red", which then gets blended with
 *  the GM's white background.
 *
 *  The portion in the layer, will end up SRC_OVERing the 0x80 layer pixels onto the canvas' red
 *  pixels, making magenta.
 *
 *  Thus the expected result is the upper half to be magenta 0xFF7F0080, and the lower half to be
 *  light blue 0xFF7F7FFF.
 */
DEF_SIMPLE_GM(dont_clip_to_layer, canvas, 120, 120) {
    const SkRect r { 10, 10, 110, 110 };

    // Wrap the entire test inside a red layer, so we don't punch the actual gm's alpha with
    // kSrc_Mode, which makes it hard to view (we like our GMs to have opaque pixels).
    canvas->saveLayer(&r, nullptr);
    canvas->drawColor(SK_ColorRED);

    SkRect r0 = { 20, 20, 100, 55 };
    SkRect r1 = { 20, 65, 100, 100 };

    SkCanvas::SaveLayerRec rec;
    rec.fPaint = nullptr;
    rec.fBounds = &r0;
    rec.fBackdrop = nullptr;
    rec.fSaveLayerFlags = 1 << 31;//SkCanvas::kDontClipToLayer_Legacy_SaveLayerFlag;
    canvas->saveLayer(rec);
    rec.fBounds = &r1;
    canvas->saveLayer(rec);
    do_draw(canvas, r);
    canvas->restore();
    canvas->restore();

    canvas->restore();  // red-layer
}

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

class AAClipGM : public skiagm::GM {
public:
    AAClipGM() {

    }

protected:
    SkString onShortName() override {
        return SkString("aaclip");
    }

    SkISize onISize() override {
        return SkISize::Make(240, 120);
    }

    void onDraw(SkCanvas* canvas) override {
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

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new AAClipGM;)

/////////////////////////////////////////////////////////////////////////

#ifdef SK_BUILD_FOR_MAC

static std::unique_ptr<SkCanvas> make_canvas(const SkBitmap& bm) {
    const SkImageInfo& info = bm.info();
    if (info.bytesPerPixel() == 4) {
        return SkCanvas::MakeRasterDirectN32(info.width(), info.height(),
                                             (SkPMColor*)bm.getPixels(),
                                             bm.rowBytes());
    } else {
        return skstd::make_unique<SkCanvas>(bm);
    }
}

#include "SkCGUtils.h"
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
    canvas->drawBitmap(bm, 10, 10);

    CGImageRef image = SkCreateCGImageRefWithColorspace(bm, nullptr);

    SkBitmap bm2;
    SkCreateBitmapFromCGImage(&bm2, image);
    canvas->drawBitmap(bm2, 10, 120);
    canvas->drawImage(SkMakeImageFromCGImage(image), 10, 120 + bm2.height() + 10);

    CGImageRelease(image);
}

class CGImageGM : public skiagm::GM {
public:
    CGImageGM() {}

protected:
    SkString onShortName() override {
        return SkString("cgimage");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 250);
    }

    void onDraw(SkCanvas* canvas) override {
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

        for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
            SkImageInfo info = SkImageInfo::Make(100, 100, rec[i].fCT, rec[i].fAT);
            test_image(canvas, info);
            canvas->translate(info.width() + 10, 0);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
//DEF_GM( return new CGImageGM; )
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

// https://bug.skia.org/3716
class ClipCubicGM : public skiagm::GM {
    const SkScalar W = 100;
    const SkScalar H = 240;

    SkPath fVPath, fHPath;
public:
    ClipCubicGM() {
        fVPath.moveTo(W, 0);
        fVPath.cubicTo(W, H-10, 0, 10, 0, H);

        SkMatrix pivot;
        pivot.setRotate(90, W/2, H/2);
        fVPath.transform(pivot, &fHPath);
    }

protected:
    SkString onShortName() override {
        return SkString("clipcubic");
    }

    SkISize onISize() override {
        return SkISize::Make(400, 410);
    }

    void doDraw(SkCanvas* canvas, const SkPath& path) {
        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
        canvas->drawPath(path, paint);

        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, paint);
    }

    void drawAndClip(SkCanvas* canvas, const SkPath& path, SkScalar dx, SkScalar dy) {
        SkAutoCanvasRestore acr(canvas, true);

        SkRect r = SkRect::MakeXYWH(0, H/4, W, H/2);
        SkPaint paint;
        paint.setColor(sk_tool_utils::color_to_565(0xFF8888FF));

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
    typedef skiagm::GM INHERITED;
};
DEF_GM(return new ClipCubicGM;)
