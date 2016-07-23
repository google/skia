/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <functional>
#include "gm.h"
#include "SkAutoPixmapStorage.h"
#include "SkData.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

static void drawJpeg(SkCanvas* canvas, const SkISize& size) {
    // TODO: Make this draw a file that is checked in, so it can
    // be exercised on machines other than mike's. Will require a
    // rebaseline.
    sk_sp<SkData> data(SkData::MakeFromFileName("/Users/mike/Downloads/skia.google.jpeg"));
    if (nullptr == data) {
        return;
    }
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(data));
    if (image) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->scale(size.width() * 1.0f / image->width(),
                      size.height() * 1.0f / image->height());
        canvas->drawImage(image, 0, 0, nullptr);
    }
}

static void drawContents(SkSurface* surface, SkColor fillC) {
    SkSize size = SkSize::Make(SkIntToScalar(surface->width()),
                               SkIntToScalar(surface->height()));
    SkCanvas* canvas = surface->getCanvas();

    SkScalar stroke = size.fWidth / 10;
    SkScalar radius = (size.fWidth - stroke) / 2;

    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setColor(fillC);
    canvas->drawCircle(size.fWidth/2, size.fHeight/2, radius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(stroke);
    paint.setColor(SK_ColorBLACK);
    canvas->drawCircle(size.fWidth/2, size.fHeight/2, radius, paint);
}

static void test_surface(SkCanvas* canvas, SkSurface* surf, bool usePaint) {
    drawContents(surf, SK_ColorRED);
    sk_sp<SkImage> imgR = surf->makeImageSnapshot();

    if (true) {
        sk_sp<SkImage> imgR2 = surf->makeImageSnapshot();
        SkASSERT(imgR == imgR2);
    }

    drawContents(surf, SK_ColorGREEN);
    sk_sp<SkImage> imgG = surf->makeImageSnapshot();

    // since we've drawn after we snapped imgR, imgG will be a different obj
    SkASSERT(imgR != imgG);

    drawContents(surf, SK_ColorBLUE);

    SkPaint paint;
//    paint.setFilterBitmap(true);
//    paint.setAlpha(0x80);

    canvas->drawImage(imgR, 0, 0, usePaint ? &paint : nullptr);
    canvas->drawImage(imgG, 0, 80, usePaint ? &paint : nullptr);
    surf->draw(canvas, 0, 160, usePaint ? &paint : nullptr);

    SkRect src1, src2, src3;
    src1.iset(0, 0, surf->width(), surf->height());
    src2.iset(-surf->width() / 2, -surf->height() / 2,
             surf->width(), surf->height());
    src3.iset(0, 0, surf->width() / 2, surf->height() / 2);

    SkRect dst1, dst2, dst3, dst4;
    dst1.set(0, 240, 65, 305);
    dst2.set(0, 320, 65, 385);
    dst3.set(0, 400, 65, 465);
    dst4.set(0, 480, 65, 545);

    canvas->drawImageRect(imgR, src1, dst1, usePaint ? &paint : nullptr);
    canvas->drawImageRect(imgG, src2, dst2, usePaint ? &paint : nullptr);
    canvas->drawImageRect(imgR, src3, dst3, usePaint ? &paint : nullptr);
    canvas->drawImageRect(imgG, dst4, usePaint ? &paint : nullptr);
}

class ImageGM : public skiagm::GM {
    void*   fBuffer;
    size_t  fBufferSize;
    SkSize  fSize;
    enum {
        W = 64,
        H = 64,
        RB = W * 4 + 8,
    };
public:
    ImageGM() {
        fBufferSize = RB * H;
        fBuffer = sk_malloc_throw(fBufferSize);
        fSize.set(SkIntToScalar(W), SkIntToScalar(H));
    }

    virtual ~ImageGM() {
        sk_free(fBuffer);
    }

protected:
    SkString onShortName() override {
        return SkString("image-surface");
    }

    SkISize onISize() override {
        return SkISize::Make(960, 1200);
    }

    void onDraw(SkCanvas* canvas) override {
        drawJpeg(canvas, this->getISize());

        canvas->scale(2, 2);

        static const char* kLabel1 = "Original Img";
        static const char* kLabel2 = "Modified Img";
        static const char* kLabel3 = "Cur Surface";
        static const char* kLabel4 = "Full Crop";
        static const char* kLabel5 = "Over-crop";
        static const char* kLabel6 = "Upper-left";
        static const char* kLabel7 = "No Crop";

        static const char* kLabel8 = "Pre-Alloc Img";
        static const char* kLabel9 = "New Alloc Img";
        static const char* kLabel10 = "GPU";

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&textPaint);
        textPaint.setTextSize(8);

        canvas->drawText(kLabel1, strlen(kLabel1), 10,  60, textPaint);
        canvas->drawText(kLabel2, strlen(kLabel2), 10, 140, textPaint);
        canvas->drawText(kLabel3, strlen(kLabel3), 10, 220, textPaint);
        canvas->drawText(kLabel4, strlen(kLabel4), 10, 300, textPaint);
        canvas->drawText(kLabel5, strlen(kLabel5), 10, 380, textPaint);
        canvas->drawText(kLabel6, strlen(kLabel6), 10, 460, textPaint);
        canvas->drawText(kLabel7, strlen(kLabel7), 10, 540, textPaint);

        canvas->drawText(kLabel8, strlen(kLabel8),  80, 10, textPaint);
        canvas->drawText(kLabel9, strlen(kLabel9), 160, 10, textPaint);
        canvas->drawText(kLabel10, strlen(kLabel10), 265, 10, textPaint);

        canvas->translate(80, 20);

        // since we draw into this directly, we need to start fresh
        sk_bzero(fBuffer, fBufferSize);

        SkImageInfo info = SkImageInfo::MakeN32Premul(W, H);
        sk_sp<SkSurface> surf0(SkSurface::MakeRasterDirect(info, fBuffer, RB));
        sk_sp<SkSurface> surf1(SkSurface::MakeRaster(info));
        sk_sp<SkSurface> surf2;  // gpu

#if SK_SUPPORT_GPU
        surf2 = SkSurface::MakeRenderTarget(canvas->getGrContext(), SkBudgeted::kNo, info);
#endif

        test_surface(canvas, surf0.get(), true);
        canvas->translate(80, 0);
        test_surface(canvas, surf1.get(), true);
        if (surf2) {
            canvas->translate(80, 0);
            test_surface(canvas, surf2.get(), true);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ImageGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPictureRecorder.h"

static void draw_pixmap(SkCanvas* canvas, const SkPixmap& pmap) {
    SkBitmap bitmap;
    bitmap.installPixels(pmap);
    canvas->drawBitmap(bitmap, 0, 0, nullptr);
}

static void show_scaled_pixels(SkCanvas* canvas, SkImage* image) {
    SkAutoCanvasRestore acr(canvas, true);

    canvas->drawImage(image, 0, 0, nullptr);
    canvas->translate(110, 10);

    const SkImageInfo info = SkImageInfo::MakeN32Premul(40, 40);
    SkAutoPixmapStorage storage;
    storage.alloc(info);

    const SkImage::CachingHint chints[] = {
        SkImage::kAllow_CachingHint, SkImage::kDisallow_CachingHint,
    };
    const SkFilterQuality qualities[] = {
        kNone_SkFilterQuality, kLow_SkFilterQuality, kMedium_SkFilterQuality, kHigh_SkFilterQuality,
    };

    for (auto ch : chints) {
        canvas->save();
        for (auto q : qualities) {
            if (image->scalePixels(storage, q, ch)) {
                draw_pixmap(canvas, storage);
            }
            canvas->translate(70, 0);
        }
        canvas->restore();
        canvas->translate(0, 45);
    }
}

static void draw_contents(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    canvas->drawCircle(50, 50, 35, paint);
}

static sk_sp<SkImage> make_raster(const SkImageInfo& info, GrContext*, void (*draw)(SkCanvas*)) {
    auto surface(SkSurface::MakeRaster(info));
    draw(surface->getCanvas());
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_picture(const SkImageInfo& info, GrContext*, void (*draw)(SkCanvas*)) {
    SkPictureRecorder recorder;
    draw(recorder.beginRecording(SkRect::MakeIWH(info.width(), info.height())));
    return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(),
                                    info.dimensions(), nullptr, nullptr);
}

static sk_sp<SkImage> make_codec(const SkImageInfo& info, GrContext*, void (*draw)(SkCanvas*)) {
    sk_sp<SkImage> image(make_raster(info, nullptr, draw));
    sk_sp<SkData> data(image->encode());
    return SkImage::MakeFromEncoded(data);
}

static sk_sp<SkImage> make_gpu(const SkImageInfo& info, GrContext* ctx, void (*draw)(SkCanvas*)) {
    if (!ctx) { return nullptr; }
    auto surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info));
    if (!surface) { return nullptr; }
    draw(surface->getCanvas());
    return surface->makeImageSnapshot();
}

typedef sk_sp<SkImage> (*ImageMakerProc)(const SkImageInfo&, GrContext*, void (*)(SkCanvas*));

class ScalePixelsGM : public skiagm::GM {
public:
    ScalePixelsGM() {}

protected:
    SkString onShortName() override {
        return SkString("scale-pixels");
    }

    SkISize onISize() override {
        return SkISize::Make(960, 1200);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

        const ImageMakerProc procs[] = {
            make_codec, make_raster, make_picture, make_codec, make_gpu,
        };
        for (auto& proc : procs) {
            sk_sp<SkImage> image(proc(info, canvas->getGrContext(), draw_contents));
            if (image) {
                show_scaled_pixels(canvas, image.get());
            }
            canvas->translate(0, 120);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ScalePixelsGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkImageGenerator.h"

static SkImageInfo make_info(SkImage* img) {
    return SkImageInfo::MakeN32(img->width(), img->height(),
                                img->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType);
}

// Its simple, but I wonder if we should expose this formally?
//
class ImageGeneratorFromImage : public SkImageGenerator {
public:
    ImageGeneratorFromImage(SkImage* img) : INHERITED(make_info(img)), fImg(SkRef(img)) {}

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
                     int* ctableCount) override {
        return fImg->readPixels(info, pixels, rowBytes, 0, 0);
    }

private:
    SkAutoTUnref<SkImage> fImg;

    typedef SkImageGenerator INHERITED;
};

static void draw_opaque_contents(SkCanvas* canvas) {
    canvas->drawColor(0xFFFF8844);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    canvas->drawCircle(50, 50, 35, paint);
}

static SkImageGenerator* gen_raster(const SkImageInfo& info) {
    auto surface(SkSurface::MakeRaster(info));
    draw_opaque_contents(surface->getCanvas());
    return new ImageGeneratorFromImage(surface->makeImageSnapshot().get());
}

static SkImageGenerator* gen_picture(const SkImageInfo& info) {
    SkPictureRecorder recorder;
    draw_opaque_contents(recorder.beginRecording(SkRect::MakeIWH(info.width(), info.height())));
    sk_sp<SkPicture> pict(recorder.finishRecordingAsPicture());
    return SkImageGenerator::NewFromPicture(info.dimensions(), pict.get(), nullptr, nullptr);
}

static SkImageGenerator* gen_png(const SkImageInfo& info) {
    sk_sp<SkImage> image(make_raster(info, nullptr, draw_opaque_contents));
    SkAutoTUnref<SkData> data(image->encode(SkImageEncoder::kPNG_Type, 100));
    return SkImageGenerator::NewFromEncoded(data);
}

static SkImageGenerator* gen_jpg(const SkImageInfo& info) {
    sk_sp<SkImage> image(make_raster(info, nullptr, draw_opaque_contents));
    SkAutoTUnref<SkData> data(image->encode(SkImageEncoder::kJPEG_Type, 100));
    return SkImageGenerator::NewFromEncoded(data);
}

typedef SkImageGenerator* (*GeneratorMakerProc)(const SkImageInfo&);

static void show_scaled_generator(SkCanvas* canvas, SkImageGenerator* gen) {
    const SkImageInfo genInfo = gen->getInfo();

    SkAutoCanvasRestore acr(canvas, true);

    SkBitmap bm;
    bm.allocPixels(genInfo);
    if (gen->getPixels(bm.info(), bm.getPixels(), bm.rowBytes())) {
        canvas->drawBitmap(bm, 0, 0, nullptr);
    }
    canvas->translate(110, 0);

    const float scales[] = { 0.75f, 0.5f, 0.25f };
    for (auto scale : scales) {
        SkImageGenerator::SupportedSizes sizes;
        if (gen->computeScaledDimensions(scale, &sizes)) {
            const SkImageInfo info = SkImageInfo::MakeN32Premul(sizes.fSizes[0].width(),
                                                                sizes.fSizes[0].height());
            bm.allocPixels(info);
            SkPixmap pmap;
            bm.peekPixels(&pmap);
            if (gen->generateScaledPixels(pmap)) {
                canvas->drawBitmap(bm, 0, SkIntToScalar(genInfo.height() - info.height())/2);
            }
        }
        canvas->translate(100, 0);
    }
}

class ScaleGeneratorGM : public skiagm::GM {
public:
    ScaleGeneratorGM() {}

protected:
    SkString onShortName() override {
        return SkString("scale-generator");
    }

    SkISize onISize() override {
        return SkISize::Make(500, 500);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(10, 10);

        // explicitly make it opaque, so we can test JPEG (which is only ever opaque)
        const SkImageInfo info = SkImageInfo::MakeN32(100, 100, kOpaque_SkAlphaType);

        const GeneratorMakerProc procs[] = {
            gen_raster, gen_picture, gen_png, gen_jpg,
        };
        for (auto& proc : procs) {
            SkAutoTDelete<SkImageGenerator> gen(proc(info));
            if (gen) {
                show_scaled_generator(canvas, gen);
            }
            canvas->translate(0, 120);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ScaleGeneratorGM; )

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#endif

DEF_SIMPLE_GM(new_texture_image, canvas, 225, 60) {
    GrContext* context = nullptr;
#if SK_SUPPORT_GPU
    context = canvas->getGrContext();
    sk_gpu_test::GrContextFactory factory;
#endif
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }

    auto render_image = [](SkCanvas* canvas) {
        canvas->clear(SK_ColorBLUE);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(10.f,10.f,10.f,10.f), paint);
        paint.setColor(SK_ColorGREEN);
        canvas->drawRect(SkRect::MakeXYWH(30.f,10.f,10.f,10.f), paint);
        paint.setColor(SK_ColorYELLOW);
        canvas->drawRect(SkRect::MakeXYWH(10.f,30.f,10.f,10.f), paint);
        paint.setColor(SK_ColorCYAN);
        canvas->drawRect(SkRect::MakeXYWH(30.f,30.f,10.f,10.f), paint);
    };

    static const int kSize = 50;
    SkBitmap bmp;
    bmp.allocN32Pixels(kSize, kSize);
    SkCanvas bmpCanvas(bmp);
    render_image(&bmpCanvas);

    std::function<sk_sp<SkImage>()> imageFactories[] = {
        // Create sw raster image.
        [bmp] {
            return SkImage::MakeFromBitmap(bmp);
        },
        // Create encoded image.
        [bmp] {
            sk_sp<SkData> src(
                SkImageEncoder::EncodeData(bmp, SkImageEncoder::kPNG_Type, 100));
            return SkImage::MakeFromEncoded(std::move(src));
        },
        // Create a picture image.
        [render_image] {
            SkPictureRecorder recorder;
            SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kSize), SkIntToScalar(kSize));
            render_image(canvas);
            return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(),
                                           SkISize::Make(kSize, kSize), nullptr, nullptr);
        },
        // Create a texture image
        [context, render_image]() -> sk_sp<SkImage> {
            auto surface(
                SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                            SkImageInfo::MakeN32Premul(kSize, kSize)));
            if (!surface) {
                return nullptr;
            }
            render_image(surface->getCanvas());
            return surface->makeImageSnapshot();
        }
    };

    static const SkScalar kPad = 5.f;
    canvas->translate(kPad, kPad);
    for (auto factory : imageFactories) {
        auto image(factory());
        if (!image) {
            continue;
        }
        if (context) {
            sk_sp<SkImage> texImage(image->makeTextureImage(context));
            if (texImage) {
                canvas->drawImage(texImage, 0, 0);
            }
        }
        canvas->translate(image->width() + kPad, 0);
    }
}
