/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/image/GrImageUtils.h"

#include "include/core/SkImage.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "src/core/SkSamplingPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrBicubicEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Raster.h"

#include <string_view>
#include <utility>

class SkMatrix;
enum SkAlphaType : int;
enum class GrColorType;
enum class SkTileMode;
struct SkRect;

namespace skgpu::ganesh {

GrSurfaceProxyView CopyView(GrRecordingContext* context,
                            GrSurfaceProxyView src,
                            GrMipmapped mipmapped,
                            GrImageTexGenPolicy policy,
                            std::string_view label) {
    skgpu::Budgeted budgeted = policy == GrImageTexGenPolicy::kNew_Uncached_Budgeted
                                       ? skgpu::Budgeted::kYes
                                       : skgpu::Budgeted::kNo;
    return GrSurfaceProxyView::Copy(context,
                                    std::move(src),
                                    mipmapped,
                                    SkBackingFit::kExact,
                                    budgeted,
                                    /*label=*/label);
}

std::tuple<GrSurfaceProxyView, GrColorType> RasterAsView(GrRecordingContext* rContext,
                                                         const SkImage_Raster* raster,
                                                         GrMipmapped mipmapped,
                                                         GrImageTexGenPolicy policy) {
    if (policy == GrImageTexGenPolicy::kDraw) {
        // If the draw doesn't require mipmaps but this SkImage has them go ahead and make a
        // mipmapped texture. There are three reasons for this:
        // 1) Avoiding another texture creation if a later draw requires mipmaps.
        // 2) Ensuring we upload the bitmap's levels instead of generating on the GPU from the base.
        if (raster->hasMipmaps()) {
            mipmapped = GrMipmapped::kYes;
        }
        return GrMakeCachedBitmapProxyView(rContext,
                                           raster->bitmap(),
                                           /*label=*/"TextureForImageRasterWithPolicyEqualKDraw",
                                           mipmapped);
    }
    auto budgeted = (policy == GrImageTexGenPolicy::kNew_Uncached_Unbudgeted)
                            ? skgpu::Budgeted::kNo
                            : skgpu::Budgeted::kYes;
    return GrMakeUncachedBitmapProxyView(
            rContext, raster->bitmap(), mipmapped, SkBackingFit::kExact, budgeted);
}

std::tuple<GrSurfaceProxyView, GrColorType> AsView(GrRecordingContext* rContext,
                                                   const SkImage* img,
                                                   GrMipmapped mipmapped,
                                                   GrImageTexGenPolicy policy) {
    SkASSERT(img);
    if (!rContext) {
        return {};
    }
    if (!rContext->priv().caps()->mipmapSupport() || img->dimensions().area() <= 1) {
        mipmapped = GrMipmapped::kNo;
    }

    auto ib = static_cast<const SkImage_Base*>(img);
    switch (ib->type()) {
        case SkImage_Base::Type::kRaster:
            // Note: kRasterPinnable has implemented its own onAsView (using pinned data).
            return skgpu::ganesh::RasterAsView(
                    rContext, static_cast<const SkImage_Raster*>(ib), mipmapped, policy);
        default:
            return ib->onAsView(rContext, mipmapped, policy);
    }
    SkUNREACHABLE;
}

static std::unique_ptr<GrFragmentProcessor> make_fp_from_view(GrRecordingContext* rContext,
                                                              GrSurfaceProxyView view,
                                                              SkAlphaType at,
                                                              SkSamplingOptions sampling,
                                                              const SkTileMode tileModes[2],
                                                              const SkMatrix& m,
                                                              const SkRect* subset,
                                                              const SkRect* domain) {
    if (!view) {
        return nullptr;
    }
    const GrCaps& caps = *rContext->priv().caps();
    auto wmx = SkTileModeToWrapMode(tileModes[0]);
    auto wmy = SkTileModeToWrapMode(tileModes[1]);
    if (sampling.useCubic) {
        if (subset) {
            if (domain) {
                return GrBicubicEffect::MakeSubset(std::move(view),
                                                   at,
                                                   m,
                                                   wmx,
                                                   wmy,
                                                   *subset,
                                                   *domain,
                                                   sampling.cubic,
                                                   GrBicubicEffect::Direction::kXY,
                                                   *rContext->priv().caps());
            }
            return GrBicubicEffect::MakeSubset(std::move(view),
                                               at,
                                               m,
                                               wmx,
                                               wmy,
                                               *subset,
                                               sampling.cubic,
                                               GrBicubicEffect::Direction::kXY,
                                               *rContext->priv().caps());
        }
        return GrBicubicEffect::Make(std::move(view),
                                     at,
                                     m,
                                     wmx,
                                     wmy,
                                     sampling.cubic,
                                     GrBicubicEffect::Direction::kXY,
                                     *rContext->priv().caps());
    }
    if (sampling.isAniso()) {
        if (!rContext->priv().caps()->anisoSupport()) {
            // Fallback to linear
            sampling = SkSamplingPriv::AnisoFallback(view.mipmapped() == GrMipmapped::kYes);
        }
    } else if (view.mipmapped() == GrMipmapped::kNo) {
        sampling = SkSamplingOptions(sampling.filter);
    }
    GrSamplerState sampler;
    if (sampling.isAniso()) {
        sampler = GrSamplerState::Aniso(wmx, wmy, sampling.maxAniso, view.mipmapped());
    } else {
        sampler = GrSamplerState(wmx, wmy, sampling.filter, sampling.mipmap);
    }
    if (subset) {
        if (domain) {
            return GrTextureEffect::MakeSubset(
                    std::move(view), at, m, sampler, *subset, *domain, caps);
        }
        return GrTextureEffect::MakeSubset(std::move(view), at, m, sampler, *subset, caps);
    } else {
        return GrTextureEffect::Make(std::move(view), at, m, sampler, caps);
    }
}

std::unique_ptr<GrFragmentProcessor> raster_as_fp(GrRecordingContext* rContext,
                                                  const SkImage_Raster* img,
                                                  SkSamplingOptions sampling,
                                                  const SkTileMode tileModes[2],
                                                  const SkMatrix& m,
                                                  const SkRect* subset,
                                                  const SkRect* domain) {
    auto mm = sampling.mipmap == SkMipmapMode::kNone ? GrMipmapped::kNo : GrMipmapped::kYes;
    return make_fp_from_view(rContext,
                             std::get<0>(AsView(rContext, img, mm)),
                             img->alphaType(),
                             sampling,
                             tileModes,
                             m,
                             subset,
                             domain);
}

std::unique_ptr<GrFragmentProcessor> AsFragmentProcessor(GrRecordingContext* rContext,
                                                         const SkImage* img,
                                                         SkSamplingOptions sampling,
                                                         const SkTileMode tileModes[2],
                                                         const SkMatrix& m,
                                                         const SkRect* subset,
                                                         const SkRect* domain) {
    if (!rContext) {
        return {};
    }
    if (sampling.useCubic && !GrValidCubicResampler(sampling.cubic)) {
        return {};
    }
    if (sampling.mipmap != SkMipmapMode::kNone &&
        (!rContext->priv().caps()->mipmapSupport() || img->dimensions().area() <= 1)) {
        sampling = SkSamplingOptions(sampling.filter);
    }

    auto ib = static_cast<const SkImage_Base*>(img);
    switch (ib->type()) {
        case SkImage_Base::Type::kRaster:
        case SkImage_Base::Type::kRasterPinnable:
            return raster_as_fp(rContext,
                                static_cast<const SkImage_Raster*>(ib),
                                sampling,
                                tileModes,
                                m,
                                subset,
                                domain);
        default:
            return ib->onAsFragmentProcessor(rContext, sampling, tileModes, m, subset, domain);
    }
    SkUNREACHABLE;
}

}  // namespace skgpu::ganesh
