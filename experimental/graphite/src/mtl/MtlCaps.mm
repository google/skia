/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlCaps.h"

#include "experimental/graphite/include/TextureInfo.h"
#include "experimental/graphite/include/mtl/MtlTypes.h"
#include "experimental/graphite/src/CommandBuffer.h"
#include "experimental/graphite/src/GraphicsPipelineDesc.h"
#include "experimental/graphite/src/mtl/MtlUtils.h"
#include "src/sksl/SkSLUtil.h"

namespace skgpu::mtl {

Caps::Caps(const id<MTLDevice> device)
        : skgpu::Caps() {
    fShaderCaps = std::make_unique<SkSL::ShaderCaps>();

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
    if (this->isMac() || fFamilyGroup >= 3) {
        fMaxTextureSize = 16384;
    } else {
        fMaxTextureSize = 8192;
    }

    // We use constant address space for our uniform buffers which has various alignment
    // requirements for the offset when binding the buffer. On MacOS the offset must align to 256.
    // On iOS we must align to the max of the data type consumed by the vertex function or 4 bytes.
    // We can ignore the data type and just always use 16 bytes on iOS.
    if (this->isMac()) {
        fRequiredUniformBufferAlignment = 256;
    } else {
        fRequiredUniformBufferAlignment = 16;
    }

    if (@available(macOS 10.12, ios 14.0, *)) {
        fClampToBorderSupport = (this->isMac() || fFamilyGroup >= 7);
    } else {
        fClampToBorderSupport = false;
    }
}

void Caps::initShaderCaps() {
    SkSL::ShaderCaps* shaderCaps = fShaderCaps.get();

    // Setting this true with the assumption that this cap will eventually mean we support varying
    // precisions and not just via modifiers.
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;

    shaderCaps->fShaderDerivativeSupport = true;

    // TODO(skia:8270): Re-enable this once bug 8270 is fixed
#if 0
    if (this->isApple()) {
        shaderCaps->fFBFetchSupport = true;
        shaderCaps->fFBFetchNeedsCustomOutput = true; // ??
        shaderCaps->fFBFetchColorName = ""; // Somehow add [[color(0)]] to arguments to frag shader
    }
#endif

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fNonsquareMatrixSupport = true;
    shaderCaps->fInverseHyperbolicSupport = true;

    // Metal uses IEEE floats so assuming those values here.
    // TODO: add fHalfIs32Bits?
    shaderCaps->fFloatIs32Bits = true;
}

void Caps::initFormatTable() {
    // TODO
}

skgpu::TextureInfo Caps::getDefaultSampledTextureInfo(SkColorType colorType,
                                                      uint32_t levelCount,
                                                      Protected,
                                                      Renderable renderable) const {
    MTLTextureUsage usage = MTLTextureUsageShaderRead;
    if (renderable == Renderable::kYes) {
        usage |= MTLTextureUsageRenderTarget;
    }

    TextureInfo info;
    info.fSampleCount = 1;
    info.fLevelCount = levelCount;
    info.fFormat = SkColorTypeToFormat(colorType);
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

skgpu::TextureInfo Caps::getDefaultMSAATextureInfo(SkColorType colorType,
                                                   uint32_t sampleCount,
                                                   Protected) const {
    MTLTextureUsage usage = MTLTextureUsageRenderTarget;

    TextureInfo info;
    info.fSampleCount = sampleCount;
    info.fLevelCount = 1;
    info.fFormat = SkColorTypeToFormat(colorType);
    info.fUsage = usage;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

skgpu::TextureInfo Caps::getDefaultDepthStencilTextureInfo(Mask<DepthStencilFlags> depthStencilType,
                                                           uint32_t sampleCount,
                                                           Protected) const {
    TextureInfo info;
    info.fSampleCount = sampleCount;
    info.fLevelCount = 1;
    info.fFormat = DepthStencilFlagsToFormat(depthStencilType);
    info.fUsage = MTLTextureUsageRenderTarget;
    info.fStorageMode = MTLStorageModePrivate;
    info.fFramebufferOnly = false;

    return info;
}

UniqueKey Caps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                        const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        static const skgpu::UniqueKey::Domain kGraphicsPipelineDomain = UniqueKey::GenerateDomain();
        SkSpan<const uint32_t> pipelineDescKey = pipelineDesc.asKey();
        UniqueKey::Builder builder(&pipelineKey, kGraphicsPipelineDomain,
                                   pipelineDescKey.size() + 1, "GraphicsPipeline");
        // add graphicspipelinedesc key
        for (unsigned int i = 0; i < pipelineDescKey.size(); ++i) {
            builder[i] = pipelineDescKey[i];
        }
        // add renderpassdesc key
        mtl::TextureInfo colorInfo, depthStencilInfo;
        renderPassDesc.fColorAttachment.fTextureInfo.getMtlTextureInfo(&colorInfo);
        renderPassDesc.fDepthStencilAttachment.fTextureInfo.getMtlTextureInfo(&depthStencilInfo);
        SkASSERT(colorInfo.fFormat < 65535 && depthStencilInfo.fFormat < 65535);
        uint32_t renderPassKey = colorInfo.fFormat << 16 | depthStencilInfo.fFormat;
        builder[pipelineDescKey.size()] = renderPassKey;
        builder.finish();
    }

    return pipelineKey;
}

bool Caps::onIsTexturable(const skgpu::TextureInfo& info) const {
    if (!(info.mtlTextureSpec().fUsage & MTLTextureUsageShaderRead)) {
        return false;
    }
    if (info.mtlTextureSpec().fFramebufferOnly) {
        return false;
    }
    return this->isTexturable((MTLPixelFormat)info.mtlTextureSpec().fFormat);
}

bool Caps::isTexturable(MTLPixelFormat format) const {
    // TODO: Fill out format table so that we can query all formats. For now we only support RGBA8
    // and BGRA8 which is supported everywhere.
    return format == MTLPixelFormatRGBA8Unorm || format == MTLPixelFormatBGRA8Unorm;
}

bool Caps::isRenderable(const skgpu::TextureInfo& info) const {
    return info.mtlTextureSpec().fUsage & MTLTextureUsageRenderTarget &&
    this->isRenderable((MTLPixelFormat)info.mtlTextureSpec().fFormat, info.numSamples());
}

bool Caps::isRenderable(MTLPixelFormat format, uint32_t numSamples) const {
    // TODO: Fill out format table so that we can query all formats. For now we only support RGBA8
    // and BGRA8 with a sampleCount of 1 which is supported everywhere.
    if ((format != MTLPixelFormatRGBA8Unorm && format != MTLPixelFormatBGRA8Unorm) ||
        numSamples != 1) {
        return false;
    }
    return true;
}


bool Caps::onAreColorTypeAndTextureInfoCompatible(SkColorType type,
                                                  const skgpu::TextureInfo& info) const {
    // TODO: Fill out format table so that we can query all formats. For now we only support RGBA8
    // or BGRA8 for both the color type and format.
    return (type == kRGBA_8888_SkColorType &&
            info.mtlTextureSpec().fFormat == MTLPixelFormatRGBA8Unorm) ||
           (type == kBGRA_8888_SkColorType &&
            info.mtlTextureSpec().fFormat == MTLPixelFormatBGRA8Unorm);
}

} // namespace skgpu::mtl
