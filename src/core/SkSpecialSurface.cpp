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
#include "include/core/SkMallocPixelRef.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkSpecialImage.h"
#include "src/core/SkSurfacePriv.h"

SkSpecialSurface::SkSpecialSurface(sk_sp<SkBaseDevice> device, const SkIRect& subset)
        : fSubset(subset) {
    SkASSERT(fSubset.width() > 0);
    SkASSERT(fSubset.height() > 0);

    fCanvas = std::make_unique<SkCanvas>(std::move(device));
    fCanvas->clipRect(SkRect::Make(subset));
#ifdef SK_IS_BOT
    fCanvas->clear(SK_ColorRED);  // catch any imageFilter sloppiness
#endif
}

sk_sp<SkSpecialImage> SkSpecialSurface::makeImageSnapshot() {
    fCanvas->restoreToCount(0);

    // Because of the above 'restoreToCount(0)' we know we're getting the base device here.
    SkBaseDevice* baseDevice = SkCanvasPriv::TopDevice(fCanvas.get());
    if (!baseDevice) {
        return nullptr;
    }

    sk_sp<SkSpecialImage> image = baseDevice->snapSpecial(this->subset());

    fCanvas.reset();
    return image;
}

///////////////////////////////////////////////////////////////////////////////
sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRaster(const SkImageInfo& info,
                                                     const SkSurfaceProps& props) {
    if (!SkSurfaceValidateRasterInfo(info)) {
        return nullptr;
    }

    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, 0);
    if (!pr) {
        return nullptr;
    }

    SkBitmap bitmap;
    bitmap.setInfo(info, info.minRowBytes());
    bitmap.setPixelRef(std::move(pr), 0, 0);

    sk_sp<SkBaseDevice> device(new SkBitmapDevice(bitmap,
                                                  { props.flags(), kUnknown_SkPixelGeometry }));
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(info.dimensions());

    return sk_make_sp<SkSpecialSurface>(std::move(device), subset);
}

///////////////////////////////////////////////////////////////////////////////
#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeRenderTarget(GrRecordingContext* rContext,
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
                                                skgpu::v1::Device::InitContents::kUninit);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<SkSpecialSurface>(std::move(device), subset);
}

#endif // SK_SUPPORT_GPU

///////////////////////////////////////////////////////////////////////////////
#if SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/Device.h"

sk_sp<SkSpecialSurface> SkSpecialSurface::MakeGraphite(skgpu::graphite::Recorder* recorder,
                                                       const SkImageInfo& ii,
                                                       const SkSurfaceProps& props) {
    using namespace skgpu::graphite;

    if (!recorder) {
        return nullptr;
    }

    sk_sp<Device> device = Device::Make(recorder,
                                        ii,
                                        skgpu::Budgeted::kYes,
                                        skgpu::Mipmapped::kNo,
                                        {props.flags(), kUnknown_SkPixelGeometry},
                                        /* addInitialClear= */ false);
    if (!device) {
        return nullptr;
    }

    const SkIRect subset = SkIRect::MakeSize(ii.dimensions());

    return sk_make_sp<SkSpecialSurface>(std::move(device), subset);
}

#endif // SK_GRAPHITE_ENABLED
