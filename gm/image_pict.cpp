/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"

#include <memory>
#include <utility>

class GrRecordingContext;

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
        SkMatrix matrix = SkMatrix::Translate(-100, -100);
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
    using INHERITED = skiagm::GM;
};
DEF_GM( return new ImagePictGM; )

///////////////////////////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<SkImageGenerator> make_pic_generator(GrDirectContext*,
                                                            sk_sp<SkPicture> pic) {
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
static std::unique_ptr<SkImageGenerator> make_ras_generator(GrDirectContext*,
                                                            sk_sp<SkPicture> pic) {
    SkBitmap bm;
    bm.allocN32Pixels(100, 100);
    SkCanvas canvas(bm);
    canvas.clear(0);
    canvas.translate(-100, -100);
    canvas.drawPicture(pic);
    return std::make_unique<RasterGenerator>(bm);
}

class TextureGenerator : public SkImageGenerator {
public:
    TextureGenerator(GrRecordingContext* rContext, const SkImageInfo& info, sk_sp<SkPicture> pic)
            : SkImageGenerator(info)
            , fRContext(SkRef(rContext)) {

        sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(rContext, SkBudgeted::kYes, info, 0,
                                                             kTopLeft_GrSurfaceOrigin, nullptr));
        if (surface) {
            surface->getCanvas()->clear(0);
            surface->getCanvas()->translate(-100, -100);
            surface->getCanvas()->drawPicture(pic);
            sk_sp<SkImage> image(surface->makeImageSnapshot());
            std::tie(fView, std::ignore) = as_IB(image)->asView(rContext, GrMipmapped::kNo);
        }
    }
protected:
    GrSurfaceProxyView onGenerateTexture(GrRecordingContext* rContext,
                                         const SkImageInfo& info,
                                         const SkIPoint& origin,
                                         GrMipmapped mipMapped,
                                         GrImageTexGenPolicy policy) override {
        SkASSERT(rContext);
        SkASSERT(rContext->priv().matches(fRContext.get()));

        if (!fView) {
            return {};
        }

        if (origin.fX == 0 && origin.fY == 0 && info.dimensions() == fView.proxy()->dimensions() &&
            policy == GrImageTexGenPolicy::kDraw) {
            return fView;
        }
        auto budgeted = policy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted ? SkBudgeted::kNo
                                                                                : SkBudgeted::kYes;
        return GrSurfaceProxyView::Copy(
                fRContext.get(), fView, mipMapped,
                SkIRect::MakeXYWH(origin.x(), origin.y(), info.width(), info.height()),
                SkBackingFit::kExact, budgeted);
    }

private:
    sk_sp<GrRecordingContext> fRContext;
    GrSurfaceProxyView        fView;
};

static std::unique_ptr<SkImageGenerator> make_tex_generator(GrDirectContext* dContext,
                                                            sk_sp<SkPicture> pic) {
    if (!dContext) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);

    return std::make_unique<TextureGenerator>(dContext, info, pic);
}

class ImageCacheratorGM : public skiagm::GM {
    typedef std::unique_ptr<SkImageGenerator> (*FactoryFunc)(GrDirectContext*, sk_sp<SkPicture>);

    SkString         fName;
    FactoryFunc      fFactory;
    sk_sp<SkPicture> fPicture;
    sk_sp<SkImage>   fImage;
    sk_sp<SkImage>   fImageSubset;

public:
    ImageCacheratorGM(const char suffix[], FactoryFunc factory) : fFactory(factory) {
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

    bool makeCaches(GrDirectContext* dContext) {
        {
            auto gen = fFactory(dContext, fPicture);
            if (!gen) {
                return false;
            }
            fImage = SkImage::MakeFromGenerator(std::move(gen));
            if (!fImage) {
                return false;
            }
            SkASSERT(fImage->dimensions() == SkISize::Make(100, 100));
        }

        {
            const SkIRect subset = SkIRect::MakeLTRB(50, 50, 100, 100);

            // We re-create the generator here on the off chance that making a subset from
            // 'fImage' might perturb its state.
            auto gen = fFactory(dContext, fPicture);
            if (!gen) {
                return false;
            }
            fImageSubset = SkImage::MakeFromGenerator(std::move(gen))->makeSubset(subset, dContext);
            if (!fImageSubset) {
                return false;
            }
            SkASSERT(fImageSubset->dimensions() == SkISize::Make(50, 50));
        }

        return true;
    }

    static void draw_placeholder(SkCanvas* canvas, SkScalar x, SkScalar y, int w, int h) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        SkRect r = SkRect::MakeXYWH(x, y, SkIntToScalar(w), SkIntToScalar(h));
        canvas->drawRect(r, paint);
        canvas->drawLine(r.left(), r.top(), r.right(), r.bottom(), paint);
        canvas->drawLine(r.left(), r.bottom(), r.right(), r.top(), paint);
    }

    static void draw_as_bitmap(GrDirectContext* dContext, SkCanvas* canvas, SkImage* image,
                               SkScalar x, SkScalar y) {
        SkBitmap bitmap;
        if (as_IB(image)->getROPixels(dContext, &bitmap)) {
            canvas->drawImage(bitmap.asImage(), x, y);
        } else {
            draw_placeholder(canvas, x, y, image->width(), image->height());
        }
    }

    static void draw_as_tex(SkCanvas* canvas, SkImage* image, SkScalar x, SkScalar y) {
        // The gpu-backed images are drawn in this manner bc the generator backed images
        // aren't considered texture-backed
        auto [view, ct] = as_IB(image)->asView(canvas->recordingContext(), GrMipmapped::kNo);
        if (!view) {
            // show placeholder if we have no texture
            draw_placeholder(canvas, x, y, image->width(), image->height());
            return;
        }
        SkColorInfo colorInfo(GrColorTypeToSkColorType(ct),
                              image->alphaType(),
                              image->refColorSpace());
        // No API to draw a GrTexture directly, so we cheat and create a private image subclass
        sk_sp<SkImage> texImage(new SkImage_Gpu(sk_ref_sp(canvas->recordingContext()),
                                                image->uniqueID(),
                                                std::move(view),
                                                std::move(colorInfo)));
        canvas->drawImage(texImage.get(), x, y);
    }

    void drawRow(GrDirectContext* dContext, SkCanvas* canvas, float scale) const {
        canvas->scale(scale, scale);

        SkMatrix matrix = SkMatrix::Translate(-100, -100);
        canvas->drawPicture(fPicture, &matrix, nullptr);

        // Draw the tex first, so it doesn't hit a lucky cache from the raster version. This
        // way we also can force the generateTexture call.

        draw_as_tex(canvas, fImage.get(), 150, 0);
        draw_as_tex(canvas, fImageSubset.get(), 150+101, 0);

        draw_as_bitmap(dContext, canvas, fImage.get(), 310, 0);
        draw_as_bitmap(dContext, canvas, fImageSubset.get(), 310+101, 0);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (!this->makeCaches(dContext)) {
            errorMsg->printf("Could not create cached images");
            return DrawResult::kSkip;
        }

        canvas->save();
            canvas->translate(20, 20);
            this->drawRow(dContext, canvas, 1.0);
        canvas->restore();

        canvas->save();
            canvas->translate(20, 150);
            this->drawRow(dContext, canvas, 0.25f);
        canvas->restore();

        canvas->save();
            canvas->translate(20, 220);
            this->drawRow(dContext, canvas, 2.0f);
        canvas->restore();

        return DrawResult::kOk;
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM( return new ImageCacheratorGM("picture", make_pic_generator); )
DEF_GM( return new ImageCacheratorGM("raster", make_ras_generator); )
DEF_GM( return new ImageCacheratorGM("texture", make_tex_generator); )
