/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "tools/ToolUtils.h"

#include <functional>

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

    ~ImageGM() override {
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
        canvas->scale(2, 2);

        SkFont font(ToolUtils::create_portable_typeface(), 8);

        canvas->drawString("Original Img",  10,  60, font, SkPaint());
        canvas->drawString("Modified Img",  10, 140, font, SkPaint());
        canvas->drawString("Cur Surface",   10, 220, font, SkPaint());
        canvas->drawString("Full Crop",     10, 300, font, SkPaint());
        canvas->drawString("Over-crop",     10, 380, font, SkPaint());
        canvas->drawString("Upper-left",    10, 460, font, SkPaint());
        canvas->drawString("No Crop",       10, 540, font, SkPaint());

        canvas->drawString("Pre-Alloc Img", 80,  10, font, SkPaint());
        canvas->drawString("New Alloc Img", 160, 10, font, SkPaint());
        canvas->drawString( "GPU",          265, 10, font, SkPaint());

        canvas->translate(80, 20);

        // since we draw into this directly, we need to start fresh
        sk_bzero(fBuffer, fBufferSize);

        SkImageInfo info = SkImageInfo::MakeN32Premul(W, H);
        sk_sp<SkSurface> surf0(SkSurface::MakeRasterDirect(info, fBuffer, RB));
        sk_sp<SkSurface> surf1(SkSurface::MakeRaster(info));
        sk_sp<SkSurface> surf2(SkSurface::MakeRenderTarget(canvas->getGrContext(),
                                                           SkBudgeted::kNo, info));

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

#include "include/core/SkPictureRecorder.h"

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
                                    info.dimensions(), nullptr, nullptr, SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
}

static sk_sp<SkImage> make_codec(const SkImageInfo& info, GrContext*, void (*draw)(SkCanvas*)) {
    sk_sp<SkImage> image(make_raster(info, nullptr, draw));
    return SkImage::MakeFromEncoded(image->encodeToData());
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

DEF_SIMPLE_GPU_GM(new_texture_image, context, rtc, canvas, 280, 60) {
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

    static constexpr int kSize = 50;
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::MakeS32(kSize, kSize, kPremul_SkAlphaType));
    SkCanvas bmpCanvas(bmp);
    render_image(&bmpCanvas);

    std::function<sk_sp<SkImage>()> imageFactories[] = {
        // Create sw raster image.
        [bmp] {
            return SkImage::MakeFromBitmap(bmp);
        },
        // Create encoded image.
        [bmp] {
            auto src = SkEncodeBitmap(bmp, SkEncodedImageFormat::kPNG, 100);
            return SkImage::MakeFromEncoded(std::move(src));
        },
        // Create YUV encoded image.
        [bmp] {
            auto src = SkEncodeBitmap(bmp, SkEncodedImageFormat::kJPEG, 100);
            return SkImage::MakeFromEncoded(std::move(src));
        },
        // Create a picture image.
        [render_image] {
            SkPictureRecorder recorder;
            SkCanvas* canvas = recorder.beginRecording(SkIntToScalar(kSize), SkIntToScalar(kSize));
            render_image(canvas);
            sk_sp<SkColorSpace> srgbColorSpace = SkColorSpace::MakeSRGB();
            return SkImage::MakeFromPicture(recorder.finishRecordingAsPicture(),
                                            SkISize::Make(kSize, kSize), nullptr, nullptr,
                                            SkImage::BitDepth::kU8, srgbColorSpace);
        },
        // Create a texture image
        [context, render_image]() -> sk_sp<SkImage> {
            auto surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kYes,
                                                     SkImageInfo::MakeS32(kSize, kSize,
                                                                          kPremul_SkAlphaType)));
            if (!surface) {
                return nullptr;
            }
            render_image(surface->getCanvas());
            return surface->makeImageSnapshot();
        }
    };

    constexpr SkScalar kPad = 5.f;
    canvas->translate(kPad, kPad);
    for (auto factory : imageFactories) {
        auto image(factory());
        if (image) {
            sk_sp<SkImage> texImage(image->makeTextureImage(context,
                                                            canvas->imageInfo().colorSpace()));
            if (texImage) {
                canvas->drawImage(texImage, 0, 0);
            }
        }
        canvas->translate(kSize + kPad, 0);
    }
}

static void draw_pixmap(SkCanvas* canvas, const SkPixmap& pm, SkScalar x, SkScalar y) {
    canvas->drawImage(SkImage::MakeRasterCopy(pm), x, y, nullptr);
}

static void slam_ff(const SkPixmap& pm) {
    for (int y = 0; y < pm.height(); ++y) {
        for (int x = 0; x < pm.width(); ++x) {
            *pm.writable_addr32(x, y) = *pm.addr32(x, y) | SkPackARGB32(0xFF, 0, 0, 0);
        }
    }
}

DEF_SIMPLE_GM(scalepixels_unpremul, canvas, 1080, 280) {
    SkImageInfo info = SkImageInfo::MakeN32(16, 16, kUnpremul_SkAlphaType);
    SkAutoPixmapStorage pm;
    pm.alloc(info);
    for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 16; ++x) {
            *pm.writable_addr32(x, y) = SkPackARGB32NoCheck(0, (y << 4) | y, (x << 4) | x, 0xFF);
        }
    }
    SkAutoPixmapStorage pm2;
    pm2.alloc(SkImageInfo::MakeN32(256, 256, kUnpremul_SkAlphaType));

    const SkFilterQuality qualities[] = {
        kNone_SkFilterQuality, kLow_SkFilterQuality, kMedium_SkFilterQuality, kHigh_SkFilterQuality
    };

    for (auto fq : qualities) {
        pm.scalePixels(pm2, fq);
        slam_ff(pm2);
        draw_pixmap(canvas, pm2, 10, 10);
        canvas->translate(pm2.width() + 10.0f, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> make_lazy_image(SkSurface* surf) {
    surf->getCanvas()->drawCircle(100, 100, 100, SkPaint());
    sk_sp<SkData> data = surf->makeImageSnapshot()->encodeToData();
    if (!data) {
        return nullptr;
    }
    return SkImage::MakeFromEncoded(std::move(data));
}

#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
static sk_sp<SkImage> serial_deserial(SkImage* img) {
    SkBinaryWriteBuffer writer;
    writer.writeImage(img);
    size_t length = writer.bytesWritten();
    auto data = SkData::MakeUninitialized(length);
    writer.writeToMemory(data->writable_data());

    SkReadBuffer reader(data->data(), length);
    return reader.readImage();
}

DEF_SIMPLE_GM_CAN_FAIL(image_subset, canvas, errorMsg, 440, 220) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(200, 200, nullptr);
    auto        surf = ToolUtils::makeSurface(canvas, info, nullptr);
    auto img = make_lazy_image(surf.get());
    if (!img) {
        *errorMsg = "Failed to make lazy image.";
        return skiagm::DrawResult::kFail;
    }

    canvas->drawImage(img, 10, 10, nullptr);
    auto sub = img->makeSubset({100, 100, 200, 200});
    canvas->drawImage(sub, 220, 10);
    sub = serial_deserial(sub.get());
    canvas->drawImage(sub, 220+110, 10);
    return skiagm::DrawResult::kOk;
}
