/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/mtl/GrMtlSampler.h"

#include "src/gpu/ganesh/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GR_NORETAIN_BEGIN

static inline MTLSamplerAddressMode wrap_mode_to_mtl_sampler_address(
        GrSamplerState::WrapMode wrapMode, const GrCaps& caps) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return MTLSamplerAddressModeClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return MTLSamplerAddressModeRepeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return MTLSamplerAddressModeMirrorRepeat;
        case GrSamplerState::WrapMode::kClampToBorder:
            // Must guard the reference to the clamp to border address mode by macro since iOS or
            // older MacOS builds will fail if it's referenced, even if other code makes sure it's
            // never used.
#ifdef SK_BUILD_FOR_MAC
            if (@available(macOS 10.12, *)) {
                SkASSERT(caps.clampToBorderSupport());
                return MTLSamplerAddressModeClampToBorderColor;
            } else
#endif
            {
                SkASSERT(false);
                return MTLSamplerAddressModeClampToEdge;
            }
    }
    SK_ABORT("Unknown wrap mode.");
}

GrMtlSampler* GrMtlSampler::Create(const GrMtlGpu* gpu, GrSamplerState samplerState) {
    MTLSamplerMinMagFilter minMagFilter = [&] {
        switch (samplerState.filter()) {
            case GrSamplerState::Filter::kNearest: return MTLSamplerMinMagFilterNearest;
            case GrSamplerState::Filter::kLinear:  return MTLSamplerMinMagFilterLinear;
        }
        SkUNREACHABLE;
    }();

    MTLSamplerMipFilter mipFilter = [&] {
      switch (samplerState.mipmapMode()) {
          case GrSamplerState::MipmapMode::kNone:    return MTLSamplerMipFilterNotMipmapped;
          case GrSamplerState::MipmapMode::kNearest: return MTLSamplerMipFilterNearest;
          case GrSamplerState::MipmapMode::kLinear:  return MTLSamplerMipFilterLinear;
      }
      SkUNREACHABLE;
    }();

    auto samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.sAddressMode = wrap_mode_to_mtl_sampler_address(samplerState.wrapModeX(),
                                                                gpu->mtlCaps());
    samplerDesc.tAddressMode = wrap_mode_to_mtl_sampler_address(samplerState.wrapModeY(),
                                                                gpu->mtlCaps());
    samplerDesc.magFilter = minMagFilter;
    samplerDesc.minFilter = minMagFilter;
    samplerDesc.mipFilter = mipFilter;
    samplerDesc.lodMinClamp = 0.0f;
    samplerDesc.lodMaxClamp = FLT_MAX;  // default value according to docs.
    // Metal documents that maxAnisotropy must be between 1 and 16 inclusive.
    samplerDesc.maxAnisotropy = std::min(samplerState.maxAniso(), 16);
    samplerDesc.normalizedCoordinates = true;
    if (@available(macOS 10.11, iOS 9.0, tvOS 9.0, *)) {
        samplerDesc.compareFunction = MTLCompareFunctionNever;
    }

    return new GrMtlSampler([gpu->device() newSamplerStateWithDescriptor: samplerDesc],
                            GenerateKey(samplerState));
}

GrMtlSampler::Key GrMtlSampler::GenerateKey(GrSamplerState samplerState) {
    // We haven't found any documentation on how anisotropy interacts with other filter settings
    // so assume they are all considered.
    return samplerState.asKey(/*anisoIsOrthogonal=*/true);
}

GR_NORETAIN_END
