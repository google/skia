/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

static void draw(SkCanvas* canvas, int width, int height, SkColor colors[2]) {
    const SkPoint center = { SkIntToScalar(width)/2, SkIntToScalar(height)/2 };
    const SkScalar radius = 40;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(center, radius, colors, nullptr, 2,
                                                 SkTileMode::kMirror));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);
}

static sk_sp<SkImage> make_raster_image(int width, int height, SkColor colors[2]) {
    auto surface(SkSurface::MakeRasterN32Premul(width, height));
    draw(surface->getCanvas(), width, height, colors);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_picture_image(int width, int height, SkColor colors[2]) {
    SkPictureRecorder recorder;
    draw(recorder.beginRecording(SkRect::MakeIWH(width, height)), width, height, colors);
    return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(),
                                    SkISize::Make(width, height), nullptr, nullptr,
                                    SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
}

typedef sk_sp<SkImage> (*ImageMakerProc)(int width, int height, SkColor colors[2]);

static void show_image(SkCanvas* canvas, int width, int height, SkColor colors[2],
                       ImageMakerProc proc) {
    sk_sp<SkImage> image(proc(width, height, colors));

    SkPaint borderPaint;

    borderPaint.setStyle(SkPaint::kStroke_Style);

    SkRect dstRect = SkRect::MakeWH(128.f, 128.f);

    canvas->save();
    canvas->clipRect(dstRect);
    canvas->drawImage(image, 0, 0, nullptr);
    canvas->restore();
    canvas->drawRect(dstRect, borderPaint);

    dstRect.offset(SkIntToScalar(150), 0);
    int hw = width / 2;
    int hh = height / 2;
    SkIRect subset = SkIRect::MakeLTRB(hw - 64, hh - 32, hw + 64, hh + 32);
    canvas->drawImageRect(image, subset, dstRect, nullptr);
    canvas->drawRect(dstRect, borderPaint);

    dstRect.offset(SkIntToScalar(150), 0);
    canvas->drawImageRect(image, dstRect, nullptr);
    canvas->drawRect(dstRect, borderPaint);
}

class VeryLargeBitmapGM : public skiagm::GM {
    ImageMakerProc  fProc;
    SkString        fName;

public:
    VeryLargeBitmapGM(ImageMakerProc proc, const char suffix[]) : fProc(proc) {
        fName.printf("verylarge%s", suffix);
    }

protected:
    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(500, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        int veryBig = 65*1024; // 64K < size
        int big = 33*1024;     // 32K < size < 64K
        // smaller than many max texture sizes, but large enough to gpu-tile for memory reasons.
        int medium = 5*1024;
        int small = 150;

        SkColor colors[2];

        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        colors[0] = SK_ColorRED;
        colors[1] = SK_ColorGREEN;
        show_image(canvas, small, small, colors, fProc);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorBLUE;
        colors[1] = SK_ColorMAGENTA;
        show_image(canvas, big, small, colors, fProc);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorMAGENTA;
        colors[1] = SK_ColorYELLOW;
        show_image(canvas, medium, medium, colors, fProc);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorGREEN;
        colors[1] = SK_ColorYELLOW;
        // This used to be big enough that we didn't draw on CPU, but now we do.
        show_image(canvas, veryBig, small, colors, fProc);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new VeryLargeBitmapGM(make_raster_image, "bitmap"); )
DEF_GM( return new VeryLargeBitmapGM(make_picture_image, "_picture_image"); )
