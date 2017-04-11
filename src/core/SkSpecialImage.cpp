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
#include "GrContextPriv.h"
#include "GrResourceProvider.h"
#include "GrSurfaceContext.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrSamplerParams.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
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

    virtual GrContext* onGetContext() const { return nullptr; }

    virtual SkColorSpace* onGetColorSpace() const = 0;

#if SK_SUPPORT_GPU
    virtual sk_sp<GrTextureProxy> onAsTextureProxyRef(GrContext* context) const = 0;
#endif

    virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;

    virtual sk_sp<SkSpecialSurface> onMakeSurface(const SkImageFilter::OutputProperties& outProps,
                                                  const SkISize& size, SkAlphaType at) const = 0;

    virtual sk_sp<SkImage> onAsImage(const SkIRect* subset) const = 0;

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
    if (GrContext* curContext = as_SIB(this)->onGetContext()) {
        return curContext == context ? sk_sp<SkSpecialImage>(SkRef(this)) : nullptr;
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

    // TODO: this is a tight copy of 'bmp' but it doesn't have to be (given SkSpecialImage's
    // semantics). Since this is cached though we would have to bake the fit into the cache key.
    sk_sp<GrTextureProxy> proxy = GrMakeCachedBitmapProxy(context->resourceProvider(), bmp);
    if (!proxy) {
        return nullptr;
    }

    const SkIRect rect = SkIRect::MakeWH(proxy->width(), proxy->height());

    // GrMakeCachedBitmapProxy has uploaded only the specified subset of 'bmp' so we need not
    // bother with SkBitmap::getSubset
    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               rect,
                                               this->uniqueID(),
                                               std::move(proxy),
                                               sk_ref_sp(this->getColorSpace()),
                                               &this->props(),
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
    return SkToBool(as_SIB(this)->onGetContext());
}

GrContext* SkSpecialImage::getContext() const {
    return as_SIB(this)->onGetContext();
}

SkColorSpace* SkSpecialImage::getColorSpace() const {
    return as_SIB(this)->onGetColorSpace();
}

#if SK_SUPPORT_GPU
sk_sp<GrTextureProxy> SkSpecialImage::asTextureProxyRef(GrContext* context) const {
    return as_SIB(this)->onAsTextureProxyRef(context);
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

sk_sp<SkImage> SkSpecialImage::asImage(const SkIRect* subset) const {
    return as_SIB(this)->onAsImage(subset);
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
                                                    SkColorSpace* dstColorSpace,
                                                    const SkSurfaceProps* props) {
    SkASSERT(rect_fits(subset, image->width(), image->height()));

#if SK_SUPPORT_GPU
    if (sk_sp<GrTextureProxy> proxy = as_IB(image)->asTextureProxyRef()) {
        GrContext* context = ((SkImage_Gpu*) as_IB(image))->context();

        return MakeDeferredFromGpu(context, subset, image->uniqueID(), std::move(proxy),
                                   as_IB(image)->onImageInfo().refColorSpace(), props);
    } else
#endif
    {
        SkBitmap bm;
        if (as_IB(image)->getROPixels(&bm, dstColorSpace)) {
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
    sk_sp<GrTextureProxy> onAsTextureProxyRef(GrContext* context) const override {
        if (context) {
            return GrMakeCachedBitmapProxy(context->resourceProvider(), fBitmap);
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

    sk_sp<SkImage> onAsImage(const SkIRect* subset) const override {
        if (subset) {
            SkBitmap subsetBM;

            if (!fBitmap.extractSubset(&subsetBM, *subset)) {
                return nullptr;
            }

            return SkImage::MakeFromBitmap(subsetBM);
        }

        return SkImage::MakeFromBitmap(fBitmap);
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

static sk_sp<SkImage> wrap_proxy_in_image(GrContext* context, sk_sp<GrTextureProxy> proxy,
                                          SkAlphaType alphaType, sk_sp<SkColorSpace> colorSpace) {
    return sk_make_sp<SkImage_Gpu>(context, kNeedNewImageUniqueID, alphaType,
                                   std::move(proxy), std::move(colorSpace), SkBudgeted::kYes);
}

class SkSpecialImage_Gpu : public SkSpecialImage_Base {
public:
    SkSpecialImage_Gpu(GrContext* context, const SkIRect& subset,
                       uint32_t uniqueID, sk_sp<GrTextureProxy> proxy, SkAlphaType at,
                       sk_sp<SkColorSpace> colorSpace, const SkSurfaceProps* props)
        : INHERITED(subset, uniqueID, props)
        , fContext(context)
        , fTextureProxy(std::move(proxy))
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

    size_t getSize() const override { return fTextureProxy->gpuMemorySize(); }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        // TODO: In this instance we know we're going to draw a sub-portion of the backing
        // texture into the canvas so it is okay to wrap it in an SkImage. This poses
        // some problems for full deferral however in that when the deferred SkImage_Gpu
        // instantiates itself it is going to have to either be okay with having a larger
        // than expected backing texture (unlikely) or the 'fit' of the SurfaceProxy needs
        // to be tightened (if it is deferred).
        sk_sp<SkImage> img = sk_sp<SkImage>(new SkImage_Gpu(canvas->getGrContext(),
                                                            this->uniqueID(), fAlphaType,
                                                            fTextureProxy,
                                                            fColorSpace, SkBudgeted::kNo));

        canvas->drawImageRect(img, this->subset(),
                              dst, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    GrContext* onGetContext() const override { return fContext; }

    sk_sp<GrTextureProxy> onAsTextureProxyRef(GrContext*) const override {
        return fTextureProxy;
    }

    bool onGetROPixels(SkBitmap* dst) const override {
        const auto desc = SkBitmapCacheDesc::Make(this->uniqueID(), this->width(), this->height());
        if (SkBitmapCache::Find(desc, dst)) {
            SkASSERT(dst->getGenerationID() == this->uniqueID());
            SkASSERT(dst->isImmutable());
            SkASSERT(dst->getPixels());
            return true;
        }

        SkPixmap pmap;
        SkImageInfo info = SkImageInfo::MakeN32(this->width(), this->height(),
                                                this->alphaType(), fColorSpace);
        auto rec = SkBitmapCache::Alloc(desc, info, &pmap);
        if (!rec) {
            return false;
        }

        sk_sp<GrSurfaceContext> sContext = fContext->contextPriv().makeWrappedSurfaceContext(
                                                                          fTextureProxy, nullptr);
        if (!sContext) {
            return false;
        }

        if (!sContext->readPixels(info, pmap.writable_addr(), pmap.rowBytes(), 0, 0)) {
            return false;
        }

        SkBitmapCache::Add(std::move(rec), dst);
        fAddedRasterVersionToCache.store(true);
        return true;
    }

    SkColorSpace* onGetColorSpace() const override {
        return fColorSpace.get();
    }

    sk_sp<SkSpecialSurface> onMakeSurface(const SkImageFilter::OutputProperties& outProps,
                                          const SkISize& size, SkAlphaType at) const override {
        if (!fContext) {
            return nullptr;
        }

        SkColorSpace* colorSpace = outProps.colorSpace();
        return SkSpecialSurface::MakeRenderTarget(
            fContext, size.width(), size.height(),
            GrRenderableConfigForColorSpace(colorSpace), sk_ref_sp(colorSpace));
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImage::MakeDeferredFromGpu(fContext,
                                                   subset,
                                                   this->uniqueID(),
                                                   fTextureProxy,
                                                   fColorSpace,
                                                   &this->props(),
                                                   fAlphaType);
    }

    // TODO: move all the logic here into the subset-flavor GrSurfaceProxy::copy?
    sk_sp<SkImage> onAsImage(const SkIRect* subset) const override {
        if (subset) {
            // TODO: if this becomes a bottle neck we could base this logic on what the size
            // will be when it is finally instantiated - but that is more fraught.
            if (GrResourceProvider::IsFunctionallyExact(fTextureProxy.get()) &&
                0 == subset->fLeft && 0 == subset->fTop &&
                fTextureProxy->width() == subset->width() &&
                fTextureProxy->height() == subset->height()) {
                // The existing GrTexture is already tight so reuse it in the SkImage
                return wrap_proxy_in_image(fContext, fTextureProxy, fAlphaType, fColorSpace);
            }

            sk_sp<GrTextureProxy> subsetProxy(GrSurfaceProxy::Copy(fContext, fTextureProxy.get(),
                                                                   *subset, SkBudgeted::kYes));
            if (!subsetProxy) {
                return nullptr;
            }

            SkASSERT(subsetProxy->priv().isExact());
            // MDB: this is acceptable (wrapping subsetProxy in an SkImage) bc Copy will
            // return a kExact-backed proxy
            return wrap_proxy_in_image(fContext, std::move(subsetProxy), fAlphaType, fColorSpace);
        }

        fTextureProxy->priv().exactify();

        return wrap_proxy_in_image(fContext, fTextureProxy, fAlphaType, fColorSpace);
    }

    sk_sp<SkSurface> onMakeTightSurface(const SkImageFilter::OutputProperties& outProps,
                                        const SkISize& size, SkAlphaType at) const override {
        SkColorSpace* colorSpace = outProps.colorSpace();
        SkColorType colorType = colorSpace && colorSpace->gammaIsLinear()
            ? kRGBA_F16_SkColorType : kRGBA_8888_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), colorType, at,
                                             sk_ref_sp(colorSpace));
        return SkSurface::MakeRenderTarget(fContext, SkBudgeted::kYes, info);
    }

private:
    GrContext*              fContext;
    sk_sp<GrTextureProxy>   fTextureProxy;
    const SkAlphaType       fAlphaType;
    sk_sp<SkColorSpace>     fColorSpace;
    mutable SkAtomic<bool>  fAddedRasterVersionToCache;

    typedef SkSpecialImage_Base INHERITED;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeDeferredFromGpu(GrContext* context,
                                                          const SkIRect& subset,
                                                          uint32_t uniqueID,
                                                          sk_sp<GrTextureProxy> proxy,
                                                          sk_sp<SkColorSpace> colorSpace,
                                                          const SkSurfaceProps* props,
                                                          SkAlphaType at) {
    SkASSERT(rect_fits(subset, proxy->width(), proxy->height()));
    return sk_make_sp<SkSpecialImage_Gpu>(context, subset, uniqueID, std::move(proxy), at,
                                          std::move(colorSpace), props);
}
#endif
