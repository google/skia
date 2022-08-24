/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file
 */

#include "src/core/SkSpecialSurface.h"

#include <memory>

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSurfacePriv.h"

SkSpecialSurface::SkSpecialSurface(const SkIRect& subset,
                                   const SkSurfaceProps& props)
        : fProps(props.flags(), kUnknown_SkPixelGeometry)
        , fSubset(subset) {
    SkASSERT(fSubset.width() > 0);
    SkASSERT(fSubset.height() > 0);
}

sk_sp<SkSpecialImage> SkSpecialSurface::makeImageSnapshot() {
    sk_sp<SkSpecialImage> image(this->onMakeImageSnapshot());
    fCanvas.reset();
    return image;
}

///////////////////////////////////////////////////////////////////////////////
#include "include/core/SkMallocPixelRef.h"

class SkSpecialSurface_Raster : public SkSpecialSurface {
public:
    SkSpecialSurface_Raster(const SkImageInfo& info,
                            sk_sp<SkPixelRef> pr,
                            const SkIRect& subset,
                            const SkSurfaceProps& props)
            : SkSpecialSurface(subset, props) {
        SkASSERT(info.width() == pr->width() && info.height() == pr->height());
        fBitmap.setInfo(info, info.minRowBytes());
        fBitmap.setPixelRef(std::move(pr), 0, 0);

        fCanvas = std::make_unique<SkCanvas>(fBitmap, this->props());
        fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
        fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
    }

    ~SkSpecialSurface_Raster() override { }

protected:
    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        return SkSpecialImage::MakeFromRaster(this->subset(), fBitmap, this->props());
    }

private:
    SkBitmap fBitmap;
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeFromBitmap(const SkIRect& subset, SkBitmap& bm,
                                                         const SkSurfaceProps& props) {
    if (subset.isEmpty() || !SkSurfaceValidateRasterInfo(bm.info(), bm.rowBytes())) {
        return nullptr;
    }
    return sk_make_sp<SkSpecialSurface_Raster>(bm.info(), sk_ref_sp(bm.pixelRef()), subset, props);
}

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRaster(const SkImageInfo& info,
                                                     const SkSurfaceProps& props) {
    if (!SkSurfaceValidateRasterInfo(info)) {
        return nullptr;
    }

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, 0);
    if (!pr) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeWH(info.width(), info.height());

    return sk_make_sp<SkSpecialSurface_Raster>(info, std::move(pr), subset, props);
}

#if SK_SUPPORT_GPU
///////////////////////////////////////////////////////////////////////////////
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"

class SkSpecialSurface_Gpu : public SkSpecialSurface {
public:
    SkSpecialSurface_Gpu(sk_sp<skgpu::v1::Device> device, SkIRect subset)
            : SkSpecialSurface(subset, device->surfaceProps())
            , fReadView(device->readSurfaceView()) {

        fCanvas = std::make_unique<SkCanvas>(std::move(device));
        fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
        fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
    }

protected:
    sk_sp<SkSpecialImage> onMakeImageSnapshot() override {
        if (!fReadView.asTextureProxy()) {
            return nullptr;
        }
        GrColorType ct = SkColorTypeToGrColorType(fCanvas->imageInfo().colorType());

        // Note: SkSpecialImages can only be snapShotted once, so this call is destructive and we
        // move fReadMove.
        return SkSpecialImage::MakeDeferredFromGpu(fCanvas->recordingContext(),
                                                   this->subset(),
                                                   kNeedNewImageUniqueID_SpecialImage,
                                                   std::move(fReadView),
                                                   { ct, kPremul_SkAlphaType,
                                                     fCanvas->imageInfo().refColorSpace() },
                                                   this->props());
    }

private:
    GrSurfaceProxyView fReadView;
};

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRenderTarget(GrRecordingContext* rContext,
                                                           const SkImageInfo& ii,
                                                           const SkSurfaceProps& props,
                                                           GrSurfaceOrigin surfaceOrigin) {
    if (!rContext) {
        return nullptr;
    }

    auto device = rContext->priv().createDevice(SkBudgeted::kYes, ii, SkBackingFit::kApprox, 1,
                                                GrMipmapped::kNo, GrProtected::kNo,
                                                surfaceOrigin, props,
                                                skgpu::v1::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<SkSpecialSurface_Gpu>(std::move(device), subset);
}

#endif
