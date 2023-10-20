/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/base/SkMalloc.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/image/SkImage_Base.h"
#include "tools/GpuToolUtils.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Surface.h"
#endif

#include <functional>
#include <utility>

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Image.h"
#endif

const SkSamplingOptions gSamplings[] = {
    SkSamplingOptions(SkFilterMode::kNearest),
    SkSamplingOptions(SkFilterMode::kLinear),
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
    SkSamplingOptions(SkCubicResampler::Mitchell()),
};

static void draw_contents(SkSurface* surface, SkColor fillC) {
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
    draw_contents(surf, SK_ColorRED);
    sk_sp<SkImage> imgR = surf->makeImageSnapshot();

    if (true) {
        sk_sp<SkImage> imgR2 = surf->makeImageSnapshot();
        SkASSERT(imgR == imgR2);
    }

    imgR = ToolUtils::MakeTextureImage(canvas, std::move(imgR));
    draw_contents(surf, SK_ColorGREEN);
    sk_sp<SkImage> imgG = ToolUtils::MakeTextureImage(canvas, surf->makeImageSnapshot());

    // since we've drawn after we snapped imgR, imgG will be a different obj unless the
    // gpu context has been abandoned (in which case they will both be null)
    SkASSERT(imgR != imgG || (!imgR && !imgG));

    draw_contents(surf, SK_ColorBLUE);

    SkSamplingOptions sampling;
    SkPaint paint;

    canvas->drawImage(imgR, 0, 0, sampling, usePaint ? &paint : nullptr);
    canvas->drawImage(imgG, 0, 80, sampling, usePaint ? &paint : nullptr);
    surf->draw(canvas, 0, 160, SkSamplingOptions(), usePaint ? &paint : nullptr);

    SkRect src1, src2, src3;
    src1.setIWH(surf->width(), surf->height());
    src2.setLTRB(SkIntToScalar(-surf->width() / 2), SkIntToScalar(-surf->height() / 2),
                 SkIntToScalar(surf->width()),       SkIntToScalar(surf->height()));
    src3.setIWH(surf->width() / 2, surf->height() / 2);

    SkRect dst1, dst2, dst3, dst4;
    dst1.setLTRB(0, 240, 65, 305);
    dst2.setLTRB(0, 320, 65, 385);
    dst3.setLTRB(0, 400, 65, 465);
    dst4.setLTRB(0, 480, 65, 545);

    canvas->drawImageRect(imgR, src1, dst1, sampling, usePaint ? &paint : nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
    canvas->drawImageRect(imgG, src2, dst2, sampling, usePaint ? &paint : nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
    canvas->drawImageRect(imgR, src3, dst3, sampling, usePaint ? &paint : nullptr,
                          SkCanvas::kStrict_SrcRectConstraint);
    canvas->drawImageRect(imgG, dst4, sampling, usePaint ? &paint : nullptr);
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
    SkString getName() const override { return SkString("image-surface"); }

    SkISize getISize() override { return SkISize::Make(960, 1200); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(2, 2);

        SkFont font(ToolUtils::DefaultPortableTypeface(), 8);

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
        sk_sp<SkSurface> surf0(SkSurfaces::WrapPixels(info, fBuffer, RB));
        sk_sp<SkSurface> surf1(SkSurfaces::Raster(info));
        sk_sp<SkSurface> surf2(
                SkSurfaces::RenderTarget(canvas->recordingContext(), skgpu::Budgeted::kNo, info));

        test_surface(canvas, surf0.get(), true);
        canvas->translate(80, 0);
        test_surface(canvas, surf1.get(), true);
        if (surf2) {
            canvas->translate(80, 0);
            test_surface(canvas, surf2.get(), true);
        }
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new ImageGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

static void draw_pixmap(SkCanvas* canvas, const SkPixmap& pmap) {
    SkBitmap bitmap;
    bitmap.installPixels(pmap);
    canvas->drawImage(bitmap.asImage(), 0, 0);
}

static void show_scaled_pixels(SkCanvas* canvas, SkImage* image) {
    SkAutoCanvasRestore acr(canvas, true);

    canvas->drawImage(image, 0, 0);
    canvas->translate(110, 10);

    const SkImageInfo info = SkImageInfo::MakeN32Premul(40, 40);
    SkAutoPixmapStorage storage;
    storage.alloc(info);

    const SkImage::CachingHint chints[] = {
        SkImage::kAllow_CachingHint, SkImage::kDisallow_CachingHint,
    };

    for (auto ch : chints) {
        canvas->save();
        for (auto s : gSamplings) {
            if (image->scalePixels(storage, s, ch)) {
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

static sk_sp<SkImage> make_raster(const SkImageInfo& info,
                                  GrRecordingContext*,
                                  void (*draw)(SkCanvas*)) {
    auto surface(SkSurfaces::Raster(info));
    draw(surface->getCanvas());
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_picture(const SkImageInfo& info,
                                   GrRecordingContext*,
                                   void (*draw)(SkCanvas*)) {
    SkPictureRecorder recorder;
    draw(recorder.beginRecording(SkRect::MakeIWH(info.width(), info.height())));
    return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                         info.dimensions(),
                                         nullptr,
                                         nullptr,
                                         SkImages::BitDepth::kU8,
                                         SkColorSpace::MakeSRGB());
}

static sk_sp<SkImage> make_codec(const SkImageInfo& info,
                                 GrRecordingContext*,
                                 void (*draw)(SkCanvas*)) {
    sk_sp<SkImage> image(make_raster(info, nullptr, draw));
    return SkImages::DeferredFromEncodedData(SkPngEncoder::Encode(nullptr, image.get(), {}));
}

static sk_sp<SkImage> make_gpu(const SkImageInfo& info,
                               GrRecordingContext* ctx,
                               void (*draw)(SkCanvas*)) {
    if (!ctx) {
        return nullptr;
    }

    auto surface(SkSurfaces::RenderTarget(ctx, skgpu::Budgeted::kNo, info));
    if (!surface) {
        return nullptr;
    }

    draw(surface->getCanvas());
    return surface->makeImageSnapshot();
}

typedef sk_sp<SkImage> (*ImageMakerProc)(const SkImageInfo&,
                                         GrRecordingContext*,
                                         void (*)(SkCanvas*));

class ScalePixelsGM : public skiagm::GM {
public:
    ScalePixelsGM() {}

protected:
    SkString getName() const override { return SkString("scale-pixels"); }

    SkISize getISize() override { return SkISize::Make(960, 1200); }

    void onDraw(SkCanvas* canvas) override {
        const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

        const ImageMakerProc procs[] = {
            make_codec, make_raster, make_picture, make_codec, make_gpu,
        };
        for (auto& proc : procs) {
            sk_sp<SkImage> image(proc(info, canvas->recordingContext(), draw_contents));
            if (image) {
                show_scaled_pixels(canvas, image.get());
            }
            canvas->translate(0, 120);
        }
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new ScalePixelsGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

DEF_SIMPLE_GM_CAN_FAIL(new_texture_image, canvas, errorMsg, 280, 115) {

    GrDirectContext* dContext = GrAsDirectContext(canvas->recordingContext());
    bool isGPU = SkToBool(dContext);

#if defined(SK_GRAPHITE)
    skgpu::graphite::Recorder* recorder = canvas->recorder();
    isGPU = isGPU || SkToBool(recorder);
#endif

    if (!isGPU) {
        *errorMsg = skiagm::GM::kErrorMsg_DrawSkippedGpuOnly;
        return skiagm::DrawResult::kSkip;
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

    static constexpr int kSize = 50;
    SkImageInfo ii = SkImageInfo::Make(kSize, kSize,
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                       SkColorSpace::MakeSRGB());
    SkBitmap bmp;
    bmp.allocPixels(ii);
    SkCanvas bmpCanvas(bmp);
    render_image(&bmpCanvas);

    std::function<sk_sp<SkImage>()> imageFactories[] = {
            // Create sw raster image.
            [&] { return bmp.asImage(); },
            // Create encoded image.
            [&] {
                SkDynamicMemoryWStream stream;
                SkASSERT_RELEASE(SkPngEncoder::Encode(&stream, bmp.pixmap(), {}));
                return SkImages::DeferredFromEncodedData(stream.detachAsData());
            },
            // Create YUV encoded image.
            [&] {
                SkDynamicMemoryWStream stream;
                SkASSERT_RELEASE(SkJpegEncoder::Encode(&stream, bmp.pixmap(), {}));
                return SkImages::DeferredFromEncodedData(stream.detachAsData());
            },
            // Create a picture image.
            [&] {
                SkPictureRecorder recorder;
                SkCanvas* canvas =
                        recorder.beginRecording(SkIntToScalar(kSize), SkIntToScalar(kSize));
                render_image(canvas);
                sk_sp<SkColorSpace> srgbColorSpace = SkColorSpace::MakeSRGB();
                return SkImages::DeferredFromPicture(recorder.finishRecordingAsPicture(),
                                                     SkISize::Make(kSize, kSize),
                                                     nullptr,
                                                     nullptr,
                                                     SkImages::BitDepth::kU8,
                                                     srgbColorSpace);
            },
            // Create a texture image
            [&]() -> sk_sp<SkImage> {
                sk_sp<SkSurface> surface;
                if (dContext) {
                    surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kYes, ii);
                } else {
#if defined(SK_GRAPHITE)
                    surface = SkSurfaces::RenderTarget(recorder, ii);
#endif
                }

                if (!surface) {
                    return nullptr;
                }
                render_image(surface->getCanvas());
                return surface->makeImageSnapshot();
            }};

    constexpr SkScalar kPad = 5.f;
    canvas->translate(kPad, kPad);
    for (const auto& factory : imageFactories) {
        sk_sp<SkImage> image(factory());
        if (image) {
            for (auto mm : { false, true }) {
                sk_sp<SkImage> texImage;
                if (dContext) {
                    texImage = SkImages::TextureFromImage(dContext,
                                                          image,
                                                          mm ? skgpu::Mipmapped::kYes
                                                             : skgpu::Mipmapped::kNo);
                } else {
#if defined(SK_GRAPHITE)
                    texImage = SkImages::TextureFromImage(recorder, image, {mm});
#endif
                }
                if (texImage) {
                    canvas->drawImage(texImage, 0, mm ? kSize + kPad : 0);
                }
            }
        }
        canvas->translate(kSize + kPad, 0);
    }

    return skiagm::DrawResult::kOk;
}

static void draw_pixmap(SkCanvas* canvas, const SkPixmap& pm, SkScalar x, SkScalar y) {
    canvas->drawImage(SkImages::RasterFromPixmapCopy(pm), x, y);
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

    for (auto s : gSamplings) {
        pm.scalePixels(pm2, s);
        slam_ff(pm2);
        draw_pixmap(canvas, pm2, 10, 10);
        canvas->translate(pm2.width() + 10.0f, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkImage> make_lazy_image() {
    sk_sp<SkPicture> picture;
    {
        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(SkRect::MakeIWH(200, 200));
        canvas->drawCircle(100, 100, 100, SkPaint());
        picture = recorder.finishRecordingAsPicture();
    }

    return SkImages::DeferredFromPicture(std::move(picture),
                                         {200, 200},
                                         /* matrix= */ nullptr,
                                         /* paint= */ nullptr,
                                         SkImages::BitDepth::kU8,
                                         SkColorSpace::MakeSRGB());
}

static sk_sp<SkImage> serial_deserial(SkImage* img) {
    if (!img) {
        return nullptr;
    }

    SkSerialProcs sProcs;
    sProcs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
        return SkPngEncoder::Encode(as_IB(img)->directContext(), img, SkPngEncoder::Options{});
    };
    SkBinaryWriteBuffer writer(sProcs);

    writer.writeImage(img);
    size_t length = writer.bytesWritten();
    auto data = SkData::MakeUninitialized(length);
    writer.writeToMemory(data->writable_data());

    SkReadBuffer reader(data->data(), length);
    return reader.readImage();
}

DEF_SIMPLE_GM_CAN_FAIL(image_subset, canvas, errorMsg, 440, 220) {
    auto img = make_lazy_image();
    if (!img) {
        *errorMsg = "Failed to make lazy image.";
        return skiagm::DrawResult::kFail;
    }

    GrDirectContext* dContext = GrAsDirectContext(canvas->recordingContext());
#if defined(SK_GRAPHITE)
    auto recorder = canvas->recorder();
#endif

    canvas->drawImage(img, 10, 10);

    sk_sp<SkImage> subset;

#if defined(SK_GRAPHITE)
    if (recorder) {
        subset = img->makeSubset(recorder, {100, 100, 200, 200}, {});
    } else
#endif
    {
        subset = img->makeSubset(dContext, {100, 100, 200, 200});
    }

    canvas->drawImage(subset, 220, 10);
    subset = serial_deserial(subset.get());
    canvas->drawImage(subset, 220+110, 10);
    return skiagm::DrawResult::kOk;
}
