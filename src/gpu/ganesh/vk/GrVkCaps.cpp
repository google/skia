/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkCaps.h"

#include <memory>

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/GrUtil.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/TestFormatColorTypeCombination.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkImage.h"
#include "src/gpu/ganesh/vk/GrVkRenderTarget.h"
#include "src/gpu/ganesh/vk/GrVkTexture.h"
#include "src/gpu/ganesh/vk/GrVkUniformHandler.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#ifdef SK_BUILD_FOR_ANDROID
#include <sys/system_properties.h>
#endif

GrVkCaps::GrVkCaps(const GrContextOptions& contextOptions,
                   const skgpu::VulkanInterface* vkInterface,
                   VkPhysicalDevice physDev,
                   const VkPhysicalDeviceFeatures2& features,
                   uint32_t instanceVersion,
                   uint32_t physicalDeviceVersion,
                   const skgpu::VulkanExtensions& extensions,
                   GrProtected isProtected)
        : INHERITED(contextOptions) {
    /**************************************************************************
     * GrCaps fields
     **************************************************************************/
    fMipmapSupport = true;   // always available in Vulkan
    fAnisoSupport = true;   // always available in Vulkan
    fNPOTTextureTileSupport = true;  // always available in Vulkan
    fReuseScratchTextures = true; //TODO: figure this out
    fGpuTracingSupport = false; //TODO: figure this out
    fOversizedStencilSupport = false; //TODO: figure this out
    fDrawInstancedSupport = true;

    fSemaphoreSupport = true;   // always available in Vulkan
    fFenceSyncSupport = true;   // always available in Vulkan
    fBackendSemaphoreSupport = true;
    fCrossContextTextureSupport = true;
    fHalfFloatVertexAttributeSupport = true;

    // We always copy in/out of a transfer buffer so it's trivial to support row bytes.
    fReadPixelsRowBytesSupport = true;
    fWritePixelsRowBytesSupport = true;

    fTransferFromBufferToTextureSupport = true;
    fTransferFromSurfaceToBufferSupport = true;
    fTransferFromBufferToBufferSupport  = true;

    fMaxRenderTargetSize = 4096; // minimum required by spec
    fMaxTextureSize = 4096; // minimum required by spec

    fDynamicStateArrayGeometryProcessorTextureSupport = true;

    fTextureBarrierSupport = true;

    fShaderCaps = std::make_unique<GrShaderCaps>();

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
    k10x6_64_6_1,
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

        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
            return FormatCompatibilityClass::k10x6_64_6_1;

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

bool GrVkCaps::onCanCopySurface(const GrSurfaceProxy* dst, const SkIRect& dstRect,
                                const GrSurfaceProxy* src, const SkIRect& srcRect) const {
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
        if (this->preferDiscardableMSAAAttachment() && dst->asTextureProxy() &&
            rtProxy->supportsVkInputAttachment()) {
            dstSampleCnt = 1;
        } else {
            dstSampleCnt = rtProxy->numSamples();
        }
    }
    if (const GrRenderTargetProxy* rtProxy = src->asRenderTargetProxy()) {
        // Copying to or from render targets that wrap a secondary command buffer is not allowed
        // since they would require us to know the VkImage, which we don't have, as well as need us
        // to stop and start the VkRenderPass which we don't have access to.
        if (rtProxy->wrapsVkSecondaryCB()) {
            return false;
        }
        if (this->preferDiscardableMSAAAttachment() && src->asTextureProxy() &&
            rtProxy->supportsVkInputAttachment()) {
            srcSampleCnt = 1;
        } else {
            srcSampleCnt = rtProxy->numSamples();
        }
    }
    SkASSERT((dstSampleCnt > 0) == SkToBool(dst->asRenderTargetProxy()));
    SkASSERT((srcSampleCnt > 0) == SkToBool(src->asRenderTargetProxy()));

    bool dstHasYcbcr = false;
    if (auto ycbcr = GrBackendFormats::GetVkYcbcrConversionInfo(dst->backendFormat())) {
        if (ycbcr->isValid()) {
            dstHasYcbcr = true;
        }
    }

    bool srcHasYcbcr = false;
    if (auto ycbcr = GrBackendFormats::GetVkYcbcrConversionInfo(src->backendFormat())) {
        if (ycbcr->isValid()) {
            srcHasYcbcr = true;
        }
    }

    VkFormat dstFormat, srcFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(dst->backendFormat(), &dstFormat));
    SkAssertResult(GrBackendFormats::AsVkFormat(src->backendFormat(), &srcFormat));

    // Only blits support scaling, but since we've already clamped the src and dst rects,
    // the dimensions of the scaled blit aren't important to know if it's allowed.
    const bool copyScales = srcRect.size() != dstRect.size();
    if (!copyScales && (this->canCopyImage(dstFormat, dstSampleCnt, dstHasYcbcr,
                                           srcFormat, srcSampleCnt, srcHasYcbcr) ||
                        this->canCopyAsResolve(dstFormat, dstSampleCnt, dstHasYcbcr,
                                               srcFormat, srcSampleCnt, srcHasYcbcr))) {
        return true;
    }
    return this->canCopyAsBlit(dstFormat, dstSampleCnt, dstIsLinear, dstHasYcbcr,
                               srcFormat, srcSampleCnt, srcIsLinear, srcHasYcbcr);

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

void GrVkCaps::init(const GrContextOptions& contextOptions,
                    const skgpu::VulkanInterface* vkInterface,
                    VkPhysicalDevice physDev,
                    const VkPhysicalDeviceFeatures2& features,
                    uint32_t physicalDeviceVersion,
                    const skgpu::VulkanExtensions& extensions,
                    GrProtected isProtected) {
    VkPhysicalDeviceProperties properties;
    GR_VK_CALL(vkInterface, GetPhysicalDeviceProperties(physDev, &properties));

#if defined(GR_TEST_UTILS)
    this->setDeviceName(properties.deviceName);
#endif

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
        fSupportsProtectedContent = true;
        fAvoidUpdateBuffers = true;
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

    if (extensions.hasExtension(VK_EXT_IMAGE_DRM_FORMAT_MODIFIER_EXTENSION_NAME, 1)) {
        fSupportsDRMFormatModifiers = true;
    }

    fMaxInputAttachmentDescriptors = properties.limits.maxDescriptorSetInputAttachments;

    fMaxSamplerAnisotropy = properties.limits.maxSamplerAnisotropy;

    // On desktop GPUs we have found that this does not provide much benefit. The perf results show
    // a mix of regressions, some improvements, and lots of no changes. Thus it is no worth enabling
    // this (especially with the rendering artifacts) on desktop.
    //
    // On Adreno devices we were expecting to see perf gains. But instead there were actually a lot
    // of perf regressions and only a few perf wins. This needs some follow up with qualcomm since
    // we do expect this to be a big win on tilers.
    //
    // On ARM devices we are seeing an average perf win of around 50%-60% across the board.
    if (kARM_VkVendor == properties.vendorID) {
        fPreferDiscardableMSAAAttachment = true;
        fSupportsMemorylessAttachments = true;
    }

    this->initGrCaps(vkInterface, physDev, properties, memoryProperties, features, extensions);
    this->initShaderCaps(properties, features);

    if (kQualcomm_VkVendor == properties.vendorID) {
        // A "clear" load for atlases runs faster on QC than a "discard" load followed by a
        // scissored clear.
        // On NVIDIA and Intel, the discard load followed by clear is faster.
        // TODO: Evaluate on ARM, Imagination, and ATI.
        fPreferFullscreenClears = true;
    }

    if (properties.vendorID == kNvidia_VkVendor || properties.vendorID == kAMD_VkVendor) {
        // On discrete GPUs it can be faster to read gpu only memory compared to memory that is also
        // mappable on the host.
        fGpuOnlyBuffersMorePerformant = true;

        // On discrete GPUs we try to use special DEVICE_LOCAL and HOST_VISIBLE memory for our
        // cpu write, gpu read buffers. This memory is not ideal to be kept persistently mapped.
        // Some discrete GPUs do not expose this special memory, however we still disable
        // persistently mapped buffers for all of them since most GPUs with updated drivers do
        // expose it. If this becomes an issue we can try to be more fine grained.
        fShouldPersistentlyMapCpuToGpuBuffers = false;
    }

    if (kQualcomm_VkVendor == properties.vendorID) {
        // On Qualcomm it looks like using vkCmdUpdateBuffer is slower than using a transfer buffer
        // even for small sizes.
        fAvoidUpdateBuffers = true;
    }

    if (kQualcomm_VkVendor == properties.vendorID) {
        // Adreno devices don't support push constants well
        fMaxPushConstantsSize = 0;
    }

    fNativeDrawIndirectSupport = features.features.drawIndirectFirstInstance;
    if (properties.vendorID == kQualcomm_VkVendor) {
        // Indirect draws seem slow on QC. Disable until we can investigate. http://skbug.com/11139
        fNativeDrawIndirectSupport = false;
    }

    if (fNativeDrawIndirectSupport) {
        fMaxDrawIndirectDrawCount = properties.limits.maxDrawIndirectCount;
        SkASSERT(fMaxDrawIndirectDrawCount == 1 || features.features.multiDrawIndirect);
    }

#ifdef SK_BUILD_FOR_UNIX
    if (kNvidia_VkVendor == properties.vendorID) {
        // On nvidia linux we see a big perf regression when not using dedicated image allocations.
        fShouldAlwaysUseDedicatedImageMemory = true;
    }
#endif

    this->initFormatTable(contextOptions, vkInterface, physDev, properties, features, extensions);
    this->initStencilFormat(vkInterface, physDev);

    if (contextOptions.fMaxCachedVulkanSecondaryCommandBuffers >= 0) {
        fMaxPerPoolCachedSecondaryCommandBuffers =
                contextOptions.fMaxCachedVulkanSecondaryCommandBuffers;
    }

    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(properties);
    }

    this->finishInitialization(contextOptions);
}

void GrVkCaps::applyDriverCorrectnessWorkarounds(const VkPhysicalDeviceProperties& properties) {
#if defined(SK_BUILD_FOR_WIN)
    if (kNvidia_VkVendor == properties.vendorID || kIntel_VkVendor == properties.vendorID) {
        fMustSyncCommandBuffersWithQueue = true;
    }
#elif defined(SK_BUILD_FOR_ANDROID)
    if (kImagination_VkVendor == properties.vendorID) {
        fMustSyncCommandBuffersWithQueue = true;
    }
#endif

    // Defaults to zero since all our workaround checks that use this consider things "fixed" once
    // above a certain api level. So this will just default to it being less which will enable
    // workarounds.
    int androidAPIVersion = 0;
#if defined(SK_BUILD_FOR_ANDROID)
    char androidAPIVersionStr[PROP_VALUE_MAX];
    int strLength = __system_property_get("ro.build.version.sdk", androidAPIVersionStr);
    // Defaults to zero since most checks care if it is greater than a specific value. So this will
    // just default to it being less.
    androidAPIVersion = (strLength == 0) ? 0 : atoi(androidAPIVersionStr);
#endif

    // Protected memory features have problems in Android P and earlier.
    if (fSupportsProtectedContent && (kQualcomm_VkVendor == properties.vendorID)) {
        if (androidAPIVersion <= 28) {
            fSupportsProtectedContent = false;
        }
    }

    // On Mali galaxy s7 we see lots of rendering issues when we suballocate VkImages.
    if (kARM_VkVendor == properties.vendorID && androidAPIVersion <= 28) {
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

    // On Mali galaxy s7 and s9 we see lots of rendering issues with image filters dropping out when
    // using only primary command buffers. We also see issues on the P30 running android 28.
    if (kARM_VkVendor == properties.vendorID && androidAPIVersion <= 28) {
        fPreferPrimaryOverSecondaryCommandBuffers = false;
        // If we are using secondary command buffers our code isn't setup to insert barriers into
        // the secondary cb so we need to disable support for them.
        fTextureBarrierSupport = false;
        fBlendEquationSupport = kBasic_BlendEquationSupport;
    }

    // We've seen numerous driver bugs on qualcomm devices running on android P (api 28) or earlier
    // when trying to using discardable msaa attachments and loading from resolve. So we disable the
    // feature for those devices.
    if (properties.vendorID == kQualcomm_VkVendor && androidAPIVersion <= 28) {
        fPreferDiscardableMSAAAttachment = false;
        fSupportsDiscardableMSAAForDMSAA = false;
    }

    // On the Mali G76 and T880, the Perlin noise code needs to aggressively snap to multiples
    // of 1/255 to avoid artifacts in the double table lookup.
    if (kARM_VkVendor == properties.vendorID) {
        fShaderCaps->fPerlinNoiseRoundingFix = true;
    }

    // On various devices, when calling vkCmdClearAttachments on a primary command buffer, it
    // corrupts the bound buffers on the command buffer. As a workaround we invalidate our knowledge
    // of bound buffers so that we will rebind them on the next draw.
    if (kQualcomm_VkVendor == properties.vendorID || kAMD_VkVendor == properties.vendorID) {
        fMustInvalidatePrimaryCmdBufferStateAfterClearAttachments = true;
    }

    // On Qualcomm and Arm the gpu resolves an area larger than the render pass bounds when using
    // discardable msaa attachments. This causes the resolve to resolve uninitialized data from the
    // msaa image into the resolve image.
    // This also occurs on swiftshader: b/303705884
    if (properties.vendorID == kQualcomm_VkVendor ||
        properties.vendorID == kARM_VkVendor ||
        (properties.vendorID == kGoogle_VkVendor &&
         properties.deviceID == kSwiftshader_DeviceID)) {
        fMustLoadFullImageWithDiscardableMSAA = true;
    }

    // There seems to be bug in swiftshader when we reuse scratch buffers for uploads. We end up
    // with very slight pixel diffs. For example:
    // (https://ci.chromium.org/ui/p/chromium/builders/try/linux-rel/1585128/overview).
    // Since swiftshader is only really used for testing, to try and make things more stable we
    // disable the reuse of buffers.
    if (properties.vendorID == kGoogle_VkVendor && properties.deviceID == kSwiftshader_DeviceID) {
        fReuseScratchBuffers = false;
    }

#ifdef SK_BUILD_FOR_UNIX
    if (kIntel_VkVendor == properties.vendorID) {
        // At least on our linux Debug Intel HD405 bot we are seeing issues doing read pixels with
        // non-conherent memory. It seems like the device is not properly honoring the
        // vkInvalidateMappedMemoryRanges calls correctly. Other linux intel devices seem to work
        // okay. However, since I'm not sure how to target a specific intel devices or driver
        // version I am going to stop all intel linux from using non-coherent memory. Currently we
        // are not shipping anything on these platforms and the only real thing that will regress is
        // read backs. If we find later we do care about this performance we can come back to figure
        // out how to do a more narrow workaround.
        fMustUseCoherentHostVisibleMemory = true;
    }
#endif

    ////////////////////////////////////////////////////////////////////////////
    // GrCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

    if (kARM_VkVendor == properties.vendorID) {
        fAvoidWritePixelsFastPath = true; // bugs.skia.org/8064
    }

    // AMD advertises support for MAX_UINT vertex input attributes, but in reality only supports 32.
    if (kAMD_VkVendor == properties.vendorID) {
        fMaxVertexAttributes = std::min(fMaxVertexAttributes, 32);
    }

    // Adreno devices fail when trying to read the dest using an input attachment and texture
    // barriers.
    if (kQualcomm_VkVendor == properties.vendorID) {
        fTextureBarrierSupport = false;
    }

#ifdef SK_BUILD_FOR_WIN
    // Gen 12 Intel devices running on windows has issues using barriers for dst reads. This is seen
    // when running the unit tests SkRuntimeEffect_Blender_GPU and DMSAA_aa_dst_read_after_dmsaa.
    //
    // Additionally, as of 2023-01-19 the latest driver compatible with Intel Iris Graphics 540
    // (9th gen Skylake microarchitecture) produce SkRuntimeEffect_Blender and DMSAA deltas that
    // are unacceptable and break our tests. The drivers in question are version 31.0.101.2115 and
    // can be downloaded from
    // https://www.intel.com/content/www/us/en/download/762755/intel-6th-10th-gen-processor-graphics-windows.html.
    // This is likely due to bugs in the driver. As a temporary workaround, we disable texture
    // barrier support in Skylake and newer generations (i.e. 9th gen or newer).
    if (kIntel_VkVendor == properties.vendorID &&
        GetIntelGen(GetIntelGPUType(properties.deviceID)) >= 9) {
        fTextureBarrierSupport = false;
    }
#endif

    // On ARM indirect draws are broken on Android 9 and earlier. This was tested on a P30 and
    // Mate 20x running android 9.
    if (properties.vendorID == kARM_VkVendor && androidAPIVersion <= 28) {
        fNativeDrawIndirectSupport = false;
    }

    ////////////////////////////////////////////////////////////////////////////
    // GrShaderCaps workarounds
    ////////////////////////////////////////////////////////////////////////////

    if (kImagination_VkVendor == properties.vendorID) {
        fShaderCaps->fAtan2ImplementedAsAtanYOverX = true;
    }

    // ARM GPUs calculate `matrix * vector` in SPIR-V at full precision, even when the inputs are
    // RelaxedPrecision. Rewriting the multiply as a sum of vector*scalar fixes this. (skia:11769)
    if (kARM_VkVendor == properties.vendorID) {
        fShaderCaps->fRewriteMatrixVectorMultiply = true;
    }
}

void GrVkCaps::initGrCaps(const skgpu::VulkanInterface* vkInterface,
                          VkPhysicalDevice physDev,
                          const VkPhysicalDeviceProperties& properties,
                          const VkPhysicalDeviceMemoryProperties& memoryProperties,
                          const VkPhysicalDeviceFeatures2& features,
                          const skgpu::VulkanExtensions& extensions) {
    // So GPUs, like AMD, are reporting MAX_INT support vertex attributes. In general, there is no
    // need for us ever to support that amount, and it makes tests which tests all the vertex
    // attribs timeout looping over that many. For now, we'll cap this at 64 max and can raise it if
    // we ever find that need.
    static const uint32_t kMaxVertexAttributes = 64;
    fMaxVertexAttributes = std::min(properties.limits.maxVertexInputAttributes,
                                    kMaxVertexAttributes);

    // GrCaps::fSampleLocationsSupport refers to the ability to *query* the sample locations (not
    // program them). For now we just set this to true if the device uses standard locations, and
    // return the standard locations back when queried.
    if (properties.limits.standardSampleLocations) {
        fSampleLocationsSupport = true;
    }

    if (extensions.hasExtension(VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME, 1)) {
        fConservativeRasterSupport = true;
    }

    fWireframeSupport = true;

    // We could actually query and get a max size for each config, however maxImageDimension2D will
    // give the minimum max size across all configs. So for simplicity we will use that for now.
    fMaxRenderTargetSize = std::min(properties.limits.maxImageDimension2D, (uint32_t)INT_MAX);
    fMaxTextureSize = std::min(properties.limits.maxImageDimension2D, (uint32_t)INT_MAX);

    // TODO: check if RT's larger than 4k incur a performance cost on ARM.
    fMaxPreferredRenderTargetSize = fMaxRenderTargetSize;

    fMaxPushConstantsSize = std::min(properties.limits.maxPushConstantsSize, (uint32_t)INT_MAX);

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
                fBlendEquationSupport = kAdvanced_BlendEquationSupport;
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

    // Ganesh + Vulkan always emits `sk_Clockwise` to avoid some Adreno rendering errors.
    shaderCaps->fMustDeclareFragmentFrontFacing = true;

    // Vulkan is based off ES 3.0 so the following should all be supported
    shaderCaps->fUsesPrecisionModifiers = true;
    shaderCaps->fFlatInterpolationSupport = true;
    // Flat interpolation appears to be slow on Qualcomm GPUs. This was tested in GL and is assumed
    // to be true with Vulkan as well.
    shaderCaps->fPreferFlatInterpolation = kQualcomm_VkVendor != properties.vendorID;

    shaderCaps->fSampleMaskSupport = true;

    shaderCaps->fShaderDerivativeSupport = true;
    shaderCaps->fExplicitTextureLodSupport = true;

    shaderCaps->fDualSourceBlendingSupport = features.features.dualSrcBlend;

    shaderCaps->fIntegerSupport = true;
    shaderCaps->fNonsquareMatrixSupport = true;
    shaderCaps->fInverseHyperbolicSupport = true;
    shaderCaps->fVertexIDSupport = true;
    shaderCaps->fInfinitySupport = true;
    shaderCaps->fNonconstantArrayIndexSupport = true;
    shaderCaps->fBitManipulationSupport = true;

    // Assume the minimum precisions mandated by the SPIR-V spec.
    shaderCaps->fFloatIs32Bits = true;
    shaderCaps->fHalfIs32Bits = false;

    shaderCaps->fMaxFragmentSamplers = std::min(
                                       std::min(properties.limits.maxPerStageDescriptorSampledImages,
                                              properties.limits.maxPerStageDescriptorSamplers),
                                              (uint32_t)INT_MAX);
}

bool stencil_format_supported(const skgpu::VulkanInterface* interface,
                              VkPhysicalDevice physDev,
                              VkFormat format) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    return SkToBool(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & props.optimalTilingFeatures);
}

void GrVkCaps::initStencilFormat(const skgpu::VulkanInterface* interface,
                                 VkPhysicalDevice physDev) {
    if (stencil_format_supported(interface, physDev, VK_FORMAT_S8_UINT)) {
        fPreferredStencilFormat = VK_FORMAT_S8_UINT;
    } else if (stencil_format_supported(interface, physDev, VK_FORMAT_D24_UNORM_S8_UINT)) {
        fPreferredStencilFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    } else {
        SkASSERT(stencil_format_supported(interface, physDev, VK_FORMAT_D32_SFLOAT_S8_UINT));
        fPreferredStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
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
    VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
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
    static_assert(std::size(kVkFormats) == GrVkCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");
    for (size_t i = 0; i < std::size(kVkFormats); ++i) {
        if (kVkFormats[i] == format) {
            return fFormatTable[i];
        }
    }
    static FormatInfo kInvalidFormat;
    return kInvalidFormat;
}

void GrVkCaps::initFormatTable(const GrContextOptions& contextOptions,
                               const skgpu::VulkanInterface* interface,
                               VkPhysicalDevice physDev,
                               const VkPhysicalDeviceProperties& properties,
                               const VkPhysicalDeviceFeatures2& features,
                               const skgpu::VulkanExtensions& extensions) {
    static_assert(std::size(kVkFormats) == GrVkCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");

    std::fill_n(fColorTypeToFormatTable, kGrColorTypeCnt, VK_FORMAT_UNDEFINED);

    // Go through all the formats and init their support surface and data GrColorTypes.
    // Format: VK_FORMAT_R8G8B8A8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGBA_8888
            {
                constexpr GrColorType ct = GrColorType::kRGBA_8888;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle::RGB1();
            }
        }
    }

    // Format: VK_FORMAT_R8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 3;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8_UNORM, Surface: kR_8
            {
                constexpr GrColorType ct = GrColorType::kR_8;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kAlpha_8
            {
                constexpr GrColorType ct = GrColorType::kAlpha_8;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kGray_8
            {
                constexpr GrColorType ct = GrColorType::kGray_8;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("rrr1");
            }
        }
    }
    // Format: VK_FORMAT_B8G8R8A8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_B8G8R8A8_UNORM, Surface: kBGRA_8888
            {
                constexpr GrColorType ct = GrColorType::kBGRA_8888;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R5G6B5_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R5G6B5_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R5G6B5_UNORM_PACK16, Surface: kBGR_565
            {
                constexpr GrColorType ct = GrColorType::kBGR_565;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: GrColorType::kRGBA_F16
            {
                constexpr GrColorType ct = GrColorType::kRGBA_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: GrColorType::kRGBA_F16_Clamped
            {
                constexpr GrColorType ct = GrColorType::kRGBA_F16_Clamped;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_SFLOAT, Surface: kAlpha_F16
            {
                constexpr GrColorType ct = GrColorType::kAlpha_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                // The Vulkan format is 3 bpp so we must convert to/from that when transferring.
                ctInfo.fTransferColorType = GrColorType::kRGB_888;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8_UNORM, Surface: kRG_88
            {
                constexpr GrColorType ct = GrColorType::kRG_88;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32
    {
        constexpr VkFormat format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32, Surface: kRGBA_1010102
            {
                constexpr GrColorType ct = GrColorType::kRGBA_1010102;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32
    {
        constexpr VkFormat format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32, Surface: kBGRA_1010102
            {
                constexpr GrColorType ct = GrColorType::kBGRA_1010102;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }

    bool supportsRGBA10x6 = false;
    if (extensions.hasExtension(VK_EXT_RGBA10X6_FORMATS_EXTENSION_NAME, 1)) {
        auto rgba10x6Feature =
            get_extension_feature_struct<VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT>(
                features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT);
        // Technically without this extension and exabled feature we could still use this format to
        // sample with a ycbcr sampler. But for simplicity until we have clients requesting that, we
        // limit the use of this format to cases where we have the extension supported.
        supportsRGBA10x6 = rgba10x6Feature  && rgba10x6Feature->formatRgba10x6WithoutYCbCrSampler;
    }

    // Format: VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16
    if (supportsRGBA10x6) {
        constexpr VkFormat format = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, Surface: kRGBA_10x6
            {
                constexpr GrColorType ct = GrColorType::kRGBA_10x6;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }

    // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16, Surface: kABGR_4444
            {
                constexpr GrColorType ct = GrColorType::kABGR_4444;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle::BGRA();
                ctInfo.fWriteSwizzle = skgpu::Swizzle::BGRA();
            }
        }
    }

    // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16, Surface: kABGR_4444
            {
                constexpr GrColorType ct = GrColorType::kABGR_4444;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8B8A8_SRGB
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_SRGB, Surface: kRGBA_8888_SRGB
            {
                constexpr GrColorType ct = GrColorType::kRGBA_8888_SRGB;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_UNORM, Surface: kAlpha_16
            {
                constexpr GrColorType ct = GrColorType::kAlpha_16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
        }
    }
    // Format: VK_FORMAT_R16G16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16G16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_UNORM, Surface: kRG_1616
            {
                constexpr GrColorType ct = GrColorType::kRG_1616;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16B16A16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16G16B16A16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_UNORM, Surface: kRGBA_16161616
            {
                constexpr GrColorType ct = GrColorType::kRGBA_16161616;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16G16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_SFLOAT, Surface: kRG_F16
            {
                constexpr GrColorType ct = GrColorType::kRG_F16;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        if (fSupportsYcbcrConversion) {
            info.init(contextOptions, interface, physDev, properties, format);
        }
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kWrappedOnly_Flag;
            }
        }
    }
    // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        if (fSupportsYcbcrConversion) {
            info.init(contextOptions, interface, physDev, properties, format);
        }
        if (SkToBool(info.fOptimalFlags & FormatInfo::kTexturable_Flag)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr GrColorType ct = GrColorType::kRGB_888x;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kWrappedOnly_Flag;
            }
        }
    }
    // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        // Setting this to texel block size
        // No supported GrColorTypes.
    }

    // Format: VK_FORMAT_BC1_RGB_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        // Setting this to texel block size
        // No supported GrColorTypes.
    }

    // Format: VK_FORMAT_BC1_RGBA_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(contextOptions, interface, physDev, properties, format);
        // Setting this to texel block size
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
    // TODO: For Vk w/ VK_KHR_maintenance1 extension support, check
    //  VK_FORMAT_FEATURE_TRANSFER_[SRC|DST]_BIT_KHR explicitly to set copy flags
    //  Can do similar check for VK_KHR_sampler_ycbcr_conversion added bits

    if (SkToBool(VK_FORMAT_FEATURE_BLIT_SRC_BIT & vkFlags)) {
        *flags = *flags | kBlitSrc_Flag;
    }

    if (SkToBool(VK_FORMAT_FEATURE_BLIT_DST_BIT & vkFlags)) {
        *flags = *flags | kBlitDst_Flag;
    }
}

void GrVkCaps::FormatInfo::initSampleCounts(const GrContextOptions& contextOptions,
                                            const skgpu::VulkanInterface* interface,
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
        if (GetIntelGen(GetIntelGPUType(physProps.deviceID)) < 12 ||
            !contextOptions.fAllowMSAAOnNewIntel) {
            // MSAA doesn't work well on Intel GPUs chromium:527565, chromium:983926
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
    // Standard sample locations are not defined for more than 16 samples, and we don't need more
    // than 16. Omit 32 and 64.
}

void GrVkCaps::FormatInfo::init(const GrContextOptions& contextOptions,
                                const skgpu::VulkanInterface* interface,
                                VkPhysicalDevice physDev,
                                const VkPhysicalDeviceProperties& properties,
                                VkFormat format) {
    VkFormatProperties props;
    memset(&props, 0, sizeof(VkFormatProperties));
    GR_VK_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &props));
    InitFormatFlags(props.linearTilingFeatures, &fLinearFlags);
    InitFormatFlags(props.optimalTilingFeatures, &fOptimalFlags);
    if (fOptimalFlags & kRenderable_Flag) {
        this->initSampleCounts(contextOptions, interface, physDev, properties, format);
    }
}

// For many checks in caps, we need to know whether the GrBackendFormat is external or not. If it is
// external the VkFormat will be VK_NULL_HANDLE which is not handled by our various format
// capability checks.
static bool backend_format_is_external(const GrBackendFormat& format) {
    const GrVkYcbcrConversionInfo* ycbcrInfo = GrBackendFormats::GetVkYcbcrConversionInfo(format);
    SkASSERT(ycbcrInfo);

    // All external formats have a valid ycbcrInfo used for sampling and a non zero external format.
    if (ycbcrInfo->isValid() && ycbcrInfo->fExternalFormat != 0) {
#ifdef SK_DEBUG
        VkFormat vkFormat;
        SkAssertResult(GrBackendFormats::AsVkFormat(format, &vkFormat));
        SkASSERT(vkFormat == VK_FORMAT_UNDEFINED);
#endif
        return true;
    }
    return false;
}

bool GrVkCaps::isFormatSRGB(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
        return false;
    }
    if (backend_format_is_external(format)) {
        return false;
    }

    return format_is_srgb(vkFormat);
}

bool GrVkCaps::isFormatTexturable(const GrBackendFormat& format, GrTextureType) const {
    VkFormat vkFormat;
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
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
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
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
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
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
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
        return 0;
    }

    return this->getRenderTargetSampleCount(requestedCount, vkFormat);
}

int GrVkCaps::getRenderTargetSampleCount(int requestedCount, VkFormat format) const {
    requestedCount = std::max(1, requestedCount);

    const FormatInfo& info = this->getFormatInfo(format);

    int count = info.fColorSampleCounts.size();

    if (!count) {
        return 0;
    }

    if (1 == requestedCount) {
        SkASSERT(info.fColorSampleCounts.size() && info.fColorSampleCounts[0] == 1);
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
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
        return 0;
    }
    return this->maxRenderTargetSampleCount(vkFormat);
}

int GrVkCaps::maxRenderTargetSampleCount(VkFormat format) const {
    const FormatInfo& info = this->getFormatInfo(format);

    const auto& table = info.fColorSampleCounts;
    if (!table.size()) {
        return 0;
    }
    return table[table.size() - 1];
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
    if (!GrBackendFormats::AsVkFormat(surfaceFormat, &vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    // We don't support the ability to upload to external formats or formats that require a ycbcr
    // sampler. In general these types of formats are only used for sampling in a shader.
    if (backend_format_is_external(surfaceFormat) || skgpu::VkFormatNeedsYcbcrSampler(vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    // The VkBufferImageCopy bufferOffset field must be both a multiple of 4 and of a single texel.
    size_t offsetAlignment = align_to_4(skgpu::VkFormatBytesPerBlock(vkFormat));

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == surfaceColorType) {
            return {ctInfo.fTransferColorType, offsetAlignment};
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
        auto texImage = tex->textureImage();
        if (!texImage) {
            return SurfaceReadPixelsSupport::kUnsupported;
        }
        // We can't directly read from a VkImage that has a ycbcr sampler.
        if (texImage->ycbcrConversionInfo().isValid()) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
        // We can't directly read from a compressed format
        if (skgpu::VkFormatIsCompressed(texImage->imageFormat())) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
        return SurfaceReadPixelsSupport::kSupported;
    } else if (auto rt = surface->asRenderTarget()) {
        if (rt->numSamples() > 1) {
            return SurfaceReadPixelsSupport::kCopyToTexture2D;
        }
        return SurfaceReadPixelsSupport::kSupported;
    }
    return SurfaceReadPixelsSupport::kUnsupported;
}

GrColorType GrVkCaps::transferColorType(VkFormat vkFormat, GrColorType surfaceColorType) const {
    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == surfaceColorType) {
            return info.fColorTypeInfos[i].fTransferColorType;
        }
    }
    return GrColorType::kUnknown;
}

bool GrVkCaps::onSurfaceSupportsWritePixels(const GrSurface* surface) const {
    if (auto rt = surface->asRenderTarget()) {
        return rt->numSamples() <= 1 && SkToBool(surface->asTexture());
    }
    // We can't write to a texture that has a ycbcr sampler.
    if (auto tex = static_cast<const GrVkTexture*>(surface->asTexture())) {
        auto texImage = tex->textureImage();
        if (!texImage) {
            return false;
        }
        // We can't directly read from a VkImage that has a ycbcr sampler.
        if (texImage->ycbcrConversionInfo().isValid()) {
            return false;
        }
    }
    return true;
}

bool GrVkCaps::onAreColorTypeAndFormatCompatible(GrColorType ct,
                                                 const GrBackendFormat& format) const {
    VkFormat vkFormat;
    if (!GrBackendFormats::AsVkFormat(format, &vkFormat)) {
        return false;
    }
    const GrVkYcbcrConversionInfo* ycbcrInfo = GrBackendFormats::GetVkYcbcrConversionInfo(format);
    SkASSERT(ycbcrInfo);

    if (ycbcrInfo->isValid() && !skgpu::VkFormatNeedsYcbcrSampler(vkFormat)) {
        // Format may be undefined for external images, which are required to have YCbCr conversion.
        if (VK_FORMAT_UNDEFINED == vkFormat && ycbcrInfo->fExternalFormat != 0) {
            return true;
        }
        return false;
    }

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        if (info.fColorTypeInfos[i].fColorType == ct) {
            return true;
        }
    }
    return false;
}

GrBackendFormat GrVkCaps::onGetDefaultBackendFormat(GrColorType ct) const {
    VkFormat format = this->getFormatFromColorType(ct);
    if (format == VK_FORMAT_UNDEFINED) {
        return {};
    }
    return GrBackendFormats::MakeVk(format);
}

bool GrVkCaps::onSupportsDynamicMSAA(const GrRenderTargetProxy* rtProxy) const {
    // We must be able to use the rtProxy as an input attachment to load into the discardable msaa
    // attachment. Also the rtProxy should have a sample count of 1 so that it can be used as a
    // resolve attachment.
    return this->supportsDiscardableMSAAForDMSAA() &&
           rtProxy->supportsVkInputAttachment() &&
           rtProxy->numSamples() == 1;
}

bool GrVkCaps::renderTargetSupportsDiscardableMSAA(const GrVkRenderTarget* rt) const {
    return rt->resolveAttachment() &&
           rt->resolveAttachment()->supportsInputAttachmentUsage() &&
           ((rt->numSamples() > 1 && this->preferDiscardableMSAAAttachment()) ||
            (rt->numSamples() == 1 && this->supportsDiscardableMSAAForDMSAA()));
}

bool GrVkCaps::programInfoWillUseDiscardableMSAA(const GrProgramInfo& programInfo) const {
    return programInfo.targetHasVkResolveAttachmentWithInput() &&
           programInfo.numSamples() > 1 &&
           ((programInfo.targetsNumSamples() > 1 && this->preferDiscardableMSAAAttachment()) ||
            (programInfo.targetsNumSamples() == 1 && this->supportsDiscardableMSAAForDMSAA()));
}

GrBackendFormat GrVkCaps::getBackendFormatFromCompressionType(
        SkTextureCompressionType compressionType) const {
    switch (compressionType) {
        case SkTextureCompressionType::kNone:
            return {};
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
            if (this->isVkFormatTexturable(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)) {
                return GrBackendFormats::MakeVk(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
            }
            return {};
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
            if (this->isVkFormatTexturable(VK_FORMAT_BC1_RGB_UNORM_BLOCK)) {
                return GrBackendFormats::MakeVk(VK_FORMAT_BC1_RGB_UNORM_BLOCK);
            }
            return {};
        case SkTextureCompressionType::kBC1_RGBA8_UNORM:
            if (this->isVkFormatTexturable(VK_FORMAT_BC1_RGBA_UNORM_BLOCK)) {
                return GrBackendFormats::MakeVk(VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
            }
            return {};
    }

    SkUNREACHABLE;
}

skgpu::Swizzle GrVkCaps::onGetReadSwizzle(const GrBackendFormat& format,
                                          GrColorType colorType) const {
    VkFormat vkFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(format, &vkFormat));
    const GrVkYcbcrConversionInfo* ycbcrInfo = GrBackendFormats::GetVkYcbcrConversionInfo(format);
    SkASSERT(ycbcrInfo);
    if (ycbcrInfo->isValid() && ycbcrInfo->fExternalFormat != 0) {
        // We allow these to work with any color type and never swizzle. See
        // onAreColorTypeAndFormatCompatible.
        return skgpu::Swizzle{"rgba"};
    }

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fReadSwizzle;
        }
    }
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.",
                 (int)colorType, (int)vkFormat);
    return {};
}

skgpu::Swizzle GrVkCaps::getWriteSwizzle(const GrBackendFormat& format,
                                         GrColorType colorType) const {
    VkFormat vkFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(format, &vkFormat));
    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == colorType) {
            return ctInfo.fWriteSwizzle;
        }
    }
    SkDEBUGFAILF("Illegal color type (%d) and format (%d) combination.",
                 (int)colorType, (int)vkFormat);
    return {};
}

GrDstSampleFlags GrVkCaps::onGetDstSampleFlagsForProxy(const GrRenderTargetProxy* rt) const {
    bool isMSAAWithResolve = rt->numSamples() > 1 && rt->asTextureProxy();
    // TODO: Currently if we have an msaa rt with a resolve, the supportsVkInputAttachment call
    // references whether the resolve is supported as an input attachment. We need to add a check to
    // allow checking the color attachment (msaa or not) supports input attachment specifically.
    if (!isMSAAWithResolve && rt->supportsVkInputAttachment()) {
        return GrDstSampleFlags::kRequiresTextureBarrier | GrDstSampleFlags::kAsInputAttachment;
    }
    return GrDstSampleFlags::kNone;
}

uint64_t GrVkCaps::computeFormatKey(const GrBackendFormat& format) const {
    VkFormat vkFormat;
    SkAssertResult(GrBackendFormats::AsVkFormat(format, &vkFormat));

#ifdef SK_DEBUG
    // We should never be trying to compute a key for an external format
    const GrVkYcbcrConversionInfo* ycbcrInfo = GrBackendFormats::GetVkYcbcrConversionInfo(format);
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
    if (!GrBackendFormats::AsVkFormat(srcBackendFormat, &vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    if (skgpu::VkFormatNeedsYcbcrSampler(vkFormat)) {
        return {GrColorType::kUnknown, 0};
    }

    SkTextureCompressionType compression = GrBackendFormatToCompressionType(srcBackendFormat);
    if (compression != SkTextureCompressionType::kNone) {
        return { SkTextureCompressionTypeIsOpaque(compression) ? GrColorType::kRGB_888x
                                                        : GrColorType::kRGBA_8888, 0 };
    }

    // The VkBufferImageCopy bufferOffset field must be both a multiple of 4 and of a single texel.
    size_t offsetAlignment = align_to_4(skgpu::VkFormatBytesPerBlock(vkFormat));

    const auto& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {ctInfo.fTransferColorType, offsetAlignment};
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

void GrVkCaps::addExtraSamplerKey(skgpu::KeyBuilder* b,
                                  GrSamplerState samplerState,
                                  const GrBackendFormat& format) const {
    const GrVkYcbcrConversionInfo* ycbcrInfo = GrBackendFormats::GetVkYcbcrConversionInfo(format);
    if (!ycbcrInfo) {
        return;
    }

    GrVkSampler::Key key = GrVkSampler::GenerateKey(samplerState, *ycbcrInfo);

    constexpr size_t numInts = (sizeof(key) + 3) / 4;
    uint32_t tmp[numInts];
    memcpy(tmp, &key, sizeof(key));

    for (size_t i = 0; i < numInts; ++i) {
        b->add32(tmp[i]);
    }
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
GrProgramDesc GrVkCaps::makeDesc(GrRenderTarget* rt,
                                 const GrProgramInfo& programInfo,
                                 ProgramDescOverrideFlags overrideFlags) const {
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, programInfo, *this);

    skgpu::KeyBuilder b(desc.key());

    // This will become part of the sheared off key used to persistently cache
    // the SPIRV code. It needs to be added right after the base key so that,
    // when the base-key is sheared off, the shearing code can include it in the
    // reduced key (c.f. the +4s in the SkData::MakeWithCopy calls in
    // GrVkPipelineStateBuilder.cpp).
    b.add32(GrVkGpu::kShader_PersistentCacheKeyType);

    GrVkRenderPass::SelfDependencyFlags selfDepFlags = GrVkRenderPass::SelfDependencyFlags::kNone;
    if (programInfo.renderPassBarriers() & GrXferBarrierFlags::kBlend) {
        selfDepFlags |= GrVkRenderPass::SelfDependencyFlags::kForNonCoherentAdvBlend;
    }
    if (programInfo.renderPassBarriers() & GrXferBarrierFlags::kTexture) {
        selfDepFlags |= GrVkRenderPass::SelfDependencyFlags::kForInputAttachment;
    }

    bool needsResolve = this->programInfoWillUseDiscardableMSAA(programInfo);

    bool forceLoadFromResolve =
            overrideFlags & GrCaps::ProgramDescOverrideFlags::kVulkanHasResolveLoadSubpass;
    SkASSERT(!forceLoadFromResolve || needsResolve);

    GrVkRenderPass::LoadFromResolve loadFromResolve = GrVkRenderPass::LoadFromResolve::kNo;
    if (needsResolve && (programInfo.colorLoadOp() == GrLoadOp::kLoad || forceLoadFromResolve)) {
        loadFromResolve = GrVkRenderPass::LoadFromResolve::kLoad;
    }

    if (rt) {
        GrVkRenderTarget* vkRT = (GrVkRenderTarget*) rt;

        SkASSERT(!needsResolve || (vkRT->resolveAttachment() &&
                                   vkRT->resolveAttachment()->supportsInputAttachmentUsage()));

        bool needsStencil = programInfo.needsStencil() || programInfo.isStencilEnabled();
        // TODO: support failure in getSimpleRenderPass
        auto rp = vkRT->getSimpleRenderPass(needsResolve, needsStencil, selfDepFlags,
                                            loadFromResolve);
        SkASSERT(rp);
        rp->genKey(&b);

#ifdef SK_DEBUG
        if (!rp->isExternal()) {
            // This is to ensure ReconstructAttachmentsDescriptor keeps matching
            // getSimpleRenderPass' result
            GrVkRenderPass::AttachmentsDescriptor attachmentsDescriptor;
            GrVkRenderPass::AttachmentFlags attachmentFlags;
            GrVkRenderTarget::ReconstructAttachmentsDescriptor(*this, programInfo,
                                                               &attachmentsDescriptor,
                                                               &attachmentFlags);
            SkASSERT(rp->isCompatible(attachmentsDescriptor, attachmentFlags, selfDepFlags,
                                      loadFromResolve));
        }
#endif
    } else {
        GrVkRenderPass::AttachmentsDescriptor attachmentsDescriptor;
        GrVkRenderPass::AttachmentFlags attachmentFlags;
        GrVkRenderTarget::ReconstructAttachmentsDescriptor(*this, programInfo,
                                                           &attachmentsDescriptor,
                                                           &attachmentFlags);

        // kExternal_AttachmentFlag is only set for wrapped secondary command buffers - which
        // will always go through the above 'rt' path (i.e., we can always pass 0 as the final
        // parameter to GenKey).
        GrVkRenderPass::GenKey(&b, attachmentFlags, attachmentsDescriptor, selfDepFlags,
                               loadFromResolve, 0);
    }

    GrStencilSettings stencil = programInfo.nonGLStencilSettings();
    stencil.genKey(&b, true);

    programInfo.pipeline().genKey(&b, *this);
    b.add32(programInfo.numSamples());

    // Vulkan requires the full primitive type as part of its key
    b.add32(programInfo.primitiveTypeKey());

    b.flush();
    return desc;
}

GrInternalSurfaceFlags GrVkCaps::getExtraSurfaceFlagsForDeferredRT() const {
    // We always create vulkan RT with the input attachment flag;
    return GrInternalSurfaceFlags::kVkRTSupportsInputAttachment;
}

VkShaderStageFlags GrVkCaps::getPushConstantStageFlags() const {
    VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    return stageFlags;
}

template <size_t N>
static bool intel_deviceID_present(const std::array<uint32_t, N>& array, uint32_t deviceID) {
    return std::find(array.begin(), array.end(), deviceID) != array.end();
}


GrVkCaps::IntelGPUType GrVkCaps::GetIntelGPUType(uint32_t deviceID) {
    // Some common Intel GPU models, currently we cover SKL/ICL/RKL/TGL/ADL
    // Referenced from the following Mesa source files:
    // https://github.com/mesa3d/mesa/blob/master/include/pci_ids/i965_pci_ids.h
    // https://github.com/mesa3d/mesa/blob/master/include/pci_ids/iris_pci_ids.h
    static constexpr std::array<uint32_t, 25> kSkyLakeIDs = {
        {0x1902, 0x1906, 0x190A, 0x190B, 0x190E, 0x1912, 0x1913,
         0x1915, 0x1916, 0x1917, 0x191A, 0x191B, 0x191D, 0x191E,
         0x1921, 0x1923, 0x1926, 0x1927, 0x192A, 0x192B, 0x192D,
         0x1932, 0x193A, 0x193B, 0x193D}};
    static constexpr std::array<uint32_t, 14> kIceLakeIDs = {
        {0x8A50, 0x8A51, 0x8A52, 0x8A53, 0x8A54, 0x8A56, 0x8A57,
         0x8A58, 0x8A59, 0x8A5A, 0x8A5B, 0x8A5C, 0x8A5D, 0x8A71}};
    static constexpr  std::array<uint32_t, 5> kRocketLakeIDs = {
        {0x4c8a, 0x4c8b, 0x4c8c, 0x4c90, 0x4c9a}};
    static constexpr  std::array<uint32_t, 11> kTigerLakeIDs = {
        {0x9A40, 0x9A49, 0x9A59, 0x9A60, 0x9A68, 0x9A70,
         0x9A78, 0x9AC0, 0x9AC9, 0x9AD9, 0x9AF8}};
    static constexpr  std::array<uint32_t, 10> kAlderLakeIDs = {
        {0x4680, 0x4681, 0x4682, 0x4683, 0x4690,
         0x4691, 0x4692, 0x4693, 0x4698, 0x4699}};

    if (intel_deviceID_present(kSkyLakeIDs, deviceID)) {
        return IntelGPUType::kSkyLake;
    }
    if (intel_deviceID_present(kIceLakeIDs, deviceID)) {
        return IntelGPUType::kIceLake;
    }
    if (intel_deviceID_present(kRocketLakeIDs, deviceID)) {
        return IntelGPUType::kRocketLake;
    }
    if (intel_deviceID_present(kTigerLakeIDs, deviceID)) {
        return IntelGPUType::kTigerLake;
    }
    if (intel_deviceID_present(kAlderLakeIDs, deviceID)) {
        return IntelGPUType::kAlderLake;
    }
    return IntelGPUType::kOther;
}

#if defined(GR_TEST_UTILS)
std::vector<GrTest::TestFormatColorTypeCombination> GrVkCaps::getTestingCombinations() const {
    std::vector<GrTest::TestFormatColorTypeCombination> combos = {
        { GrColorType::kAlpha_8,          GrBackendFormats::MakeVk(VK_FORMAT_R8_UNORM)            },
        { GrColorType::kBGR_565,          GrBackendFormats::MakeVk(VK_FORMAT_R5G6B5_UNORM_PACK16) },
        { GrColorType::kABGR_4444,       GrBackendFormats::MakeVk(VK_FORMAT_R4G4B4A4_UNORM_PACK16)},
        { GrColorType::kABGR_4444,       GrBackendFormats::MakeVk(VK_FORMAT_B4G4R4A4_UNORM_PACK16)},
        { GrColorType::kRGBA_8888,        GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8A8_UNORM)      },
        { GrColorType::kRGBA_8888_SRGB,   GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8A8_SRGB)       },
        { GrColorType::kRGB_888x,         GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8A8_UNORM)      },
        { GrColorType::kRGB_888x,         GrBackendFormats::MakeVk(VK_FORMAT_R8G8B8_UNORM)        },
        { GrColorType::kRG_88,            GrBackendFormats::MakeVk(VK_FORMAT_R8G8_UNORM)          },
        { GrColorType::kBGRA_8888,        GrBackendFormats::MakeVk(VK_FORMAT_B8G8R8A8_UNORM)      },
        { GrColorType::kRGBA_1010102, GrBackendFormats::MakeVk(VK_FORMAT_A2B10G10R10_UNORM_PACK32)},
        { GrColorType::kBGRA_1010102, GrBackendFormats::MakeVk(VK_FORMAT_A2R10G10B10_UNORM_PACK32)},
        { GrColorType::kRGBA_10x6,
          GrBackendFormats::MakeVk(VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16)},
        { GrColorType::kGray_8,           GrBackendFormats::MakeVk(VK_FORMAT_R8_UNORM)            },
        { GrColorType::kAlpha_F16,        GrBackendFormats::MakeVk(VK_FORMAT_R16_SFLOAT)          },
        { GrColorType::kRGBA_F16,         GrBackendFormats::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT) },
        { GrColorType::kRGBA_F16_Clamped, GrBackendFormats::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT) },
        { GrColorType::kAlpha_16,         GrBackendFormats::MakeVk(VK_FORMAT_R16_UNORM)           },
        { GrColorType::kRG_1616,          GrBackendFormats::MakeVk(VK_FORMAT_R16G16_UNORM)        },
        { GrColorType::kRGBA_16161616,    GrBackendFormats::MakeVk(VK_FORMAT_R16G16B16A16_UNORM)  },
        { GrColorType::kRG_F16,           GrBackendFormats::MakeVk(VK_FORMAT_R16G16_SFLOAT)       },
        // These two compressed formats both have an effective colorType of kRGB_888x
        { GrColorType::kRGB_888x,      GrBackendFormats::MakeVk(VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)},
        { GrColorType::kRGB_888x,         GrBackendFormats::MakeVk(VK_FORMAT_BC1_RGB_UNORM_BLOCK) },
        { GrColorType::kRGBA_8888,        GrBackendFormats::MakeVk(VK_FORMAT_BC1_RGBA_UNORM_BLOCK)},
    };

    return combos;
}
#endif
