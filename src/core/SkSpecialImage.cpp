/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "SkSpecialImage.h"
#include "SkBitmap.h"
#include "SkImage.h"
#include "SkBitmapCache.h"
#include "SkCanvas.h"
#include "SkImage_Base.h"
#include "SkSpecialSurface.h"
#include "SkSurfacePriv.h"
#include "SkPixelRef.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTexture.h"
#include "GrTextureParams.h"
#include "SkGr.h"
#include "SkGrPriv.h"
#endif

// Currently the raster imagefilters can only handle certain imageinfos. Call this to know if
// a given info is supported.
static bool valid_for_imagefilters(const SkImageInfo& info) {
    // no support for other swizzles/depths yet
    return info.colorType() == kN32_SkColorType;
}

///////////////////////////////////////////////////////////////////////////////
class SkSpecialImage_Base : public SkSpecialImage {
public:
    SkSpecialImage_Base(const SkIRect& subset, uint32_t uniqueID, const SkSurfaceProps* props)
        : INHERITED(subset, uniqueID, props) {
    }
    ~SkSpecialImage_Base() override { }

    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) const = 0;

    virtual bool onGetROPixels(SkBitmap*) const = 0;

    virtual GrTexture* onPeekTexture() const { return nullptr; }

    virtual SkColorSpace* onGetColorSpace() const = 0;

#if SK_SUPPORT_GPU
    virtual sk_sp<GrTexture> onAsTextureRef(GrContext* context) const = 0;
#endif

    virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;

    virtual sk_sp<SkSpecialSurface> onMakeSurface(const SkImageFilter::OutputProperties& outProps,
                                                  const SkISize& size, SkAlphaType at) const = 0;

    virtual sk_sp<SkImage> onMakeTightSubset(const SkIRect& subset) const = 0;

    virtual sk_sp<SkSurface> onMakeTightSurface(const SkImageFilter::OutputProperties& outProps,
                                                const SkISize& size, SkAlphaType at) const = 0;

private:
    typedef SkSpecialImage INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
static inline const SkSpecialImage_Base* as_SIB(const SkSpecialImage* image) {
    return static_cast<const SkSpecialImage_Base*>(image);
}

SkSpecialImage::SkSpecialImage(const SkIRect& subset,
                               uint32_t uniqueID,
                               const SkSurfaceProps* props)
    : fProps(SkSurfacePropsCopyOrDefault(props))
    , fSubset(subset)
    , fUniqueID(kNeedNewImageUniqueID_SpecialImage == uniqueID ? SkNextID::ImageID() : uniqueID) {
}

sk_sp<SkSpecialImage> SkSpecialImage::makeTextureImage(GrContext* context) {
#if SK_SUPPORT_GPU
    if (!context) {
        return nullptr;
    }
    if (GrTexture* peek = as_SIB(this)->onPeekTexture()) {
        return peek->getContext() == context ? sk_sp<SkSpecialImage>(SkRef(this)) : nullptr;
    }

    SkBitmap bmp;
    // At this point, we are definitely not texture-backed, so we must be raster or generator
    // backed. If we remove the special-wrapping-an-image subclass, we may be able to assert that
    // we are strictly raster-backed (i.e. generator images become raster when they are specialized)
    // in which case getROPixels could turn into peekPixels...
    if (!this->getROPixels(&bmp)) {
        return nullptr;
    }

    if (bmp.empty()) {
        return SkSpecialImage::MakeFromRaster(SkIRect::MakeEmpty(), bmp, &this->props());
    }

    sk_sp<GrTexture> resultTex(GrRefCachedBitmapTexture(context,
                                                        bmp,
                                                        GrTextureParams::ClampNoFilter(),
                                                        SkSourceGammaTreatment::kRespect));
    if (!resultTex) {
        return nullptr;
    }

    return SkSpecialImage::MakeFromGpu(SkIRect::MakeWH(resultTex->width(), resultTex->height()),
                                       this->uniqueID(),
                                       resultTex, sk_ref_sp(this->getColorSpace()), &this->props(),
                                       this->alphaType());
#else
    return nullptr;
#endif
}

void SkSpecialImage::draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const {
    return as_SIB(this)->onDraw(canvas, x, y, paint);
}

bool SkSpecialImage::getROPixels(SkBitmap* bm) const {
    return as_SIB(this)->onGetROPixels(bm);
}

bool SkSpecialImage::isTextureBacked() const {
#if SK_SUPPORT_GPU
    return as_SIB(this)->onPeekTexture() && as_SIB(this)->onPeekTexture()->getContext();
#else
    return false;
#endif
}

GrContext* SkSpecialImage::getContext() const {
#if SK_SUPPORT_GPU
    GrTexture* texture = as_SIB(this)->onPeekTexture();
    
    if (texture) {
        return texture->getContext();
    }
#endif
    return nullptr;
}

SkColorSpace* SkSpecialImage::getColorSpace() const {
    return as_SIB(this)->onGetColorSpace();
}

#if SK_SUPPORT_GPU
sk_sp<GrTexture> SkSpecialImage::asTextureRef(GrContext* context) const {
    return as_SIB(this)->onAsTextureRef(context);
}
#endif

sk_sp<SkSpecialSurface> SkSpecialImage::makeSurface(const SkImageFilter::OutputProperties& outProps,
                                                    const SkISize& size, SkAlphaType at) const {
    return as_SIB(this)->onMakeSurface(outProps, size, at);
}

sk_sp<SkSurface> SkSpecialImage::makeTightSurface(const SkImageFilter::OutputProperties& outProps,
                                                  const SkISize& size, SkAlphaType at) const {
    return as_SIB(this)->onMakeTightSurface(outProps, size, at);
}

sk_sp<SkSpecialImage> SkSpecialImage::makeSubset(const SkIRect& subset) const {
    return as_SIB(this)->onMakeSubset(subset);
}

sk_sp<SkImage> SkSpecialImage::makeTightSubset(const SkIRect& subset) const {
    return as_SIB(this)->onMakeTightSubset(subset);
}

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

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromImage(const SkIRect& subset,
                                                    sk_sp<SkImage> image,
                                                    const SkSurfaceProps* props) {
    SkASSERT(rect_fits(subset, image->width(), image->height()));

#if SK_SUPPORT_GPU
    if (GrTexture* texture = as_IB(image)->peekTexture()) {
        return MakeFromGpu(subset, image->uniqueID(), sk_ref_sp(texture),
                           sk_ref_sp(as_IB(image)->onImageInfo().colorSpace()), props);
    } else
#endif
    {
        SkBitmap bm;
        if (as_IB(image)->getROPixels(&bm)) {
            return MakeFromRaster(subset, bm, props);
        }
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

class SkSpecialImage_Raster : public SkSpecialImage_Base {
public:
    SkSpecialImage_Raster(const SkIRect& subset, const SkBitmap& bm, const SkSurfaceProps* props)
        : INHERITED(subset, bm.getGenerationID(), props)
        , fBitmap(bm)
    {
        SkASSERT(bm.pixelRef());

        // We have to lock now, while bm is still in scope, since it may have come from our
        // cache, which means we need to keep it locked until we (the special) are done, since
        // we cannot re-generate the cache entry (if bm came from a generator).
        fBitmap.lockPixels();
        SkASSERT(fBitmap.getPixels());
    }

    SkAlphaType alphaType() const override { return fBitmap.alphaType(); }

    size_t getSize() const override { return fBitmap.getSize(); }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        canvas->drawBitmapRect(fBitmap, this->subset(),
                               dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    bool onGetROPixels(SkBitmap* bm) const override {
        *bm = fBitmap;
        return true;
    }

    SkColorSpace* onGetColorSpace() const override {
        return fBitmap.colorSpace();
    }

#if SK_SUPPORT_GPU
    sk_sp<GrTexture> onAsTextureRef(GrContext* context) const override {
        if (context) {
            return sk_ref_sp(GrRefCachedBitmapTexture(context,
                                                      fBitmap,
                                                      GrTextureParams::ClampNoFilter(),
                                                      SkSourceGammaTreatment::kRespect));
        }

        return nullptr;
    }
#endif

// TODO: The raster implementations of image filters all currently assume that the pixels are
// legacy N32. Until they actually check the format and operate on sRGB or F16 data appropriately,
// we can't enable this. (They will continue to produce incorrect results, but less-so).
#define RASTER_IMAGE_FILTERS_SUPPORT_SRGB_AND_F16 0

    sk_sp<SkSpecialSurface> onMakeSurface(const SkImageFilter::OutputProperties& outProps,
                                          const SkISize& size, SkAlphaType at) const override {
#if RASTER_IMAGE_FILTERS_SUPPORT_SRGB_AND_F16
        SkColorSpace* colorSpace = outProps.colorSpace();
#else
        SkColorSpace* colorSpace = nullptr;
#endif
        SkColorType colorType = colorSpace && colorSpace->gammaIsLinear()
            ? kRGBA_F16_SkColorType : kN32_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), colorType, at,
                                             sk_ref_sp(colorSpace));
        return SkSpecialSurface::MakeRaster(info, nullptr);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        SkBitmap subsetBM;

        if (!fBitmap.extractSubset(&subsetBM, subset)) {
            return nullptr;
        }

        return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(subset.width(), subset.height()),
                                              subsetBM,
                                              &this->props());
    }

    sk_sp<SkImage> onMakeTightSubset(const SkIRect& subset) const override {
        SkBitmap subsetBM;

        if (!fBitmap.extractSubset(&subsetBM, subset)) {
            return nullptr;
        }

        return SkImage::MakeFromBitmap(subsetBM);
    }

    sk_sp<SkSurface> onMakeTightSurface(const SkImageFilter::OutputProperties& outProps,
                                        const SkISize& size, SkAlphaType at) const override {
#if RASTER_IMAGE_FILTERS_SUPPORT_SRGB_AND_F16
        SkColorSpace* colorSpace = outProps.colorSpace();
#else
        SkColorSpace* colorSpace = nullptr;
#endif
        SkColorType colorType = colorSpace && colorSpace->gammaIsLinear()
            ? kRGBA_F16_SkColorType : kN32_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), colorType, at,
                                             sk_ref_sp(colorSpace));
        return SkSurface::MakeRaster(info);
    }

private:
    SkBitmap fBitmap;

    typedef SkSpecialImage_Base INHERITED;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromRaster(const SkIRect& subset,
                                                     const SkBitmap& bm,
                                                     const SkSurfaceProps* props) {
    SkASSERT(rect_fits(subset, bm.width(), bm.height()));

    if (!bm.pixelRef()) {
        return nullptr;
    }

    const SkBitmap* srcBM = &bm;
    SkBitmap tmpStorage;
    // ImageFilters only handle N32 at the moment, so force our src to be that
    if (!valid_for_imagefilters(bm.info())) {
        if (!bm.copyTo(&tmpStorage, kN32_SkColorType)) {
            return nullptr;
        }
        srcBM = &tmpStorage;
    }
    return sk_make_sp<SkSpecialImage_Raster>(subset, *srcBM, props);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "GrTexture.h"
#include "SkImage_Gpu.h"

class SkSpecialImage_Gpu : public SkSpecialImage_Base {
public:
    SkSpecialImage_Gpu(const SkIRect& subset,
                       uint32_t uniqueID, sk_sp<GrTexture> tex, SkAlphaType at,
                       sk_sp<SkColorSpace> colorSpace, const SkSurfaceProps* props)
        : INHERITED(subset, uniqueID, props)
        , fTexture(std::move(tex))
        , fAlphaType(at)
        , fColorSpace(std::move(colorSpace))
        , fAddedRasterVersionToCache(false) {
    }

    ~SkSpecialImage_Gpu() override {
        if (fAddedRasterVersionToCache.load()) {
            SkNotifyBitmapGenIDIsStale(this->uniqueID());
        }
    }

    SkAlphaType alphaType() const override { return fAlphaType; }

    size_t getSize() const override { return fTexture->gpuMemorySize(); }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        auto img = sk_sp<SkImage>(new SkImage_Gpu(fTexture->width(), fTexture->height(),
                                                  this->uniqueID(), fAlphaType, fTexture.get(),
                                                  fColorSpace, SkBudgeted::kNo));

        canvas->drawImageRect(img, this->subset(),
                               dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    GrTexture* onPeekTexture() const override { return fTexture.get(); }

    sk_sp<GrTexture> onAsTextureRef(GrContext*) const override { return fTexture; }

    bool onGetROPixels(SkBitmap* dst) const override {
        if (SkBitmapCache::Find(this->uniqueID(), dst)) {
            SkASSERT(dst->getGenerationID() == this->uniqueID());
            SkASSERT(dst->isImmutable());
            SkASSERT(dst->getPixels());
            return true;
        }

        SkImageInfo info = SkImageInfo::MakeN32(this->width(), this->height(),
                                                this->alphaType(), fColorSpace);

        if (!dst->tryAllocPixels(info)) {
            return false;
        }

        if (!fTexture->readPixels(0, 0, dst->width(), dst->height(), kSkia8888_GrPixelConfig,
                                  dst->getPixels(), dst->rowBytes())) {
            return false;
        }

        dst->pixelRef()->setImmutableWithID(this->uniqueID());
        SkBitmapCache::Add(this->uniqueID(), *dst);
        fAddedRasterVersionToCache.store(true);
        return true;
    }

    SkColorSpace* onGetColorSpace() const override {
        return fColorSpace.get();
    }

    sk_sp<SkSpecialSurface> onMakeSurface(const SkImageFilter::OutputProperties& outProps,
                                          const SkISize& size, SkAlphaType at) const override {
        if (!fTexture->getContext()) {
            return nullptr;
        }

        SkColorSpace* colorSpace = outProps.colorSpace();
        return SkSpecialSurface::MakeRenderTarget(
            fTexture->getContext(), size.width(), size.height(),
            GrRenderableConfigForColorSpace(colorSpace), sk_ref_sp(colorSpace));
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImage::MakeFromGpu(subset,
                                           this->uniqueID(),
                                           fTexture,
                                           fColorSpace,
                                           &this->props(),
                                           fAlphaType);
    }

    sk_sp<SkImage> onMakeTightSubset(const SkIRect& subset) const override {
        if (0 == subset.fLeft && 0 == subset.fTop &&
            fTexture->width() == subset.width() &&
            fTexture->height() == subset.height()) {
            // The existing GrTexture is already tight so reuse it in the SkImage
            return sk_make_sp<SkImage_Gpu>(fTexture->width(), fTexture->height(),
                                           kNeedNewImageUniqueID,
                                           fAlphaType, fTexture.get(), fColorSpace,
                                           SkBudgeted::kYes);
        }

        GrContext* ctx = fTexture->getContext();
        GrSurfaceDesc desc = fTexture->desc();
        desc.fWidth = subset.width();
        desc.fHeight = subset.height();

        sk_sp<GrTexture> subTx(ctx->textureProvider()->createTexture(desc, SkBudgeted::kYes));
        if (!subTx) {
            return nullptr;
        }
        ctx->copySurface(subTx.get(), fTexture.get(), subset, SkIPoint::Make(0, 0));
        return sk_make_sp<SkImage_Gpu>(desc.fWidth, desc.fHeight, kNeedNewImageUniqueID,
                                       fAlphaType, subTx.get(), fColorSpace, SkBudgeted::kYes);
    }

    sk_sp<SkSurface> onMakeTightSurface(const SkImageFilter::OutputProperties& outProps,
                                        const SkISize& size, SkAlphaType at) const override {
        SkColorSpace* colorSpace = outProps.colorSpace();
        SkColorType colorType = colorSpace && colorSpace->gammaIsLinear()
            ? kRGBA_F16_SkColorType : kRGBA_8888_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), colorType, at,
                                             sk_ref_sp(colorSpace));
        return SkSurface::MakeRenderTarget(fTexture->getContext(), SkBudgeted::kYes, info);
    }

private:
    sk_sp<GrTexture>        fTexture;
    const SkAlphaType       fAlphaType;
    sk_sp<SkColorSpace>     fColorSpace;
    mutable SkAtomic<bool>  fAddedRasterVersionToCache;

    typedef SkSpecialImage_Base INHERITED;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromGpu(const SkIRect& subset,
                                                  uint32_t uniqueID,
                                                  sk_sp<GrTexture> tex,
                                                  sk_sp<SkColorSpace> colorSpace,
                                                  const SkSurfaceProps* props,
                                                  SkAlphaType at) {
    SkASSERT(rect_fits(subset, tex->width(), tex->height()));
    return sk_make_sp<SkSpecialImage_Gpu>(subset, uniqueID, std::move(tex), at,
                                          std::move(colorSpace), props);
}

#endif
