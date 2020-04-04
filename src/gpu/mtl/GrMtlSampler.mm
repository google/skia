/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlSampler.h"

#include "src/gpu/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

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

GrMtlSampler* GrMtlSampler::Create(const GrMtlGpu* gpu, const GrSamplerState& samplerState) {
    static MTLSamplerMinMagFilter mtlMinMagFilterModes[] = {
        MTLSamplerMinMagFilterNearest,
        MTLSamplerMinMagFilterLinear,
        MTLSamplerMinMagFilterLinear
    };

    GR_STATIC_ASSERT((int)GrSamplerState::Filter::kNearest == 0);
    GR_STATIC_ASSERT((int)GrSamplerState::Filter::kBilerp == 1);
    GR_STATIC_ASSERT((int)GrSamplerState::Filter::kMipMap == 2);

    auto samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    samplerDesc.sAddressMode = wrap_mode_to_mtl_sampler_address(samplerState.wrapModeX(),
                                                                gpu->mtlCaps());
    samplerDesc.tAddressMode = wrap_mode_to_mtl_sampler_address(samplerState.wrapModeY(),
                                                                gpu->mtlCaps());
    samplerDesc.magFilter = mtlMinMagFilterModes[static_cast<int>(samplerState.filter())];
    samplerDesc.minFilter = mtlMinMagFilterModes[static_cast<int>(samplerState.filter())];
    samplerDesc.mipFilter = MTLSamplerMipFilterLinear;
    samplerDesc.lodMinClamp = 0.0f;
    bool useMipMaps = GrSamplerState::Filter::kMipMap == samplerState.filter();
    samplerDesc.lodMaxClamp = !useMipMaps ? 0.0f : 10000.0f;
    samplerDesc.maxAnisotropy = 1.0f;
    samplerDesc.normalizedCoordinates = true;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        samplerDesc.compareFunction = MTLCompareFunctionNever;
    }

    return new GrMtlSampler([gpu->device() newSamplerStateWithDescriptor: samplerDesc],
                            GenerateKey(samplerState));
}

GrMtlSampler::Key GrMtlSampler::GenerateKey(const GrSamplerState& samplerState) {
    return GrSamplerState::GenerateKey(samplerState);
}
