/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */
#include "SkSpecialImage.h"

#if SK_SUPPORT_GPU
#include "GrTexture.h"
#include "GrTextureParams.h"
#include "SkGr.h"
#endif

#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkSpecialSurface.h"

///////////////////////////////////////////////////////////////////////////////
class SkSpecialImage_Base : public SkSpecialImage {
public:
    SkSpecialImage_Base(SkImageFilter::Proxy* proxy, const SkIRect& subset, uint32_t uniqueID)
        : INHERITED(proxy, subset, uniqueID) {
    }
    virtual ~SkSpecialImage_Base() { }

    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) const = 0;

    virtual bool onPeekPixels(SkPixmap*) const { return false; }

    virtual GrTexture* onPeekTexture() const { return nullptr; }

    virtual bool testingOnlyOnGetROPixels(SkBitmap*) const = 0;

    // Delete this entry point ASAP (see skbug.com/4965)
    virtual bool getBitmapDeprecated(SkBitmap* result) const = 0;

    virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;

    virtual sk_sp<SkSpecialSurface> onMakeSurface(const SkImageInfo& info) const = 0;

    virtual sk_sp<SkImage> onMakeTightSubset(const SkIRect& subset) const = 0;

    virtual SkSurface* onMakeTightSurface(const SkImageInfo& info) const = 0;

private:
    typedef SkSpecialImage INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
static inline const SkSpecialImage_Base* as_SIB(const SkSpecialImage* image) {
    return static_cast<const SkSpecialImage_Base*>(image);
}

sk_sp<SkSpecialImage> SkSpecialImage::makeTextureImage(SkImageFilter::Proxy* proxy,
                                                       GrContext* context) {
#if SK_SUPPORT_GPU
    if (!context) {
        return nullptr;
    }
    if (GrTexture* peek = as_SIB(this)->peekTexture()) {
        return peek->getContext() == context ? sk_sp<SkSpecialImage>(SkRef(this)) : nullptr;
    }

    SkBitmap bmp;
    if (!this->internal_getBM(&bmp)) {
        return nullptr;
    }

    SkAutoTUnref<GrTexture> resultTex(
        GrRefCachedBitmapTexture(context, bmp, GrTextureParams::ClampNoFilter()));
    if (!resultTex) {
        return nullptr;
    }

    SkAlphaType at = this->isOpaque() ? kOpaque_SkAlphaType : kPremul_SkAlphaType;

    return SkSpecialImage::MakeFromGpu(proxy,
                                       SkIRect::MakeWH(resultTex->width(), resultTex->height()),
                                       this->uniqueID(),
                                       resultTex, at);
#else
    return nullptr;
#endif
}

void SkSpecialImage::draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const {
    return as_SIB(this)->onDraw(canvas, x, y, paint);
}

bool SkSpecialImage::peekPixels(SkPixmap* pixmap) const {
    return as_SIB(this)->onPeekPixels(pixmap);
}

GrTexture* SkSpecialImage::peekTexture() const {
    return as_SIB(this)->onPeekTexture();
}

bool SkSpecialImage::testingOnlyGetROPixels(SkBitmap* result) const {
    return as_SIB(this)->testingOnlyOnGetROPixels(result);
}

sk_sp<SkSpecialSurface> SkSpecialImage::makeSurface(const SkImageInfo& info) const {
    return as_SIB(this)->onMakeSurface(info);
}

sk_sp<SkSurface> SkSpecialImage::makeTightSurface(const SkImageInfo& info) const {
    sk_sp<SkSurface> tmp(as_SIB(this)->onMakeTightSurface(info));
    return tmp;
}

sk_sp<SkSpecialImage> SkSpecialImage::makeSubset(const SkIRect& subset) const {
    return as_SIB(this)->onMakeSubset(subset);
}

sk_sp<SkImage> SkSpecialImage::makeTightSubset(const SkIRect& subset) const {
    return as_SIB(this)->onMakeTightSubset(subset);
}

#if SK_SUPPORT_GPU
#include "SkGr.h"
#include "SkGrPixelRef.h"
#endif

sk_sp<SkSpecialImage> SkSpecialImage::internal_fromBM(SkImageFilter::Proxy* proxy,
                                                       const SkBitmap& src) {
    // Need to test offset case! (see skbug.com/4967)
    if (src.getTexture()) {
        return SkSpecialImage::MakeFromGpu(proxy,
                                           src.bounds(),
                                           src.getGenerationID(),
                                           src.getTexture());
    }

    return SkSpecialImage::MakeFromRaster(proxy, src.bounds(), src);
}

bool SkSpecialImage::internal_getBM(SkBitmap* result) {
    const SkSpecialImage_Base* ib = as_SIB(this);

    // TODO: need to test offset case! (see skbug.com/4967)
    return ib->getBitmapDeprecated(result);
}

SkImageFilter::Proxy* SkSpecialImage::internal_getProxy() const {
    return fProxy;
}

///////////////////////////////////////////////////////////////////////////////
#include "SkImage.h"
#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "SkGrPriv.h"
#endif

class SkSpecialImage_Image : public SkSpecialImage_Base {
public:
    SkSpecialImage_Image(SkImageFilter::Proxy* proxy,
                         const SkIRect& subset,
                         sk_sp<SkImage> image)
        : INHERITED(proxy, subset, image->uniqueID())
        , fImage(image) {
    }

    ~SkSpecialImage_Image() override { }
    
    bool isOpaque() const override { return fImage->isOpaque(); }

    size_t getSize() const override {
#if SK_SUPPORT_GPU
        if (GrTexture* texture = as_IB(fImage.get())->peekTexture()) {
            return texture->gpuMemorySize();
        } else
#endif
        {
            SkPixmap pm;
            if (fImage->peekPixels(&pm)) {
                return pm.height() * pm.rowBytes();
            }
        }
        return 0;
    }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y, this->subset().width(), this->subset().height());

        canvas->drawImageRect(fImage.get(), this->subset(),
                              dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    bool onPeekPixels(SkPixmap* pixmap) const override {
        return fImage->peekPixels(pixmap);
    }

    GrTexture* onPeekTexture() const override { return as_IB(fImage.get())->peekTexture(); }

    bool getBitmapDeprecated(SkBitmap* result) const override {
#if SK_SUPPORT_GPU
        if (GrTexture* texture = as_IB(fImage.get())->peekTexture()) {
            const SkImageInfo info = GrMakeInfoFromTexture(texture, 
                                                           fImage->width(), fImage->height(),
                                                           fImage->isOpaque());
            if (!result->setInfo(info)) {
                return false;
            }

            result->setPixelRef(new SkGrPixelRef(info, texture))->unref();
            return true;
        }
#endif

        return as_IB(fImage.get())->asBitmapForImageFilters(result);
    }

    bool testingOnlyOnGetROPixels(SkBitmap* result) const override {
        return fImage->asLegacyBitmap(result, SkImage::kRO_LegacyBitmapMode);
    }

    sk_sp<SkSpecialSurface> onMakeSurface(const SkImageInfo& info) const override {
#if SK_SUPPORT_GPU
        GrTexture* texture = as_IB(fImage.get())->peekTexture();
        if (texture) {
            GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(info, *texture->getContext()->caps());
            desc.fFlags = kRenderTarget_GrSurfaceFlag;

            return SkSpecialSurface::MakeRenderTarget(this->proxy(), texture->getContext(), desc);
        }
#endif
        return SkSpecialSurface::MakeRaster(this->proxy(), info, nullptr);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        sk_sp<SkImage> subsetImg(fImage->makeSubset(subset));
        if (!subsetImg) {
            return nullptr;
        }

        return SkSpecialImage::MakeFromImage(this->internal_getProxy(),
                                             SkIRect::MakeWH(subset.width(), subset.height()),
                                             subsetImg);
    }

    sk_sp<SkImage> onMakeTightSubset(const SkIRect& subset) const override {
        return fImage->makeSubset(subset);
    }

    SkSurface* onMakeTightSurface(const SkImageInfo& info) const override {
#if SK_SUPPORT_GPU
        GrTexture* texture = as_IB(fImage.get())->peekTexture();
        if (texture) {
            return SkSurface::NewRenderTarget(texture->getContext(), SkBudgeted::kYes, info, 0);
        }
#endif
        return SkSurface::NewRaster(info, nullptr);
    }

private:
    sk_sp<SkImage> fImage;

    typedef SkSpecialImage_Base INHERITED;
};

#ifdef SK_DEBUG
static bool rect_fits(const SkIRect& rect, int width, int height) {
    if (0 == width && 0 == height) {
        SkASSERT(0 == rect.fLeft && 0 == rect.fRight && 0 == rect.fTop && 0 == rect.fBottom);
        return true;
    }

    return rect.fLeft >= 0 && rect.fLeft < width && rect.fLeft < rect.fRight &&
           rect.fRight >= 0 && rect.fRight <= width &&
           rect.fTop >= 0 && rect.fTop < height && rect.fTop < rect.fBottom &&
           rect.fBottom >= 0 && rect.fBottom <= height;
}
#endif

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromImage(SkImageFilter::Proxy* proxy,
                                                    const SkIRect& subset,
                                                    sk_sp<SkImage> image) {
    SkASSERT(rect_fits(subset, image->width(), image->height()));

    return sk_make_sp<SkSpecialImage_Image>(proxy, subset, image);
}

///////////////////////////////////////////////////////////////////////////////
#include "SkBitmap.h"
#include "SkImageInfo.h"
#include "SkPixelRef.h"

class SkSpecialImage_Raster : public SkSpecialImage_Base {
public:
    SkSpecialImage_Raster(SkImageFilter::Proxy* proxy, const SkIRect& subset, const SkBitmap& bm)
        : INHERITED(proxy, subset, bm.getGenerationID())
        , fBitmap(bm) {
        if (bm.pixelRef() && bm.pixelRef()->isPreLocked()) {
            // we only preemptively lock if there is no chance of triggering something expensive
            // like a lazy decode or imagegenerator. PreLocked means it is flat pixels already.
            fBitmap.lockPixels();
        }
    }

    SkSpecialImage_Raster(SkImageFilter::Proxy* proxy,
                          const SkIRect& subset,
                          const SkPixmap& pixmap,
                          RasterReleaseProc releaseProc,
                          ReleaseContext context)
        : INHERITED(proxy, subset, kNeedNewImageUniqueID_SpecialImage) {
        fBitmap.installPixels(pixmap.info(), pixmap.writable_addr(),
                              pixmap.rowBytes(), pixmap.ctable(),
                              releaseProc, context);
    }

    ~SkSpecialImage_Raster() override { }

    bool isOpaque() const override { return fBitmap.isOpaque(); }

    size_t getSize() const override { return fBitmap.getSize(); }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        canvas->drawBitmapRect(fBitmap, this->subset(),
                               dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    bool onPeekPixels(SkPixmap* pixmap) const override {
        const SkImageInfo info = fBitmap.info();
        if ((kUnknown_SkColorType == info.colorType()) || !fBitmap.getPixels()) {
            return false;
        }

        return fBitmap.peekPixels(pixmap);
    }

    bool getBitmapDeprecated(SkBitmap* result) const override {
        *result = fBitmap;
        return true;
    }

    bool testingOnlyOnGetROPixels(SkBitmap* result) const override {
        *result = fBitmap;
        return true;
    }

    sk_sp<SkSpecialSurface> onMakeSurface(const SkImageInfo& info) const override {
        return SkSpecialSurface::MakeRaster(this->proxy(), info, nullptr);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        SkBitmap subsetBM;
        
        if (!fBitmap.extractSubset(&subsetBM, subset)) {
            return nullptr;
        }

        return SkSpecialImage::MakeFromRaster(this->internal_getProxy(),
                                              SkIRect::MakeWH(subset.width(), subset.height()),
                                              subsetBM);
    }

    sk_sp<SkImage> onMakeTightSubset(const SkIRect& subset) const override {
        SkBitmap subsetBM;

        if (!fBitmap.extractSubset(&subsetBM, subset)) {
            return nullptr;
        }

        return SkImage::MakeFromBitmap(subsetBM);
    }

    SkSurface* onMakeTightSurface(const SkImageInfo& info) const override {
        return SkSurface::NewRaster(info);
    }

private:
    SkBitmap fBitmap;

    typedef SkSpecialImage_Base INHERITED;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromRaster(SkImageFilter::Proxy* proxy,
                                                     const SkIRect& subset,
                                                     const SkBitmap& bm) {
    SkASSERT(nullptr == bm.getTexture());
    SkASSERT(rect_fits(subset, bm.width(), bm.height()));

    return sk_make_sp<SkSpecialImage_Raster>(proxy, subset, bm);
}

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromPixmap(SkImageFilter::Proxy* proxy,
                                                     const SkIRect& subset,
                                                     const SkPixmap& src,
                                                     RasterReleaseProc releaseProc,
                                                     ReleaseContext context) {
    if (!src.addr()) {
        return nullptr;
    }

    return sk_make_sp<SkSpecialImage_Raster>(proxy, subset, src, releaseProc, context);
}


#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "GrTexture.h"
#include "SkImage_Gpu.h"

class SkSpecialImage_Gpu : public SkSpecialImage_Base {
public:                                       
    SkSpecialImage_Gpu(SkImageFilter::Proxy* proxy, const SkIRect& subset,
                       uint32_t uniqueID, GrTexture* tex, SkAlphaType at)
        : INHERITED(proxy, subset, uniqueID)
        , fTexture(SkRef(tex))
        , fAlphaType(at) {
    }

    ~SkSpecialImage_Gpu() override { }

    bool isOpaque() const override {
        return GrPixelConfigIsOpaque(fTexture->config()) || fAlphaType == kOpaque_SkAlphaType;
    }

    size_t getSize() const override { return fTexture->gpuMemorySize(); }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        SkBitmap bm;

        GrWrapTextureInBitmap(fTexture,
                              fTexture->width(), fTexture->height(), this->isOpaque(), &bm);

        canvas->drawBitmapRect(bm, this->subset(),
                               dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    GrTexture* onPeekTexture() const override { return fTexture; }

    bool getBitmapDeprecated(SkBitmap* result) const override {
        const SkImageInfo info = GrMakeInfoFromTexture(fTexture, 
                                                       this->width(), this->height(),
                                                       this->isOpaque());
        if (!result->setInfo(info)) {
            return false;
        }

        const SkImageInfo prInfo = info.makeWH(fTexture->width(), fTexture->height());

        SkAutoTUnref<SkGrPixelRef> pixelRef(new SkGrPixelRef(prInfo, fTexture));
        result->setPixelRef(pixelRef, this->subset().fLeft, this->subset().fTop);
        return true;
    }

    bool testingOnlyOnGetROPixels(SkBitmap* result) const override {

        const SkImageInfo info = SkImageInfo::MakeN32(this->width(), 
                                                      this->height(),
                                                      this->isOpaque() ? kOpaque_SkAlphaType
                                                                       : kPremul_SkAlphaType);
        if (!result->tryAllocPixels(info)) {
            return false;
        }

        if (!fTexture->readPixels(0, 0, result->width(), result->height(), kSkia8888_GrPixelConfig,
                                  result->getPixels(), result->rowBytes())) {
            return false;
        }

        result->pixelRef()->setImmutable();
        return true;
    }

    sk_sp<SkSpecialSurface> onMakeSurface(const SkImageInfo& info) const override {
        GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(info, *fTexture->getContext()->caps());
        desc.fFlags = kRenderTarget_GrSurfaceFlag;

        return SkSpecialSurface::MakeRenderTarget(this->proxy(), fTexture->getContext(), desc);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImage::MakeFromGpu(this->internal_getProxy(),
                                           subset,
                                           this->uniqueID(),
                                           fTexture, 
                                           fAlphaType);
    }

    sk_sp<SkImage> onMakeTightSubset(const SkIRect& subset) const override {
        if (0 == subset.fLeft && 0 == subset.fTop &&
            fTexture->width() == subset.width() &&
            fTexture->height() == subset.height()) {
            // The existing GrTexture is already tight so reuse it in the SkImage
            return sk_make_sp<SkImage_Gpu>(fTexture->width(), fTexture->height(),
                                           kNeedNewImageUniqueID,
                                           fAlphaType, fTexture, SkBudgeted::kYes);
        }

        GrContext* ctx = fTexture->getContext();
        GrSurfaceDesc desc = fTexture->desc();
        desc.fWidth = subset.width();
        desc.fHeight = subset.height();

        SkAutoTUnref<GrTexture> subTx(ctx->textureProvider()->createTexture(desc,
                                                                            SkBudgeted::kYes));
        if (!subTx) {
            return nullptr;
        }
        ctx->copySurface(subTx, fTexture, subset, SkIPoint::Make(0, 0));
        return sk_make_sp<SkImage_Gpu>(desc.fWidth, desc.fHeight, kNeedNewImageUniqueID,
                                       fAlphaType, subTx, SkBudgeted::kYes);
    }

    SkSurface* onMakeTightSurface(const SkImageInfo& info) const override {
        return SkSurface::NewRenderTarget(fTexture->getContext(), SkBudgeted::kYes, info);
    }

private:
    SkAutoTUnref<GrTexture> fTexture;
    const SkAlphaType       fAlphaType;

    typedef SkSpecialImage_Base INHERITED;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromGpu(SkImageFilter::Proxy* proxy,
                                                  const SkIRect& subset, 
                                                  uint32_t uniqueID,
                                                  GrTexture* tex,
                                                  SkAlphaType at) {
    SkASSERT(rect_fits(subset, tex->width(), tex->height()));
    return sk_make_sp<SkSpecialImage_Gpu>(proxy, subset, uniqueID, tex, at);
}

#else

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromGpu(SkImageFilter::Proxy* proxy,
                                                  const SkIRect& subset,
                                                  uint32_t uniqueID,
                                                  GrTexture* tex,
                                                  SkAlphaType at) {
    return nullptr;
}

#endif
