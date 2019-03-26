/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkCaps.h"
#include "GrBackendSurface.h"
#include "GrRenderTargetProxy.h"
#include "GrRenderTarget.h"
#include "GrShaderCaps.h"
#include "GrVkInterface.h"
#include "GrVkTexture.h"
#include "GrVkUtil.h"
#include "SkGr.h"
#include "vk/GrVkBackendContext.h"
#include "vk/GrVkExtensions.h"

GrVkCaps::GrVkCaps(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                   VkPhysicalDevice physDev, const VkPhysicalDeviceFeatures2& features,
                   uint32_t instanceVersion, uint32_t physicalDeviceVersion,
                   const GrVkExtensions& extensions)
    : INHERITED(contextOptions) {

    /**************************************************************************
     * GrCaps fields
     **************************************************************************/
    fMipMapSupport = true;   // always available in Vulkan
    fSRGBSupport = true;   // always available in Vulkan
    fNPOTTextureTileSupport = true;  // always available in Vulkan
    fDiscardRenderTargetSupport = true;
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fCompressedTexSubImageSupport = true;
    fOversizedStencilSupport = false; //TODO: figure this out
    fInstanceAttribSupport = true;

    fFenceSyncSupport = true;   // always available in Vulkan
    fCrossContextTextureSupport = true;
    fHalfFloatVertexAttributeSupport = true;

    fMapBufferFlags = kNone_MapFlags; //TODO: figure this out
    fBufferMapThreshold = SK_MaxS32;  //TODO: figure this out

    fMaxRenderTargetSize = 4096; // minimum required by spec
    fMaxTextureSize = 4096; // minimum required by spec

    fDynamicStateArrayGeometryProcessorTextureSupport = true;

    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    this->init(contextOptions, vkInterface, physDev, features, physicalDeviceVersion, extensions);
}

bool GrVkCaps::initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                                  GrSurfaceOrigin* origin, bool* rectsMustMatch,
                                  bool* disallowSubrect) const {
    // Vk doesn't use rectsMustMatch or disallowSubrect. Always return false.
    *rectsMustMatch = false;
    *disallowSubrect = false;

    // We can always succeed here with either a CopyImage (none msaa src) or ResolveImage (msaa).
    // For CopyImage we can make a simple texture, for ResolveImage we require the dst to be a
    // render target as well.
    *origin = src->origin();
    desc->fConfig = src->config();
    if (src->numColorSamples() > 1 || src->asTextureProxy()) {
        desc->fFlags = kRenderTarget_GrSurfaceFlag;
    } else {
        // Just going to use CopyImage here
        desc->fFlags = kNone_GrSurfaceFlags;
    }

    return true;
}

bool GrVkCaps::canCopyImage(GrPixelConfig dstConfig, int dstSampleCnt, GrSurfaceOrigin dstOrigin,
                            bool dstHasYcbcr, GrPixelConfig srcConfig, int srcSampleCnt,
                            GrSurfaceOrigin srcOrigin, bool srcHasYcbcr) const {
    if ((dstSampleCnt > 1 || srcSampleCnt > 1) && dstSampleCnt != srcSampleCnt) {
        return false;
    }

    if (dstHasYcbcr || srcHasYcbcr) {
        return false;
    }

    // We require that all vulkan GrSurfaces have been created with transfer_dst and transfer_src
    // as image usage flags.
    if (srcOrigin != dstOrigin || GrBytesPerPixel(srcConfig) != GrBytesPerPixel(dstConfig)) {
        return false;
    }

    if (this->shaderCaps()->configOutputSwizzle(srcConfig) !=
        this->shaderCaps()->configOutputSwizzle(dstConfig)) {
        return false;
    }

    return true;
}

bool GrVkCaps::canCopyAsBlit(GrPixelConfig dstConfig, int dstSampleCnt, bool dstIsLinear,
                             bool dstHasYcbcr, GrPixelConfig srcConfig, int srcSampleCnt,
                             bool srcIsLinear, bool srcHasYcbcr) const {
    // We require that all vulkan GrSurfaces have been created with transfer_dst and transfer_src
    // as image usage flags.
    if (!this->configCanBeDstofBlit(dstConfig, dstIsLinear) ||
        !this->configCanBeSrcofBlit(srcConfig, srcIsLinear)) {
        return false;
    }

    if (this->shaderCaps()->configOutputSwizzle(srcConfig) !=
        this->shaderCaps()->configOutputSwizzle(dstConfig)) {
        return false;
    }

    // We cannot blit images that are multisampled. Will need to figure out if we can blit the
    // resolved msaa though.
    if (dstSampleCnt > 1 || srcSampleCnt > 1) {
        return false;
    }

    if (dstHasYcbcr || srcHasYcbcr) {
        return false;
    }

    return true;
}

bool GrVkCaps::canCopyAsResolve(GrPixelConfig dstConfig, int dstSampleCnt,
                                GrSurfaceOrigin dstOrigin, bool dstHasYcbcr,
                                GrPixelConfig srcConfig, int srcSampleCnt,
                                GrSurfaceOrigin srcOrigin, bool srcHasYcbcr) const {
    // The src surface must be multisampled.
    if (srcSampleCnt <= 1) {
        return false;
    }

    // The dst must not be multisampled.
    if (dstSampleCnt > 1) {
        return false;
    }

    // Surfaces must have the same format.
    if (dstConfig != srcConfig) {
        return false;
    }

    // Surfaces must have the same origin.
    if (srcOrigin != dstOrigin) {
        return false;
    }

    if (dstHasYcbcr || srcHasYcbcr) {
        return false;
    }

    return true;
}

bool GrVkCaps::canCopyAsDraw(GrPixelConfig dstConfig, bool dstIsRenderable, bool dstHasYcbcr,
                             GrPixelConfig srcConfig, bool srcIsTextureable,
                             bool srcHasYcbcr) const {
    // TODO: Make copySurfaceAsDraw handle the swizzle
    if (this->shaderCaps()->configOutputSwizzle(srcConfig) !=
        this->shaderCaps()->configOutputSwizzle(dstConfig)) {
        return false;
    }

    // Make sure the dst is a render target and the src is a texture.
    if (!dstIsRenderable || !srcIsTextureable) {
        return false;
    }

    if (dstHasYcbcr) {
        return false;
    }

    return true;
}

bool GrVkCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    GrSurfaceOrigin dstOrigin = dst->origin();
    GrSurfaceOrigin srcOrigin = src->origin();

    GrPixelConfig dstConfig = dst->config();
    GrPixelConfig srcConfig = src->config();

    // TODO: Figure out a way to track if we've wrapped a linear texture in a proxy (e.g.
    // PromiseImage which won't get instantiated right away. Does this need a similar thing like the
    // tracking of external or rectangle textures in GL? For now we don't create linear textures
    // internally, and I don't believe anyone is wrapping them.
    bool srcIsLinear = false;
    bool dstIsLinear = false;

    int dstSampleCnt = 0;
    int srcSampleCnt = 0;
    if (const GrRenderTargetProxy* rtProxy = dst->asRenderTargetProxy()) {
        // Copying to or from render targets that wrap a secondary command buffer is not allowed
        // since they would require us to know the VkImage, which we don't have, as well as need us
        // to stop and start the VkRenderPass which we don't have access to.
        if (rtProxy->wrapsVkSecondaryCB()) {
            return false;
        }
        dstSampleCnt = rtProxy->numColorSamples();
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        // Copying to or from render targets that wrap a secondary command buffer is not allowed
        // since they would require us to know the VkImage, which we don't have, as well as need us
        // to stop and start the VkRenderPass which we don't have access to.
        if (rtProxy->wrapsVkSecondaryCB()) {
            return false;
        }
        srcSampleCnt = rtProxy->numColorSamples();
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTargetProxy()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTargetProxy()));

    bool dstHasYcbcr = false;
    if (auto ycbcr = dst->backendFormat().getVkYcbcrConversionInfo()) {
        if (ycbcr->isValid()) {
            dstHasYcbcr = true;
        }
    }

    bool srcHasYcbcr = false;
    if (auto ycbcr = src->backendFormat().getVkYcbcrConversionInfo()) {
        if (ycbcr->isValid()) {
            srcHasYcbcr = true;
        }
    }

    return this->canCopyImage(dstConfig, dstSampleCnt, dstOrigin, dstHasYcbcr,
                              srcConfig, srcSampleCnt, srcOrigin, srcHasYcbcr) ||
           this->canCopyAsBlit(dstConfig, dstSampleCnt, dstIsLinear, dstHasYcbcr,
                               srcConfig, srcSampleCnt, srcIsLinear, srcHasYcbcr) ||
           this->canCopyAsResolve(dstConfig, dstSampleCnt, dstOrigin, dstHasYcbcr,
                                  srcConfig, srcSampleCnt, srcOrigin, srcHasYcbcr) ||
           this->canCopyAsDraw(dstConfig, dstSampleCnt > 0, dstHasYcbcr,
                               srcConfig, SkToBool(src->asTextureProxy()), srcHasYcbcr);
}

template<typename T> T* get_extension_feature_struct(const VkPhysicalDeviceFeatures2& features,
                                                     VkStructureType type) {
    // All Vulkan structs that could be part of the features chain will start with the
    // structure type followed by the pNext pointer. We cast to the CommonVulkanHeader
    // so we can get access to the pNext for the next struct.
    struct CommonVulkanHeader {
        VkStructureType sType;
        void*           pNext;
    };

    void* pNext = features.pNext;
    while (pNext) {
        CommonVulkanHeader* header = static_cast<CommonVulkanHeader*>(pNext);
        if (header->sType == type) {
            return static_cast<T*>(pNext);
        }
        pNext = header->pNext;
    }
    return nullptr;
}

void GrVkCaps::init(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                    VkPhysicalDevice physDev, const VkPhysicalDeviceFeatures2& features,
                    uint32_t physicalDeviceVersion, const GrVkExtensions& extensions) {
    VkPhysicalDeviceProperties properties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceProperties(physDev, &properties));

    VkPhysicalDeviceMemoryProperties memoryProperties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceMemoryProperties(physDev, &memoryProperties));

    SkASSERT(physicalDeviceVersion <= properties.apiVersion);

    if (extensions.hasExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME, 1)) {
        fSupportsSwapchain = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions.hasExtension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, 1)) {
        fSupportsPhysicalDeviceProperties2 = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions.hasExtension(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, 1)) {
        fSupportsMemoryRequirements2 = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions.hasExtension(VK_KHR_BIND_MEMORY_2_EXTENSION_NAME, 1)) {
        fSupportsBindMemory2 = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions.hasExtension(VK_KHR_MAINTENANCE1_EXTENSION_NAME, 1)) {
        fSupportsMaintenance1 = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions.hasExtension(VK_KHR_MAINTENANCE2_EXTENSION_NAME, 1)) {
        fSupportsMaintenance2 = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        extensions.hasExtension(VK_KHR_MAINTENANCE3_EXTENSION_NAME, 1)) {
        fSupportsMaintenance3 = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        (extensions.hasExtension(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, 1) &&
         this->supportsMemoryRequirements2())) {
        fSupportsDedicatedAllocation = true;
    }

    if (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
        (extensions.hasExtension(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME, 1) &&
         this->supportsPhysicalDeviceProperties2() &&
         extensions.hasExtension(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME, 1) &&
         this->supportsDedicatedAllocation())) {
        fSupportsExternalMemory = true;
    }

#ifdef SK_BUILD_FOR_ANDROID
    // Currently Adreno devices are not supporting the QUEUE_FAMILY_FOREIGN_EXTENSION, so until they
    // do we don't explicitly require it here even the spec says it is required.
    if (extensions.hasExtension(
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, 2) &&
       /* extensions.hasExtension(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME, 1) &&*/
        this->supportsExternalMemory() &&
        this->supportsBindMemory2()) {
        fSupportsAndroidHWBExternalMemory = true;
        fSupportsAHardwareBufferImages = true;
    }
#endif

    auto ycbcrFeatures =
            get_extension_feature_struct<VkPhysicalDeviceSamplerYcbcrConversionFeatures>(
                    features,
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES);
    if (ycbcrFeatures && ycbcrFeatures->samplerYcbcrConversion &&
        fSupportsAndroidHWBExternalMemory &&
        (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
         (extensions.hasExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, 1) &&
          this->supportsMaintenance1() &&
          this->supportsBindMemory2() &&
          this->supportsMemoryRequirements2() &&
          this->supportsPhysicalDeviceProperties2()))) {
        fSupportsYcbcrConversion = true;
    }
    // We always push back the default GrVkYcbcrConversionInfo so that the case of no conversion
    // will return a key of 0.
    fYcbcrInfos.push_back(GrVkYcbcrConversionInfo());

    this->initGrCaps(vkInterface, physDev, properties, memoryProperties, features, extensions);
    this->initShaderCaps(properties, features);

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
#if defined(SK_CPU_X86)
        // We need to do this before initing the config table since it uses fSRGBSupport
        if (kImagination_VkVendor == properties.vendorID) {
            fSRGBSupport = false;
        }
#endif
    }

    if (kQualcomm_VkVendor == properties.vendorID) {
        // A "clear" load for the CCPR atlas runs faster on QC than a "discard" load followed by a
        // scissored clear.
        // On NVIDIA and Intel, the discard load followed by clear is faster.
        // TODO: Evaluate on ARM, Imagination, and ATI.
        fPreferFullscreenClears = true;
    }

    if (kQualcomm_VkVendor == properties.vendorID || kARM_VkVendor == properties.vendorID) {
        // On Qualcomm and ARM mapping a gpu buffer and doing both reads and writes to it is slow.
        // Thus for index and vertex buffers we will force to use a cpu side buffer and then copy
        // the whole buffer up to the gpu.
        fBufferMapThreshold = SK_MaxS32;
    }

    if (kQualcomm_VkVendor == properties.vendorID) {
        // On Qualcomm it looks like using vkCmdUpdateBuffer is slower than using a transfer buffer
        // even for small sizes.
        fAvoidUpdateBuffers = true;
    }

    if (kARM_VkVendor == properties.vendorID) {
        // ARM seems to do better with more fine triangles as opposed to using the sample mask.
        // (At least in our current round rect op.)
        fPreferTrianglesOverSampleMask = true;
    }

    this->initConfigTable(vkInterface, physDev, properties);
    this->initStencilFormat(vkInterface, physDev);

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(properties);
    }

    // On nexus player we disable suballocating VkImage memory since we've seen large slow downs on
    // bot run times.
    if (kImagination_VkVendor == properties.vendorID) {
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

    this->applyOptionsOverrides(contextOptions);
    fShaderCaps->applyOptionsOverrides(contextOptions);
}

void GrVkCaps::applyDriverCorrectnessWorkarounds(const VkPhysicalDeviceProperties& properties) {
    if (kQualcomm_VkVendor == properties.vendorID) {
        fMustDoCopiesFromOrigin = true;
    }

#if defined(SK_BUILD_FOR_WIN)
    if (kNvidia_VkVendor == properties.vendorID || kIntel_VkVendor == properties.vendorID) {
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

    // On Mali galaxy s7 we see lots of rendering issues when we suballocate VkImages.
    if (kARM_VkVendor == properties.vendorID) {
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    // GrCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

    if (kARM_VkVendor == properties.vendorID) {
        fInstanceAttribSupport = false;
        fAvoidWritePixelsFastPath = true; // bugs.skia.org/8064
    }

    // AMD advertises support for MAX_UINT vertex input attributes, but in reality only supports 32.
    if (kAMD_VkVendor == properties.vendorID) {
        fMaxVertexAttributes = SkTMin(fMaxVertexAttributes, 32);
    }

    ////////////////////////////////////////////////////////////////////////////
    // GrShaderCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

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

void GrVkCaps::initGrCaps(const GrVkInterface* vkInterface,
                          VkPhysicalDevice physDev,
                          const VkPhysicalDeviceProperties& properties,
                          const VkPhysicalDeviceMemoryProperties& memoryProperties,
                          const VkPhysicalDeviceFeatures2& features,
                          const GrVkExtensions& extensions) {
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
    if (fDriverBugWorkarounds.max_texture_size_limit_4096) {
        fMaxTextureSize = SkTMin(fMaxTextureSize, 4096);
    }
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fMaxRenderTargetSize = SkTMin(fMaxTextureSize, fMaxRenderTargetSize);

    // TODO: check if RT's larger than 4k incur a performance cost on ARM.
    fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;

    // Assuming since we will always map in the end to upload the data we might as well just map
    // from the get go. There is no hard data to suggest this is faster or slower.
    fBufferMapThreshold = 0;

    fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag;

    fOversizedStencilSupport = true;

    if (extensions.hasExtension(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME, 2) &&
        this->supportsPhysicalDeviceProperties2()) {

        VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blendProps;
        blendProps.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT;
        blendProps.pNext = nullptr;

        VkPhysicalDeviceProperties2 props;
        props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        props.pNext = &blendProps;

        GR_VK_CALL(vkInterface, GetPhysicalDeviceProperties2(physDev, &props));

        if (blendProps.advancedBlendAllOperations == VK_TRUE) {
            fShaderCaps->fAdvBlendEqInteraction = GrShaderCaps::kAutomatic_AdvBlendEqInteraction;

            auto blendFeatures =
                get_extension_feature_struct<VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT>(
                    features,
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT);
            if (blendFeatures && blendFeatures->advancedBlendCoherentOperations == VK_TRUE) {
                fBlendEquationSupport = kAdvancedCoherent_BlendEquationSupport;
            } else {
                // TODO: Currently non coherent blends are not supported in our vulkan backend. They
                // require us to support self dependencies in our render passes.
                // fBlendEquationSupport = kAdvanced_BlendEquationSupport;
            }
        }
    }
}

void GrVkCaps::initShaderCaps(const VkPhysicalDeviceProperties& properties,
                              const VkPhysicalDeviceFeatures2& features) {
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
            } else if (kRGB_888X_GrPixelConfig == config) {
                shaderCaps->fConfigTextureSwizzle[i] = GrSwizzle::RGB1();
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

    // FIXME: http://skbug.com/7733: Disable geometry shaders until Intel/Radeon GMs draw correctly.
    // shaderCaps->fGeometryShaderSupport =
    //         shaderCaps->fGSInvocationsSupport = features.features.geometryShader;

    shaderCaps->fDualSourceBlendingSupport = features.features.dualSrcBlend;

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fVertexIDSupport = true;
    shaderCaps->fFPManipulationSupport = true;

    // Assume the minimum precisions mandated by the SPIR-V spec.
    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits = false;

    // SPIR-V supports unsigned integers.
    shaderCaps->fUnsignedSupport = true;

    shaderCaps->fMaxFragmentSamplers = SkTMin(
                                       SkTMin(properties.limits.maxPerStageDescriptorSampledImages,
                                              properties.limits.maxPerStageDescriptorSamplers),
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
        fPreferredStencilFormat = gS8;
    } else if (stencil_format_supported(interface, physDev, VK_FORMAT_D24_UNORM_S8_UINT)) {
        fPreferredStencilFormat = gD24S8;
    } else {
        SkASSERT(stencil_format_supported(interface, physDev, VK_FORMAT_D32_SFLOAT_S8_UINT));
        fPreferredStencilFormat = gD32S8;
    }
}

void GrVkCaps::initConfigTable(const GrVkInterface* interface, VkPhysicalDevice physDev,
                               const VkPhysicalDeviceProperties& properties) {
    for (int i = 0; i < kGrPixelConfigCnt; ++i) {
        VkFormat format;
        if (GrPixelConfigToVkFormat(static_cast<GrPixelConfig>(i), &format)) {
            if (!GrPixelConfigIsSRGB(static_cast<GrPixelConfig>(i)) || fSRGBSupport) {
                bool disableRendering = false;
                if (static_cast<GrPixelConfig>(i) == kRGB_888X_GrPixelConfig) {
                    // Currently we don't allow RGB_888X to be renderable because we don't have a
                    // way to handle blends that reference dst alpha when the values in the dst
                    // alpha channel are uninitialized.
                    disableRendering = true;
                }
                fConfigTable[i].init(interface, physDev, properties, format, disableRendering);
            }
        }
    }
}

void GrVkCaps::ConfigInfo::InitConfigFlags(VkFormatFeatureFlags vkFlags, uint16_t* flags,
                                           bool disableRendering) {
    if (SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & vkFlags) &&
        SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & vkFlags)) {
        *flags = *flags | kTextureable_Flag;

        // Ganesh assumes that all renderable surfaces are also texturable
        if (SkToBool(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & vkFlags) & !disableRendering) {
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
    VkImageFormatProperties properties;
    GR_VK_CALL(interface, GetPhysicalDeviceImageFormatProperties(physDev,
                                                                 format,
                                                                 VK_IMAGE_TYPE_2D,
                                                                 VK_IMAGE_TILING_OPTIMAL,
                                                                 usage,
                                                                 0,  // createFlags
                                                                 &properties));
    VkSampleCountFlags flags = properties.sampleCounts;
    if (flags & VK_SAMPLE_COUNT_1_BIT) {
        fColorSampleCounts.push_back(1);
    }
    if (kImagination_VkVendor == physProps.vendorID) {
        // MSAA does not work on imagination
        return;
    }
    if (flags & VK_SAMPLE_COUNT_2_BIT) {
        fColorSampleCounts.push_back(2);
    }
    if (flags & VK_SAMPLE_COUNT_4_BIT) {
        fColorSampleCounts.push_back(4);
    }
    if (flags & VK_SAMPLE_COUNT_8_BIT) {
        fColorSampleCounts.push_back(8);
    }
    if (flags & VK_SAMPLE_COUNT_16_BIT) {
        fColorSampleCounts.push_back(16);
    }
    if (flags & VK_SAMPLE_COUNT_32_BIT) {
        fColorSampleCounts.push_back(32);
    }
    if (flags & VK_SAMPLE_COUNT_64_BIT) {
        fColorSampleCounts.push_back(64);
    }
}

void GrVkCaps::ConfigInfo::init(const GrVkInterface* interface,
                                VkPhysicalDevice physDev,
                                const VkPhysicalDeviceProperties& properties,
                                VkFormat format,
                                bool disableRendering) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    InitConfigFlags(props.linearTilingFeatures, &fLinearFlags, disableRendering);
    InitConfigFlags(props.optimalTilingFeatures, &fOptimalFlags, disableRendering);
    if (fOptimalFlags & kRenderable_Flag) {
        this->initSampleCounts(interface, physDev, properties, format);
    }
}

int GrVkCaps::getRenderTargetSampleCount(int requestedCount, GrPixelConfig config) const {
    requestedCount = SkTMax(1, requestedCount);
    int count = fConfigTable[config].fColorSampleCounts.count();

    if (!count) {
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

int GrVkCaps::maxRenderTargetSampleCount(GrPixelConfig config) const {
    const auto& table = fConfigTable[config].fColorSampleCounts;
    if (!table.count()) {
        return 0;
    }
    return table[table.count() - 1];
}

bool GrVkCaps::surfaceSupportsReadPixels(const GrSurface* surface) const {
    if (auto tex = static_cast<const GrVkTexture*>(surface->asTexture())) {
        // We can't directly read from a VkImage that has a ycbcr sampler.
        if (tex->ycbcrConversionInfo().isValid()) {
            return false;
        }
    }
    return true;
}

bool GrVkCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    if (auto rt = surface->asRenderTarget()) {
        return rt->numColorSamples() <= 1 && SkToBool(surface->asTexture());
    }
    // We can't write to a texture that has a ycbcr sampler.
    if (auto tex = static_cast<const GrVkTexture*>(surface->asTexture())) {
        // We can't directly read from a VkImage that has a ycbcr sampler.
        if (tex->ycbcrConversionInfo().isValid()) {
            return false;
        }
    }
    return true;
}

static GrPixelConfig validate_image_info(VkFormat format, SkColorType ct, bool hasYcbcrConversion) {
    if (format == VK_FORMAT_UNDEFINED) {
        // If the format is undefined then it is only valid as an external image which requires that
        // we have a valid VkYcbcrConversion.
        if (hasYcbcrConversion) {
            // We don't actually care what the color type or config are since we won't use those
            // values for external textures. However, for read pixels we will draw to a non ycbcr
            // texture of this config so we set RGBA here for that.
            return kRGBA_8888_GrPixelConfig;
        } else {
            return kUnknown_GrPixelConfig;
        }
    }

    if (hasYcbcrConversion) {
        // We only support having a ycbcr conversion for external images.
        return kUnknown_GrPixelConfig;
    }

    switch (ct) {
        case kUnknown_SkColorType:
            break;
        case kAlpha_8_SkColorType:
            if (VK_FORMAT_R8_UNORM == format) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGB_565_SkColorType:
            if (VK_FORMAT_R5G6B5_UNORM_PACK16 == format) {
                return kRGB_565_GrPixelConfig;
            }
            break;
        case kARGB_4444_SkColorType:
            if (VK_FORMAT_B4G4R4A4_UNORM_PACK16 == format) {
                return kRGBA_4444_GrPixelConfig;
            }
            break;
        case kRGBA_8888_SkColorType:
            if (VK_FORMAT_R8G8B8A8_UNORM == format) {
                return kRGBA_8888_GrPixelConfig;
            } else if (VK_FORMAT_R8G8B8A8_SRGB == format) {
                return kSRGBA_8888_GrPixelConfig;
            }
            break;
        case kRGB_888x_SkColorType:
            if (VK_FORMAT_R8G8B8_UNORM == format) {
                return kRGB_888_GrPixelConfig;
            }
            if (VK_FORMAT_R8G8B8A8_UNORM == format) {
                return kRGB_888X_GrPixelConfig;
            }
            break;
        case kBGRA_8888_SkColorType:
            if (VK_FORMAT_B8G8R8A8_UNORM == format) {
                return kBGRA_8888_GrPixelConfig;
            } else if (VK_FORMAT_B8G8R8A8_SRGB == format) {
                return kSBGRA_8888_GrPixelConfig;
            }
            break;
        case kRGBA_1010102_SkColorType:
            if (VK_FORMAT_A2B10G10R10_UNORM_PACK32 == format) {
                return kRGBA_1010102_GrPixelConfig;
            }
            break;
        case kRGB_101010x_SkColorType:
            return kUnknown_GrPixelConfig;
        case kGray_8_SkColorType:
            if (VK_FORMAT_R8_UNORM == format) {
                return kGray_8_as_Red_GrPixelConfig;
            }
            break;
        case kRGBA_F16Norm_SkColorType:
            if (VK_FORMAT_R16G16B16A16_SFLOAT == format) {
                return kRGBA_half_Clamped_GrPixelConfig;
            }
            break;
        case kRGBA_F16_SkColorType:
            if (VK_FORMAT_R16G16B16A16_SFLOAT == format) {
                return kRGBA_half_GrPixelConfig;
            }
            break;
        case kRGBA_F32_SkColorType:
            if (VK_FORMAT_R32G32B32A32_SFLOAT == format) {
                return kRGBA_float_GrPixelConfig;
            }
            break;
    }

    return kUnknown_GrPixelConfig;
}

GrPixelConfig GrVkCaps::validateBackendRenderTarget(const GrBackendRenderTarget& rt,
                                                    SkColorType ct) const {
    GrVkImageInfo imageInfo;
    if (!rt.getVkImageInfo(&imageInfo)) {
        return kUnknown_GrPixelConfig;
    }
    return validate_image_info(imageInfo.fFormat, ct, imageInfo.fYcbcrConversionInfo.isValid());
}

GrPixelConfig GrVkCaps::getConfigFromBackendFormat(const GrBackendFormat& format,
                                                   SkColorType ct) const {
    const VkFormat* vkFormat = format.getVkFormat();
    const GrVkYcbcrConversionInfo* ycbcrInfo = format.getVkYcbcrConversionInfo();
    if (!vkFormat || !ycbcrInfo) {
        return kUnknown_GrPixelConfig;
    }
    return validate_image_info(*vkFormat, ct, ycbcrInfo->isValid());
}

static GrPixelConfig get_yuva_config(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8_UNORM:
            return kAlpha_8_as_Red_GrPixelConfig;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return kRGBA_8888_GrPixelConfig;
        case VK_FORMAT_R8G8B8_UNORM:
            return kRGB_888_GrPixelConfig;
        case VK_FORMAT_R8G8_UNORM:
            return kRG_88_GrPixelConfig;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return kBGRA_8888_GrPixelConfig;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return kRGBA_1010102_GrPixelConfig;
        default:
            return kUnknown_GrPixelConfig;
    }
}

GrPixelConfig GrVkCaps::getYUVAConfigFromBackendFormat(const GrBackendFormat& format) const {
    const VkFormat* vkFormat = format.getVkFormat();
    if (!vkFormat) {
        return kUnknown_GrPixelConfig;
    }
    return get_yuva_config(*vkFormat);
}

GrBackendFormat GrVkCaps::getBackendFormatFromGrColorType(GrColorType ct,
                                                          GrSRGBEncoded srgbEncoded) const {
    GrPixelConfig config = GrColorTypeToPixelConfig(ct, srgbEncoded);
    if (config == kUnknown_GrPixelConfig) {
        return GrBackendFormat();
    }
    VkFormat format;
    if (!GrPixelConfigToVkFormat(config, &format)) {
        return GrBackendFormat();
    }
    return GrBackendFormat::MakeVk(format);
}

