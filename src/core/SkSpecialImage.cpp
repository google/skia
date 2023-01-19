/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "src/core/SkSpecialImage.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "src/core/SkSpecialSurface.h"
#include "src/core/SkSurfacePriv.h"
#include "src/image/SkImage_Base.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/image/SkImage_Gpu.h"
#include "src/shaders/SkImageShader.h"
#endif

// Currently, the raster imagefilters can only handle certain imageinfos. Call this to know if
// a given info is supported.
static bool valid_for_imagefilters(const SkImageInfo& info) {
    // no support for other swizzles/depths yet
    return info.colorType() == kN32_SkColorType;
}

SkSpecialImage::SkSpecialImage(const SkIRect& subset,
                               uint32_t uniqueID,
                               const SkColorInfo& colorInfo,
                               const SkSurfaceProps& props)
    : fSubset(subset)
    , fUniqueID(kNeedNewImageUniqueID_SpecialImage == uniqueID ? SkNextID::ImageID() : uniqueID)
    , fColorInfo(colorInfo)
    , fProps(props) {
}

sk_sp<SkSpecialSurface> SkSpecialImage::makeSurface(SkColorType colorType,
                                                    const SkColorSpace* colorSpace,
                                                    const SkISize& size,
                                                    SkAlphaType at,
                                                    const SkSurfaceProps& props) const {
    return this->onMakeSurface(colorType, colorSpace, size, at, props);
}

sk_sp<SkSurface> SkSpecialImage::makeTightSurface(SkColorType colorType,
                                                  const SkColorSpace* colorSpace,
                                                  const SkISize& size,
                                                  SkAlphaType at) const {
    return this->onMakeTightSurface(colorType, colorSpace, size, at);
}

sk_sp<SkImage> SkSpecialImage::asImage(const SkIRect* subset) const {
    if (subset) {
        SkIRect absolute = subset->makeOffset(this->subset().topLeft());
        return this->onAsImage(&absolute);
    } else {
        return this->onAsImage(nullptr);
    }
}

sk_sp<SkShader> SkSpecialImage::asShader(SkTileMode tileMode,
                                         const SkSamplingOptions& sampling,
                                         const SkMatrix& lm) const {
    return this->onAsShader(tileMode, sampling, lm);
}

sk_sp<SkShader> SkSpecialImage::asShader(const SkSamplingOptions& sampling) const {
    return this->asShader(sampling, SkMatrix::I());
}

sk_sp<SkShader> SkSpecialImage::asShader(const SkSamplingOptions& sampling,
                                         const SkMatrix& lm) const {
    return this->asShader(SkTileMode::kClamp, sampling, lm);
}

#if SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/TextureProxyView.h"

bool SkSpecialImage::isGraphiteBacked() const {
    return SkToBool(this->textureProxyView());
}

skgpu::graphite::TextureProxyView SkSpecialImage::textureProxyView() const {
    return this->onTextureProxyView();
}

skgpu::graphite::TextureProxyView SkSpecialImage::onTextureProxyView() const {
    // To get here we would need to be trying to retrieve a Graphite-backed resource from
    // either a raster or Ganesh-backed special image. That should never happen.
    // TODO: re-enable this assert. Right now, since image filters can fallback to raster
    // in Graphite, we can get here.
    //SkASSERT(false);
    return {};
}
#endif

#ifdef SK_DEBUG
bool SkSpecialImage::RectFits(const SkIRect& rect, int width, int height) {
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
                                                    const SkSurfaceProps& props) {
    SkASSERT(RectFits(subset, image->width(), image->height()));

#if SK_SUPPORT_GPU
    if (rContext) {
        auto [view, ct] = as_IB(image)->asView(rContext, GrMipmapped::kNo);
        return MakeDeferredFromGpu(rContext,
                                   subset,
                                   image->uniqueID(),
                                   std::move(view),
                                   { ct, image->alphaType(), image->refColorSpace() },
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

class SkSpecialImage_Raster final : public SkSpecialImage {
public:
    SkSpecialImage_Raster(const SkIRect& subset, const SkBitmap& bm, const SkSurfaceProps& props)
            : SkSpecialImage(subset, bm.getGenerationID(), bm.info().colorInfo(), props)
            , fBitmap(bm) {
        SkASSERT(bm.pixelRef());
        SkASSERT(fBitmap.getPixels());
    }

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

#if SK_SUPPORT_GPU
    GrSurfaceProxyView onView(GrRecordingContext* context) const override {
        if (context) {
            return std::get<0>(GrMakeCachedBitmapProxyView(
                    context, fBitmap, /*label=*/"SpecialImageRaster_OnView", GrMipmapped::kNo));
        }

        return {};
    }
#endif

    sk_sp<SkSpecialSurface> onMakeSurface(SkColorType colorType, const SkColorSpace* colorSpace,
                                          const SkISize& size, SkAlphaType at,
                                          const SkSurfaceProps& props) const override {
        // Ignore the requested color type, the raster backend currently only supports N32
        colorType = kN32_SkColorType;   // TODO: find ways to allow f16
        SkImageInfo info = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));
        return SkSpecialSurface::MakeRaster(info, props);
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        // No need to extract subset, onGetROPixels handles that when needed
        return SkSpecialImage::MakeFromRaster(subset, fBitmap, this->props());
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

    sk_sp<SkShader> onAsShader(SkTileMode tileMode,
                               const SkSamplingOptions& sampling,
                               const SkMatrix& lm) const override {
        // TODO(skbug.com/12784): SkImage::makeShader() doesn't support a subset yet, but SkBitmap
        // supports subset views so create the shader from the subset bitmap instead of fBitmap.
        SkBitmap subsetBM;
        if (!this->getROPixels(&subsetBM)) {
            return nullptr;
        }
        return subsetBM.asImage()->makeShader(tileMode, tileMode, sampling, lm);
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
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeFromRaster(const SkIRect& subset,
                                                     const SkBitmap& bm,
                                                     const SkSurfaceProps& props) {
    SkASSERT(RectFits(subset, bm.width(), bm.height()));

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
                                                     const SkSurfaceProps& props) {
    SkASSERT(RectFits(subset, bm.width(), bm.height()));

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
static sk_sp<SkImage> wrap_proxy_in_image(GrRecordingContext* context,
                                          GrSurfaceProxyView view,
                                          const SkColorInfo& colorInfo) {

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context),
                                   kNeedNewImageUniqueID,
                                   std::move(view),
                                   colorInfo);
}

class SkSpecialImage_Gpu final : public SkSpecialImage {
public:
    SkSpecialImage_Gpu(GrRecordingContext* context,
                       const SkIRect& subset,
                       uint32_t uniqueID,
                       GrSurfaceProxyView view,
                       const SkColorInfo& colorInfo,
                       const SkSurfaceProps& props)
        : SkSpecialImage(subset, uniqueID, colorInfo, props)
        , fContext(context)
        , fView(std::move(view)) {
    }

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
        sk_sp<SkImage> img = sk_sp<SkImage>(
                new SkImage_Gpu(sk_ref_sp(canvas->recordingContext()),
                                this->uniqueID(),
                                fView,
                                this->colorInfo()));

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

    sk_sp<SkSpecialSurface> onMakeSurface(SkColorType colorType, const SkColorSpace* colorSpace,
                                          const SkISize& size, SkAlphaType at,
                                          const SkSurfaceProps& props) const override {
        if (!fContext) {
            return nullptr;
        }

        SkImageInfo ii = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));

        return SkSpecialSurface::MakeRenderTarget(fContext, ii, props, fView.origin());
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImage::MakeDeferredFromGpu(fContext,
                                                   subset,
                                                   this->uniqueID(),
                                                   fView,
                                                   this->colorInfo(),
                                                   this->props());
    }

    sk_sp<SkImage> onAsImage(const SkIRect* subset) const override {
        GrSurfaceProxy* proxy = fView.proxy();
        if (subset) {
            if (proxy->isFunctionallyExact() && *subset == SkIRect::MakeSize(proxy->dimensions())) {
                proxy->priv().exactify(false);
                // The existing GrTexture is already tight so reuse it in the SkImage
                return wrap_proxy_in_image(fContext, fView, this->colorInfo());
            }

            auto subsetView = GrSurfaceProxyView::Copy(fContext,
                                                       fView,
                                                       GrMipmapped::kNo,
                                                       *subset,
                                                       SkBackingFit::kExact,
                                                       skgpu::Budgeted::kYes,
                                                       /*label=*/"SkSpecialImage_AsImage");
            if (!subsetView) {
                return nullptr;
            }
            SkASSERT(subsetView.asTextureProxy());
            SkASSERT(subsetView.proxy()->priv().isExact());

            // MDB: this is acceptable (wrapping subsetProxy in an SkImage) bc Copy will
            // return a kExact-backed proxy
            return wrap_proxy_in_image(fContext, std::move(subsetView), this->colorInfo());
        }

        proxy->priv().exactify(true);

        return wrap_proxy_in_image(fContext, fView, this->colorInfo());
    }

    sk_sp<SkShader> onAsShader(SkTileMode tileMode,
                               const SkSamplingOptions& sampling,
                               const SkMatrix& lm) const override {
        // The special image's logical (0,0) is at its subset's topLeft() so we need to account for
        // that in the local matrix used when sampling.
        SkMatrix subsetOrigin = SkMatrix::Translate(-this->subset().topLeft());
        subsetOrigin.postConcat(lm);
        // However, we don't need to modify the subset itself since that is defined with respect to
        // the base image, and the local matrix is applied before any tiling/clamping.
        const SkRect subset = SkRect::Make(this->subset());

        // asImage() w/o a subset makes no copy; create the SkImageShader directly to remember the
        // subset used to access the image.
        return SkImageShader::MakeSubset(
                this->asImage(), subset, tileMode, tileMode, sampling, &subsetOrigin);
    }

    sk_sp<SkSurface> onMakeTightSurface(SkColorType colorType, const SkColorSpace* colorSpace,
                                        const SkISize& size, SkAlphaType at) const override {
        // TODO (michaelludwig): Why does this ignore colorType but onMakeSurface doesn't ignore it?
        //    Once makeTightSurface() goes away, should this type overriding behavior be moved into
        //    onMakeSurface() or is this unnecessary?
        colorType = colorSpace && colorSpace->gammaIsLinear()
            ? kRGBA_F16_SkColorType : kRGBA_8888_SkColorType;
        SkImageInfo info = SkImageInfo::Make(size, colorType, at, sk_ref_sp(colorSpace));
        return SkSurface::MakeRenderTarget(
                fContext, skgpu::Budgeted::kYes, info, 0, fView.origin(), nullptr);
    }

private:
    GrRecordingContext* fContext;
    GrSurfaceProxyView  fView;
};

sk_sp<SkSpecialImage> SkSpecialImage::MakeDeferredFromGpu(GrRecordingContext* context,
                                                          const SkIRect& subset,
                                                          uint32_t uniqueID,
                                                          GrSurfaceProxyView view,
                                                          const GrColorInfo& colorInfo,
                                                          const SkSurfaceProps& props) {
    if (!context || context->abandoned() || !view.asTextureProxy()) {
        return nullptr;
    }

    SkColorType ct = GrColorTypeToSkColorType(colorInfo.colorType());

    SkASSERT(RectFits(subset, view.proxy()->width(), view.proxy()->height()));
    return sk_make_sp<SkSpecialImage_Gpu>(context, subset, uniqueID, std::move(view),
                                          SkColorInfo(ct,
                                                      colorInfo.alphaType(),
                                                      colorInfo.refColorSpace()),
                                          props);
}
#endif
