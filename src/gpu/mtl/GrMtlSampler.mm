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

GrMtlSampler* GrMtlSampler::Create(const GrMtlGpu* gpu, GrSamplerState samplerState) {
    // Hack: don't allow linear MIP filtering without linear min/mag filter.
    if (samplerState.mipmapMode() == GrSamplerState::MipmapMode::kLinear) {
        samplerState.setFilterMode(GrSamplerState::Filter::kLinear);
    }
    MTLSamplerMinMagFilter minMagFilter = [&] {
        switch (samplerState.filter()) {
            case GrSamplerState::Filter::kNearest: return MTLSamplerMinMagFilterNearest;
            case GrSamplerState::Filter::kLinear:  return MTLSamplerMinMagFilterLinear;
        }
        SkUNREACHABLE;
    }();

    MTLSamplerMipFilter mipFilter = [&] {
      switch (samplerState.mipmapMode()) {
          // It would make more sense to use MTLSamplerMipFilterNotMipmapped here but
          // see comment below about lodMaxClamp.
          case GrSamplerState::MipmapMode::kNone:    return MTLSamplerMipFilterNearest;
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
    // It seems like we should be able to leave this at the default of FLT_MAX but our
    // 10.13 MacBook10.1 bot started failing tests and hanging when we did that.
    samplerDesc.lodMaxClamp = samplerState.mipmapped() == GrMipmapped::kYes ? 1000.f : 0.f;

    samplerDesc.maxAnisotropy = 1.0f;
    samplerDesc.normalizedCoordinates = true;
    if (@available(macOS 10.11, iOS 9.0, *)) {
        samplerDesc.compareFunction = MTLCompareFunctionNever;
    }

    return new GrMtlSampler([gpu->device() newSamplerStateWithDescriptor: samplerDesc],
                            GenerateKey(samplerState));
}

GrMtlSampler::Key GrMtlSampler::GenerateKey(GrSamplerState samplerState) {
    return samplerState.asIndex();
}
