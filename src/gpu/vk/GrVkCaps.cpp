/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkCaps.h"

#include "GrVkUtil.h"
#include "glsl/GrGLSLCaps.h"
#include "vk/GrVkInterface.h"
#include "vk/GrVkBackendContext.h"

GrVkCaps::GrVkCaps(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                   VkPhysicalDevice physDev, uint32_t featureFlags, uint32_t extensionFlags)
    : INHERITED(contextOptions) {
    fCanUseGLSLForShaderModule = false;

    /**************************************************************************
    * GrDrawTargetCaps fields
    **************************************************************************/
    fMipMapSupport = false; //TODO: figure this out
    fNPOTTextureTileSupport = false; //TODO: figure this out
    fTwoSidedStencilSupport = false; //TODO: figure this out
    fStencilWrapOpsSupport = false; //TODO: figure this out
    fDiscardRenderTargetSupport = false; //TODO: figure this out
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fCompressedTexSubImageSupport = false; //TODO: figure this out
    fOversizedStencilSupport = false; //TODO: figure this out

    fUseDrawInsteadOfClear = false; //TODO: figure this out

    fMapBufferFlags = kNone_MapFlags; //TODO: figure this out
    fBufferMapThreshold = SK_MaxS32;  //TODO: figure this out

    fMaxRenderTargetSize = 4096; // minimum required by spec
    fMaxTextureSize = 4096; // minimum required by spec
    fMaxColorSampleCount = 4; // minimum required by spec
    fMaxStencilSampleCount = 4; // minimum required by spec


    fShaderCaps.reset(new GrGLSLCaps(contextOptions));

    this->init(contextOptions, vkInterface, physDev, featureFlags, extensionFlags);
}

void GrVkCaps::init(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                    VkPhysicalDevice physDev, uint32_t featureFlags, uint32_t extensionFlags) {

    VkPhysicalDeviceProperties properties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceProperties(physDev, &properties));

    VkPhysicalDeviceMemoryProperties memoryProperties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceMemoryProperties(physDev, &memoryProperties));

    this->initGrCaps(properties, memoryProperties, featureFlags);
    this->initGLSLCaps(properties, featureFlags);
    this->initConfigTable(vkInterface, physDev);
    this->initStencilFormat(vkInterface, physDev);

    if (SkToBool(extensionFlags & kNV_glsl_shader_GrVkExtensionFlag)) {
        // Currently disabling this feature since it does not play well with validation layers which
        // expect a SPIR-V shader
        // fCanUseGLSLForShaderModule = true;
    }

    this->applyOptionsOverrides(contextOptions);
    GrGLSLCaps* glslCaps = static_cast<GrGLSLCaps*>(fShaderCaps.get());
    glslCaps->applyOptionsOverrides(contextOptions);
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

void GrVkCaps::initSampleCount(const VkPhysicalDeviceProperties& properties) {
    VkSampleCountFlags colorSamples = properties.limits.framebufferColorSampleCounts;
    VkSampleCountFlags stencilSamples = properties.limits.framebufferStencilSampleCounts;

    fMaxColorSampleCount = get_max_sample_count(colorSamples);
    fMaxStencilSampleCount = get_max_sample_count(stencilSamples);
}

void GrVkCaps::initGrCaps(const VkPhysicalDeviceProperties& properties,
                          const VkPhysicalDeviceMemoryProperties& memoryProperties,
                          uint32_t featureFlags) {
    fMaxVertexAttributes = properties.limits.maxVertexInputAttributes;
    // We could actually query and get a max size for each config, however maxImageDimension2D will
    // give the minimum max size across all configs. So for simplicity we will use that for now.
    fMaxRenderTargetSize = properties.limits.maxImageDimension2D;
    fMaxTextureSize = properties.limits.maxImageDimension2D;

    this->initSampleCount(properties);

    // Assuming since we will always map in the end to upload the data we might as well just map
    // from the get go. There is no hard data to suggest this is faster or slower.
    fBufferMapThreshold = 0;

    fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag;

    fStencilWrapOpsSupport = true;
    fOversizedStencilSupport = true;
    fSampleShadingSupport = SkToBool(featureFlags & kSampleRateShading_GrVkFeatureFlag);
}

void GrVkCaps::initGLSLCaps(const VkPhysicalDeviceProperties& properties,
                            uint32_t featureFlags) {
    GrGLSLCaps* glslCaps = static_cast<GrGLSLCaps*>(fShaderCaps.get());
    glslCaps->fVersionDeclString = "#version 330\n";


    // fConfigOutputSwizzle will default to RGBA so we only need to set it for alpha only config.
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        GrPixelConfig config = static_cast<GrPixelConfig>(i);
        if (GrPixelConfigIsAlphaOnly(config)) {
            glslCaps->fConfigTextureSwizzle[i] = GrSwizzle::RRRR();
            glslCaps->fConfigOutputSwizzle[i] = GrSwizzle::AAAA();
        } else {
            glslCaps->fConfigTextureSwizzle[i] = GrSwizzle::RGBA();
        }
    }

    // Vulkan is based off ES 3.0 so the following should all be supported
    glslCaps->fUsesPrecisionModifiers = true;
    glslCaps->fFlatInterpolationSupport = true;

    // GrShaderCaps

    glslCaps->fShaderDerivativeSupport = true;
    glslCaps->fGeometryShaderSupport = SkToBool(featureFlags & kGeometryShader_GrVkFeatureFlag);

    glslCaps->fDualSourceBlendingSupport = SkToBool(featureFlags & kDualSrcBlend_GrVkFeatureFlag);

    glslCaps->fIntegerSupport = true;

    glslCaps->fMaxVertexSamplers =
    glslCaps->fMaxGeometrySamplers =
    glslCaps->fMaxFragmentSamplers = SkTMin(properties.limits.maxPerStageDescriptorSampledImages,
                                            properties.limits.maxPerStageDescriptorSamplers);
    glslCaps->fMaxCombinedSamplers = SkTMin(properties.limits.maxDescriptorSetSampledImages,
                                            properties.limits.maxDescriptorSetSamplers);
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
    // VK_FORMAT_D24_UNORM_S8_UINT or VK_FORMAT_D24_SFLOAT_S8_UINT. VK_FORMAT_D32_SFLOAT_S8_UINT
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

void GrVkCaps::initConfigTable(const GrVkInterface* interface, VkPhysicalDevice physDev) {
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        VkFormat format;
        if (GrPixelConfigToVkFormat(static_cast<GrPixelConfig>(i), &format)) {
            fConfigTable[i].init(interface, physDev, format);
        }
    }
}

void GrVkCaps::ConfigInfo::InitConfigFlags(VkFormatFeatureFlags vkFlags, uint16_t* flags) {
    if (SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & vkFlags) &&
        SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & vkFlags)) {
        *flags = *flags | kTextureable_Flag;
    }

    if (SkToBool(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & vkFlags)) {
        *flags = *flags | kRenderable_Flag;
    }

    if (SkToBool(VK_FORMAT_FEATURE_BLIT_SRC_BIT & vkFlags)) {
        *flags = *flags | kBlitSrc_Flag;
    }

    if (SkToBool(VK_FORMAT_FEATURE_BLIT_DST_BIT & vkFlags)) {
        *flags = *flags | kBlitDst_Flag;
    }
}

void GrVkCaps::ConfigInfo::init(const GrVkInterface* interface,
                                VkPhysicalDevice physDev,
                                VkFormat format) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    InitConfigFlags(props.linearTilingFeatures, &fLinearFlags);
    InitConfigFlags(props.optimalTilingFeatures, &fOptimalFlags);
}
