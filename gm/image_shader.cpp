/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"

#include <utility>

static void draw_something(SkCanvas* canvas, const SkRect& bounds) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorRED);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    canvas->drawRect(bounds, paint);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(SK_ColorBLUE);
    canvas->drawOval(bounds, paint);
}

typedef sk_sp<SkImage> (*ImageMakerProc)(GrRecordingContext*, SkPicture*, const SkImageInfo&);

static sk_sp<SkImage> make_raster(GrRecordingContext*,
                                  SkPicture* pic,
                                  const SkImageInfo& info) {
    auto surface(SkSurface::MakeRaster(info));
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawPicture(pic);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_texture(GrRecordingContext* ctx,
                                   SkPicture* pic,
                                   const SkImageInfo& info) {
    if (!ctx) {
        return nullptr;
    }
    auto surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));
    if (!surface) {
        return nullptr;
    }
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawPicture(pic);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_pict_gen(GrRecordingContext*,
                                    SkPicture* pic,
                                    const SkImageInfo& info) {
    return SkImage::MakeFromPicture(sk_ref_sp(pic), info.dimensions(), nullptr, nullptr,
                                    SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
}

static sk_sp<SkImage> make_encode_gen(GrRecordingContext* ctx,
                                      SkPicture* pic,
                                      const SkImageInfo& info) {
    sk_sp<SkImage> src(make_raster(ctx, pic, info));
    if (!src) {
        return nullptr;
    }
    sk_sp<SkData> encoded = src->encodeToData(SkEncodedImageFormat::kPNG, 100);
    if (!encoded) {
        return nullptr;
    }
    return SkImage::MakeFromEncoded(std::move(encoded));
}

const ImageMakerProc gProcs[] = {
    make_raster,
    make_texture,
    make_pict_gen,
    make_encode_gen,
};

/*
 *  Exercise drawing pictures inside an image, showing that the image version is pixelated
 *  (correctly) when it is inside an image.
 */
class ImageShaderGM : public skiagm::GM {
    sk_sp<SkPicture> fPicture;

public:
    ImageShaderGM() {}

protected:
    SkString onShortName() override {
        return SkString("image-shader");
    }

    SkISize onISize() override {
        return SkISize::Make(850, 450);
    }

    void onOnceBeforeDraw() override {
        const SkRect bounds = SkRect::MakeWH(100, 100);
        SkPictureRecorder recorder;
        draw_something(recorder.beginRecording(bounds), bounds);
        fPicture = recorder.finishRecordingAsPicture();
    }

    void testImage(SkCanvas* canvas, SkImage* image) {
        SkAutoCanvasRestore acr(canvas, true);

        canvas->drawImage(image, 0, 0);
        canvas->translate(0, 120);

        const SkTileMode tile = SkTileMode::kRepeat;
        const SkMatrix localM = SkMatrix::Translate(-50, -50);
        SkPaint paint;
        paint.setShader(image->makeShader(tile, tile, &localM));
        paint.setAntiAlias(true);
        canvas->drawCircle(50, 50, 50, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20, 20);

        const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

        for (size_t i = 0; i < SK_ARRAY_COUNT(gProcs); ++i) {
            sk_sp<SkImage> image(gProcs[i](canvas->recordingContext(), fPicture.get(), info));
            if (image) {
                this->testImage(canvas, image.get());
            }
            canvas->translate(120, 0);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ImageShaderGM; )
