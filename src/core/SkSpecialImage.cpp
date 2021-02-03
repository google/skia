/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "src/core/SkSpecialImage.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkSurfacePriv.h"
#include "src/image/SkImage_Base.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/image/SkImage_Gpu.h"
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

    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkSamplingOptions&,
                        const SkPaint*) const = 0;

    virtual bool onGetROPixels(SkBitmap*) const = 0;

    virtual GrRecordingContext* onGetContext() const { return nullptr; }

    virtual SkColorSpace* onGetColorSpace() const = 0;

#if SK_SUPPORT_GPU
    virtual GrSurfaceProxyView onView(GrRecordingContext* context) const = 0;
#endif

    // This subset is relative to the backing store's coordinate frame, it has already been mapped
    // from the content rect by the non-virtual makeSubset().
    virtual sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const = 0;

    virtual sk_sp<SkSpecialSurface> onMakeSurface(
            SkColorType colorType, const SkColorSpace* colorSpace, const SkISize& size,
            SkAlphaType at, const SkSurfaceProps* = nullptr) const = 0;

    // This subset (when not null) is relative to the backing store's coordinate frame, it has
    // already been mapped from the content rect by the non-virtual asImage().
    virtual sk_sp<SkImage> onAsImage(const SkIRect* subset) const = 0;

    virtual sk_sp<SkSurface> onMakeTightSurface(
            SkColorType colorType, const SkColorSpace* colorSpace,
            const SkISize& size, SkAlphaType at) const = 0;

private:
    using INHERITED = SkSpecialImage;
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

void SkSpecialImage::draw(SkCanvas* canvas, SkScalar x, SkScalar y,
                          const SkSamplingOptions& sampling, const SkPaint* paint) const {
    return as_SIB(this)->onDraw(canvas, x, y, sampling, paint);
}

bool SkSpecialImage::getROPixels(SkBitmap* bm) const {
    return as_SIB(this)->onGetROPixels(bm);
}

bool SkSpecialImage::isTextureBacked() const {
    return SkToBool(as_SIB(this)->onGetContext());
}

GrRecordingContext* SkSpecialImage::getContext() const {
    return as_SIB(this)->onGetContext();
}

SkColorSpace* SkSpecialImage::getColorSpace() const {
    return as_SIB(this)->onGetColorSpace();
}

#if SK_SUPPORT_GPU
GrSurfaceProxyView SkSpecialImage::view(GrRecordingContext* context) const {
    return as_SIB(this)->onView(context);
}
#endif

sk_sp<SkSpecialSurface> SkSpecialImage::makeSurface(
        SkColorType colorType, const SkColorSpace* colorSpace, const SkISize& size,
        SkAlphaType at, const SkSurfaceProps* props) const {
    return as_SIB(this)->onMakeSurface(colorType, colorSpace, size, at, props);
}

sk_sp<SkSurface> SkSpecialImage::makeTightSurface(
        SkColorType colorType, const SkColorSpace* colorSpace, const SkISize& size,
        SkAlphaType at) const {
    return as_SIB(this)->onMakeTightSurface(colorType, colorSpace, size, at);
}

sk_sp<SkSpecialImage> SkSpecialImage::makeSubset(const SkIRect& subset) const {
    SkIRect absolute = subset.makeOffset(this->subset().topLeft());
    return as_SIB(this)->onMakeSubset(absolute);
}

sk_sp<SkImage> SkSpecialImage::asImage(const SkIRect* subset) const {
    if (subset) {
        SkIRect absolute = subset->makeOffset(this->subset().topLeft());
        return as_SIB(this)->onAsImage(&absolute);
    } else {
        return as_SIB(this)->onAsImage(nullptr);
    }
}

#if defined(SK_DEBUG) || SK_SUPPORT_GPU
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

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromImage(GrRecordingContext* rContext,
                                                    const SkIRect& subset,
                                                    sk_sp<SkImage> image,
                                                    const SkSurfaceProps* props) {
    SkASSERT(rect_fits(subset, image->width(), image->height()));

#if SK_SUPPORT_GPU
    if (rContext) {
        auto [view, ct] = as_IB(image)->asView(rContext, GrMipmapped::kNo);
        return MakeDeferredFromGpu(rContext,
                                   subset,
                                   image->uniqueID(),
                                   std::move(view),
                                   ct,
                                   image->refColorSpace(),
                                   props);
    }
#endif

    // raster to gpu is supported here, but gpu to raster is not
    SkBitmap bm;
    if (as_IB(image)->getROPixels(nullptr, &bm)) {
        return MakeFromRaster(subset, bm, props);
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
        SkASSERT(fBitmap.getPixels());
    }

    SkAlphaType alphaType() const override { return fBitmap.alphaType(); }

    SkColorType colorType() const override { return fBitmap.colorType(); }

    size_t getSize() const override { return fBitmap.computeByteSize(); }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkSamplingOptions& sampling,
                const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        canvas->drawImageRect(fBitmap.asImage(), SkRect::Make(this->subset()), dst,
                              sampling, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    bool onGetROPixels(SkBitmap* bm) const override {
        return fBitmap.extractSubset(bm, this->subset());
    }

    SkColorSpace* onGetColorSpace() const override {
        return fBitmap.colorSpace();
    }

#if SK_SUPPORT_GPU
    GrSurfaceProxyView onView(GrRecordingContext* context) const override {
        if (context) {
            return GrMakeCachedBitmapProxyView(context, fBitmap);
        }

        return {};
    }
#endif

    sk_sp<SkSpecialSurface> onMakeSurface(SkColorType colorType, const SkColorSpace* colorSpace,
                                          const SkISize& size, SkAlphaType at,
                                          const SkSurfaceProps* props) const override {
        // Ignore the requested color type, the raster backend currently only supports N32
        colorType = kN32_SkColorType;   // TODO: find ways to allow f16
        SkImageInfo info = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));
        return SkSpecialSurface::MakeRaster(info, props);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        // No need to extract subset, onGetROPixels handles that when needed
        return SkSpecialImage::MakeFromRaster(subset, fBitmap, &this->props());
    }

    sk_sp<SkImage> onAsImage(const SkIRect* subset) const override {
        if (subset) {
            SkBitmap subsetBM;

            if (!fBitmap.extractSubset(&subsetBM, *subset)) {
                return nullptr;
            }

            return subsetBM.asImage();
        }

        return fBitmap.asImage();
    }

sk_sp<SkSurface> onMakeTightSurface(SkColorType colorType, const SkColorSpace* colorSpace,
                                        const SkISize& size, SkAlphaType at) const override {
        // Ignore the requested color type, the raster backend currently only supports N32
        colorType = kN32_SkColorType;   // TODO: find ways to allow f16
        SkImageInfo info = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));
        return SkSurface::MakeRaster(info);
    }

private:
    SkBitmap fBitmap;

    using INHERITED = SkSpecialImage_Base;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromRaster(const SkIRect& subset,
                                                     const SkBitmap& bm,
                                                     const SkSurfaceProps* props) {
    SkASSERT(rect_fits(subset, bm.width(), bm.height()));

    if (!bm.pixelRef()) {
        return nullptr;
    }

    const SkBitmap* srcBM = &bm;
    SkBitmap tmp;
    // ImageFilters only handle N32 at the moment, so force our src to be that
    if (!valid_for_imagefilters(bm.info())) {
        if (!tmp.tryAllocPixels(bm.info().makeColorType(kN32_SkColorType)) ||
            !bm.readPixels(tmp.info(), tmp.getPixels(), tmp.rowBytes(), 0, 0))
        {
            return nullptr;
        }
        srcBM = &tmp;
    }
    return sk_make_sp<SkSpecialImage_Raster>(subset, *srcBM, props);
}

sk_sp<SkSpecialImage> SkSpecialImage::CopyFromRaster(const SkIRect& subset,
                                                     const SkBitmap& bm,
                                                     const SkSurfaceProps* props) {
    SkASSERT(rect_fits(subset, bm.width(), bm.height()));

    if (!bm.pixelRef()) {
        return nullptr;
    }

    SkBitmap tmp;
    SkImageInfo info = bm.info().makeDimensions(subset.size());
    // As in MakeFromRaster, must force src to N32 for ImageFilters
    if (!valid_for_imagefilters(bm.info())) {
        info = info.makeColorType(kN32_SkColorType);
    }
    if (!tmp.tryAllocPixels(info)) {
        return nullptr;
    }
    if (!bm.readPixels(tmp.info(), tmp.getPixels(), tmp.rowBytes(), subset.x(), subset.y())) {
        return nullptr;
    }

    // Since we're making a copy of the raster, the resulting special image is the exact size
    // of the requested subset of the original and no longer needs to be offset by subset's left
    // and top, since those were relative to the original's buffer.
    return sk_make_sp<SkSpecialImage_Raster>(
            SkIRect::MakeWH(subset.width(), subset.height()), tmp, props);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
static sk_sp<SkImage> wrap_proxy_in_image(GrRecordingContext* context, GrSurfaceProxyView view,
                                          SkColorType colorType, SkAlphaType alphaType,
                                          sk_sp<SkColorSpace> colorSpace) {
    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context),
                                   kNeedNewImageUniqueID, std::move(view), colorType, alphaType,
                                   std::move(colorSpace));
}

class SkSpecialImage_Gpu : public SkSpecialImage_Base {
public:
    SkSpecialImage_Gpu(GrRecordingContext* context, const SkIRect& subset,
                       uint32_t uniqueID, GrSurfaceProxyView view, GrColorType ct,
                       SkAlphaType at, sk_sp<SkColorSpace> colorSpace, const SkSurfaceProps* props)
        : INHERITED(subset, uniqueID, props)
        , fContext(context)
        , fView(std::move(view))
        , fColorType(ct)
        , fAlphaType(at)
        , fColorSpace(std::move(colorSpace)) {
    }

    SkAlphaType alphaType() const override { return fAlphaType; }

    SkColorType colorType() const override { return GrColorTypeToSkColorType(fColorType); }

    size_t getSize() const override {
        return fView.proxy()->gpuMemorySize();
    }

    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkSamplingOptions& sampling,
                const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y,
                                      this->subset().width(), this->subset().height());

        // TODO: In this instance we know we're going to draw a sub-portion of the backing
        // texture into the canvas so it is okay to wrap it in an SkImage. This poses
        // some problems for full deferral however in that when the deferred SkImage_Gpu
        // instantiates itself it is going to have to either be okay with having a larger
        // than expected backing texture (unlikely) or the 'fit' of the SurfaceProxy needs
        // to be tightened (if it is deferred).
        sk_sp<SkImage> img =
                sk_sp<SkImage>(new SkImage_Gpu(sk_ref_sp(canvas->recordingContext()),
                                               this->uniqueID(), fView, this->colorType(),
                                               fAlphaType, fColorSpace));

        canvas->drawImageRect(img, SkRect::Make(this->subset()), dst,
                              sampling, paint, SkCanvas::kStrict_SrcRectConstraint);
    }

    GrRecordingContext* onGetContext() const override { return fContext; }

    GrSurfaceProxyView onView(GrRecordingContext* context) const override { return fView; }

    bool onGetROPixels(SkBitmap* dst) const override {
        // This should never be called: All GPU image filters are implemented entirely on the GPU,
        // so we never perform read-back.
        SkASSERT(false);
        return false;
    }

    SkColorSpace* onGetColorSpace() const override {
        return fColorSpace.get();
    }

    sk_sp<SkSpecialSurface> onMakeSurface(SkColorType colorType, const SkColorSpace* colorSpace,
                                          const SkISize& size, SkAlphaType at,
                                          const SkSurfaceProps* props) const override {
        if (!fContext) {
            return nullptr;
        }

        return SkSpecialSurface::MakeRenderTarget(fContext, size.width(), size.height(),
                                                  SkColorTypeToGrColorType(colorType),
                                                  sk_ref_sp(colorSpace), props);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImage::MakeDeferredFromGpu(fContext,
                                                   subset,
                                                   this->uniqueID(),
                                                   fView,
                                                   fColorType,
                                                   fColorSpace,
                                                   &this->props(),
                                                   fAlphaType);
    }

    sk_sp<SkImage> onAsImage(const SkIRect* subset) const override {
        GrSurfaceProxy* proxy = fView.proxy();
        if (subset) {
            if (proxy->isFunctionallyExact() && *subset == SkIRect::MakeSize(proxy->dimensions())) {
                proxy->priv().exactify(false);
                // The existing GrTexture is already tight so reuse it in the SkImage
                return wrap_proxy_in_image(fContext, fView, this->colorType(), fAlphaType,
                                           fColorSpace);
            }

            auto subsetView = GrSurfaceProxyView::Copy(fContext, fView, GrMipmapped::kNo, *subset,
                                                       SkBackingFit::kExact, SkBudgeted::kYes);
            if (!subsetView) {
                return nullptr;
            }
            SkASSERT(subsetView.asTextureProxy());
            SkASSERT(subsetView.proxy()->priv().isExact());

            // MDB: this is acceptable (wrapping subsetProxy in an SkImage) bc Copy will
            // return a kExact-backed proxy
            return wrap_proxy_in_image(fContext, std::move(subsetView), this->colorType(),
                                       fAlphaType, fColorSpace);
        }

        proxy->priv().exactify(true);

        return wrap_proxy_in_image(fContext, fView, this->colorType(), fAlphaType, fColorSpace);
    }

    sk_sp<SkSurface> onMakeTightSurface(SkColorType colorType, const SkColorSpace* colorSpace,
                                        const SkISize& size, SkAlphaType at) const override {
        // TODO (michaelludwig): Why does this ignore colorType but onMakeSurface doesn't ignore it?
        //    Once makeTightSurface() goes away, should this type overriding behavior be moved into
        //    onMakeSurface() or is this unnecessary?
        colorType = colorSpace && colorSpace->gammaIsLinear()
            ? kRGBA_F16_SkColorType : kRGBA_8888_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));
        return SkSurface::MakeRenderTarget(fContext, SkBudgeted::kYes, info);
    }

private:
    GrRecordingContext*       fContext;
    GrSurfaceProxyView        fView;
    const GrColorType         fColorType;
    const SkAlphaType         fAlphaType;
    sk_sp<SkColorSpace>       fColorSpace;

    using INHERITED = SkSpecialImage_Base;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeDeferredFromGpu(GrRecordingContext* context,
                                                          const SkIRect& subset,
                                                          uint32_t uniqueID,
                                                          GrSurfaceProxyView view,
                                                          GrColorType colorType,
                                                          sk_sp<SkColorSpace> colorSpace,
                                                          const SkSurfaceProps* props,
                                                          SkAlphaType at) {
    if (!context || context->abandoned() || !view.asTextureProxy()) {
        return nullptr;
    }
    SkASSERT_RELEASE(rect_fits(subset, view.proxy()->width(), view.proxy()->height()));
    return sk_make_sp<SkSpecialImage_Gpu>(context, subset, uniqueID, std::move(view), colorType,
                                          at, std::move(colorSpace), props);
}
#endif
