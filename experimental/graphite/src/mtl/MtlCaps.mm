/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCaps.h"

#include "experimental/graphite/include/TextureInfo.h"
#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"

namespace skgpu::mtl {

Caps::Caps(const id<MTLDevice> device)
        : skgpu::Caps() {
    // TODO: allocate shadercaps

    this->initGPUFamily(device);
    this->initCaps(device);
    this->initShaderCaps();

    this->initFormatTable();

    // Metal-specific caps
}

// translates from older MTLFeatureSet interface to MTLGPUFamily interface
bool Caps::GetGPUFamilyFromFeatureSet(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
#if defined(SK_BUILD_FOR_MAC)
    // Apple Silicon is only available in later OSes
    *gpuFamily = GPUFamily::kMac;
    // Mac OSX 14
    if (@available(macOS 10.14, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily2_v1]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v4]) {
            *group = 1;
            return true;
        }
    }
    // Mac OSX 13
    if (@available(macOS 10.13, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v3]) {
            *group = 1;
            return true;
        }
    }
    // Mac OSX 12
    if (@available(macOS 10.12, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v2]) {
            *group = 1;
            return true;
        }
    }
    // Mac OSX 11
    if (@available(macOS 10.11, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v1]) {
            *group = 1;
            return true;
        }
    }
#elif defined(SK_BUILD_FOR_IOS)
    // TODO: support tvOS
   *gpuFamily = GPUFamily::kApple;
    // iOS 12
    if (@available(iOS 12.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily5_v1]) {
            *group = 5;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v2]) {
            *group = 4;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v4]) {
            *group = 3;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v5]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v5]) {
            *group = 1;
            return true;
        }
    }
    // iOS 11
    if (@available(iOS 11.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1]) {
            *group = 4;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v3]) {
            *group = 3;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v4]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v4]) {
            *group = 1;
            return true;
        }
    }
    // iOS 10
    if (@available(iOS 10.0, *)) {
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v2]) {
            *group = 3;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v3]) {
            *group = 2;
            return true;
        }
        if ([device supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v3]) {
            *group = 1;
            return true;
        }
    }
    // We don't support earlier OSes
#endif

    // No supported GPU families were found
    return false;
}

bool Caps::GetGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group) {
#if GR_METAL_SDK_VERSION >= 220
    if (@available(macOS 10.15, iOS 13.0, tvOS 13.0, *)) {
        // Apple Silicon
#if GR_METAL_SDK_VERSION >= 230
        if ([device supportsFamily:MTLGPUFamilyApple7]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 7;
            return true;
        }
#endif
#ifdef SK_BUILD_FOR_IOS
        if ([device supportsFamily:MTLGPUFamilyApple6]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 6;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple5]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 5;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple4]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 4;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple3]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 3;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple2]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 2;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyApple1]) {
            *gpuFamily = GPUFamily::kApple;
            *group = 1;
            return true;
        }
#endif

        // Older Macs
        // At the moment MacCatalyst families have the same features as Mac,
        // so we treat them the same
        if ([device supportsFamily:MTLGPUFamilyMac2] ||
            [device supportsFamily:MTLGPUFamilyMacCatalyst2]) {
            *gpuFamily = GPUFamily::kMac;
            *group = 2;
            return true;
        }
        if ([device supportsFamily:MTLGPUFamilyMac1] ||
            [device supportsFamily:MTLGPUFamilyMacCatalyst1]) {
            *gpuFamily = GPUFamily::kMac;
            *group = 1;
            return true;
        }
    }
#endif

    // No supported GPU families were found
    return false;
}

void Caps::initGPUFamily(id<MTLDevice> device) {
    if (!GetGPUFamily(device, &fGPUFamily, &fFamilyGroup) &&
        !GetGPUFamilyFromFeatureSet(device, &fGPUFamily, &fFamilyGroup)) {
        // We don't know what this is, fall back to minimum defaults
#ifdef SK_BUILD_FOR_MAC
        fGPUFamily = GPUFamily::kMac;
        fFamilyGroup = 1;
#else
        fGPUFamily = GPUFamily::kApple;
        fFamilyGroup = 1;
#endif
    }
}

void Caps::initCaps(const id<MTLDevice> device) {
    // TODO
}

void Caps::initShaderCaps() {
    // TODO
}

void Caps::initFormatTable() {
    // TODO
}

skgpu::TextureInfo Caps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                      uint32_t sampleCount,
                                                      uint32_t levelCount,
                                                      Protected,
                                                      Renderable renderable) {
    MTLTextureUsage usage = MTLTextureUsageShaderRead;
    if (renderable == Renderable::kYes) {
        usage |= MTLTextureUsageRenderTarget;
    }

    TextureInfo info;
    info.fSampleCount = sampleCount;
    info.fLevelCount = levelCount;
    info.fFormat = SkColorTypeToFormat(colorType);
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;

    return info;
}

skgpu::TextureInfo Caps::getDefaultDepthStencilTextureInfo(DepthStencilType depthStencilType,
                                                           uint32_t sampleCount,
                                                           Protected) {
    TextureInfo info;
    info.fSampleCount = sampleCount;
    info.fLevelCount = 1;
    info.fFormat = DepthStencilTypeToFormat(depthStencilType);
    info.fUsage = MTLTextureUsageRenderTarget;
    info.fStorageMode = MTLStorageModePrivate;

    return info;
}




} // namespace skgpu::mtl
