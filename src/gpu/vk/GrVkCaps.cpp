/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkCaps.h"

#include "GrBackendSurface.h"
#include "GrRenderTargetProxy.h"
#include "GrShaderCaps.h"
#include "GrVkUtil.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkInterface.h"

GrVkCaps::GrVkCaps(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                   VkPhysicalDevice physDev, uint32_t featureFlags, uint32_t extensionFlags)
    : INHERITED(contextOptions) {
    fCanUseGLSLForShaderModule = false;
    fMustDoCopiesFromOrigin = false;
    fMustSubmitCommandsBeforeCopyOp = false;
    fMustSleepOnTearDown  = false;
    fNewCBOnPipelineChange = false;
    fCanUseWholeSizeOnFlushMappedMemory = true;

    /**************************************************************************
    * GrDrawTargetCaps fields
    **************************************************************************/
    fMipMapSupport = true;   // always available in Vulkan
    fSRGBSupport = true;   // always available in Vulkan
    fSRGBDecodeDisableSupport = true;  // always available in Vulkan
    fNPOTTextureTileSupport = true;  // always available in Vulkan
    fDiscardRenderTargetSupport = true;
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fOversizedStencilSupport = false; //TODO: figure this out
    fInstanceAttribSupport = true;

    fBlacklistCoverageCounting = true; // blacklisting ccpr until we work through a few issues.
    fFenceSyncSupport = true;   // always available in Vulkan
    fCrossContextTextureSupport = true;

    fMapBufferFlags = kNone_MapFlags; //TODO: figure this out
    fBufferMapThreshold = SK_MaxS32;  //TODO: figure this out

    fMaxRenderTargetSize = 4096; // minimum required by spec
    fMaxTextureSize = 4096; // minimum required by spec

    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    this->init(contextOptions, vkInterface, physDev, featureFlags, extensionFlags);
}

bool GrVkCaps::initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                                  bool* rectsMustMatch, bool* disallowSubrect) const {
    // Vk doesn't use rectsMustMatch or disallowSubrect. Always return false.
    *rectsMustMatch = false;
    *disallowSubrect = false;

    // We can always succeed here with either a CopyImage (none msaa src) or ResolveImage (msaa).
    // For CopyImage we can make a simple texture, for ResolveImage we require the dst to be a
    // render target as well.
    desc->fOrigin = src->origin();
    desc->fConfig = src->config();
    if (src->numColorSamples() > 1 || src->asTextureProxy()) {
        desc->fFlags = kRenderTarget_GrSurfaceFlag;
    } else {
        // Just going to use CopyImage here
        desc->fFlags = kNone_GrSurfaceFlags;
    }

    return true;
}

void GrVkCaps::init(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                    VkPhysicalDevice physDev, uint32_t featureFlags, uint32_t extensionFlags) {

    VkPhysicalDeviceProperties properties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceProperties(physDev, &properties));

    VkPhysicalDeviceMemoryProperties memoryProperties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceMemoryProperties(physDev, &memoryProperties));

    this->initGrCaps(properties, memoryProperties, featureFlags);
    this->initShaderCaps(properties, featureFlags);

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
#if defined(SK_CPU_X86)
        // We need to do this before initing the config table since it uses fSRGBSupport
        if (kImagination_VkVendor == properties.vendorID) {
            fSRGBSupport = false;
        }
#endif
    }

    this->initConfigTable(vkInterface, physDev, properties);
    this->initStencilFormat(vkInterface, physDev);

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(properties);
    }

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);
}

void GrVkCaps::applyDriverCorrectnessWorkarounds(const VkPhysicalDeviceProperties& properties) {
    if (kQualcomm_VkVendor == properties.vendorID) {
        fMustDoCopiesFromOrigin = true;
    }

    if (kNvidia_VkVendor == properties.vendorID) {
        fMustSubmitCommandsBeforeCopyOp = true;
    }

#if defined(SK_BUILD_FOR_WIN)
    if (kNvidia_VkVendor == properties.vendorID) {
        fMustSleepOnTearDown = true;
    }
#elif defined(SK_BUILD_FOR_ANDROID)
    if (kImagination_VkVendor == properties.vendorID) {
        fMustSleepOnTearDown = true;
    }
#endif

    // AMD seems to have issues binding new VkPipelines inside a secondary command buffer.
    // Current workaround is to use a different secondary command buffer for each new VkPipeline.
    if (kAMD_VkVendor == properties.vendorID) {
        fNewCBOnPipelineChange = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    // GrCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

    if (kARM_VkVendor == properties.vendorID) {
        fInstanceAttribSupport = false;
    }

    // AMD advertises support for MAX_UINT vertex input attributes, but in reality only supports 32.
    if (kAMD_VkVendor == properties.vendorID) {
        fMaxVertexAttributes = SkTMin(fMaxVertexAttributes, 32);
    }

    if (kIntel_VkVendor == properties.vendorID) {
        fCanUseWholeSizeOnFlushMappedMemory = false;
    }

    ////////////////////////////////////////////////////////////////////////////
    // GrShaderCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

    if (kAMD_VkVendor == properties.vendorID) {
        // Currently DualSourceBlending is not working on AMD. vkCreateGraphicsPipeline fails when
        // using a draw with dual source. Looking into whether it is driver bug or issue with our
        // SPIR-V. Bug skia:6405
        fShaderCaps->fDualSourceBlendingSupport = false;
    }

    if (kImagination_VkVendor == properties.vendorID) {
        fShaderCaps->fAtan2ImplementedAsAtanYOverX = true;
    }

}

int get_max_sample_count(VkSampleCountFlags flags) {
    SkASSERT(flags & VK_SAMPLE_COUNT_1_BIT);
    if (!(flags & VK_SAMPLE_COUNT_2_BIT)) {
        return 0;
    }
    if (!(flags & VK_SAMPLE_COUNT_4_BIT)) {
        return 2;
    }
    if (!(flags & VK_SAMPLE_COUNT_8_BIT)) {
        return 4;
    }
    if (!(flags & VK_SAMPLE_COUNT_16_BIT)) {
        return 8;
    }
    if (!(flags & VK_SAMPLE_COUNT_32_BIT)) {
        return 16;
    }
    if (!(flags & VK_SAMPLE_COUNT_64_BIT)) {
        return 32;
    }
    return 64;
}

void GrVkCaps::initGrCaps(const VkPhysicalDeviceProperties& properties,
                          const VkPhysicalDeviceMemoryProperties& memoryProperties,
                          uint32_t featureFlags) {
    // So GPUs, like AMD, are reporting MAX_INT support vertex attributes. In general, there is no
    // need for us ever to support that amount, and it makes tests which tests all the vertex
    // attribs timeout looping over that many. For now, we'll cap this at 64 max and can raise it if
    // we ever find that need.
    static const uint32_t kMaxVertexAttributes = 64;
    fMaxVertexAttributes = SkTMin(properties.limits.maxVertexInputAttributes, kMaxVertexAttributes);

    // We could actually query and get a max size for each config, however maxImageDimension2D will
    // give the minimum max size across all configs. So for simplicity we will use that for now.
    fMaxRenderTargetSize = SkTMin(properties.limits.maxImageDimension2D, (uint32_t)INT_MAX);
    fMaxTextureSize = SkTMin(properties.limits.maxImageDimension2D, (uint32_t)INT_MAX);

    // Assuming since we will always map in the end to upload the data we might as well just map
    // from the get go. There is no hard data to suggest this is faster or slower.
    fBufferMapThreshold = 0;

    fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag;

    fOversizedStencilSupport = true;
    fSampleShadingSupport = SkToBool(featureFlags & kSampleRateShading_GrVkFeatureFlag);


}

void GrVkCaps::initShaderCaps(const VkPhysicalDeviceProperties& properties, uint32_t featureFlags) {
    GrShaderCaps* shaderCaps = fShaderCaps.get();
    shaderCaps->fVersionDeclString = "#version 330\n";


    // fConfigOutputSwizzle will default to RGBA so we only need to set it for alpha only config.
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        // Vulkan doesn't support a single channel format stored in alpha.
        if (GrPixelConfigIsAlphaOnly(config) &&
            kAlpha_8_as_Alpha_GrPixelConfig != config) {
            shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRR();
            shaderCaps->fConfigOutputSwizzle[i] = GrSwizzle::AAAA();
        } else {
            if (kGray_8_GrPixelConfig == config ||
                kGray_8_as_Red_GrPixelConfig == config) {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRA();
            } else if (kRGBA_4444_GrPixelConfig == config) {
                // The vulkan spec does not require R4G4B4A4 to be supported for texturing so we
                // store the data in a B4G4R4A4 texture and then swizzle it when doing texture reads
                // or writing to outputs. Since we're not actually changing the data at all, the
                // only extra work is the swizzle in the shader for all operations.
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::BGRA();
                shaderCaps->fConfigOutputSwizzle[i] = GrSwizzle::BGRA();
            } else {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RGBA();
            }
        }
    }

    // Vulkan is based off ES 3.0 so the following should all be supported
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;
    // Flat interpolation appears to be slow on Qualcomm GPUs. This was tested in GL and is assumed
    // to be true with Vulkan as well.
    shaderCaps->fPreferFlatInterpolation = kQualcomm_VkVendor != properties.vendorID;

    // GrShaderCaps

    shaderCaps->fShaderDerivativeSupport = true;

    shaderCaps->fGeometryShaderSupport = SkToBool(featureFlags & kGeometryShader_GrVkFeatureFlag);
    shaderCaps->fGSInvocationsSupport = shaderCaps->fGeometryShaderSupport;

    shaderCaps->fDualSourceBlendingSupport = SkToBool(featureFlags & kDualSrcBlend_GrVkFeatureFlag);

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fTexelBufferSupport = true;
    shaderCaps->fTexelFetchSupport = true;
    shaderCaps->fVertexIDSupport = true;

    // Assume the minimum precisions mandated by the SPIR-V spec.
    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits = false;

    shaderCaps->fMaxVertexSamplers =
    shaderCaps->fMaxGeometrySamplers =
    shaderCaps->fMaxFragmentSamplers = SkTMin(
                                       SkTMin(properties.limits.maxPerStageDescriptorSampledImages,
                                              properties.limits.maxPerStageDescriptorSamplers),
                                              (uint32_t)INT_MAX);
    shaderCaps->fMaxCombinedSamplers = SkTMin(
                                       SkTMin(properties.limits.maxDescriptorSetSampledImages,
                                              properties.limits.maxDescriptorSetSamplers),
                                              (uint32_t)INT_MAX);
}

bool stencil_format_supported(const GrVkInterface* interface,
                              VkPhysicalDevice physDev,
                              VkFormat format) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    return SkToBool(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & props.optimalTilingFeatures);
}

void GrVkCaps::initStencilFormat(const GrVkInterface* interface, VkPhysicalDevice physDev) {
    // List of legal stencil formats (though perhaps not supported on
    // the particular gpu/driver) from most preferred to least. We are guaranteed to have either
    // VK_FORMAT_D24_UNORM_S8_UINT or VK_FORMAT_D32_SFLOAT_S8_UINT. VK_FORMAT_D32_SFLOAT_S8_UINT
    // can optionally have 24 unused bits at the end so we assume the total bits is 64.
    static const StencilFormat
                  // internal Format             stencil bits      total bits        packed?
        gS8    = { VK_FORMAT_S8_UINT,            8,                 8,               false },
        gD24S8 = { VK_FORMAT_D24_UNORM_S8_UINT,  8,                32,               true },
        gD32S8 = { VK_FORMAT_D32_SFLOAT_S8_UINT, 8,                64,               true };

    if (stencil_format_supported(interface, physDev, VK_FORMAT_S8_UINT)) {
        fPreferedStencilFormat = gS8;
    } else if (stencil_format_supported(interface, physDev, VK_FORMAT_D24_UNORM_S8_UINT)) {
        fPreferedStencilFormat = gD24S8;
    } else {
        SkASSERT(stencil_format_supported(interface, physDev, VK_FORMAT_D32_SFLOAT_S8_UINT));
        fPreferedStencilFormat = gD32S8;
    }
}

void GrVkCaps::initConfigTable(const GrVkInterface* interface, VkPhysicalDevice physDev,
                               const VkPhysicalDeviceProperties& properties) {
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        VkFormat format;
        if (GrPixelConfigToVkFormat(static_cast<GrPixelConfig>(i), &format)) {
            if (!GrPixelConfigIsSRGB(static_cast<GrPixelConfig>(i)) || fSRGBSupport) {
                fConfigTable[i].init(interface, physDev, properties, format);
            }
        }
    }
}

void GrVkCaps::ConfigInfo::InitConfigFlags(VkFormatFeatureFlags vkFlags, uint16_t* flags) {
    if (SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & vkFlags) &&
        SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & vkFlags)) {
        *flags = *flags | kTextureable_Flag;

        // Ganesh assumes that all renderable surfaces are also texturable
        if (SkToBool(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & vkFlags)) {
            *flags = *flags | kRenderable_Flag;
        }
    }

    if (SkToBool(VK_FORMAT_FEATURE_BLIT_SRC_BIT & vkFlags)) {
        *flags = *flags | kBlitSrc_Flag;
    }

    if (SkToBool(VK_FORMAT_FEATURE_BLIT_DST_BIT & vkFlags)) {
        *flags = *flags | kBlitDst_Flag;
    }
}

void GrVkCaps::ConfigInfo::initSampleCounts(const GrVkInterface* interface,
                                            VkPhysicalDevice physDev,
                                            const VkPhysicalDeviceProperties& physProps,
                                            VkFormat format) {
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT |
                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    VkImageCreateFlags createFlags = GrVkFormatIsSRGB(format, nullptr)
        ? VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT : 0;
    VkImageFormatProperties properties;
    GR_VK_CALL(interface, GetPhysicalDeviceImageFormatProperties(physDev,
                                                                 format,
                                                                 VK_IMAGE_TYPE_2D,
                                                                 VK_IMAGE_TILING_OPTIMAL,
                                                                 usage,
                                                                 createFlags,
                                                                 &properties));
    VkSampleCountFlags flags = properties.sampleCounts;
    if (flags & VK_SAMPLE_COUNT_1_BIT) {
        fColorSampleCounts.push(1);
    }
    if (kImagination_VkVendor == physProps.vendorID) {
        // MSAA does not work on imagination
        return;
    }
    if (flags & VK_SAMPLE_COUNT_2_BIT) {
        fColorSampleCounts.push(2);
    }
    if (flags & VK_SAMPLE_COUNT_4_BIT) {
        fColorSampleCounts.push(4);
    }
    if (flags & VK_SAMPLE_COUNT_8_BIT) {
        fColorSampleCounts.push(8);
    }
    if (flags & VK_SAMPLE_COUNT_16_BIT) {
        fColorSampleCounts.push(16);
    }
    if (flags & VK_SAMPLE_COUNT_32_BIT) {
        fColorSampleCounts.push(32);
    }
    if (flags & VK_SAMPLE_COUNT_64_BIT) {
        fColorSampleCounts.push(64);
    }
}

void GrVkCaps::ConfigInfo::init(const GrVkInterface* interface,
                                VkPhysicalDevice physDev,
                                const VkPhysicalDeviceProperties& properties,
                                VkFormat format) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    InitConfigFlags(props.linearTilingFeatures, &fLinearFlags);
    InitConfigFlags(props.optimalTilingFeatures, &fOptimalFlags);
    if (fOptimalFlags & kRenderable_Flag) {
        this->initSampleCounts(interface, physDev, properties, format);
    }
}

bool GrVkCaps::isConfigRenderable(GrPixelConfig config, bool withMSAA) const {
    if (!SkToBool(ConfigInfo::kRenderable_Flag & fConfigTable[config].fOptimalFlags)) {
        return false;
    }
    if (withMSAA && fConfigTable[config].fColorSampleCounts.count() < 2) {
        // We expect any renderable config to support non-MSAA rendering.
        SkASSERT(1 == fConfigTable[config].fColorSampleCounts.count());
        SkASSERT(1 == fConfigTable[config].fColorSampleCounts[0]);
        return false;
    }
    return true;
}

int GrVkCaps::getSampleCount(int requestedCount, GrPixelConfig config) const {
    requestedCount = SkTMax(1, requestedCount);
    int count = fConfigTable[config].fColorSampleCounts.count();

    if (!count || !this->isConfigRenderable(config, requestedCount > 1)) {
        return 0;
    }

    if (1 == requestedCount) {
        SkASSERT(fConfigTable[config].fColorSampleCounts.count() &&
                 fConfigTable[config].fColorSampleCounts[0] == 1);
        return 1;
    }

    for (int i = 0; i < count; ++i) {
        if (fConfigTable[config].fColorSampleCounts[i] >= requestedCount) {
            return fConfigTable[config].fColorSampleCounts[i];
        }
    }
    return 0;
}

bool validate_image_info(const GrVkImageInfo* imageInfo, SkColorType ct, GrPixelConfig* config) {
    if (!imageInfo) {
        return false;
    }
    VkFormat format = imageInfo->fFormat;
    *config = kUnknown_GrPixelConfig;

    switch (ct) {
        case kUnknown_SkColorType:
            return false;
        case kAlpha_8_SkColorType:
            if (VK_FORMAT_R8_UNORM == format) {
                *config = kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGB_565_SkColorType:
            if (VK_FORMAT_R5G6B5_UNORM_PACK16 == format) {
                *config = kRGB_565_GrPixelConfig;
            }
            break;
        case kARGB_4444_SkColorType:
            if (VK_FORMAT_B4G4R4A4_UNORM_PACK16 == format) {
                *config = kRGBA_4444_GrPixelConfig;
            }
            break;
        case kRGBA_8888_SkColorType:
            if (VK_FORMAT_R8G8B8A8_UNORM == format) {
                *config = kRGBA_8888_GrPixelConfig;
            } else if (VK_FORMAT_R8G8B8A8_SRGB == format) {
                *config = kSRGBA_8888_GrPixelConfig;
            }
            break;
        case kRGB_888x_SkColorType:
            return false;
        case kBGRA_8888_SkColorType:
            if (VK_FORMAT_B8G8R8A8_UNORM == format) {
                *config = kBGRA_8888_GrPixelConfig;
            } else if (VK_FORMAT_B8G8R8A8_SRGB == format) {
                *config = kSBGRA_8888_GrPixelConfig;
            }
            break;
        case kRGBA_1010102_SkColorType:
            return false;
        case kRGB_101010x_SkColorType:
            return false;
        case kGray_8_SkColorType:
            if (VK_FORMAT_R8_UNORM == format) {
                *config = kGray_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGBA_F16_SkColorType:
            if (VK_FORMAT_R16G16B16A16_SFLOAT == format) {
                *config = kRGBA_half_GrPixelConfig;
            }
            break;
    }

    return kUnknown_GrPixelConfig != *config;
}

bool GrVkCaps::validateBackendTexture(const GrBackendTexture& tex, SkColorType ct,
                                      GrPixelConfig* config) const {
    return validate_image_info(tex.getVkImageInfo(), ct, config);
}

bool GrVkCaps::validateBackendRenderTarget(const GrBackendRenderTarget& rt, SkColorType ct,
                                           GrPixelConfig* config) const {
    return validate_image_info(rt.getVkImageInfo(), ct, config);
}

