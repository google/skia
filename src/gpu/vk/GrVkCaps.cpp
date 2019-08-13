/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrUtil.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/vk/GrVkCaps.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "src/gpu/vk/GrVkTexture.h"
#include "src/gpu/vk/GrVkUtil.h"

#ifdef SK_BUILD_FOR_ANDROID
#include <sys/system_properties.h>
#endif

GrVkCaps::GrVkCaps(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
                   VkPhysicalDevice physDev, const VkPhysicalDeviceFeatures2& features,
                   uint32_t instanceVersion, uint32_t physicalDeviceVersion,
                   const GrVkExtensions& extensions, GrProtected isProtected)
        : INHERITED(contextOptions) {
    /**************************************************************************
     * GrCaps fields
     **************************************************************************/
    fMipMapSupport = true;   // always available in Vulkan
    fSRGBSupport = true;   // always available in Vulkan
    fNPOTTextureTileSupport = true;  // always available in Vulkan
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fOversizedStencilSupport = false; //TODO: figure this out
    fInstanceAttribSupport = true;

    fSemaphoreSupport = true;   // always available in Vulkan
    fFenceSyncSupport = true;   // always available in Vulkan
    fCrossContextTextureSupport = true;
    fHalfFloatVertexAttributeSupport = true;

    // We always copy in/out of a transfer buffer so it's trivial to support row bytes.
    fReadPixelsRowBytesSupport = true;
    fWritePixelsRowBytesSupport = true;

    fTransferBufferSupport = true;

    fMaxRenderTargetSize = 4096; // minimum required by spec
    fMaxTextureSize = 4096; // minimum required by spec

    fDynamicStateArrayGeometryProcessorTextureSupport = true;

    fShaderCaps.reset(new GrShaderCaps(contextOptions));

    this->init(contextOptions, vkInterface, physDev, features, physicalDeviceVersion, extensions,
               isProtected);
}

namespace {
/**
 * This comes from section 37.1.6 of the Vulkan spec. Format is
 * (<bits>|<tag>)_<block_size>_<texels_per_block>.
 */
enum class FormatCompatibilityClass {
    k8_1_1,
    k16_2_1,
    k24_3_1,
    k32_4_1,
    k64_8_1,
    k128_16_1,
    kETC2_RGB_8_16,
};
}  // anonymous namespace

static FormatCompatibilityClass format_compatibility_class(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SFLOAT:
            return FormatCompatibilityClass::k32_4_1;

        case VK_FORMAT_R8_UNORM:
            return FormatCompatibilityClass::k8_1_1;

        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_R16_UNORM:
            return FormatCompatibilityClass::k16_2_1;

        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16G16B16A16_UNORM:
            return FormatCompatibilityClass::k64_8_1;

        case VK_FORMAT_R8G8B8_UNORM:
            return FormatCompatibilityClass::k24_3_1;

        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return FormatCompatibilityClass::k128_16_1;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return FormatCompatibilityClass::kETC2_RGB_8_16;

        default:
            SK_ABORT("Unsupported VkFormat");
    }
}

bool GrVkCaps::canCopyImage(VkFormat dstFormat, int dstSampleCnt, bool dstHasYcbcr,
                            VkFormat srcFormat, int srcSampleCnt, bool srcHasYcbcr) const {
    if ((dstSampleCnt > 1 || srcSampleCnt > 1) && dstSampleCnt != srcSampleCnt) {
        return false;
    }

    if (dstHasYcbcr || srcHasYcbcr) {
        return false;
    }

    // We require that all Vulkan GrSurfaces have been created with transfer_dst and transfer_src
    // as image usage flags.
    return format_compatibility_class(srcFormat) == format_compatibility_class(dstFormat);
}

bool GrVkCaps::canCopyAsBlit(VkFormat dstFormat, int dstSampleCnt, bool dstIsLinear,
                             bool dstHasYcbcr, VkFormat srcFormat, int srcSampleCnt,
                             bool srcIsLinear, bool srcHasYcbcr) const {
    // We require that all vulkan GrSurfaces have been created with transfer_dst and transfer_src
    // as image usage flags.
    if (!this->formatCanBeDstofBlit(dstFormat, dstIsLinear) ||
        !this->formatCanBeSrcofBlit(srcFormat, srcIsLinear)) {
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

bool GrVkCaps::canCopyAsResolve(VkFormat dstFormat, int dstSampleCnt, bool dstHasYcbcr,
                                VkFormat srcFormat, int srcSampleCnt, bool srcHasYcbcr) const {
    // The src surface must be multisampled.
    if (srcSampleCnt <= 1) {
        return false;
    }

    // The dst must not be multisampled.
    if (dstSampleCnt > 1) {
        return false;
    }

    // Surfaces must have the same format.
    if (srcFormat != dstFormat) {
        return false;
    }

    if (dstHasYcbcr || srcHasYcbcr) {
        return false;
    }

    return true;
}

bool GrVkCaps::onCanCopySurface(const GrSurfaceProxy* dst, const GrSurfaceProxy* src,
                                const SkIRect& srcRect, const SkIPoint& dstPoint) const {
    if (src->isProtected() && !dst->isProtected()) {
        return false;
    }

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
        dstSampleCnt = rtProxy->numSamples();
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        // Copying to or from render targets that wrap a secondary command buffer is not allowed
        // since they would require us to know the VkImage, which we don't have, as well as need us
        // to stop and start the VkRenderPass which we don't have access to.
        if (rtProxy->wrapsVkSecondaryCB()) {
            return false;
        }
        srcSampleCnt = rtProxy->numSamples();
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

    VkFormat dstFormat, srcFormat;
    SkAssertResult(dst->backendFormat().asVkFormat(&dstFormat));
    SkAssertResult(src->backendFormat().asVkFormat(&srcFormat));

    return this->canCopyImage(dstFormat, dstSampleCnt, dstHasYcbcr,
                              srcFormat, srcSampleCnt, srcHasYcbcr) ||
           this->canCopyAsBlit(dstFormat, dstSampleCnt, dstIsLinear, dstHasYcbcr,
                               srcFormat, srcSampleCnt, srcIsLinear, srcHasYcbcr) ||
           this->canCopyAsResolve(dstFormat, dstSampleCnt, dstHasYcbcr,
                                  srcFormat, srcSampleCnt, srcHasYcbcr);
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
                    uint32_t physicalDeviceVersion, const GrVkExtensions& extensions,
                    GrProtected isProtected) {
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
                    features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES);
    if (ycbcrFeatures && ycbcrFeatures->samplerYcbcrConversion &&
        (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0) ||
         (extensions.hasExtension(VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME, 1) &&
          this->supportsMaintenance1() && this->supportsBindMemory2() &&
          this->supportsMemoryRequirements2() && this->supportsPhysicalDeviceProperties2()))) {
        fSupportsYcbcrConversion = true;
    }

    // We always push back the default GrVkYcbcrConversionInfo so that the case of no conversion
    // will return a key of 0.
    fYcbcrInfos.push_back(GrVkYcbcrConversionInfo());

    if ((isProtected == GrProtected::kYes) &&
        (physicalDeviceVersion >= VK_MAKE_VERSION(1, 1, 0))) {
        fSupportsProtectedMemory = true;
        fAvoidUpdateBuffers = true;
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

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

    this->initFormatTable(vkInterface, physDev, properties);
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
        // Transfer doesn't support this workaround.
        fTransferBufferSupport = false;
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

#if defined(SK_BUILD_FOR_ANDROID)
    // Protected memory features have problems in Android P and earlier.
    if (fSupportsProtectedMemory && (kQualcomm_VkVendor == properties.vendorID)) {
        char androidAPIVersion[PROP_VALUE_MAX];
        int strLength = __system_property_get("ro.build.version.sdk", androidAPIVersion);
        if (strLength == 0 || atoi(androidAPIVersion) <= 28) {
            fSupportsProtectedMemory = false;
        }
    }
#endif

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

    fMapBufferFlags = kCanMap_MapFlag | kSubset_MapFlag | kAsyncRead_MapFlag;

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

static bool format_is_srgb(VkFormat format) {
    SkASSERT(GrVkFormatIsSupported(format));

    switch (format) {
        case VK_FORMAT_R8G8B8A8_SRGB:
            return true;
        default:
            return false;
    }
}

// These are all the valid VkFormats that we support in Skia. They are roughly ordered from most
// frequently used to least to improve look up times in arrays.
static constexpr VkFormat kVkFormats[] = {
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_R8_UNORM,
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R5G6B5_UNORM_PACK16,
    VK_FORMAT_R16G16B16A16_SFLOAT,
    VK_FORMAT_R16_SFLOAT,
    VK_FORMAT_R8G8B8_UNORM,
    VK_FORMAT_R8G8_UNORM,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32,
    VK_FORMAT_B4G4R4A4_UNORM_PACK16,
    VK_FORMAT_R4G4B4A4_UNORM_PACK16,
    VK_FORMAT_R32G32B32A32_SFLOAT,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
    VK_FORMAT_R16_UNORM,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    // Experimental (for Y416 and mutant P016/P010)
    VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16_SFLOAT,
};

const GrVkCaps::FormatInfo& GrVkCaps::getFormatInfo(VkFormat format) const {
    GrVkCaps* nonConstThis = const_cast<GrVkCaps*>(this);
    return nonConstThis->getFormatInfo(format);
}

GrVkCaps::FormatInfo& GrVkCaps::getFormatInfo(VkFormat format) {
    static_assert(SK_ARRAY_COUNT(kVkFormats) == GrVkCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");
    for (size_t i = 0; i < SK_ARRAY_COUNT(kVkFormats); ++i) {
        if (kVkFormats[i] == format) {
            return fFormatTable[i];
        }
    }
    static FormatInfo kInvalidFormat;
    return kInvalidFormat;
}

void GrVkCaps::initFormatTable(const GrVkInterface* interface, VkPhysicalDevice physDev,
                               const VkPhysicalDeviceProperties& properties) {
    static_assert(SK_ARRAY_COUNT(kVkFormats) == GrVkCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");

    // Go through all the formats and init their support surface and data GrColorTypes.

    // Format: VK_FORMAT_R8G8B8A8_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R8G8B8A8_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_R8G8B8A8_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGBA_8888
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_8888;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGB_888x
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGB_888x;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fTextureSwizzle = GrSwizzle::RGB1();
            }
        }
    }

    // Format: VK_FORMAT_R8_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R8_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_R8_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8_UNORM, Surface: kAlpha_8
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kAlpha_8;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fTextureSwizzle = GrSwizzle::RRRR();
                ctInfo.fOutputSwizzle = GrSwizzle::AAAA();
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kGray_8
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kGray_8;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fTextureSwizzle = GrSwizzle("rrr1");
            }
        }
    }
    // Format: VK_FORMAT_B8G8R8A8_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_B8G8R8A8_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_B8G8R8A8_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_B8G8R8A8_UNORM, Surface: kBGRA_8888
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kBGRA_8888;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R5G6B5_UNORM_PACK16
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R5G6B5_UNORM_PACK16);
        info.init(interface, physDev, properties, VK_FORMAT_R5G6B5_UNORM_PACK16);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R5G6B5_UNORM_PACK16, Surface: kBGR_565
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kBGR_565;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_SFLOAT
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R16G16B16A16_SFLOAT);
        info.init(interface, physDev, properties, VK_FORMAT_R16G16B16A16_SFLOAT);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: GrColorType::kRGBA_F16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_F16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: GrColorType::kRGBA_F16_Clamped
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_F16_Clamped;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_SFLOAT
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R16_SFLOAT);
        info.init(interface, physDev, properties, VK_FORMAT_R16_SFLOAT);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_SFLOAT, Surface: kAlpha_F16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kAlpha_F16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fTextureSwizzle = GrSwizzle::RRRR();
                ctInfo.fOutputSwizzle = GrSwizzle::AAAA();
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R8G8B8_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_R8G8B8_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8_UNORM, Surface: kRGB_888x
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGB_888x;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R8G8_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_R8G8_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8_UNORM, Surface: kRG_88
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRG_88;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32
    {
        auto& info = this->getFormatInfo(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
        info.init(interface, physDev, properties, VK_FORMAT_A2B10G10R10_UNORM_PACK32);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32, Surface: kRGBA_1010102
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_1010102;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16
    {
        auto& info = this->getFormatInfo(VK_FORMAT_B4G4R4A4_UNORM_PACK16);
        info.init(interface, physDev, properties, VK_FORMAT_B4G4R4A4_UNORM_PACK16);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16, Surface: kABGR_4444
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kABGR_4444;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fTextureSwizzle = GrSwizzle::BGRA();
                ctInfo.fOutputSwizzle = GrSwizzle::BGRA();
            }
        }
    }
    // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
        info.init(interface, physDev, properties, VK_FORMAT_R4G4B4A4_UNORM_PACK16);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16, Surface: kABGR_4444
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kABGR_4444;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R32G32B32A32_SFLOAT
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R32G32B32A32_SFLOAT);
        info.init(interface, physDev, properties, VK_FORMAT_R32G32B32A32_SFLOAT);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R32G32B32A32_SFLOAT, Surface: kRGBA_F32
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_F32;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8A8_SRGB
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R8G8B8A8_SRGB);
        if (fSRGBSupport) {
            info.init(interface, physDev, properties, VK_FORMAT_R8G8B8A8_SRGB);
        }
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_SRGB, Surface: kRGBA_8888_SRGB
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_8888_SRGB;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R16_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_R16_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_UNORM, Surface: kR_16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kR_16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R16G16_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_R16G16_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_UNORM, Surface: kRG_1616
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRG_1616;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R16G16B16A16_UNORM);
        info.init(interface, physDev, properties, VK_FORMAT_R16G16B16A16_UNORM);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_UNORM, Surface: kRGBA_16161616
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGBA_16161616;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16_SFLOAT
    {
        auto& info = this->getFormatInfo(VK_FORMAT_R16G16_SFLOAT);
        info.init(interface, physDev, properties, VK_FORMAT_R16G16_SFLOAT);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_SFLOAT, Surface: kRG_F16
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRG_F16;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM);
        if (fSupportsYcbcrConversion) {
            info.init(interface, physDev, properties, VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM);
        }
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, Surface: kRGB_888x
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGB_888x;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    {
        auto& info = this->getFormatInfo(VK_FORMAT_G8_B8R8_2PLANE_420_UNORM);
        if (fSupportsYcbcrConversion) {
            info.init(interface, physDev, properties, VK_FORMAT_G8_B8R8_2PLANE_420_UNORM);
        }
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, Surface: kRGB_888x
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = GrColorType::kRGB_888x;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }
    // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    {
        auto& info = this->getFormatInfo(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
        info.init(interface, physDev, properties, VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
        // No supported GrColorTypes.
    }
}

void GrVkCaps::FormatInfo::InitFormatFlags(VkFormatFeatureFlags vkFlags, uint16_t* flags) {
    if (SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & vkFlags) &&
        SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & vkFlags)) {
        *flags = *flags | kTexturable_Flag;

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

void GrVkCaps::FormatInfo::initSampleCounts(const GrVkInterface* interface,
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
    if (kIntel_VkVendor == physProps.vendorID) {
        // MSAA on Intel before Gen 9 is slow and/or buggy
        if (GrGetIntelGpuFamily(physProps.deviceID) < kFirstGen9_IntelGpuFamily) {
            return;
        }
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

void GrVkCaps::FormatInfo::init(const GrVkInterface* interface,
                                VkPhysicalDevice physDev,
                                const VkPhysicalDeviceProperties& properties,
                                VkFormat format) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    InitFormatFlags(props.linearTilingFeatures, &fLinearFlags);
    InitFormatFlags(props.optimalTilingFeatures, &fOptimalFlags);
    if (fOptimalFlags & kRenderable_Flag) {
        this->initSampleCounts(interface, physDev, properties, format);
    }
}

bool GrVkCaps::isFormatSRGB(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }

    return format_is_srgb(vkFormat);
}

bool GrVkCaps::isFormatCompressed(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }

    SkASSERT(GrVkFormatIsSupported(vkFormat));

    return vkFormat == VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
}

bool GrVkCaps::isFormatTexturableAndUploadable(GrColorType ct,
                                               const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }

    uint32_t ctFlags = this->getFormatInfo(vkFormat).colorTypeFlags(ct);
    return this->isVkFormatTexturable(vkFormat) &&
           SkToBool(ctFlags & ColorTypeInfo::kUploadData_Flag);
}

bool GrVkCaps::isFormatTexturable(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }
    return this->isVkFormatTexturable(vkFormat);
}

bool GrVkCaps::isVkFormatTexturable(VkFormat format) const {
    const FormatInfo& info = this->getFormatInfo(format);
    return SkToBool(FormatInfo::kTexturable_Flag & info.fOptimalFlags);
}

bool GrVkCaps::isFormatAsColorTypeRenderable(GrColorType ct, const GrBackendFormat& format,
                                             int sampleCount) const {
    if (!this->isFormatRenderable(format, sampleCount)) {
        return false;
    }
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }
    const auto& info = this->getFormatInfo(vkFormat);
    if (!SkToBool(info.colorTypeFlags(ct) & ColorTypeInfo::kRenderable_Flag)) {
        return false;
    }
    return true;
}

bool GrVkCaps::isFormatRenderable(const GrBackendFormat& format, int sampleCount) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }
    return this->isFormatRenderable(vkFormat, sampleCount);
}

bool GrVkCaps::isFormatRenderable(VkFormat format, int sampleCount) const {
    return sampleCount <= this->maxRenderTargetSampleCount(format);
}

int GrVkCaps::getRenderTargetSampleCount(int requestedCount,
                                         const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return 0;
    }

    return this->getRenderTargetSampleCount(requestedCount, vkFormat);
}

int GrVkCaps::getRenderTargetSampleCount(int requestedCount, VkFormat format) const {
    requestedCount = SkTMax(1, requestedCount);

    const FormatInfo& info = this->getFormatInfo(format);

    int count = info.fColorSampleCounts.count();

    if (!count) {
        return 0;
    }

    if (1 == requestedCount) {
        SkASSERT(info.fColorSampleCounts.count() && info.fColorSampleCounts[0] == 1);
        return 1;
    }

    for (int i = 0; i < count; ++i) {
        if (info.fColorSampleCounts[i] >= requestedCount) {
            return info.fColorSampleCounts[i];
        }
    }
    return 0;
}

int GrVkCaps::maxRenderTargetSampleCount(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return 0;
    }
    return this->maxRenderTargetSampleCount(vkFormat);
}

int GrVkCaps::maxRenderTargetSampleCount(VkFormat format) const {
    const FormatInfo& info = this->getFormatInfo(format);

    const auto& table = info.fColorSampleCounts;
    if (!table.count()) {
        return 0;
    }
    return table[table.count() - 1];
}

static inline size_t align_to_4(size_t v) {
    switch (v & 0b11) {
        // v is already a multiple of 4.
        case 0:     return v;
        // v is a multiple of 2 but not 4.
        case 2:     return 2 * v;
        // v is not a multiple of 2.
        default:    return 4 * v;
    }
}

GrCaps::SupportedWrite GrVkCaps::supportedWritePixelsColorType(GrColorType surfaceColorType,
                                                               const GrBackendFormat& surfaceFormat,
                                                               GrColorType srcColorType) const {
    VkFormat vkFormat;
    if (!surfaceFormat.asVkFormat(&vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }


    if (GrVkFormatNeedsYcbcrSampler(vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    // The VkBufferImageCopy bufferOffset field must be both a multiple of 4 and of a single texel.
    size_t offsetAlignment = align_to_4(GrVkBytesPerFormat(vkFormat));

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == surfaceColorType) {
            return {surfaceColorType, offsetAlignment};
        }
    }
    return {GrColorType::kUnknown, 0};
}

GrCaps::SurfaceReadPixelsSupport GrVkCaps::surfaceSupportsReadPixels(
        const GrSurface* surface) const {
    if (surface->isProtected()) {
        return SurfaceReadPixelsSupport::kUnsupported;
    }
    if (auto tex = static_cast<const GrVkTexture*>(surface->asTexture())) {
        // We can't directly read from a VkImage that has a ycbcr sampler.
        if (tex->ycbcrConversionInfo().isValid()) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
        // We can't directly read from a compressed format
        SkImage::CompressionType compressionType;
        if (GrVkFormatToCompressionType(tex->imageFormat(), &compressionType)) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
    }
    return SurfaceReadPixelsSupport::kSupported;
}

bool GrVkCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    if (auto rt = surface->asRenderTarget()) {
        return rt->numSamples() <= 1 && SkToBool(surface->asTexture());
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

static GrPixelConfig validate_image_info(VkFormat format, GrColorType ct, bool hasYcbcrConversion) {
    if (hasYcbcrConversion) {
        if (GrVkFormatNeedsYcbcrSampler(format)) {
            return kRGB_888X_GrPixelConfig;
        }

        // Format may be undefined for external images, which are required to have YCbCr conversion.
        if (VK_FORMAT_UNDEFINED == format) {
            // We don't actually care what the color type or config are since we won't use those
            // values for external textures. However, for read pixels we will draw to a non ycbcr
            // texture of this config so we set RGBA here for that.
            return kRGBA_8888_GrPixelConfig;
        }

        return kUnknown_GrPixelConfig;
    }

    if (VK_FORMAT_UNDEFINED == format) {
        return kUnknown_GrPixelConfig;
    }

    switch (ct) {
        case GrColorType::kUnknown:
            break;
        case GrColorType::kAlpha_8:
            if (VK_FORMAT_R8_UNORM == format) {
                return kAlpha_8_as_Red_GrPixelConfig;
            }
            break;
        case GrColorType::kBGR_565:
            if (VK_FORMAT_R5G6B5_UNORM_PACK16 == format) {
                return kRGB_565_GrPixelConfig;
            }
            break;
        case GrColorType::kABGR_4444:
            if (VK_FORMAT_B4G4R4A4_UNORM_PACK16 == format ||
                VK_FORMAT_R4G4B4A4_UNORM_PACK16 == format) {
                return kRGBA_4444_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_8888:
            if (VK_FORMAT_R8G8B8A8_UNORM == format) {
                return kRGBA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_8888_SRGB:
            if (VK_FORMAT_R8G8B8A8_SRGB == format) {
                return kSRGBA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGB_888x:
            if (VK_FORMAT_R8G8B8_UNORM == format) {
                return kRGB_888_GrPixelConfig;
            } else if (VK_FORMAT_R8G8B8A8_UNORM == format) {
                return kRGB_888X_GrPixelConfig;
            } else if (VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK == format) {
                return kRGB_ETC1_GrPixelConfig;
            }
            break;
        case GrColorType::kRG_88:
            if (VK_FORMAT_R8G8_UNORM == format) {
                return kRG_88_GrPixelConfig;
            }
            break;
        case GrColorType::kBGRA_8888:
            if (VK_FORMAT_B8G8R8A8_UNORM == format) {
                return kBGRA_8888_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_1010102:
            if (VK_FORMAT_A2B10G10R10_UNORM_PACK32 == format) {
                return kRGBA_1010102_GrPixelConfig;
            }
            break;
        case GrColorType::kGray_8:
            if (VK_FORMAT_R8_UNORM == format) {
                return kGray_8_as_Red_GrPixelConfig;
            }
            break;
        case GrColorType::kAlpha_F16:
            if (VK_FORMAT_R16_SFLOAT == format) {
                return kAlpha_half_as_Red_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_F16:
            if (VK_FORMAT_R16G16B16A16_SFLOAT == format) {
                return kRGBA_half_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_F16_Clamped:
            if (VK_FORMAT_R16G16B16A16_SFLOAT == format) {
                return kRGBA_half_Clamped_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_F32:
            if (VK_FORMAT_R32G32B32A32_SFLOAT == format) {
                return kRGBA_float_GrPixelConfig;
            }
            break;
        case GrColorType::kR_16:
            if (VK_FORMAT_R16_UNORM == format) {
                return kR_16_GrPixelConfig;
            }
            break;
        case GrColorType::kRG_1616:
            if (VK_FORMAT_R16G16_UNORM == format) {
                return kRG_1616_GrPixelConfig;
            }
            break;
        case GrColorType::kRGBA_16161616:
            if (VK_FORMAT_R16G16B16A16_UNORM == format) {
                return kRGBA_16161616_GrPixelConfig;
            }
            break;
        case GrColorType::kRG_F16:
            if (VK_FORMAT_R16G16_SFLOAT == format) {
                return kRG_half_GrPixelConfig;
            }
            break;
        // These have no equivalent:
        case GrColorType::kAlpha_8xxx:
        case GrColorType::kAlpha_F32xxx:
        case GrColorType::kGray_8xxx:
            break;
    }

    return kUnknown_GrPixelConfig;
}

bool GrVkCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                 const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }
    const GrVkYcbcrConversionInfo* ycbcrInfo = format.getVkYcbcrConversionInfo();
    SkASSERT(ycbcrInfo);

    return kUnknown_GrPixelConfig != validate_image_info(vkFormat, ct, ycbcrInfo->isValid());
}


GrPixelConfig GrVkCaps::onGetConfigFromBackendFormat(const GrBackendFormat& format,
                                                     GrColorType ct) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return kUnknown_GrPixelConfig;
    }
    const GrVkYcbcrConversionInfo* ycbcrInfo = format.getVkYcbcrConversionInfo();
    SkASSERT(ycbcrInfo);
    return validate_image_info(vkFormat, ct, ycbcrInfo->isValid());
}

GrColorType GrVkCaps::getYUVAColorTypeFromBackendFormat(const GrBackendFormat& format,
                                                        bool isAlphaChannel) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return GrColorType::kUnknown;
    }

    switch (vkFormat) {
        case VK_FORMAT_R8_UNORM:                 return isAlphaChannel ? GrColorType::kAlpha_8
                                                                       : GrColorType::kGray_8;
        case VK_FORMAT_R8G8B8A8_UNORM:           return GrColorType::kRGBA_8888;
        case VK_FORMAT_R8G8B8_UNORM:             return GrColorType::kRGB_888x;
        case VK_FORMAT_R8G8_UNORM:               return GrColorType::kRG_88;
        case VK_FORMAT_B8G8R8A8_UNORM:           return GrColorType::kBGRA_8888;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return GrColorType::kRGBA_1010102;
        case VK_FORMAT_R16_UNORM:                return GrColorType::kR_16;
        case VK_FORMAT_R16G16_UNORM:             return GrColorType::kRG_1616;
        // Experimental (for Y416 and mutant P016/P010)
        case VK_FORMAT_R16G16B16A16_UNORM:       return GrColorType::kRGBA_16161616;
        case VK_FORMAT_R16G16_SFLOAT:            return GrColorType::kRG_F16;
        default:                                 return GrColorType::kUnknown;
    }

    SkUNREACHABLE;
}

GrBackendFormat GrVkCaps::onGetDefaultBackendFormat(GrColorType ct,
                                                    GrRenderable renderable) const {
    GrPixelConfig config = GrColorTypeToPixelConfig(ct);
    if (config == kUnknown_GrPixelConfig) {
        return GrBackendFormat();
    }
    VkFormat format;
    if (!GrPixelConfigToVkFormat(config, &format)) {
        return GrBackendFormat();
    }
    return GrBackendFormat::MakeVk(format);
}

GrBackendFormat GrVkCaps::getBackendFormatFromCompressionType(
        SkImage::CompressionType compressionType) const {
    switch (compressionType) {
        case SkImage::kETC1_CompressionType:
            return GrBackendFormat::MakeVk(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
    }
    SK_ABORT("Invalid compression type");
}

bool GrVkCaps::canClearTextureOnCreation() const { return true; }

GrSwizzle GrVkCaps::getTextureSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    VkFormat vkFormat;
    SkAssertResult(format.asVkFormat(&vkFormat));
    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fTextureSwizzle;
        }
    }
    return GrSwizzle::RGBA();
}

GrSwizzle GrVkCaps::getOutputSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    VkFormat vkFormat;
    SkAssertResult(format.asVkFormat(&vkFormat));
    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fOutputSwizzle;
        }
    }
    return GrSwizzle::RGBA();
}

GrCaps::SupportedRead GrVkCaps::onSupportedReadPixelsColorType(
        GrColorType srcColorType, const GrBackendFormat& srcBackendFormat,
        GrColorType dstColorType) const {
    VkFormat vkFormat;
    if (!srcBackendFormat.asVkFormat(&vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    if (GrVkFormatNeedsYcbcrSampler(vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    // The VkBufferImageCopy bufferOffset field must be both a multiple of 4 and of a single texel.
    size_t offsetAlignment = align_to_4(GrVkBytesPerFormat(vkFormat));

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {srcColorType, offsetAlignment};
        }
    }
    return {GrColorType::kUnknown, 0};
}

#if GR_TEST_UTILS
std::vector<GrCaps::TestFormatColorTypeCombination> GrVkCaps::getTestingCombinations() const {
    std::vector<GrCaps::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,          GrBackendFormat::MakeVk(VK_FORMAT_R8_UNORM)             },
        { GrColorType::kBGR_565,          GrBackendFormat::MakeVk(VK_FORMAT_R5G6B5_UNORM_PACK16)  },
        { GrColorType::kABGR_4444,        GrBackendFormat::MakeVk(VK_FORMAT_R4G4B4A4_UNORM_PACK16)},
        { GrColorType::kABGR_4444,        GrBackendFormat::MakeVk(VK_FORMAT_B4G4R4A4_UNORM_PACK16)},
        { GrColorType::kRGBA_8888,        GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_UNORM)       },
        { GrColorType::kRGBA_8888_SRGB,   GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_SRGB)        },
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_UNORM)       },
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8_UNORM)         },
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeVk(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)},
        { GrColorType::kRG_88,            GrBackendFormat::MakeVk(VK_FORMAT_R8G8_UNORM)           },
        { GrColorType::kBGRA_8888,        GrBackendFormat::MakeVk(VK_FORMAT_B8G8R8A8_UNORM)       },
        { GrColorType::kRGBA_1010102,     GrBackendFormat::MakeVk(VK_FORMAT_A2B10G10R10_UNORM_PACK32)},
        { GrColorType::kGray_8,           GrBackendFormat::MakeVk(VK_FORMAT_R8_UNORM)             },
        { GrColorType::kAlpha_F16,        GrBackendFormat::MakeVk(VK_FORMAT_R16_SFLOAT)           },
        { GrColorType::kRGBA_F16,         GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT)  },
        { GrColorType::kRGBA_F16_Clamped, GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT)  },
        { GrColorType::kRGBA_F32,         GrBackendFormat::MakeVk(VK_FORMAT_R32G32B32A32_SFLOAT)  },
        { GrColorType::kR_16,             GrBackendFormat::MakeVk(VK_FORMAT_R16_UNORM)            },
        { GrColorType::kRG_1616,          GrBackendFormat::MakeVk(VK_FORMAT_R16G16_UNORM)         },
        { GrColorType::kRGBA_16161616,    GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_UNORM)   },
        { GrColorType::kRG_F16,           GrBackendFormat::MakeVk(VK_FORMAT_R16G16_SFLOAT)        },
    };

#ifdef SK_DEBUG
    for (auto combo : combos) {
        SkASSERT(this->onAreColorTypeAndFormatCompatible(combo.fColorType, combo.fFormat));
    }
#endif

    return combos;
}
#endif
