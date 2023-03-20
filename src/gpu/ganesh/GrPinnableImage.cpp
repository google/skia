/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkImageAndroid.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"

#include <memory>

struct PinnedData {
    GrSurfaceProxyView fPinnedView;
    int32_t fPinnedCount = 0;
    uint32_t fPinnedUniqueID = SK_InvalidUniqueID;
    uint32_t fPinnedContextID = SK_InvalidUniqueID;
    GrColorType fPinnedColorType = GrColorType::kUnknown;
};

class SkImage_RasterPinnable final : public SkImage_Raster {
public:
    SkImage_RasterPinnable(const SkBitmap& bm): SkImage_Raster(bm, /*bitmapMayBeMutable = */true) {}

    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         GrMipmapped,
                                                         GrImageTexGenPolicy) const override;

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kRasterPinnable; }

    SkBitmap bitmap() const { return fBitmap; }

    std::unique_ptr<PinnedData> fPinnedData;
};

std::tuple<GrSurfaceProxyView, GrColorType> SkImage_RasterPinnable::onAsView(
        GrRecordingContext* rContext,
        GrMipmapped mipmapped,
        GrImageTexGenPolicy policy) const {
    if (fPinnedData) {
        // We ignore the mipmap request here. If the pinned view isn't mipmapped then we will
        // fallback to bilinear. The pin API is used by Android Framework which does not expose
        // mipmapping. Moreover, we're moving towards requiring that images be made with mip levels
        // if mipmapping is desired (skbug.com/10411)
        mipmapped = GrMipmapped::kNo;
        if (policy != GrImageTexGenPolicy::kDraw) {
            return {CopyView(rContext,
                             fPinnedData->fPinnedView,
                             mipmapped,
                             policy,
                             /*label=*/"TextureForPinnableRasterImageWithPolicyNotEqualKDraw"),
                    fPinnedData->fPinnedColorType};
        }
        return {fPinnedData->fPinnedView, fPinnedData->fPinnedColorType};
    }
    if (policy == GrImageTexGenPolicy::kDraw) {
        // If the draw doesn't require mipmaps but this SkImage has them go ahead and make a
        // mipmapped texture. There are three reasons for this:
        // 1) Avoiding another texture creation if a later draw requires mipmaps.
        // 2) Ensuring we upload the bitmap's levels instead of generating on the GPU from the base.
        if (this->hasMipmaps()) {
            mipmapped = GrMipmapped::kYes;
        }
        return GrMakeCachedBitmapProxyView(rContext,
                                           fBitmap,
                                           "TextureForPinnableRasterImageWithPolicyEqualKDraw",
                                           mipmapped);
    }
    auto budgeted = (policy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted)
                            ? skgpu::Budgeted::kNo
                            : skgpu::Budgeted::kYes;
    return GrMakeUncachedBitmapProxyView(rContext,
                                         fBitmap,
                                         mipmapped,
                                         SkBackingFit::kExact,
                                         budgeted);
}

namespace sk_image_factory {

sk_sp<SkImage> MakePinnableFromRasterBitmap(const SkBitmap& bm) {
    if (!SkImageInfoIsValid(bm.info()) || bm.rowBytes() < bm.info().minRowBytes()) {
        return nullptr;
    }

    return sk_make_sp<SkImage_RasterPinnable>(bm);
}

} // namespace sk_image_factory

namespace skgpu::ganesh {

bool PinAsTexture(GrRecordingContext* rContext, SkImage* img) {
    auto ib = as_IB(img);
    if (ib->type() != SkImage_Base::Type::kRasterPinnable) {
        // Cannot pin images which are not of subclass SkImage_RasterPinnable
        return false;
    }
    auto raster = static_cast<SkImage_RasterPinnable*>(ib);
    if (!raster->fPinnedData) {
        auto data = std::make_unique<PinnedData>();
        std::tie(data->fPinnedView, data->fPinnedColorType) =
                GrMakeCachedBitmapProxyView(rContext,
                                            raster->bitmap(),
                                            /*label=*/"ganesh_PinAsTexture",
                                            GrMipmapped::kNo);
        if (!data->fPinnedView) {
            return false;
        }
        data->fPinnedUniqueID = raster->bitmap().getGenerationID();
        data->fPinnedContextID = rContext->priv().contextID();
        raster->fPinnedData.swap(data);
    } else {
        SkASSERT(raster->fPinnedData->fPinnedCount > 0);
        SkASSERT(raster->fPinnedData->fPinnedUniqueID != 0);
        if (rContext->priv().contextID() != raster->fPinnedData->fPinnedContextID) {
            return false;
        }
    }
    // Note: we only increment if the texture was successfully pinned
    raster->fPinnedData->fPinnedCount++;
    return true;
}

void UnpinTexture(GrRecordingContext*, SkImage* img) {
    auto ib = as_IB(img);
    if (ib->type() != SkImage_Base::Type::kRasterPinnable) {
        // Cannot pin images which are not of subclass SkImage_RasterPinnable
        return;
    }
    auto raster = static_cast<SkImage_RasterPinnable*>(ib);
    if (!raster->fPinnedData) {
        SkASSERT(false);
        return;
    }

    SkASSERT(raster->fPinnedData->fPinnedCount > 0);
    SkASSERT(raster->fPinnedData->fPinnedUniqueID != 0);
    // It would be good to check rContext->priv().contextID() != fPinnedContextID
    // but Android used to (maybe still does) call Unpin with a freed context ptr

    raster->fPinnedData->fPinnedCount--;
    if (raster->fPinnedData->fPinnedCount <= 0) {
        raster->fPinnedData.reset(nullptr);
    }
}

} // namespace skgpu::ganesh
