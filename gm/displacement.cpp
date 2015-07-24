/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapSource.h"
#include "SkDisplacementMapEffect.h"
#include "gm.h"
#include "sk_tool_utils.h"

namespace skiagm {

class DisplacementMapGM : public GM {
public:
    DisplacementMapGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("displacement");
    }

    void make_bitmaps() {
        make_bitmap();
        make_checkerboard(&fCheckerboard, 80, 80);
        make_checkerboard(&fSmall,        64, 64);
        make_checkerboard(&fLarge,        96, 96);
        make_checkerboard(&fLargeW,       96, 64);
        make_checkerboard(&fLargeH,       64, 96);
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(80, 80);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(sk_tool_utils::color_to_565(0xFF884422));
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "g";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(55), paint);
    }

    void make_checkerboard(SkBitmap* bitmap, int w, int h) {
        bitmap->allocN32Pixels(w, h);
        SkCanvas canvas(*bitmap);
        sk_tool_utils::draw_checkerboard(&canvas, sk_tool_utils::color_to_565(0xFF244484),
            sk_tool_utils::color_to_565(0xFF804020), 8);
    }

    virtual SkISize onISize() {
        return SkISize::Make(500, 500);
    }

    void drawClippedBitmap(SkCanvas* canvas, int x, int y, const SkPaint& paint) {
        canvas->save();
        canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
        canvas->clipRect(SkRect::MakeWH(SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, 0, 0, &paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmaps();
            fInitialized = true;
        }
        canvas->clear(SK_ColorBLACK);
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> displ(SkBitmapSource::Create(fCheckerboard));
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType, 0.0f, displ))->unref();
        drawClippedBitmap(canvas, 0, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 16.0f, displ))->unref();
        drawClippedBitmap(canvas, 100, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType, 32.0f, displ))->unref();
        drawClippedBitmap(canvas, 200, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 48.0f, displ))->unref();
        drawClippedBitmap(canvas, 300, 0, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 64.0f, displ))->unref();
        drawClippedBitmap(canvas, 400, 0, paint);

        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType, 40.0f, displ))->unref();
        drawClippedBitmap(canvas, 0, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ))->unref();
        drawClippedBitmap(canvas, 100, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType, 40.0f, displ))->unref();
        drawClippedBitmap(canvas, 200, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ))->unref();
        drawClippedBitmap(canvas, 300, 100, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ))->unref();
        drawClippedBitmap(canvas, 400, 100, paint);

        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(30, 30, 40, 40));
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            0.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 0, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            16.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 100, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            32.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 200, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            48.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 300, 200, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            64.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 400, 200, paint);

        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            40.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 0, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 100, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            40.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 200, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 300, 300, paint);
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ, NULL, &cropRect))->unref();
        drawClippedBitmap(canvas, 400, 300, paint);

        // Tests for images of different sizes
        displ.reset(SkBitmapSource::Create(fSmall));
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            40.0f, displ))->unref();
        drawClippedBitmap(canvas, 0, 400, paint);
        displ.reset(SkBitmapSource::Create(fLarge));
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ))->unref();
        drawClippedBitmap(canvas, 100, 400, paint);
        displ.reset(SkBitmapSource::Create(fLargeW));
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kR_ChannelSelectorType,
            SkDisplacementMapEffect::kB_ChannelSelectorType,
            40.0f, displ))->unref();
        drawClippedBitmap(canvas, 200, 400, paint);
        displ.reset(SkBitmapSource::Create(fLargeH));
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, displ))->unref();
        drawClippedBitmap(canvas, 300, 400, paint);

        // Test for no given displacement input. In this case, both displacement
        // and color should use the same bitmap, given to SkCanvas::drawBitmap()
        // as an input argument.
        paint.setImageFilter(SkDisplacementMapEffect::Create(
            SkDisplacementMapEffect::kG_ChannelSelectorType,
            SkDisplacementMapEffect::kA_ChannelSelectorType,
            40.0f, NULL))->unref();
        drawClippedBitmap(canvas, 400, 400, paint);
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap, fCheckerboard, fSmall, fLarge, fLargeW, fLargeH;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DisplacementMapGM; }
static GMRegistry reg(MyFactory);

}
