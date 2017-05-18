/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAlphaThresholdFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkRegion.h"
#include "SkSurface.h"

#define WIDTH 500
#define HEIGHT 500

static void draw_rects(SkCanvas* canvas) {
    SkPaint rectPaint;
    rectPaint.setColor(SK_ColorBLUE);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, WIDTH / 2, HEIGHT / 2), rectPaint);
    rectPaint.setColor(0xBFFF0000);
    canvas->drawRect(SkRect::MakeXYWH(WIDTH / 2, 0, WIDTH / 2, HEIGHT / 2), rectPaint);
    rectPaint.setColor(0x3F00FF00);
    canvas->drawRect(SkRect::MakeXYWH(0, HEIGHT / 2, WIDTH / 2, HEIGHT / 2), rectPaint);
    rectPaint.setColor(SK_ColorTRANSPARENT);
    canvas->drawRect(SkRect::MakeXYWH(WIDTH / 2, HEIGHT / 2, WIDTH / 2, HEIGHT / 2), rectPaint);
}

static SkPaint create_filter_paint(SkImageFilter::CropRect* cropRect = nullptr) {
    SkIRect rects[2];
    rects[0] = SkIRect::MakeXYWH(0, 150, WIDTH, HEIGHT - 300);
    rects[1] = SkIRect::MakeXYWH(150, 0, WIDTH - 300, HEIGHT);
    SkRegion region;
    region.setRects(rects, 2);

    SkPaint paint;
    sk_sp<SkImageFilter> offset(SkOffsetImageFilter::Make(25, 25, nullptr));
    paint.setImageFilter(SkAlphaThresholdFilter::Make(region, 0.2f, 0.7f, std::move(offset), cropRect));
    return paint;
}

class ImageAlphaThresholdGM : public skiagm::GM {
public:
    ImageAlphaThresholdGM(bool useCropRect) : fUseCropRect(useCropRect) {
        this->setBGColor(SK_ColorWHITE);
    }

protected:

    SkString onShortName() override {
        if (fUseCropRect) {
            return SkString("imagealphathreshold_crop");
        }

        return SkString("imagealphathreshold");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        SkMatrix matrix;
        matrix.reset();
        matrix.setTranslate(WIDTH * .1f, HEIGHT * .1f);
        matrix.postScale(.8f, .8f);

        canvas->concat(matrix);

        SkRect r = SkRect::MakeLTRB(100, 100, WIDTH - 100, HEIGHT - 100);
        SkImageFilter::CropRect cropRect(r);

        SkPaint paint = create_filter_paint(fUseCropRect ? &cropRect : nullptr);
        canvas->saveLayer(nullptr, &paint);
        draw_rects(canvas);

        canvas->restore();
    }

private:
    bool fUseCropRect;

    typedef GM INHERITED;
};

// Create a 'width' x 'height' SkSurface that matches the colorType of 'canvas' as
// best we can
static sk_sp<SkSurface> make_color_matching_surface(SkCanvas* canvas, int width, int height,
                                                    SkAlphaType alphaType) {

    SkColorType ct = canvas->imageInfo().colorType();
    sk_sp<SkColorSpace> cs(canvas->imageInfo().refColorSpace());

    if (kUnknown_SkColorType == ct) {
        // For backends that aren't yet color-space aware we just fallback to N32.
        ct = kN32_SkColorType;
        cs = nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(width, height, ct, alphaType, std::move(cs));

    sk_sp<SkSurface> result = canvas->makeSurface(info);
    if (!result) {
        result = SkSurface::MakeRaster(info);
    }

    return result;
}

class ImageAlphaThresholdSurfaceGM : public skiagm::GM {
public:
    ImageAlphaThresholdSurfaceGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("imagealphathreshold_surface");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        SkMatrix matrix;
        matrix.reset();
        matrix.setTranslate(WIDTH * .1f, HEIGHT * .1f);
        matrix.postScale(.8f, .8f);

        canvas->concat(matrix);

        sk_sp<SkSurface> surface(make_color_matching_surface(canvas, WIDTH, HEIGHT,
                                                             kPremul_SkAlphaType));
        if (!surface) {
            return;
        }

        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        draw_rects(surface->getCanvas());

        SkPaint paint = create_filter_paint();
        canvas->clipRect(SkRect::MakeLTRB(100, 100, WIDTH - 100, HEIGHT - 100));
        canvas->drawImage(surface->makeImageSnapshot().get(), 0, 0, &paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageAlphaThresholdGM(true);)
DEF_GM(return new ImageAlphaThresholdGM(false);)
DEF_GM(return new ImageAlphaThresholdSurfaceGM();)
