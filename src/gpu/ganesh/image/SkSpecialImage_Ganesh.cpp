/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"  // IWYU pragma: keep
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/gpu/ganesh/GrImageContext.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/image/SkImage_Ganesh.h"
#include "src/image/SkImage_Base.h"
#include "src/shaders/SkImageShader.h"

#include <cstddef>
#include <tuple>
#include <utility>

class SkShader;
enum SkColorType : int;
enum class SkTileMode;

static sk_sp<SkImage> wrap_proxy_in_image(GrRecordingContext* context,
                                          GrSurfaceProxyView view,
                                          const SkColorInfo& colorInfo) {
    return sk_make_sp<SkImage_Ganesh>(
            sk_ref_sp(context), kNeedNewImageUniqueID, std::move(view), colorInfo);
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
            , fView(std::move(view)) {}

    size_t getSize() const override { return fView.proxy()->gpuMemorySize(); }

    bool isGaneshBacked() const override { return true; }

    void onDraw(SkCanvas* canvas,
                SkScalar x,
                SkScalar y,
                const SkSamplingOptions& sampling,
                const SkPaint* paint) const override {
        SkRect dst = SkRect::MakeXYWH(x, y, this->subset().width(), this->subset().height());

        // TODO: In this instance we know we're going to draw a sub-portion of the backing
        // texture into the canvas so it is okay to wrap it in an SkImage. This poses
        // some problems for full deferral however in that when the deferred SkImage_Ganesh
        // instantiates itself it is going to have to either be okay with having a larger
        // than expected backing texture (unlikely) or the 'fit' of the SurfaceProxy needs
        // to be tightened (if it is deferred).
        sk_sp<SkImage> img = sk_sp<SkImage>(new SkImage_Ganesh(
                sk_ref_sp(canvas->recordingContext()), this->uniqueID(), fView, this->colorInfo()));

        canvas->drawImageRect(img,
                              SkRect::Make(this->subset()),
                              dst,
                              sampling,
                              paint,
                              SkCanvas::kStrict_SrcRectConstraint);
    }

    GrRecordingContext* getContext() const override { return fContext; }

    GrSurfaceProxyView view(GrRecordingContext*) const { return fView; }

    bool onGetROPixels(SkBitmap* dst) const override {
        // This should never be called: All GPU image filters are implemented entirely on the GPU,
        // so we never perform read-back.
        SkASSERT(false);
        return false;
    }

    sk_sp<SkSpecialImage> onMakeSubset(const SkIRect& subset) const override {
        return SkSpecialImages::MakeDeferredFromGpu(
                fContext, subset, this->uniqueID(), fView, this->colorInfo(), this->props());
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

private:
    GrRecordingContext* fContext;
    GrSurfaceProxyView fView;
};

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeFromTextureImage(GrRecordingContext* rContext,
                                           const SkIRect& subset,
                                           sk_sp<SkImage> image,
                                           const SkSurfaceProps& props) {
    if (!rContext || !image || subset.isEmpty()) {
        return nullptr;
    }

    SkASSERT(image->bounds().contains(subset));

    // This will work even if the image is a raster-backed image.
    auto [view, ct] = skgpu::ganesh::AsView(rContext, image, GrMipmapped::kNo);
    return MakeDeferredFromGpu(rContext,
                               subset,
                               image->uniqueID(),
                               std::move(view),
                               {ct, image->alphaType(), image->refColorSpace()},
                               props);
}

sk_sp<SkSpecialImage> MakeDeferredFromGpu(GrRecordingContext* context,
                                          const SkIRect& subset,
                                          uint32_t uniqueID,
                                          GrSurfaceProxyView view,
                                          const GrColorInfo& colorInfo,
                                          const SkSurfaceProps& props) {
    if (!context || context->abandoned() || !view.asTextureProxy()) {
        return nullptr;
    }

    SkASSERT(view.proxy()->backingStoreBoundsIRect().contains(subset));

    SkColorType ct = GrColorTypeToSkColorType(colorInfo.colorType());
    return sk_make_sp<SkSpecialImage_Gpu>(
            context,
            subset,
            uniqueID,
            std::move(view),
            SkColorInfo(ct, colorInfo.alphaType(), colorInfo.refColorSpace()),
            props);
}

GrSurfaceProxyView AsView(GrRecordingContext* context, const SkSpecialImage* img) {
    if (!context || !img) {
        return {};
    }
    if (img->isGaneshBacked()) {
        auto grImg = static_cast<const SkSpecialImage_Gpu*>(img);
        return grImg->view(context);
    }
    SkASSERT(!img->isGraphiteBacked());
    SkBitmap bm;
    SkAssertResult(img->getROPixels(&bm));  // this should always succeed for raster images
    return std::get<0>(GrMakeCachedBitmapProxyView(
            context, bm, /*label=*/"SpecialImageRaster_AsView", GrMipmapped::kNo));
}

sk_sp<SkSpecialImage> ImageToColorSpace(const skif::Context& ctx,
                                        SkSpecialImage* src) {
    // There are several conditions that determine if we actually need to convert the source to the
    // destination's color space. Rather than duplicate that logic here, just try to make an xform
    // object. If that produces something, then both are tagged, and the source is in a different
    // gamut than the dest. There is some overhead to making the xform, but those are cached, and
    // if we get one back, that means we're about to use it during the conversion anyway.
    auto colorSpaceXform = GrColorSpaceXform::Make(src->getColorSpace(),  src->alphaType(),
                                                   ctx.colorSpace(), kPremul_SkAlphaType);

    if (!colorSpaceXform) {
        // No xform needed, just return the original image
        return sk_ref_sp(src);
    }

    sk_sp<SkSpecialSurface> surf = ctx.makeSurface(src->dimensions());
    if (!surf) {
        return sk_ref_sp(src);
    }

    SkCanvas* canvas = surf->getCanvas();
    SkASSERT(canvas);
    SkPaint p;
    p.setBlendMode(SkBlendMode::kSrc);
    src->draw(canvas, 0, 0, SkSamplingOptions(), &p);
    return surf->makeImageSnapshot();
}

}  // namespace SkSpecialImages

namespace SkSpecialSurfaces {
sk_sp<SkSpecialSurface> MakeRenderTarget(GrRecordingContext* rContext,
                                         const SkImageInfo& ii,
                                         const SkSurfaceProps& props,
                                         GrSurfaceOrigin surfaceOrigin) {
    if (!rContext) {
        return nullptr;
    }

    auto device = rContext->priv().createDevice(skgpu::Budgeted::kYes,
                                                ii,
                                                SkBackingFit::kApprox,
                                                1,
                                                GrMipmapped::kNo,
                                                GrProtected::kNo,
                                                surfaceOrigin,
                                                {props.flags(), kUnknown_SkPixelGeometry},
                                                skgpu::ganesh::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<SkSpecialSurface>(std::move(device), subset);
}
}  // namespace SkSpecialSurfaces
