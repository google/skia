/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
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
    SkAutoDataUnref data(SkData::NewFromFileName("/Users/mike/Downloads/skia.google.jpeg"));
    if (nullptr == data.get()) {
        return;
    }
    SkImage* image = SkImage::NewFromEncoded(data);
    if (image) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->scale(size.width() * 1.0f / image->width(),
                      size.height() * 1.0f / image->height());
        canvas->drawImage(image, 0, 0, nullptr);
        image->unref();
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
    SkImage* imgR = surf->newImageSnapshot();

    if (true) {
        SkImage* imgR2 = surf->newImageSnapshot();
        SkASSERT(imgR == imgR2);
        imgR2->unref();
    }

    drawContents(surf, SK_ColorGREEN);
    SkImage* imgG = surf->newImageSnapshot();

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

    imgG->unref();
    imgR->unref();
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
        SkAutoTUnref<SkSurface> surf0(SkSurface::NewRasterDirect(info, fBuffer, RB));
        SkAutoTUnref<SkSurface> surf1(SkSurface::NewRaster(info));
        SkAutoTUnref<SkSurface> surf2;  // gpu

#if SK_SUPPORT_GPU
        surf2.reset(SkSurface::NewRenderTarget(canvas->getGrContext(),
                                               SkSurface::kNo_Budgeted, info));
#endif

        test_surface(canvas, surf0, true);
        canvas->translate(80, 0);
        test_surface(canvas, surf1, true);
        if (surf2) {
            canvas->translate(80, 0);
            test_surface(canvas, surf2, true);
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

static SkImage* make_raster(const SkImageInfo& info, GrContext*, void (*draw)(SkCanvas*)) {
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    draw(surface->getCanvas());
    return surface->newImageSnapshot();
}

static SkImage* make_picture(const SkImageInfo& info, GrContext*, void (*draw)(SkCanvas*)) {
    SkPictureRecorder recorder;
    draw(recorder.beginRecording(SkRect::MakeIWH(info.width(), info.height())));
    SkAutoTUnref<SkPicture> pict(recorder.endRecording());
    return SkImage::NewFromPicture(pict, info.dimensions(), nullptr, nullptr);
}

static SkImage* make_codec(const SkImageInfo& info, GrContext*, void (*draw)(SkCanvas*)) {
    SkAutoTUnref<SkImage> image(make_raster(info, nullptr, draw));
    SkAutoTUnref<SkData> data(image->encode());
    return SkImage::NewFromEncoded(data);
}

static SkImage* make_gpu(const SkImageInfo& info, GrContext* ctx, void (*draw)(SkCanvas*)) {
    if (!ctx) { return nullptr; }
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(ctx, SkSurface::kNo_Budgeted, info));
    draw(surface->getCanvas());
    return surface->newImageSnapshot();
}

typedef SkImage* (*ImageMakerProc)(const SkImageInfo&, GrContext*, void (*)(SkCanvas*));

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
            SkAutoTUnref<SkImage> image(proc(info, canvas->getGrContext(), draw_contents));
            if (image) {
                show_scaled_pixels(canvas, image);
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
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    draw_opaque_contents(surface->getCanvas());
    SkAutoTUnref<SkImage> img(surface->newImageSnapshot());
    return new ImageGeneratorFromImage(img);
}

static SkImageGenerator* gen_picture(const SkImageInfo& info) {
    SkPictureRecorder recorder;
    draw_opaque_contents(recorder.beginRecording(SkRect::MakeIWH(info.width(), info.height())));
    SkAutoTUnref<SkPicture> pict(recorder.endRecording());
    return SkImageGenerator::NewFromPicture(info.dimensions(), pict, nullptr, nullptr);
}

static SkImageGenerator* gen_png(const SkImageInfo& info) {
    SkAutoTUnref<SkImage> image(make_raster(info, nullptr, draw_opaque_contents));
    SkAutoTUnref<SkData> data(image->encode(SkImageEncoder::kPNG_Type, 100));
    return SkImageGenerator::NewFromEncoded(data);
}

static SkImageGenerator* gen_jpg(const SkImageInfo& info) {
    SkAutoTUnref<SkImage> image(make_raster(info, nullptr, draw_opaque_contents));
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
