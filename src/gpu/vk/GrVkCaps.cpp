/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrUtil.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/vk/GrVkCaps.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkTexture.h"
#include "src/gpu/vk/GrVkUniformHandler.h"
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

    fTransferFromBufferToTextureSupport = true;
    fTransferFromSurfaceToBufferSupport = true;

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
    kBC1_RGB_8_16_1,
    kBC1_RGBA_8_16,
    kETC2_RGB_8_16,
};
}  // anonymous namespace

static FormatCompatibilityClass format_compatibility_class(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
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

        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return FormatCompatibilityClass::kETC2_RGB_8_16;

        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            return FormatCompatibilityClass::kBC1_RGB_8_16_1;

        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return FormatCompatibilityClass::kBC1_RGBA_8_16;

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
    if (src->isProtected() == GrProtected::kYes && dst->isProtected() != GrProtected::kYes) {
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

    this->finishInitialization(contextOptions);
}

void GrVkCaps::applyDriverCorrectnessWorkarounds(const VkPhysicalDeviceProperties& properties) {
    if (kQualcomm_VkVendor == properties.vendorID) {
        fMustDoCopiesFromOrigin = true;
        // Transfer doesn't support this workaround.
        fTransferFromSurfaceToBufferSupport = false;
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

    // On Mali galaxy s7 and s9 we see lots of rendering issues with image filters dropping out when
    // using only primary command buffers.
    if (kARM_VkVendor == properties.vendorID) {
        fPreferPrimaryOverSecondaryCommandBuffers = false;
    }

    // On Mali G series GPUs, applying transfer functions in the fragment shader with half-floats
    // produces answers that are much less accurate than expected/required. This forces full floats
    // for some intermediate values to get acceptable results.
    if (kARM_VkVendor == properties.vendorID) {
        fShaderCaps->fColorSpaceMathNeedsFloat = true;
    }

    // On various devices, when calling vkCmdClearAttachments on a primary command buffer, it
    // corrupts the bound buffers on the command buffer. As a workaround we invalidate our knowledge
    // of bound buffers so that we will rebind them on the next draw.
    if (kQualcomm_VkVendor == properties.vendorID || kAMD_VkVendor == properties.vendorID) {
        fMustInvalidatePrimaryCmdBufferStateAfterClearAttachments = true;
    }

    ////////////////////////////////////////////////////////////////////////////
    // GrCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

    // The GTX660 bot experiences crashes and incorrect rendering with MSAA CCPR. Block this path
    // renderer on non-mixed-sampled NVIDIA.
    // NOTE: We may lose mixed samples support later if the context options suppress dual source
    // blending, but that shouldn't be an issue because MSAA CCPR seems to work fine (even without
    // mixed samples) on later NVIDIA hardware where mixed samples would be supported.
    if ((kNvidia_VkVendor == properties.vendorID) && !fMixedSamplesSupport) {
        fDriverBlacklistMSAACCPR = true;
    }

#ifdef SK_BUILD_FOR_ANDROID
    // MSAA CCPR is slow on Android. http://skbug.com/9676
    fDriverBlacklistMSAACCPR = true;
#endif

    if (kARM_VkVendor == properties.vendorID) {
        fInstanceAttribSupport = false;
        fAvoidWritePixelsFastPath = true; // bugs.skia.org/8064
    }

    // AMD advertises support for MAX_UINT vertex input attributes, but in reality only supports 32.
    if (kAMD_VkVendor == properties.vendorID) {
        fMaxVertexAttributes = std::min(fMaxVertexAttributes, 32);
    }

    ////////////////////////////////////////////////////////////////////////////
    // GrShaderCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

    if (kImagination_VkVendor == properties.vendorID) {
        fShaderCaps->fAtan2ImplementedAsAtanYOverX = true;
    }

    if (kQualcomm_VkVendor == properties.vendorID) {
        // The sample mask round rect op draws nothing on Adreno for the srcmode gm.
        // http://skbug.com/8921
        fShaderCaps->fCanOnlyUseSampleMaskWithStencil = true;
    }
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
    fMaxVertexAttributes = std::min(properties.limits.maxVertexInputAttributes, kMaxVertexAttributes);

    if (properties.limits.standardSampleLocations) {
        fSampleLocationsSupport = true;
    }

    if (extensions.hasExtension(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME, 1)) {
        // We "disable" multisample by colocating all samples at pixel center.
        fMultisampleDisableSupport = true;
    }

    if (extensions.hasExtension(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME, 1)) {
        fMixedSamplesSupport = true;
    }

    if (extensions.hasExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, 1)) {
        fConservativeRasterSupport = true;
    }

    fWireframeSupport = true;

    // We could actually query and get a max size for each config, however maxImageDimension2D will
    // give the minimum max size across all configs. So for simplicity we will use that for now.
    fMaxRenderTargetSize = std::min(properties.limits.maxImageDimension2D, (uint32_t)INT_MAX);
    fMaxTextureSize = std::min(properties.limits.maxImageDimension2D, (uint32_t)INT_MAX);
    if (fDriverBugWorkarounds.max_texture_size_limit_4096) {
        fMaxTextureSize = std::min(fMaxTextureSize, 4096);
    }
    // Our render targets are always created with textures as the color
    // attachment, hence this min:
    fMaxRenderTargetSize = std::min(fMaxTextureSize, fMaxRenderTargetSize);

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

    if (kARM_VkVendor == properties.vendorID) {
        fShouldCollapseSrcOverToSrcWhenAble = true;
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

    shaderCaps->fSampleMaskSupport = true;

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

    shaderCaps->fMaxFragmentSamplers = std::min(
                                       std::min(properties.limits.maxPerStageDescriptorSampledImages,
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
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_B4G4R4A4_UNORM_PACK16,
    VK_FORMAT_R4G4B4A4_UNORM_PACK16,
    VK_FORMAT_R8G8B8A8_SRGB,
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
    VK_FORMAT_BC1_RGB_UNORM_BLOCK,
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    VK_FORMAT_R16_UNORM,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16_SFLOAT,
};

void GrVkCaps::setColorType(GrColorType colorType, std::initializer_list<VkFormat> formats) {
#ifdef SK_DEBUG
    for (size_t i = 0; i < kNumVkFormats; ++i) {
        const auto& formatInfo = fFormatTable[i];
        for (int j = 0; j < formatInfo.fColorTypeInfoCount; ++j) {
            const auto& ctInfo = formatInfo.fColorTypeInfos[j];
            if (ctInfo.fColorType == colorType &&
                !SkToBool(ctInfo.fFlags & ColorTypeInfo::kWrappedOnly_Flag)) {
                bool found = false;
                for (auto it = formats.begin(); it != formats.end(); ++it) {
                    if (kVkFormats[i] == *it) {
                        found = true;
                    }
                }
                SkASSERT(found);
            }
        }
    }
#endif
    int idx = static_cast<int>(colorType);
    for (auto it = formats.begin(); it != formats.end(); ++it) {
        const auto& info = this->getFormatInfo(*it);
        for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
            if (info.fColorTypeInfos[i].fColorType == colorType) {
                fColorTypeToFormatTable[idx] = *it;
                return;
            }
        }
    }
}

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

    std::fill_n(fColorTypeToFormatTable, kGrColorTypeCnt, VK_FORMAT_UNDEFINED);

    // Go through all the formats and init their support surface and data GrColorTypes.
    // Format: VK_FORMAT_R8G8B8A8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGBA_8888
            {
                constexpr GrColorType ct = GrColorType::kRGBA_8888;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = GrSwizzle::RGB1();
            }
        }
    }

    // Format: VK_FORMAT_R8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 1;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8_UNORM, Surface: kAlpha_8
            {
                constexpr GrColorType ct = GrColorType::kAlpha_8;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle::RRRR();
                ctInfo.fWriteSwizzle = GrSwizzle::AAAA();
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kGray_8
            {
                constexpr GrColorType ct = GrColorType::kGray_8;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = GrSwizzle("rrr1");
            }
        }
    }
    // Format: VK_FORMAT_B8G8R8A8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_B8G8R8A8_UNORM, Surface: kBGRA_8888
            {
                constexpr GrColorType ct = GrColorType::kBGRA_8888;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R5G6B5_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R5G6B5_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R5G6B5_UNORM_PACK16, Surface: kBGR_565
            {
                constexpr GrColorType ct = GrColorType::kBGR_565;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 8;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: GrColorType::kRGBA_F16
            {
                constexpr GrColorType ct = GrColorType::kRGBA_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: GrColorType::kRGBA_F16_Clamped
            {
                constexpr GrColorType ct = GrColorType::kRGBA_F16_Clamped;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_SFLOAT, Surface: kAlpha_F16
            {
                constexpr GrColorType ct = GrColorType::kAlpha_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle::RRRR();
                ctInfo.fWriteSwizzle = GrSwizzle::AAAA();
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 3;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8_UNORM, Surface: kRG_88
            {
                constexpr GrColorType ct = GrColorType::kRG_88;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32
    {
        constexpr VkFormat format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32, Surface: kRGBA_1010102
            {
                constexpr GrColorType ct = GrColorType::kRGBA_1010102;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32
    {
        constexpr VkFormat format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32, Surface: kBGRA_1010102
            {
                constexpr GrColorType ct = GrColorType::kBGRA_1010102;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16, Surface: kABGR_4444
            {
                constexpr GrColorType ct = GrColorType::kABGR_4444;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle::BGRA();
                ctInfo.fWriteSwizzle = GrSwizzle::BGRA();
            }
        }
    }
    // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16, Surface: kABGR_4444
            {
                constexpr GrColorType ct = GrColorType::kABGR_4444;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8A8_SRGB
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_SRGB, Surface: kRGBA_8888_SRGB
            {
                constexpr GrColorType ct = GrColorType::kRGBA_8888_SRGB;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 2;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_UNORM, Surface: kAlpha_16
            {
                constexpr GrColorType ct = GrColorType::kAlpha_16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = GrSwizzle::RRRR();
                ctInfo.fWriteSwizzle = GrSwizzle::AAAA();
            }
        }
    }
    // Format: VK_FORMAT_R16G16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16G16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_UNORM, Surface: kRG_1616
            {
                constexpr GrColorType ct = GrColorType::kRG_1616;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16G16B16A16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 8;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_UNORM, Surface: kRGBA_16161616
            {
                constexpr GrColorType ct = GrColorType::kRGBA_16161616;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16G16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 4;
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_SFLOAT, Surface: kRG_F16
            {
                constexpr GrColorType ct = GrColorType::kRG_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        // Currently we are just over estimating this value to be used in gpu size calculations even
        // though the actually size is probably less. We should instead treat planar formats similar
        // to compressed textures that go through their own special query for calculating size.
        info.fBytesPerPixel = 3;
        if (fSupportsYcbcrConversion) {
            info.init(interface, physDev, properties, format);
        }
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kWrappedOnly_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        // Currently we are just over estimating this value to be used in gpu size calculations even
        // though the actually size is probably less. We should instead treat planar formats similar
        // to compressed textures that go through their own special query for calculating size.
        info.fBytesPerPixel = 3;
        if (fSupportsYcbcrConversion) {
            info.init(interface, physDev, properties, format);
        }
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos.reset(new ColorTypeInfo[info.fColorTypeInfoCount]());
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kWrappedOnly_Flag;
            }
        }
    }
    // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 0;
        // No supported GrColorTypes.
    }

    // Format: VK_FORMAT_BC1_RGB_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 0;
        // No supported GrColorTypes.
    }

    // Format: VK_FORMAT_BC1_RGBA_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, physDev, properties, format);
        info.fBytesPerPixel = 0;
        // No supported GrColorTypes.
    }

    ////////////////////////////////////////////////////////////////////////////
    // Map GrColorTypes (used for creating GrSurfaces) to VkFormats. The order in which the formats
    // are passed into the setColorType function indicates the priority in selecting which format
    // we use for a given GrcolorType.

    this->setColorType(GrColorType::kAlpha_8,          { VK_FORMAT_R8_UNORM });
    this->setColorType(GrColorType::kBGR_565,          { VK_FORMAT_R5G6B5_UNORM_PACK16 });
    this->setColorType(GrColorType::kABGR_4444,        { VK_FORMAT_R4G4B4A4_UNORM_PACK16,
                                                         VK_FORMAT_B4G4R4A4_UNORM_PACK16 });
    this->setColorType(GrColorType::kRGBA_8888,        { VK_FORMAT_R8G8B8A8_UNORM });
    this->setColorType(GrColorType::kRGBA_8888_SRGB,   { VK_FORMAT_R8G8B8A8_SRGB });
    this->setColorType(GrColorType::kRGB_888x,         { VK_FORMAT_R8G8B8_UNORM,
                                                         VK_FORMAT_R8G8B8A8_UNORM });
    this->setColorType(GrColorType::kRG_88,            { VK_FORMAT_R8G8_UNORM });
    this->setColorType(GrColorType::kBGRA_8888,        { VK_FORMAT_B8G8R8A8_UNORM });
    this->setColorType(GrColorType::kRGBA_1010102,     { VK_FORMAT_A2B10G10R10_UNORM_PACK32 });
    this->setColorType(GrColorType::kBGRA_1010102,     { VK_FORMAT_A2R10G10B10_UNORM_PACK32 });
    this->setColorType(GrColorType::kGray_8,           { VK_FORMAT_R8_UNORM });
    this->setColorType(GrColorType::kAlpha_F16,        { VK_FORMAT_R16_SFLOAT });
    this->setColorType(GrColorType::kRGBA_F16,         { VK_FORMAT_R16G16B16A16_SFLOAT });
    this->setColorType(GrColorType::kRGBA_F16_Clamped, { VK_FORMAT_R16G16B16A16_SFLOAT });
    this->setColorType(GrColorType::kAlpha_16,         { VK_FORMAT_R16_UNORM });
    this->setColorType(GrColorType::kRG_1616,          { VK_FORMAT_R16G16_UNORM });
    this->setColorType(GrColorType::kRGBA_16161616,    { VK_FORMAT_R16G16B16A16_UNORM });
    this->setColorType(GrColorType::kRG_F16,           { VK_FORMAT_R16G16_SFLOAT });
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
        // MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
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
    // Standard sample locations are not defined for more than 16 samples, and we don't need more
    // than 16. Omit 32 and 64.
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

// For many checks in caps, we need to know whether the GrBackendFormat is external or not. If it is
// external the VkFormat will be VK_NULL_HANDLE which is not handled by our various format
// capability checks.
static bool backend_format_is_external(const GrBackendFormat& format) {
    const GrVkYcbcrConversionInfo* ycbcrInfo = format.getVkYcbcrConversionInfo();
    SkASSERT(ycbcrInfo);

    // All external formats have a valid ycbcrInfo used for sampling and a non zero external format.
    if (ycbcrInfo->isValid() && ycbcrInfo->fExternalFormat != 0) {
#ifdef SK_DEBUG
        VkFormat vkFormat;
        SkAssertResult(format.asVkFormat(&vkFormat));
        SkASSERT(vkFormat == VK_NULL_HANDLE);
#endif
        return true;
    }
    return false;
}

bool GrVkCaps::isFormatSRGB(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }
    if (backend_format_is_external(format)) {
        return false;
    }

    return format_is_srgb(vkFormat);
}

SkImage::CompressionType GrVkCaps::compressionType(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return SkImage::CompressionType::kNone;
    }

    switch (vkFormat) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return SkImage::CompressionType::kETC2_RGB8_UNORM;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:     return SkImage::CompressionType::kBC1_RGB8_UNORM;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:    return SkImage::CompressionType::kBC1_RGBA8_UNORM;
        default:                                return SkImage::CompressionType::kNone;
    }

    SkUNREACHABLE;
}

bool GrVkCaps::isFormatTexturable(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }
    if (backend_format_is_external(format)) {
        // We can always texture from an external format (assuming we have the ycbcr conversion
        // info which we require to be passed in).
        return true;
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
    requestedCount = std::max(1, requestedCount);

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

size_t GrVkCaps::bytesPerPixel(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return 0;
    }
    return this->bytesPerPixel(vkFormat);
}

size_t GrVkCaps::bytesPerPixel(VkFormat format) const {
    return this->getFormatInfo(format).fBytesPerPixel;
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

    // We don't support the ability to upload to external formats or formats that require a ycbcr
    // sampler. In general these types of formats are only used for sampling in a shader.
    if (backend_format_is_external(surfaceFormat) || GrVkFormatNeedsYcbcrSampler(vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    // The VkBufferImageCopy bufferOffset field must be both a multiple of 4 and of a single texel.
    size_t offsetAlignment = align_to_4(this->bytesPerPixel(vkFormat));

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
        if (GrVkFormatIsCompressed(tex->imageFormat())) {
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

bool GrVkCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                 const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!format.asVkFormat(&vkFormat)) {
        return false;
    }
    const GrVkYcbcrConversionInfo* ycbcrInfo = format.getVkYcbcrConversionInfo();
    SkASSERT(ycbcrInfo);

    if (ycbcrInfo->isValid() && !GrVkFormatNeedsYcbcrSampler(vkFormat)) {
        // Format may be undefined for external images, which are required to have YCbCr conversion.
        if (VK_FORMAT_UNDEFINED == vkFormat && ycbcrInfo->fExternalFormat != 0) {
            return true;
        }
        return false;
    }

    SkImage::CompressionType compression = GrVkFormatToCompressionType(vkFormat);
    if (compression != SkImage::CompressionType::kNone) {
        return ct == (SkCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                                                             : GrColorType::kRGBA_8888);
    }

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == ct) {
            return true;
        }
    }
    return false;
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
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return GrColorType::kBGRA_1010102;
        case VK_FORMAT_R16_UNORM:                return GrColorType::kAlpha_16;
        case VK_FORMAT_R16_SFLOAT:               return GrColorType::kAlpha_F16;
        case VK_FORMAT_R16G16_UNORM:             return GrColorType::kRG_1616;
        case VK_FORMAT_R16G16B16A16_UNORM:       return GrColorType::kRGBA_16161616;
        case VK_FORMAT_R16G16_SFLOAT:            return GrColorType::kRG_F16;
        default:                                 return GrColorType::kUnknown;
    }

    SkUNREACHABLE;
}

GrBackendFormat GrVkCaps::onGetDefaultBackendFormat(GrColorType ct) const {
    VkFormat format = this->getFormatFromColorType(ct);
    if (format == VK_FORMAT_UNDEFINED) {
        return {};
    }
    return GrBackendFormat::MakeVk(format);
}

GrBackendFormat GrVkCaps::getBackendFormatFromCompressionType(
        SkImage::CompressionType compressionType) const {
    switch (compressionType) {
        case SkImage::CompressionType::kNone:
            return {};
        case SkImage::CompressionType::kETC2_RGB8_UNORM:
            if (this->isVkFormatTexturable(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)) {
                return GrBackendFormat::MakeVk(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
            }
            return {};
        case SkImage::CompressionType::kBC1_RGB8_UNORM:
            if (this->isVkFormatTexturable(VK_FORMAT_BC1_RGB_UNORM_BLOCK)) {
                return GrBackendFormat::MakeVk(VK_FORMAT_BC1_RGB_UNORM_BLOCK);
            }
            return {};
        case SkImage::CompressionType::kBC1_RGBA8_UNORM:
            if (this->isVkFormatTexturable(VK_FORMAT_BC1_RGBA_UNORM_BLOCK)) {
                return GrBackendFormat::MakeVk(VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
            }
            return {};
    }

    SkUNREACHABLE;
}

GrSwizzle GrVkCaps::getReadSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    VkFormat vkFormat;
    SkAssertResult(format.asVkFormat(&vkFormat));
    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fReadSwizzle;
        }
    }
    return GrSwizzle::RGBA();
}

GrSwizzle GrVkCaps::getWriteSwizzle(const GrBackendFormat& format, GrColorType colorType) const {
    VkFormat vkFormat;
    SkAssertResult(format.asVkFormat(&vkFormat));
    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fWriteSwizzle;
        }
    }
    return GrSwizzle::RGBA();
}

uint64_t GrVkCaps::computeFormatKey(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    SkAssertResult(format.asVkFormat(&vkFormat));

#ifdef SK_DEBUG
    // We should never be trying to compute a key for an external format
    const GrVkYcbcrConversionInfo* ycbcrInfo = format.getVkYcbcrConversionInfo();
    SkASSERT(ycbcrInfo);
    SkASSERT(!ycbcrInfo->isValid() || ycbcrInfo->fExternalFormat == 0);
#endif

    // A VkFormat has a size of 64 bits.
    return (uint64_t)vkFormat;
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

    SkImage::CompressionType compression = GrVkFormatToCompressionType(vkFormat);
    if (compression != SkImage::CompressionType::kNone) {
        return { SkCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                                                        : GrColorType::kRGBA_8888, 0 };
    }

    // The VkBufferImageCopy bufferOffset field must be both a multiple of 4 and of a single texel.
    size_t offsetAlignment = align_to_4(this->bytesPerPixel(vkFormat));

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {srcColorType, offsetAlignment};
        }
    }
    return {GrColorType::kUnknown, 0};
}

int GrVkCaps::getFragmentUniformBinding() const {
    return GrVkUniformHandler::kUniformBinding;
}

int GrVkCaps::getFragmentUniformSet() const {
    return GrVkUniformHandler::kUniformBufferDescSet;
}

void GrVkCaps::addExtraSamplerKey(GrProcessorKeyBuilder* b,
                                  GrSamplerState samplerState,
                                  const GrBackendFormat& format) const {
    const GrVkYcbcrConversionInfo* ycbcrInfo = format.getVkYcbcrConversionInfo();
    if (!ycbcrInfo) {
        return;
    }

    GrVkSampler::Key key = GrVkSampler::GenerateKey(samplerState, *ycbcrInfo);

    size_t numInts = (sizeof(key) + 3) / 4;

    uint32_t* tmp = b->add32n(numInts);

    tmp[numInts - 1] = 0;
    memcpy(tmp, &key, sizeof(key));
}

/**
 * For Vulkan we want to cache the entire VkPipeline for reuse of draws. The Desc here holds all
 * the information needed to differentiate one pipeline from another.
 *
 * The GrProgramDesc contains all the information need to create the actual shaders for the
 * pipeline.
 *
 * For Vulkan we need to add to the GrProgramDesc to include the rest of the state on the
 * pipline. This includes stencil settings, blending information, render pass format, draw face
 * information, and primitive type. Note that some state is set dynamically on the pipeline for
 * each draw  and thus is not included in this descriptor. This includes the viewport, scissor,
 * and blend constant.
 */
GrProgramDesc GrVkCaps::makeDesc(const GrRenderTarget* rt, const GrProgramInfo& programInfo) const {
    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, rt, programInfo, *this)) {
        SkASSERT(!desc.isValid());
        return desc;
    }

    GrProcessorKeyBuilder b(&desc.key());

    // This will become part of the sheared off key used to persistently cache
    // the SPIRV code. It needs to be added right after the base key so that,
    // when the base-key is sheared off, the shearing code can include it in the
    // reduced key (c.f. the +4s in the SkData::MakeWithCopy calls in
    // GrVkPipelineStateBuilder.cpp).
    b.add32(GrVkGpu::kShader_PersistentCacheKeyType);

    GrVkRenderTarget* vkRT = (GrVkRenderTarget*) rt;
    // TODO: support failure in getSimpleRenderPass
    SkASSERT(vkRT->getSimpleRenderPass());
    vkRT->getSimpleRenderPass()->genKey(&b);

    GrStencilSettings stencil = programInfo.nonGLStencilSettings();
    stencil.genKey(&b);

    programInfo.pipeline().genKey(&b, *this);
    b.add32(programInfo.numRasterSamples());

    // Vulkan requires the full primitive type as part of its key
    b.add32(programInfo.primitiveTypeKey());

    if (this->mixedSamplesSupport()) {
        // Add "0" to indicate that coverage modulation will not be enabled, or the (non-zero)
        // raster sample count if it will.
        b.add32(!programInfo.isMixedSampled() ? 0 : programInfo.numRasterSamples());
    }

    return desc;
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
        { GrColorType::kRG_88,            GrBackendFormat::MakeVk(VK_FORMAT_R8G8_UNORM)           },
        { GrColorType::kBGRA_8888,        GrBackendFormat::MakeVk(VK_FORMAT_B8G8R8A8_UNORM)       },
        { GrColorType::kRGBA_1010102,  GrBackendFormat::MakeVk(VK_FORMAT_A2B10G10R10_UNORM_PACK32)},
        { GrColorType::kBGRA_1010102,  GrBackendFormat::MakeVk(VK_FORMAT_A2R10G10B10_UNORM_PACK32)},
        { GrColorType::kGray_8,           GrBackendFormat::MakeVk(VK_FORMAT_R8_UNORM)             },
        { GrColorType::kAlpha_F16,        GrBackendFormat::MakeVk(VK_FORMAT_R16_SFLOAT)           },
        { GrColorType::kRGBA_F16,         GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT)  },
        { GrColorType::kRGBA_F16_Clamped, GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT)  },
        { GrColorType::kAlpha_16,         GrBackendFormat::MakeVk(VK_FORMAT_R16_UNORM)            },
        { GrColorType::kRG_1616,          GrBackendFormat::MakeVk(VK_FORMAT_R16G16_UNORM)         },
        { GrColorType::kRGBA_16161616,    GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_UNORM)   },
        { GrColorType::kRG_F16,           GrBackendFormat::MakeVk(VK_FORMAT_R16G16_SFLOAT)        },
        // These two compressed formats both have an effective colorType of kRGB_888x
        { GrColorType::kRGB_888x,       GrBackendFormat::MakeVk(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)},
        { GrColorType::kRGB_888x,         GrBackendFormat::MakeVk(VK_FORMAT_BC1_RGB_UNORM_BLOCK)  },
        { GrColorType::kRGBA_8888,        GrBackendFormat::MakeVk(VK_FORMAT_BC1_RGBA_UNORM_BLOCK) },
    };

    return combos;
}
#endif
