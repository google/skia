/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAlphaThresholdFilter.h"
#include "SkRandom.h"
#include "SkSurface.h"

#define WIDTH 500
#define HEIGHT 500

namespace {

void draw_rects(SkCanvas* canvas) {
    SkPaint rectPaint;
    rectPaint.setColor(0xFF0000FF);
    canvas->drawRect(SkRect::MakeXYWH(0, 0, WIDTH / 2, HEIGHT / 2), rectPaint);
    rectPaint.setColor(0xBFFF0000);
    canvas->drawRect(SkRect::MakeXYWH(WIDTH / 2, 0, WIDTH / 2, HEIGHT / 2), rectPaint);
    rectPaint.setColor(0x3F00FF00);
    canvas->drawRect(SkRect::MakeXYWH(0, HEIGHT / 2, WIDTH / 2, HEIGHT / 2), rectPaint);
    rectPaint.setColor(0x00000000);
    canvas->drawRect(SkRect::MakeXYWH(WIDTH / 2, HEIGHT / 2, WIDTH / 2, HEIGHT / 2), rectPaint);
}

SkPaint create_filter_paint() {
    SkIRect rects[2];
    rects[0] = SkIRect::MakeXYWH(0, 150, WIDTH, HEIGHT - 300);
    rects[1] = SkIRect::MakeXYWH(150, 0, WIDTH - 300, HEIGHT);
    SkRegion region;
    region.setRects(rects, 2);

    SkPaint paint;
    paint.setImageFilter(SkAlphaThresholdFilter::Create(region, 0.2f, 0.7f))->unref();
    return paint;
}

};

namespace skiagm {

class ImageAlphaThresholdGM : public GM {
public:
    ImageAlphaThresholdGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:

    SkString onShortName() override {
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

        SkPaint paint = create_filter_paint();
        canvas->saveLayer(nullptr, &paint);
        draw_rects(canvas);

        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

class ImageAlphaThresholdSurfaceGM : public GM {
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
        SkImageInfo info = SkImageInfo::MakeN32(WIDTH, HEIGHT, kOpaque_SkAlphaType);
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info));
        if (nullptr == surface) {
            surface.reset(SkSurface::NewRaster(info));
        }
        surface->getCanvas()->clear(SK_ColorWHITE);
        draw_rects(surface->getCanvas());

        SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
        SkPaint paint = create_filter_paint();
        canvas->clipRect(SkRect::MakeLTRB(100, 100, WIDTH - 100, HEIGHT - 100));
        canvas->drawImage(image, 0, 0, &paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ImageAlphaThresholdGM();)
DEF_GM(return new ImageAlphaThresholdSurfaceGM();)

}
