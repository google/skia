/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

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

typedef sk_sp<SkImage> (*ImageMakerProc)(GrContext*, SkPicture*, const SkImageInfo&);

static sk_sp<SkImage> make_raster(GrContext*, SkPicture* pic, const SkImageInfo& info) {
    auto surface(SkSurface::MakeRaster(info));
    surface->getCanvas()->clear(0);
    surface->getCanvas()->drawPicture(pic);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_texture(GrContext* ctx, SkPicture* pic, const SkImageInfo& info) {
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

static sk_sp<SkImage> make_pict_gen(GrContext*, SkPicture* pic, const SkImageInfo& info) {
    return SkImage::MakeFromPicture(sk_ref_sp(pic), info.dimensions(), nullptr, nullptr,
                                    SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
}

static sk_sp<SkImage> make_encode_gen(GrContext* ctx, SkPicture* pic, const SkImageInfo& info) {
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

        const SkShader::TileMode tile = SkShader::kRepeat_TileMode;
        const SkMatrix localM = SkMatrix::MakeTrans(-50, -50);
        SkPaint paint;
        paint.setShader(image->makeShader(tile, tile, &localM));
        paint.setAntiAlias(true);
        canvas->drawCircle(50, 50, 50, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20, 20);

        const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

        for (size_t i = 0; i < SK_ARRAY_COUNT(gProcs); ++i) {
            sk_sp<SkImage> image(gProcs[i](canvas->getGrContext(), fPicture.get(), info));
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
