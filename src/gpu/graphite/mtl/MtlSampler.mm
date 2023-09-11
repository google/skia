/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlSampler.h"

#include "include/core/SkSamplingOptions.h"
#include "src/gpu/graphite/mtl/MtlCaps.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"

namespace skgpu::graphite {

MtlSampler::MtlSampler(const MtlSharedContext* sharedContext,
                       sk_cfp<id<MTLSamplerState>> samplerState)
        : Sampler(sharedContext)
        , fSamplerState(std::move(samplerState)) {}

static inline MTLSamplerAddressMode tile_mode_to_mtl_sampler_address(SkTileMode tileMode,
                                                                     const Caps& caps) {
    switch (tileMode) {
        case SkTileMode::kClamp:
            return MTLSamplerAddressModeClampToEdge;
        case SkTileMode::kRepeat:
            return MTLSamplerAddressModeRepeat;
        case SkTileMode::kMirror:
            return MTLSamplerAddressModeMirrorRepeat;
        case SkTileMode::kDecal:
            // For this tilemode, we should have checked that clamp-to-border support exists.
            // If it doesn't we should have fallen back to a shader instead.
            // TODO: for textures with alpha, we could use ClampToZero if there's no
            // ClampToBorderColor as they'll clamp to (0,0,0,0).
            // Unfortunately textures without alpha end up clamping to (0,0,0,1).
            if (@available(macOS 10.12, iOS 14.0, tvOS 14.0, *)) {
                SkASSERT(caps.clampToBorderSupport());
                return MTLSamplerAddressModeClampToBorderColor;
            } else {
                SkASSERT(false);
                return MTLSamplerAddressModeClampToZero;
            }
    }
    SkUNREACHABLE;
}

sk_sp<MtlSampler> MtlSampler::Make(const MtlSharedContext* sharedContext,
                                   const SkSamplingOptions& samplingOptions,
                                   SkTileMode xTileMode,
                                   SkTileMode yTileMode) {
    sk_cfp<MTLSamplerDescriptor*> desc([[MTLSamplerDescriptor alloc] init]);

    MTLSamplerMinMagFilter minMagFilter = [&] {
        switch (samplingOptions.filter) {
            case SkFilterMode::kNearest: return MTLSamplerMinMagFilterNearest;
            case SkFilterMode::kLinear:  return MTLSamplerMinMagFilterLinear;
        }
        SkUNREACHABLE;
    }();

    MTLSamplerMipFilter mipFilter = [&] {
      switch (samplingOptions.mipmap) {
          case SkMipmapMode::kNone:    return MTLSamplerMipFilterNotMipmapped;
          case SkMipmapMode::kNearest: return MTLSamplerMipFilterNearest;
          case SkMipmapMode::kLinear:  return MTLSamplerMipFilterLinear;
      }
      SkUNREACHABLE;
    }();

    (*desc).rAddressMode = MTLSamplerAddressModeClampToEdge;
    (*desc).sAddressMode = tile_mode_to_mtl_sampler_address(xTileMode, sharedContext->mtlCaps());
    (*desc).tAddressMode = tile_mode_to_mtl_sampler_address(yTileMode, sharedContext->mtlCaps());
    (*desc).magFilter = minMagFilter;
    (*desc).minFilter = minMagFilter;
    (*desc).mipFilter = mipFilter;
    (*desc).lodMinClamp = 0.0f;
    (*desc).lodMaxClamp = FLT_MAX;  // default value according to docs.
    (*desc).maxAnisotropy = 1;      // TODO: if we start using aniso, need to add to key
    (*desc).normalizedCoordinates = true;
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        (*desc).compareFunction = MTLCompareFunctionNever;
    }
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    NSString* tileModeLabels[] = {
        @"Clamp",
        @"Repeat",
        @"Mirror",
        @"Decal"
    };
    NSString* minMagFilterLabels[] = {
        @"Nearest",
        @"Linear"
    };
    NSString* mipFilterLabels[] = {
        @"MipNone",
        @"MipNearest",
        @"MipLinear"
    };

    (*desc).label = [NSString stringWithFormat:@"X%@Y%@%@%@",
                                               tileModeLabels[(int)xTileMode],
                                               tileModeLabels[(int)yTileMode],
                                               minMagFilterLabels[(int)samplingOptions.filter],
                                               mipFilterLabels[(int)samplingOptions.mipmap]];
#endif

    sk_cfp<id<MTLSamplerState>> sampler(
            [sharedContext->device() newSamplerStateWithDescriptor:desc.get()]);
    if (!sampler) {
        return nullptr;
    }
    return sk_sp<MtlSampler>(new MtlSampler(sharedContext, std::move(sampler)));
}

void MtlSampler::freeGpuData() {
    fSamplerState.reset();
}

} // namespace skgpu::graphite

