/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTiledImageUtils.h"
#include "include/effects/SkGradientShader.h"
#include "tools/ToolUtils.h"

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
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(width, height)));
    draw(surface->getCanvas(), width, height, colors);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_picture_image(int width, int height, SkColor colors[2]) {
    SkPictureRecorder recorder;
    draw(recorder.beginRecording(SkRect::MakeIWH(width, height)), width, height, colors);
    return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                         {width, height},
                                         nullptr,
                                         nullptr,
                                         SkImages::BitDepth::kU8,
                                         SkColorSpace::MakeSRGB());
}

typedef sk_sp<SkImage> (*ImageMakerProc)(int width, int height, SkColor colors[2]);

static void show_image(SkCanvas* canvas, int width, int height, SkColor colors[2],
                       ImageMakerProc proc, bool manuallyTile) {
    sk_sp<SkImage> image = proc(width, height, colors);
    if (!image) {
        return;
    }

    SkPaint borderPaint;

    borderPaint.setStyle(SkPaint::kStroke_Style);

    SkRect dstRect = SkRect::MakeWH(128.f, 128.f);

    canvas->save();
    canvas->clipRect(dstRect);
    if (manuallyTile) {
        SkTiledImageUtils::DrawImage(canvas, image, 0, 0);
    } else {
        canvas->drawImage(image, 0, 0);
    }
    canvas->restore();
    canvas->drawRect(dstRect, borderPaint);

    dstRect.offset(SkIntToScalar(150), 0);
    int hw = width / 2;
    int hh = height / 2;
    SkRect subset = SkRect::MakeLTRB(hw - 64, hh - 32, hw + 64, hh + 32);
    if (manuallyTile) {
        SkTiledImageUtils::DrawImageRect(canvas, image, subset, dstRect, {}, nullptr,
                                         SkCanvas::kStrict_SrcRectConstraint);
    } else {
        canvas->drawImageRect(image, subset, dstRect, {}, nullptr,
                              SkCanvas::kStrict_SrcRectConstraint);
    }
    canvas->drawRect(dstRect, borderPaint);

    dstRect.offset(SkIntToScalar(150), 0);
    if (manuallyTile) {
        SkTiledImageUtils::DrawImageRect(canvas, image, dstRect);
    } else {
        canvas->drawImageRect(image, dstRect, {});
    }
    canvas->drawRect(dstRect, borderPaint);
}

class VeryLargeBitmapGM : public skiagm::GM {
    ImageMakerProc  fProc;
    const char*     fBaseName;
    bool            fManuallyTile;

public:
    VeryLargeBitmapGM(ImageMakerProc proc, const char baseName[], bool manuallyTile)
            : fProc(proc)
            , fBaseName(baseName)
            , fManuallyTile(manuallyTile) {}

private:
    SkString getName() const override {
        SkString name(fBaseName);

        if (fManuallyTile) {
            name.append("_manual");
        }

        return name;
    }

    SkISize getISize() override { return {500, 600}; }

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
        show_image(canvas, small, small, colors, fProc, fManuallyTile);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorBLUE;
        colors[1] = SK_ColorMAGENTA;
        show_image(canvas, big, small, colors, fProc, fManuallyTile);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorMAGENTA;
        colors[1] = SK_ColorYELLOW;
        show_image(canvas, medium, medium, colors, fProc, fManuallyTile);
        canvas->translate(0, SkIntToScalar(150));

        colors[0] = SK_ColorGREEN;
        colors[1] = SK_ColorYELLOW;
        // This used to be big enough that we didn't draw on CPU, but now we do.
        show_image(canvas, veryBig, small, colors, fProc, fManuallyTile);
    }
};

DEF_GM( return new VeryLargeBitmapGM(make_raster_image, "verylargebitmap",
                                     /* manuallyTile= */ false); )
DEF_GM( return new VeryLargeBitmapGM(make_raster_image, "verylargebitmap",
                                     /* manuallyTile= */ true); )
DEF_GM( return new VeryLargeBitmapGM(make_picture_image, "verylarge_picture_image",
                                     /* manuallyTile= */ false); )
DEF_GM( return new VeryLargeBitmapGM(make_picture_image, "verylarge_picture_image",
                                     /* manuallyTile= */ true); )
