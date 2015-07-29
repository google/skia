/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"
#include "SkShader.h"

#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkGradientShader.h"
#include "SkMorphologyImageFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkOffsetImageFilter.h"

///////////////////////////////////////////////////////////////////////////////

class ImageFiltersCropExpandGM : public skiagm::GM {
public:
    ImageFiltersCropExpandGM () {}

protected:

    virtual SkString onShortName() {
        return SkString("imagefilterscropexpand");
    }

    virtual SkISize onISize() { return SkISize::Make(570, 650); }

    void make_checkerboard(SkBitmap* bitmap) {
        bitmap->allocN32Pixels(64, 64);
        SkCanvas canvas(*bitmap);
        canvas.clear(0xFFFF0000);
        SkPaint darkPaint;
        darkPaint.setColor(sk_tool_utils::color_to_565(0xFF404040));
        SkPaint lightPaint;
        lightPaint.setColor(sk_tool_utils::color_to_565(0xFFA0A0A0));
        for (int y = 8; y < 48; y += 16) {
            for (int x = 8; x < 48; x += 16) {
                canvas.save();
                canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
                canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
                canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
                canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
                canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
                canvas.restore();
            }
        }
    }

    void make_gradient_circle(int width, int height, SkBitmap* bitmap) {
        SkScalar x = SkIntToScalar(width / 2);
        SkScalar y = SkIntToScalar(height / 2);
        SkScalar radius = SkMinScalar(x, y) * 0.8f;
        bitmap->allocN32Pixels(width, height);
        SkCanvas canvas(*bitmap);
        canvas.clear(0x00000000);
        SkColor colors[2];
        colors[0] = SK_ColorWHITE;
        colors[1] = SK_ColorBLACK;
        SkAutoTUnref<SkShader> shader(
            SkGradientShader::CreateRadial(SkPoint::Make(x, y), radius, colors, NULL, 2,
                                           SkShader::kClamp_TileMode)
        );
        SkPaint paint;
        paint.setShader(shader);
        canvas.drawCircle(x, y, radius, paint);
    }

    static void draw(SkCanvas* canvas, const SkBitmap& bitmap, const SkRect& rect, SkImageFilter* filter) {
        SkPaint paint;
        paint.setImageFilter(filter)->unref();
        canvas->saveLayer(&rect, &paint);
        canvas->drawBitmap(bitmap, 0, 0);
        canvas->restore();

        SkPaint strokePaint;
        strokePaint.setColor(0xFFFF0000);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(rect, strokePaint);

        canvas->translate(SkIntToScalar(80), 0);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkAutoTUnref<SkColorFilter> cf(
            SkColorFilter::CreateModeFilter(SK_ColorBLUE, SkXfermode::kSrcIn_Mode));
        SkImageFilter::CropRect crop_rect(
            SkRect::Make(SkIRect::MakeXYWH(10, 10, 44, 44)),
            SkImageFilter::CropRect::kHasAll_CropEdge);

        SkBitmap gradient_circle, checkerboard;
        make_gradient_circle(64, 64, &gradient_circle);
        make_checkerboard(&checkerboard);

        SkAutoTUnref<SkImageFilter> gradient_circle_source(
            SkBitmapSource::Create(gradient_circle));
        SkAutoTUnref<SkImageFilter> noop_cropped(
            SkOffsetImageFilter::Create(0, 0, NULL, &crop_rect));
        SkScalar sk255 = SkIntToScalar(255);
        SkScalar matrix[20] = { 1, 0, 0, 0, 0,
                                0, 1, 0, 0, sk255,
                                0, 0, 1, 0, 0,
                                0, 0, 0, 0, sk255 };
        SkAutoTUnref<SkColorFilter> cf_alpha_trans(SkColorMatrixFilter::Create(matrix));

        SkRect r = SkRect::MakeWH(SkIntToScalar(64), SkIntToScalar(64));
        SkScalar MARGIN = SkIntToScalar(12);

        SkIRect bounds;
        r.roundOut(&bounds);

        SkPaint paint;
        canvas->translate(MARGIN, MARGIN);
        for (int outset = -15; outset <= 20; outset += 5) {
            canvas->save();
            SkRect rect = crop_rect.rect();
            rect.outset(SkIntToScalar(outset),
                        SkIntToScalar(outset));
            SkImageFilter::CropRect big_rect(rect, SkImageFilter::CropRect::kHasAll_CropEdge);

            draw(canvas, checkerboard, rect, SkColorFilterImageFilter::Create(
                cf_alpha_trans, noop_cropped.get(), &big_rect));

            draw(canvas, checkerboard, rect, SkBlurImageFilter::Create(
                8.0f, 8.0f, noop_cropped.get(), &big_rect));

            draw(canvas, checkerboard, rect, SkDilateImageFilter::Create(
                2, 2, noop_cropped.get(), &big_rect));

            draw(canvas, checkerboard, rect, SkErodeImageFilter::Create(
                2, 2, noop_cropped.get(), &big_rect));

            draw(canvas, checkerboard, rect, SkDropShadowImageFilter::Create(
                SkIntToScalar(10), SkIntToScalar(10), SkIntToScalar(3), SkIntToScalar(3),
                SK_ColorBLUE, SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
                noop_cropped.get(), &big_rect));

            draw(canvas, checkerboard, rect, SkDisplacementMapEffect::Create(
                SkDisplacementMapEffect::kR_ChannelSelectorType,
                SkDisplacementMapEffect::kR_ChannelSelectorType,
                SkIntToScalar(12),
                gradient_circle_source.get(),
                noop_cropped.get(),
                &big_rect));

            draw(canvas, checkerboard, rect, SkOffsetImageFilter::Create(
                SkIntToScalar(-8), SkIntToScalar(16), noop_cropped.get(), &big_rect));

            canvas->restore();
            canvas->translate(0, SkIntToScalar(80));
        }
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new ImageFiltersCropExpandGM; }
static skiagm::GMRegistry reg(MyFactory);
