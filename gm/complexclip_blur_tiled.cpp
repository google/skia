/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurImageFilter.h"
#include "SkRRect.h"
#include "SkSurface.h"

#define WIDTH 512
#define HEIGHT 512

namespace skiagm {

class ComplexClipBlurTiledGM : public GM {
public:
    ComplexClipBlurTiledGM() {
    }

protected:
    SkString onShortName() override {
        return SkString("complexclip_blur_tiled");
    }

    SkISize onISize() override {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint blurPaint;
        SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(5.0f, 5.0f));
        blurPaint.setImageFilter(blur);
        const SkScalar tile_size = SkIntToScalar(128);
        SkRect bounds;
        if (!canvas->getClipBounds(&bounds)) {
            bounds.setEmpty();
        }
        int ts = SkScalarCeilToInt(tile_size);
        SkImageInfo info = SkImageInfo::MakeN32Premul(ts, ts);
        SkAutoTUnref<SkSurface> tileSurface(canvas->newSurface(info));
        if (!tileSurface.get()) {
            tileSurface.reset(SkSurface::NewRaster(info));
        }
        SkCanvas* tileCanvas = tileSurface->getCanvas();
        for (SkScalar y = bounds.top(); y < bounds.bottom(); y += tile_size) {
            for (SkScalar x = bounds.left(); x < bounds.right(); x += tile_size) {
                tileCanvas->save();
                tileCanvas->clear(0);
                tileCanvas->translate(-x, -y);
                SkRect rect = SkRect::MakeWH(WIDTH, HEIGHT);
                tileCanvas->saveLayer(&rect, &blurPaint);
                SkRRect rrect = SkRRect::MakeRectXY(rect.makeInset(20, 20), 25, 25);
                tileCanvas->clipRRect(rrect, SkRegion::kDifference_Op, true);
                SkPaint paint;
                tileCanvas->drawRect(rect, paint);
                tileCanvas->restore();
                tileCanvas->restore();
                SkAutoTUnref<SkImage> tileImage(tileSurface->newImageSnapshot());
                canvas->drawImage(tileImage, x, y);
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory1(void*) { return new ComplexClipBlurTiledGM(); }
static GMRegistry reg1(MyFactory1);

}
