/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkImageGenerator.h"
#include "SkImage_Base.h"
#include "SkMakeUnique.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrSurfaceContext.h"
#include "GrTextureProxy.h"
#include "../src/image/SkImage_Gpu.h"

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

/*
 *  Exercise drawing pictures inside an image, showing that the image version is pixelated
 *  (correctly) when it is inside an image.
 */
class ImagePictGM : public skiagm::GM {
    sk_sp<SkPicture> fPicture;
    sk_sp<SkImage>   fImage0;
    sk_sp<SkImage>   fImage1;
public:
    ImagePictGM() {}

protected:
    SkString onShortName() override {
        return SkString("image-picture");
    }

    SkISize onISize() override {
        return SkISize::Make(850, 450);
    }

    void onOnceBeforeDraw() override {
        const SkRect bounds = SkRect::MakeXYWH(100, 100, 100, 100);
        SkPictureRecorder recorder;
        draw_something(recorder.beginRecording(bounds), bounds);
        fPicture = recorder.finishRecordingAsPicture();

        // extract enough just for the oval.
        const SkISize size = SkISize::Make(100, 100);
        auto srgbColorSpace = SkColorSpace::MakeSRGB();

        SkMatrix matrix;
        matrix.setTranslate(-100, -100);
        fImage0 = SkImage::MakeFromPicture(fPicture, size, &matrix, nullptr,
                                           SkImage::BitDepth::kU8, srgbColorSpace);
        matrix.postTranslate(-50, -50);
        matrix.postRotate(45);
        matrix.postTranslate(50, 50);
        fImage1 = SkImage::MakeFromPicture(fPicture, size, &matrix, nullptr,
                                           SkImage::BitDepth::kU8, srgbColorSpace);
    }

    void drawSet(SkCanvas* canvas) const {
        SkMatrix matrix = SkMatrix::MakeTrans(-100, -100);
        canvas->drawPicture(fPicture, &matrix, nullptr);
        canvas->drawImage(fImage0.get(), 150, 0);
        canvas->drawImage(fImage1.get(), 300, 0);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20, 20);

        this->drawSet(canvas);

        canvas->save();
        canvas->translate(0, 130);
        canvas->scale(0.25f, 0.25f);
        this->drawSet(canvas);
        canvas->restore();

        canvas->save();
        canvas->translate(0, 200);
        canvas->scale(2, 2);
        this->drawSet(canvas);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ImagePictGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<SkImageGenerator> make_pic_generator(GrContext*, sk_sp<SkPicture> pic) {
    SkMatrix matrix;
    matrix.setTranslate(-100, -100);
    return SkImageGenerator::MakeFromPicture({ 100, 100 }, std::move(pic), &matrix, nullptr,
                                            SkImage::BitDepth::kU8,
                                            SkColorSpace::MakeSRGB());
}

class RasterGenerator : public SkImageGenerator {
public:
    RasterGenerator(const SkBitmap& bm) : SkImageGenerator(bm.info()), fBM(bm)
    {}

protected:
    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                     const Options&) override {
        SkASSERT(fBM.width() == info.width());
        SkASSERT(fBM.height() == info.height());
        return fBM.readPixels(info, pixels, rowBytes, 0, 0);
    }
private:
    SkBitmap fBM;
};
static std::unique_ptr<SkImageGenerator> make_ras_generator(GrContext*, sk_sp<SkPicture> pic) {
    SkBitmap bm;
    bm.allocN32Pixels(100, 100);
    SkCanvas canvas(bm);
    canvas.clear(0);
    canvas.translate(-100, -100);
    canvas.drawPicture(pic);
    return skstd::make_unique<RasterGenerator>(bm);
}

class EmptyGenerator : public SkImageGenerator {
public:
    EmptyGenerator(const SkImageInfo& info) : SkImageGenerator(info) {}
};

class TextureGenerator : public SkImageGenerator {
public:
    TextureGenerator(GrContext* ctx, const SkImageInfo& info, sk_sp<SkPicture> pic)
        : SkImageGenerator(info)
        , fCtx(SkRef(ctx)) {

        sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctx, SkBudgeted::kYes, info, 0,
                                                             kTopLeft_GrSurfaceOrigin, nullptr));
        if (surface) {
            surface->getCanvas()->clear(0);
            surface->getCanvas()->translate(-100, -100);
            surface->getCanvas()->drawPicture(pic);
            sk_sp<SkImage> image(surface->makeImageSnapshot());
            fProxy = as_IB(image)->asTextureProxyRef(fCtx.get());
        }
    }
protected:
    sk_sp<GrTextureProxy> onGenerateTexture(GrRecordingContext* ctx, const SkImageInfo& info,
                                            const SkIPoint& origin,
                                            bool willBeMipped) override {
        SkASSERT(ctx);
        SkASSERT(ctx == fCtx.get());

        if (!fProxy) {
            return nullptr;
        }

        if (origin.fX == 0 && origin.fY == 0 &&
            info.width() == fProxy->width() && info.height() == fProxy->height()) {
            return fProxy;
        }

        // need to copy the subset into a new texture
        GrSurfaceDesc desc;
        desc.fWidth = info.width();
        desc.fHeight = info.height();
        desc.fConfig = fProxy->config();

        GrMipMapped mipMapped = willBeMipped ? GrMipMapped::kYes : GrMipMapped::kNo;

        sk_sp<GrSurfaceContext> dstContext(fCtx->priv().makeDeferredSurfaceContext(
                fProxy->backendFormat(), desc, fProxy->origin(), mipMapped, SkBackingFit::kExact,
                SkBudgeted::kYes));
        if (!dstContext) {
            return nullptr;
        }

        if (!dstContext->copy(
                            fProxy.get(),
                            SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height()),
                            SkIPoint::Make(0, 0))) {
            return nullptr;
        }

        return dstContext->asTextureProxyRef();
    }

private:
    sk_sp<GrContext>      fCtx;
    sk_sp<GrTextureProxy> fProxy;
};

static std::unique_ptr<SkImageGenerator> make_tex_generator(GrContext* ctx, sk_sp<SkPicture> pic) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

    if (!ctx) {
        return skstd::make_unique<EmptyGenerator>(info);
    }
    return skstd::make_unique<TextureGenerator>(ctx, info, pic);
}

class ImageCacheratorGM : public skiagm::GM {
    SkString                         fName;
    std::unique_ptr<SkImageGenerator> (*fFactory)(GrContext*, sk_sp<SkPicture>);
    sk_sp<SkPicture>                 fPicture;
    sk_sp<SkImage>                   fImage;
    sk_sp<SkImage>                   fImageSubset;

public:
    ImageCacheratorGM(const char suffix[],
                      std::unique_ptr<SkImageGenerator> (*factory)(GrContext*, sk_sp<SkPicture>))
        : fFactory(factory)
    {
        fName.printf("image-cacherator-from-%s", suffix);
    }

protected:
    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(960, 450);
    }

    void onOnceBeforeDraw() override {
        const SkRect bounds = SkRect::MakeXYWH(100, 100, 100, 100);
        SkPictureRecorder recorder;
        draw_something(recorder.beginRecording(bounds), bounds);
        fPicture = recorder.finishRecordingAsPicture();
    }

    void makeCaches(GrContext* ctx) {
        auto gen = fFactory(ctx, fPicture);
        fImage = SkImage::MakeFromGenerator(std::move(gen));

        const SkIRect subset = SkIRect::MakeLTRB(50, 50, 100, 100);

        gen = fFactory(ctx, fPicture);
        fImageSubset = SkImage::MakeFromGenerator(std::move(gen), &subset);

        SkASSERT(fImage->dimensions() == SkISize::Make(100, 100));
        SkASSERT(fImageSubset->dimensions() == SkISize::Make(50, 50));
    }

    static void draw_as_bitmap(SkCanvas* canvas, SkImage* image, SkScalar x, SkScalar y) {
        SkBitmap bitmap;
        as_IB(image)->getROPixels(&bitmap);
        canvas->drawBitmap(bitmap, x, y);
    }

    static void draw_as_tex(SkCanvas* canvas, SkImage* image, SkScalar x, SkScalar y) {
        sk_sp<GrTextureProxy> proxy(as_IB(image)->asTextureProxyRef(
                canvas->getGrContext(), GrSamplerState::ClampBilerp(), nullptr));
        if (!proxy) {
            // show placeholder if we have no texture
            SkPaint paint;
            paint.setStyle(SkPaint::kStroke_Style);
            SkRect r = SkRect::MakeXYWH(x, y, SkIntToScalar(image->width()),
                                        SkIntToScalar(image->width()));
            canvas->drawRect(r, paint);
            canvas->drawLine(r.left(), r.top(), r.right(), r.bottom(), paint);
            canvas->drawLine(r.left(), r.bottom(), r.right(), r.top(), paint);
            return;
        }

        // No API to draw a GrTexture directly, so we cheat and create a private image subclass
        sk_sp<SkImage> texImage(new SkImage_Gpu(sk_ref_sp(canvas->getGrContext()),
                                                image->uniqueID(), kPremul_SkAlphaType,
                                                std::move(proxy), image->refColorSpace()));
        canvas->drawImage(texImage.get(), x, y);
    }

    void drawSet(SkCanvas* canvas) const {
        SkMatrix matrix = SkMatrix::MakeTrans(-100, -100);
        canvas->drawPicture(fPicture, &matrix, nullptr);

        // Draw the tex first, so it doesn't hit a lucky cache from the raster version. This
        // way we also can force the generateTexture call.

        draw_as_tex(canvas, fImage.get(), 310, 0);
        draw_as_tex(canvas, fImageSubset.get(), 310+101, 0);

        draw_as_bitmap(canvas, fImage.get(), 150, 0);
        draw_as_bitmap(canvas, fImageSubset.get(), 150+101, 0);
    }

    void onDraw(SkCanvas* canvas) override {
        this->makeCaches(canvas->getGrContext());

        canvas->translate(20, 20);

        this->drawSet(canvas);

        canvas->save();
        canvas->translate(0, 130);
        canvas->scale(0.25f, 0.25f);
        this->drawSet(canvas);
        canvas->restore();

        canvas->save();
        canvas->translate(0, 200);
        canvas->scale(2, 2);
        this->drawSet(canvas);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ImageCacheratorGM("picture", make_pic_generator); )
DEF_GM( return new ImageCacheratorGM("raster", make_ras_generator); )
DEF_GM( return new ImageCacheratorGM("texture", make_tex_generator); )
