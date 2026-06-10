/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanCaps.h"

#include "include/core/SkTextureCompressionType.h"
#include "include/gpu/graphite/ContextOptions.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/SkMath.h"
#include "src/gpu/SwizzlePriv.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/GraphiteResourceKey.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanYcbcrConversion.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include "src/sksl/SkSLUtil.h"

#ifdef SK_BUILD_FOR_ANDROID
#include <sys/system_properties.h>
#endif

namespace skgpu::graphite {

namespace {
skgpu::UniqueKey::Domain get_pipeline_domain() {
    static const skgpu::UniqueKey::Domain kVulkanGraphicsPipelineDomain =
            skgpu::UniqueKey::GenerateDomain();

    return kVulkanGraphicsPipelineDomain;
}
}  // namespace

VulkanCaps::VulkanCaps(const ContextOptions& contextOptions,
                       const skgpu::VulkanInterface* vkInterface,
                       VkPhysicalDevice physDev,
                       uint32_t physicalDeviceVersion,
                       const VkPhysicalDeviceFeatures2* features,
                       const skgpu::VulkanExtensions* extensions,
                       Protected isProtected)
        : Caps() {
    this->init(contextOptions, vkInterface, physDev, physicalDeviceVersion, features, extensions,
               isProtected);
}

VulkanCaps::~VulkanCaps() {}

namespace {
void populate_resource_binding_reqs(ResourceBindingRequirements& reqs) {
    reqs.fBackendApi = BackendApi::kVulkan;
    // We can enable std430 and ensure no array stride mismatch in functions because all bound
    // buffers will either be a UBO or SSBO, depending on if storage buffers are enabled or not.
    // Although intrinsic uniforms always use uniform buffers, they do not contain any arrays.
    reqs.fStorageBufferLayout = Layout::kStd430;

    // TODO(b/374997389): Somehow convey & enforce Layout::kStd430 for push constants.
    reqs.fUniformBufferLayout = Layout::kStd140;
    reqs.fSeparateTextureAndSamplerBinding = false;

    // Vulkan uses push constants instead of an intrinsic UBO, so we do not need to assign
    // reqs.fIntrinsicBufferBinding.
    reqs.fUsePushConstantsForIntrinsicConstants = true;

    // Assign uniform buffer binding values for shader generation
    reqs.fCombinedUniformBufferBinding = VulkanGraphicsPipeline::kCombinedUniformIndex;
    reqs.fGradientBufferBinding = VulkanGraphicsPipeline::kGradientBufferIndex;

    // Assign descriptor set indices for shader generation
    reqs.fUniformsSetIdx = VulkanGraphicsPipeline::kUniformBufferDescSetIndex;
    reqs.fTextureSamplerSetIdx = VulkanGraphicsPipeline::kTextureBindDescSetIndex;
    // Note: We use kDstAsInputDescSetIndex as opposed to kLoadMsaaFromResolveInputDescSetIndex
    // because the former is what is needed for SkSL generation purposes at the graphite level. The
    // latter is simply internally referenced when defining bespoke SkSL for loading MSAA from
    // resolve.
    reqs.fInputAttachmentSetIdx = VulkanGraphicsPipeline::kDstAsInputDescSetIndex;
}
} // anonymous

void VulkanCaps::init(const ContextOptions& contextOptions,
                      const skgpu::VulkanInterface* vkInterface,
                      VkPhysicalDevice physDev,
                      uint32_t physicalDeviceVersion,
                      const VkPhysicalDeviceFeatures2* features,
                      const skgpu::VulkanExtensions* extensions,
                      Protected isProtected) {
    const EnabledFeatures enabledFeatures =
            this->getEnabledFeatures(features, physicalDeviceVersion);

    PhysicalDeviceProperties deviceProperties;
    this->getProperties(vkInterface,
                        physDev,
                        physicalDeviceVersion,
                        extensions,
                        enabledFeatures,
                        &deviceProperties);

    const VkPhysicalDeviceLimits& deviceLimits = deviceProperties.fBase.properties.limits;
    const uint32_t vendorID = deviceProperties.fBase.properties.vendorID;

#if defined(GPU_TEST_UTILS)
    this->setDeviceName(deviceProperties.fBase.properties.deviceName);
#endif

    if (isProtected == Protected::kYes && enabledFeatures.fProtectedMemory) {
        fProtectedSupport = true;
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

    fPhysicalDeviceMemoryProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    fPhysicalDeviceMemoryProperties2.pNext = nullptr;
    VULKAN_CALL(vkInterface,
                GetPhysicalDeviceMemoryProperties2(physDev, &fPhysicalDeviceMemoryProperties2));

    // We could actually query and get a max size for each config, however maxImageDimension2D will
    // give the minimum max size across all configs. So for simplicity we will use that for now.
    fMaxTextureSize = std::min(deviceLimits.maxImageDimension2D, (uint32_t)INT_MAX);

    // Assert that our push constant sizes are below the maximum allowed (which is guaranteed to be
    // at least 128 bytes per spec).
    static_assert(VulkanResourceProvider::kIntrinsicConstantSize < 128 &&
                  VulkanResourceProvider::kLoadMSAAPushConstantSize < 128);

    fRequiredUniformBufferAlignment = deviceLimits.minUniformBufferOffsetAlignment;
    fRequiredStorageBufferAlignment = deviceLimits.minStorageBufferOffsetAlignment;
    fRequiredTransferBufferAlignment = 4;

    fMaxVaryings = std::min(deviceLimits.maxVertexOutputComponents,
                            deviceLimits.maxFragmentInputComponents) / 4;

    // Unlike D3D, WebGPU, and Metal, the Vulkan NDC coordinate space is aligned with the top-left
    // Y-down coordinate space of the viewport.
    fNDCYAxisPointsDown = true;

    populate_resource_binding_reqs(fResourceBindingReqs);

    // TODO(b/353983969): Enable storage buffers once perf regressions are addressed.
    fStorageBufferSupport = false;

    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
    VULKAN_CALL(vkInterface, GetPhysicalDeviceMemoryProperties(physDev, &deviceMemoryProperties));
    fSupportsMemorylessAttachments = false;
    VkMemoryPropertyFlags requiredLazyFlags = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    if (fProtectedSupport) {
        // If we have a protected context we can only use memoryless images if they also support
        // being protected. With current devices we don't actually expect this combination to be
        // supported, but this at least covers us for future devices that may allow it.
        requiredLazyFlags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }
    for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i) {
        const uint32_t& supportedFlags = deviceMemoryProperties.memoryTypes[i].propertyFlags;
        if ((supportedFlags & requiredLazyFlags) == requiredLazyFlags) {
            fSupportsMemorylessAttachments = true;
        }
    }

#ifdef SK_BUILD_FOR_UNIX
    if (skgpu::kNvidia_VkVendor == vendorID) {
        // On NVIDIA linux we see a big perf regression when not using dedicated image allocations.
        fShouldAlwaysUseDedicatedImageMemory = true;
    }
#endif

    if (vendorID == skgpu::kNvidia_VkVendor || vendorID == skgpu::kAMD_VkVendor) {
        // On discrete GPUs, it can be faster to read gpu-only memory compared to memory that is
        // also mappable on the host.
        fGpuOnlyBuffersMorePerformant = true;

        // On discrete GPUs we try to use special DEVICE_LOCAL and HOST_VISIBLE memory for our
        // cpu write, gpu read buffers. This memory is not ideal to be kept persistently mapped.
        // Some discrete GPUs do not expose this special memory, however we still disable
        // persistently mapped buffers for all of them since most GPUs with updated drivers do
        // expose it. If this becomes an issue we can try to be more fine grained.
        fShouldPersistentlyMapCpuToGpuBuffers = false;
    }

    // AMD advertises support for MAX_UINT vertex attributes but in reality only supports 32.
    fMaxVertexAttributes =
            vendorID == skgpu::kAMD_VkVendor ? 32 : deviceLimits.maxVertexInputAttributes;
    fMaxUniformBufferRange = deviceLimits.maxUniformBufferRange;
    fMaxStorageBufferRange = deviceLimits.maxStorageBufferRange;

#ifdef SK_BUILD_FOR_ANDROID
    if (extensions->hasExtension(
                VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, 2) &&
        extensions->hasExtension(VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME, 1)) {
        fSupportsAHardwareBufferImages = true;
    }
#endif

    fSupportsYcbcrConversion = enabledFeatures.fSamplerYcbcrConversion;
    fSupportsDeviceFaultInfo = enabledFeatures.fDeviceFault;
    fSupportsFrameBoundary = enabledFeatures.fFrameBoundary;

    fSupportsPipelineCreationCacheControl = enabledFeatures.fPipelineCreationCacheControl;

    if (enabledFeatures.fAdvancedBlendModes) {
        fBlendEqSupport = enabledFeatures.fCoherentAdvancedBlendModes
                ? BlendEquationSupport::kAdvancedCoherent
                : BlendEquationSupport::kAdvancedNoncoherent;
        fShaderCaps->fAdvBlendEqInteraction =
                SkSL::ShaderCaps::AdvBlendEqInteraction::kAutomatic_AdvBlendEqInteraction;
    }

    uint32_t queueFamilyCount = 0;
    VULKAN_CALL(vkInterface,
                GetPhysicalDeviceQueueFamilyProperties(physDev, &queueFamilyCount, nullptr));
    if (queueFamilyCount > 0) {
        skia_private::TArray<VkQueueFamilyProperties> queueProps;
        queueProps.resize_back(queueFamilyCount);
        VULKAN_CALL(vkInterface,
                    GetPhysicalDeviceQueueFamilyProperties(
                            physDev, &queueFamilyCount, queueProps.data()));
        fQueueFamilyTimestampValidBits.reserve(queueFamilyCount);
        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            fQueueFamilyTimestampValidBits.push_back(queueProps[i].timestampValidBits);
        }
    }
    fTimestampPeriod = deviceProperties.fBase.properties.limits.timestampPeriod;

    if (deviceProperties.fBase.properties.limits.timestampComputeAndGraphics &&
        fTimestampPeriod > 0) {
        fSupportedGpuStats |= GpuStatsFlags::kElapsedTime;
    }

    fOcclusionQueryPrecise = enabledFeatures.fOcclusionQueryPrecise;
    if (fOcclusionQueryPrecise) {
        fSupportedGpuStats |= GpuStatsFlags::kOcclusionPassSamples;
    }

    // Note: ARM GPUs have always been coherent, do not add a subpass self-dependency even if the
    // application hasn't enabled this feature as it comes with a performance cost on this GPU. Use
    // of VK_EXT_rasterization_order_attachment_access is disabled on ARM due to an unexplained
    // memory regression (b/437907749).
    //
    // Imagination GPUs are also coherent but only within the same sample when sample-shading.
    // VK_EXT_rasterization_order_attachment_access indicates coherence when input attachment read
    // is done from any samples of the same pixel, which is why Imagination drivers cannot expose
    // this extension. This is not a problem for Graphite however, which does not enable sample
    // shading (nor would it read color from other samples even if it did).
    fSupportsRasterizationOrderColorAttachmentAccess =
            enabledFeatures.fRasterizationOrderColorAttachmentAccess && vendorID != kARM_VkVendor;
    fIsInputAttachmentReadCoherent = fSupportsRasterizationOrderColorAttachmentAccess ||
                                     vendorID == kARM_VkVendor || vendorID == kImagination_VkVendor;

    this->initShaderCaps(enabledFeatures, vendorID);

    // Vulkan 1.0 dynamic state is always supported.  Dynamic state based on features of
    // VK_EXT_extended_dynamic_state and VK_EXT_extended_dynamic_state2 are also considered basic
    // given the extensions' age and the fact that they are core since Vulkan 1.3.
    fUseBasicDynamicState =
            enabledFeatures.fExtendedDynamicState && enabledFeatures.fExtendedDynamicState2;

    // Vertex input dynamic state depends on the main feature of
    // VK_EXT_vertex_input_dynamic_state.
    fUseVertexInputDynamicState = enabledFeatures.fVertexInputDynamicState;

    // Graphics pipeline library usage depends on the main feature of
    // VK_EXT_graphics_pipeline_library.  The graphicsPipelineLibraryFastLinking property indicates
    // whether linking libraries is cheap, without which the extension is not very useful.  However,
    // this property is currently ignored for known vendors that set it to false while link is still
    // fast.
    fUsePipelineLibraries =
            enabledFeatures.fGraphicsPipelineLibrary &&
            (deviceProperties.fGpl.graphicsPipelineLibraryFastLinking || vendorID == kARM_VkVendor);


    fSupportsFrameBoundary = enabledFeatures.fFrameBoundary;

    // Multisampled render to single-sampled usage depends on the mandatory feature of
    // VK_EXT_multisampled_render_to_single_sampled.  Per format queries are needed to determine if
    // multisampled->single-sampled rendering is supported, which should in practice always be equal
    // to whether multisampled rendering is supported for that format.
    fMSAARenderToSingleSampledSupport = enabledFeatures.fMultisampledRenderToSingleSampled;

    // Host image copy depends on the main feature of VK_EXT_host_image_copy, which is core since
    // Vulkan 1.4.  The identicalMemoryTypeRequirements property indicates whether host-copyable
    // images require special (limited) memory types.  If that property is not set, use of this
    // extension is avoided to avoid incurring performance penalties or run out of the
    // likely-much-smaller memory available on those devices.  This property is expected to be set
    // on UMA devices.
    //
    // The SHADER_READ_ONLY layout is ubiquitously found in pCopyDstLayouts, so we rely on it.  If
    // that is ever missing (unlikely, given the future direction with
    // VK_KHR_unified_image_layouts), then Recorder::update*BackendTexture should first transition
    // to GENERAL with vkTransitionImageLayout and at the end use a GPU barrier to SHADER_READ_ONLY
    // (as opposed to current code that transitions directly to SHADER_READ_ONLY and uploads to it).
    fSupportsHostImageCopy = enabledFeatures.fHostImageCopy &&
                             deviceProperties.fHic.identicalMemoryTypeRequirements &&
                             deviceProperties.fHicHasShaderReadOnlyDstLayout;

    // Note: Do not add extension/feature checks after this; driver workarounds should be done last.
    if (!contextOptions.fDisableDriverCorrectnessWorkarounds) {
        this->applyDriverCorrectnessWorkarounds(deviceProperties);
    }

    // Note that format table initialization should be performed at the end of this method to ensure
    // all capability determinations are completed prior to populating the format tables.
    this->initFormatTable(vkInterface, physDev, deviceProperties.fBase.properties, enabledFeatures);

    this->finishInitialization(contextOptions);
}

// Walk the feature chain once and extract any enabled features that Graphite cares about.
VulkanCaps::EnabledFeatures VulkanCaps::getEnabledFeatures(
        const VkPhysicalDeviceFeatures2* features, uint32_t physicalDeviceVersion) {
    EnabledFeatures enabled;
    if (features) {
        // Base features:
        enabled.fDualSrcBlend = features->features.dualSrcBlend;
        enabled.fOcclusionQueryPrecise = features->features.occlusionQueryPrecise;

        if (physicalDeviceVersion >= VK_API_VERSION_1_3) {
            enabled.fExtendedDynamicState = true;
            enabled.fExtendedDynamicState2 = true;
        }

        // Extended features:
        const VkBaseInStructure* pNext = static_cast<const VkBaseInStructure*>(features->pNext);
        while (pNext) {
            switch (pNext->sType) {
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: {
                    const auto* feature =
                            reinterpret_cast<const VkPhysicalDeviceVulkan11Features*>(pNext);
                    enabled.fSamplerYcbcrConversion = feature->samplerYcbcrConversion;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES: {
                    const auto* feature =
                            reinterpret_cast<const VkPhysicalDeviceVulkan13Features*>(pNext);
                    enabled.fPipelineCreationCacheControl = feature->pipelineCreationCacheControl;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES: {
                    const auto* feature =
                            reinterpret_cast<const VkPhysicalDeviceVulkan14Features*>(pNext);
                    enabled.fHostImageCopy = feature->hostImageCopy;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES: {
                    const auto* feature =
                            reinterpret_cast<
                                const VkPhysicalDevicePipelineCreationCacheControlFeatures*>(pNext);
                    enabled.fPipelineCreationCacheControl = feature->pipelineCreationCacheControl;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES: {
                    const auto* feature =
                            reinterpret_cast<const VkPhysicalDeviceSamplerYcbcrConversionFeatures*>(
                                    pNext);
                    enabled.fSamplerYcbcrConversion = feature->samplerYcbcrConversion;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FAULT_FEATURES_EXT: {
                    const auto* feature =
                            reinterpret_cast<const VkPhysicalDeviceFaultFeaturesEXT*>(pNext);
                    enabled.fDeviceFault = feature->deviceFault;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT: {
                    const auto* feature = reinterpret_cast<
                            const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT*>(pNext);
                    // The feature struct being present at all indicated advanced blend mode
                    // support. A member of it indicates whether the device offers coherent or
                    // noncoherent support.
                    enabled.fAdvancedBlendModes = true;
                    enabled.fCoherentAdvancedBlendModes =
                            feature->advancedBlendCoherentOperations == VK_TRUE;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT: {
                    const auto* feature = reinterpret_cast<
                            const VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT*>(
                            pNext);
                    enabled.fRasterizationOrderColorAttachmentAccess =
                            feature->rasterizationOrderColorAttachmentAccess;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT: {
                    const auto* feature = reinterpret_cast<
                            const VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*>(pNext);
                    enabled.fExtendedDynamicState = feature->extendedDynamicState;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT: {
                    const auto* feature = reinterpret_cast<
                            const VkPhysicalDeviceExtendedDynamicState2FeaturesEXT*>(pNext);
                    enabled.fExtendedDynamicState2 = feature->extendedDynamicState2;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT: {
                    const auto* feature = reinterpret_cast<
                            const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT*>(pNext);
                    enabled.fVertexInputDynamicState = feature->vertexInputDynamicState;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT: {
                    const auto* feature = reinterpret_cast<
                            const VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT*>(pNext);
                    enabled.fGraphicsPipelineLibrary = feature->graphicsPipelineLibrary;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT: {
                    const auto* feature = reinterpret_cast<
                            const VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT*>(
                            pNext);
                    enabled.fMultisampledRenderToSingleSampled =
                            feature->multisampledRenderToSingleSampled;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES: {
                    const auto* feature =
                            reinterpret_cast<const VkPhysicalDeviceHostImageCopyFeatures*>(pNext);
                    enabled.fHostImageCopy = feature->hostImageCopy;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT: {
                    const auto *feature = reinterpret_cast<
                            const VkPhysicalDeviceFrameBoundaryFeaturesEXT*>(pNext);
                    enabled.fFrameBoundary = feature->frameBoundary;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT: {
                    const auto *feature = reinterpret_cast<
                            const VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT*>(pNext);
                    enabled.fFormatRGBA10x6WithoutYCbCrSampler =
                            feature->formatRgba10x6WithoutYCbCrSampler;
                    break;
                }
                case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES: {
                    const auto *feature = reinterpret_cast<
                            const VkPhysicalDeviceProtectedMemoryFeatures*>(pNext);
                    enabled.fProtectedMemory = feature->protectedMemory;
                    break;
                }
                default:
                    break;
            }

            pNext = pNext->pNext;
        }
    }
    return enabled;
}

// Query the physical device properties that Graphite cares about.
void VulkanCaps::getProperties(const skgpu::VulkanInterface* vkInterface,
                               VkPhysicalDevice physDev,
                               uint32_t physicalDeviceVersion,
                               const skgpu::VulkanExtensions* extensions,
                               const EnabledFeatures& features,
                               PhysicalDeviceProperties* props) {
    props->fBase = {};
    props->fBase.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

    props->fDriver = {};
    props->fDriver.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;

    props->fGpl = {};
    props->fGpl.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_PROPERTIES_EXT;

    props->fHic = {};
    props->fHic.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES;

    constexpr uint32_t kHicCopyDstLayoutMax = 50;
    VkImageLayout hicCopyDstLayoutStorage[kHicCopyDstLayoutMax] = {};
    props->fHic.copyDstLayoutCount = kHicCopyDstLayoutMax;
    props->fHic.pCopyDstLayouts = hicCopyDstLayoutStorage;

    const bool hasDriverProperties =
            physicalDeviceVersion >= VK_API_VERSION_1_2 ||
            extensions->hasExtension(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME, 1);
    if (hasDriverProperties) {
        AddToPNextChain(&props->fBase, &props->fDriver);
    } else {
        SKIA_LOG_W("VK_KHR_driver_properties is not enabled, driver workarounds cannot "
                    "be correctly applied");
    }

    if (features.fGraphicsPipelineLibrary) {
        AddToPNextChain(&props->fBase, &props->fGpl);
    }

    if (features.fHostImageCopy) {
        AddToPNextChain(&props->fBase, &props->fHic);
    }

    // Graphite requires Vulkan version 1.1 or later, so vkGetPhysicalDeviceProperties2 should
    // always be available.
    VULKAN_CALL(vkInterface, GetPhysicalDeviceProperties2(physDev, &props->fBase));

    if (features.fHostImageCopy) {
        // vkTransitionImageLayout from this extension can be used to transition between image
        // layouts on the host, with the allowed old and new layouts found in pCopySrcLayouts and
        // pCopyDstLayouts respectively.  The GENERAL layout is required to be found in both.  Skia
        // has two use cases for this function; initialization (UNDEFINED->GENERAL) and after
        // texture updates (GENERAL->SHADER_READ_ONLY, per Recorder::update*BackendTexture).
        props->fHicHasShaderReadOnlyDstLayout =
                std::find(props->fHic.pCopyDstLayouts,
                          props->fHic.pCopyDstLayouts + props->fHic.copyDstLayoutCount,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // If this field is not filled, driver bug workarounds won't work correctly. It should always
    // be filled, unless filling it itself is a driver bug, or the Vulkan driver is too old. In
    // that case, make a guess of what the driver ID is, but the driver is likely to be too buggy to
    // be used by Graphite either way.
    if (props->fDriver.driverID == 0) {
        switch (props->fBase.properties.vendorID) {
            case kAMD_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_AMD_PROPRIETARY;
                break;
            case kARM_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_ARM_PROPRIETARY;
                break;
            case kBroadcom_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_BROADCOM_PROPRIETARY;
                break;
            case kGoogle_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_GOOGLE_SWIFTSHADER;
                break;
            case kImagination_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_IMAGINATION_PROPRIETARY;
                break;
            case kIntel_VkVendor:
#ifdef SK_BUILD_FOR_WIN
                props->fDriver.driverID = VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS;
#else
                props->fDriver.driverID = VK_DRIVER_ID_INTEL_OPEN_SOURCE_MESA;
#endif
                break;
            case kNvidia_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_NVIDIA_PROPRIETARY;
                break;
            case kQualcomm_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_QUALCOMM_PROPRIETARY;
                break;
            case kSamsung_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_SAMSUNG_PROPRIETARY;
                break;
            case kVeriSilicon_VkVendor:
                props->fDriver.driverID = VK_DRIVER_ID_VERISILICON_PROPRIETARY;
                break;
            default:
                // Unknown device, but this means no driver workarounds are provisioned for it so
                // driver ID remaining 0 is not going to change anything.
                break;
        }
    }
}

void VulkanCaps::applyDriverCorrectnessWorkarounds(const PhysicalDeviceProperties& properties) {
    // By default, we initialize the Android API version to 0 since we consider certain things
    // "fixed" only once above a certain version. This way, we default to enabling the workarounds.
    int androidAPIVersion = 0;
#if defined(SK_BUILD_FOR_ANDROID)
    char androidAPIVersionStr[PROP_VALUE_MAX];
    int strLength = __system_property_get("ro.build.version.sdk", androidAPIVersionStr);
    // Defaults to zero since most checks care if it is greater than a specific value. So this will
    // just default to it being less.
    androidAPIVersion = (strLength == 0) ? 0 : atoi(androidAPIVersionStr);
#endif

    const uint32_t vendorID = properties.fBase.properties.vendorID;
    const VkDriverId driverID = properties.fDriver.driverID;
    const skgpu::DriverVersion driverVersion =
            skgpu::ParseVulkanDriverVersion(driverID, properties.fBase.properties.driverVersion);

    const bool isARM = skgpu::kARM_VkVendor == vendorID;
    const bool isIntel = skgpu::kIntel_VkVendor == vendorID;
    const bool isQualcomm = skgpu::kQualcomm_VkVendor == vendorID;

    const bool isARMProprietary = isARM && VK_DRIVER_ID_ARM_PROPRIETARY == driverID;
    const bool isIntelWindowsProprietary =
            isIntel && VK_DRIVER_ID_INTEL_PROPRIETARY_WINDOWS == driverID;
    const bool isQualcommProprietary = isQualcomm && VK_DRIVER_ID_QUALCOMM_PROPRIETARY == driverID;

    // All Mali Job-Manager based GPUs have maxDrawIndirectCount==1 and all Commans-Stream Front
    // GPUs have maxDrawIndirectCount>1.  This is used as proxy to detect JM GPUs.
    const bool isMaliJobManagerArch =
            isARM && properties.fBase.properties.limits.maxDrawIndirectCount <= 1;

    // On Mali galaxy s7 we see lots of rendering issues when we suballocate VkImages.
    if (isARMProprietary && androidAPIVersion <= 28) {
        fShouldAlwaysUseDedicatedImageMemory = true;
    }

    // On Qualcomm the gpu resolves an area larger than the render pass bounds when using
    // discardable msaa attachments. This causes the resolve to resolve uninitialized data from the
    // msaa image into the resolve image. This was reproed on a Pixel4 using the DstReadShuffle GM
    // where the top half of the GM would drop out. In Ganesh we had also seen this on Arm devices,
    // but the issue hasn't appeared yet in Graphite. It may just have occurred on older Arm drivers
    // that we don't even test any more. This also occurs on swiftshader: b/303705884 in Ganesh, but
    // we aren't currently testing that in Graphite yet so leaving that off the workaround for now
    // until we run into it.
    if (isQualcommProprietary) {
        fMustLoadFullImageForMSAA = true;
    }

    // MSAA doesn't work well on Intel GPUs crbug.com/40434119, crbug.com/41470715
    if (isIntel) {
        fAvoidMSAA = true;
    }

    // Too many bugs on older ARM drivers with CSF architecture.  On JM GPUs, more bugs were
    // encountered with newer drivers, unknown if ever fixed.
    const bool avoidExtendedDynamicState =
            (isARMProprietary && driverVersion < skgpu::DriverVersion(44, 1)) ||
            isMaliJobManagerArch;

    // Known bugs in addition to ARM bugs above:
    //
    // - Cull mode dynamic state on ARM drivers prior to r52; vkCmdSetCullMode incorrectly culls
    //   non-triangle topologies, according to the errata:
    //   https://developer.arm.com/documentation/SDEN-3735689/0100/?lang=en.  However,
    //   Graphite only uses triangles and cull mode is always disabled so this driver bug is not
    //   relevant.
    if (avoidExtendedDynamicState) {
        fUseBasicDynamicState = false;
    }

    // Known bugs in vertex input dynamic state:
    //
    // - Intel windows driver, unknown if fixed: http://anglebug.com/42265637#comment9
    // - Qualcomm drivers prior to 777:  http://anglebug.com/381384988
    // - In ARM drivers prior to r48, vkCmdBindVertexBuffers2 applies strides to the wrong index
    //   when the state is dynamic, according to the errata:
    //   https://developer.arm.com/documentation/SDEN-3735689/0100/?lang=en
    if (isIntelWindowsProprietary ||
        (isARMProprietary && driverVersion < skgpu::DriverVersion(48, 0)) ||
        (isQualcommProprietary && driverVersion < skgpu::DriverVersion(512, 777))) {
        fUseVertexInputDynamicState = false;
    }

    // Qualcomm driver 512.821 is known to have rendering bugs with
    // VK_EXT_multisampled_render_to_single_sampled.
    // http://crbug.com/413427770
    if (isQualcommProprietary && driverVersion < skgpu::DriverVersion(512, 822)) {
        fMSAARenderToSingleSampledSupport = false;
    }
}

void VulkanCaps::initShaderCaps(const EnabledFeatures enabledFeatures, const uint32_t vendorID) {
    // TODO(skbug.com/40045541): We must force std430 array stride when using SSBOs since SPIR-V
    // generation cannot handle mixed array strides being passed into functions.
    fShaderCaps->fForceStd430ArrayLayout =
            fStorageBufferSupport && fResourceBindingReqs.fStorageBufferLayout == Layout::kStd430;

    // Avoid RelaxedPrecision with OpImageSampleImplicitLod due to driver bug with YCbCr sampling.
    // (skbug.com/421927604)
    fShaderCaps->fCannotUseRelaxedPrecisionOnImageSample = vendorID == kNvidia_VkVendor;
    fShaderCaps->fDualSourceBlendingSupport = enabledFeatures.fDualSrcBlend;
}

void VulkanCaps::initFormatTable(const skgpu::VulkanInterface* interface,
                                 VkPhysicalDevice physDev,
                                 const VkPhysicalDeviceProperties& properties,
                                 const EnabledFeatures& enabledFeatures) {
    // Technically without this extension and enabled feature we could still use this format to
    // sample with a ycbcr sampler. But for simplicity until we have clients requesting that, we
    // limit the use of this format to cases where we have the extension supported.
    const bool disableRGBA10x6 = !enabledFeatures.fFormatRGBA10x6WithoutYCbCrSampler;

    // Qualcomm drivers will report OUT_OF_HOST_MEMORY when binding memory to a VkImage with
    // D16_UNORM in a protected context. Using D32_SFLOAT succeeds, so clearly it's not actually
    // out of memory. D16_UNORM appears to function correctly in unprotected contexts.
    const bool disableD16InProtected =
            this->protectedSupport() && skgpu::kQualcomm_VkVendor == properties.vendorID;

    for (int i = 0; i < kTextureFormatCount; ++i) {
        SkASSERT(!SkToBool(fFormatSupport[0][i].first) && !SkToBool(fFormatSupport[0][i].second));
        SkASSERT(!SkToBool(fFormatSupport[1][i].first) && !SkToBool(fFormatSupport[1][i].second));

        TextureFormat format = static_cast<TextureFormat>(i);

        // Driver workarounds and extensions checks to keep treat a format as disabled
        if (format == TextureFormat::kD16 && disableD16InProtected) {
            continue;
        }
        if (format == TextureFormat::kRGBA10x6 && disableRGBA10x6) {
            continue;
        }

        VkFormatProperties formatProperties;
        if (format == TextureFormat::kExternal) {
            // kExternal maps to VK_FORMAT_UNDEFINED and then a specific external format is held
            // in another field of the VulkanTextureInfo. This means vkFormat can't be used to query
            // format properties, but we can also assume all external foramts are the same.
            formatProperties = { /*linearTilingFeatures=*/0,
                                 /*optimalTilingFeatures=*/VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,
                                 /*bufferFeatures=*/0};
        } else {
            VULKAN_CALL(interface, GetPhysicalDeviceFormatProperties(
                    physDev, TextureFormatToVkFormat(format), &formatProperties));
        }

        for (Tiling tiling : {Tiling::kOptimal, Tiling::kLinear}) {
            fFormatSupport[(int) tiling][i] = this->getTextureSupport(
                    interface, physDev, format, tiling, formatProperties);
        }
    }
}

std::pair<SkEnumBitMask<TextureUsage>, SkEnumBitMask<SampleCount>> VulkanCaps::getTextureSupport(
        const skgpu::VulkanInterface* interface,
        VkPhysicalDevice physDev,
        TextureFormat format,
        Tiling tiling,
        const VkFormatProperties& props) const {
    const VkFormat vkFormat = TextureFormatToVkFormat(format);

    bool isEfficientWithHostImageCopy;
    VkImageUsageFlags renderUsageFlags;
    VkFormatFeatureFlags renderBits;
    if (TextureFormatIsDepthOrStencil(format)) {
        isEfficientWithHostImageCopy = false;
        renderBits = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        renderUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    } else {
        // Assume linear tiling is always efficient for copying directly.
        isEfficientWithHostImageCopy = tiling == Tiling::kLinear ||
                                       this->isEfficientWithHostCopy(interface, physDev, vkFormat);

        renderBits = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT |
                     VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT;
        // We make all renderable images support being used as input attachment
        renderUsageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                           VK_IMAGE_USAGE_SAMPLED_BIT |
                           VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                           VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }

    SkEnumBitMask<TextureUsage> supports;
    SkEnumBitMask<SampleCount> sampleCounts;
    VkFormatFeatureFlags featureFlags = tiling == Tiling::kOptimal ? props.optimalTilingFeatures
                                                                   : props.linearTilingFeatures;

    if ((featureFlags & renderBits) == renderBits) {
        supports |= TextureUsage::kRender;

        // At least 1x sample count should be supported (the limit for linear tiling).
        sampleCounts = tiling == Tiling::kOptimal
                ? this->getSupportedSampleCounts(interface, physDev, vkFormat, renderUsageFlags)
                : SampleCount::k1;

        if (this->msaaRenderToSingleSampledSupport() &&
            SkToBool(sampleCounts) &&
            sampleCounts != SampleCount::k1) {
            // SupportedSampleCounts' initialization validates the sample counts that are
            // available when using VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT
            // so if it is more than just 1x, we can assume MSRTSS is supported for this format.
            supports |= TextureUsage::kMSRTSS;
        }
    }

    if (VkFormatNeedsYcbcrSampler(vkFormat) || format == TextureFormat::kExternal) {
        // Assume all external formats are sampleable, since we support adjusting the filtering on
        // a per-immutable sampler basis.
        supports |= TextureUsage::kSample;
    } else if ((featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
               (featureFlags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        // Otherwise require full filtering control to count as sampleable
        supports |= TextureUsage::kSample;
    }

    // NOTE: We don't check the protected-ness of the Context for format support. It is handled on
    // a per-texture basis if that texture ends up being allocated with protected memory. We
    // intentionally skip adding CopySrc for compressed formats since there is no current support
    // for read back in higher-level Graphite code.
    if ((featureFlags & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT) &&
        TextureFormatCompressionType(format) == SkTextureCompressionType::kNone) {
        supports |= TextureUsage::kCopySrc;
    }

    if (featureFlags & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) {
        // Unlike CopySrc, we include CopyDst for compressed formats since there are specialized
        // upload code paths.
        supports |= TextureUsage::kCopyDst;

        if (isEfficientWithHostImageCopy) {
            // NOTE: We will check protectedness on a texture-by-texture basis.
            supports |= TextureUsage::kHostCopy;
        }
    }

    if (featureFlags & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) {
        supports |= TextureUsage::kStorage;
    }

    // Some drivers report no sample counts for multiplanar or compressed formats, even when they
    // can be sampled. There is a pedantic argument that this is valid since neither of these types
    // of textures have conventional texels to begin with, but in practice, sampling acts as though
    // its 1x. Include 1x to simplify higher-level support checks.
    if (!SkToBool(sampleCounts & SampleCount::k1) && SkToBool(supports & TextureUsage::kSample)) {
        sampleCounts |= SampleCount::k1;
    }

    return {supports, sampleCounts};
}

std::pair<SkEnumBitMask<TextureUsage>, Tiling> VulkanCaps::getTextureUsage(
        const TextureInfo& info) const {
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(info);

    SkEnumBitMask<TextureUsage> usage;

    if (TextureFormatIsDepthOrStencil(TextureInfoPriv::ViewFormat(info))) {
        if (SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
            usage |= TextureUsage::kRender;
        }
    } else {
        // All color renderable vulkan textures within graphite must have input attachment usage
        if (SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) &&
            SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
            usage |= TextureUsage::kRender;

            // And flag MSRTSS if the creation flag was set on the texture
            if (this->msaaRenderToSingleSampledSupport() && SkToBool(vkInfo.fFlags &
                        VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT)) {
                usage |= TextureUsage::kMSRTSS;
            }
        }
    }

    // All images using external formats are required to be able to be sampled per Vulkan spec.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAndroidHardwareBufferFormatPropertiesANDROID.html#_description
    if (vkInfo.fFormat == VK_FORMAT_UNDEFINED && vkInfo.fYcbcrConversionInfo.isValid()) {
        usage |= TextureUsage::kSample;
    } else if (SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)) {
        usage |= TextureUsage::kSample;
    }

    // We include CopyDst/CopySrc without worrying about format support since that is masked out
    // automatically with getTextureSupport()'s handling of compressed and external formats.
    if (info.isProtected() == Protected::kNo &&
        SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        usage |= TextureUsage::kCopySrc;
    }

    if (SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        usage |= TextureUsage::kCopyDst;

        if (info.isProtected() == Protected::kNo &&
            SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_HOST_TRANSFER_BIT)) {
            usage |= TextureUsage::kHostCopy;
        }
    }

    if (SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT)) {
        usage |= TextureUsage::kStorage;
    }

    const Tiling tiling = vkInfo.fImageTiling == VK_IMAGE_TILING_OPTIMAL ? Tiling::kOptimal
                                                                         : Tiling::kLinear;
    return {usage, tiling};
}

TextureInfo VulkanCaps::onGetDefaultTextureInfo(SkEnumBitMask<TextureUsage> usage,
                                                TextureFormat format,
                                                SampleCount sampleCount,
                                                Mipmapped mipmapped,
                                                Protected isProtected,
                                                Discardable discardable) const {
    VkFormat vkFormat = TextureFormatToVkFormat(format);
    SkASSERT(vkFormat != VK_FORMAT_UNDEFINED); // should have been caught by Caps first

    VkImageUsageFlags vkUsage = 0;
    VkImageCreateFlags createFlags =
            isProtected == Protected::kYes ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;

    if (usage & TextureUsage::kSample) {
        vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (usage & TextureUsage::kStorage) {
        vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
    if (usage & TextureUsage::kCopySrc) {
        vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (usage & TextureUsage::kCopyDst) {
        vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        if (usage & TextureUsage::kHostCopy) {
            SkASSERT(this->supportsHostImageCopy());
            vkUsage |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
        }
    }
    if (usage & TextureUsage::kRender) {
        if (TextureFormatIsDepthOrStencil(format)) {
            vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        } else {
            // We make all renderable color images support being used as input attachment
            vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }

        if (usage & TextureUsage::kMSRTSS) {
            SkASSERT(this->msaaRenderToSingleSampledSupport());
            createFlags |= VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
        }
        if (discardable == Discardable::kYes && fSupportsMemorylessAttachments) {
            vkUsage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        }
    }

    VkImageAspectFlags vkAspectMask = 0;
    if (TextureFormatIsDepthOrStencil(format)) {
        if (TextureFormatHasDepth(format)) {
            vkAspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (TextureFormatHasStencil(format)) {
            vkAspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
        vkAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VulkanTextureInfo info;
    info.fSampleCount = sampleCount;
    info.fMipmapped = mipmapped;
    info.fFlags = createFlags;
    info.fFormat = vkFormat;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = vkUsage;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = vkAspectMask;

    return TextureInfos::MakeVulkan(info);
}

SkEnumBitMask<SampleCount> VulkanCaps::getSupportedSampleCounts(
        const skgpu::VulkanInterface* interface,
        VkPhysicalDevice physDev,
        VkFormat format,
        VkImageUsageFlags usage) const {
    VkImageFormatProperties properties;

    VkResult result;
    // VULKAN_CALL_RESULT requires a VulkanSharedContext for tracking DEVICE_LOST, but VulkanCaps
    // are initialized before a VulkanSharedContext is available. The _NOCHECK variant only requires
    // a VulkanInterface, so we can use that and log failures manually.
    VULKAN_CALL_RESULT_NOCHECK(interface,
                               result,
                               GetPhysicalDeviceImageFormatProperties(physDev,
                                                                      format,
                                                                      VK_IMAGE_TYPE_2D,
                                                                      VK_IMAGE_TILING_OPTIMAL,
                                                                      usage,
                                                                      0,  // createFlags
                                                                      &properties));
    if (result != VK_SUCCESS) {
        SKIA_LOG_W("Vulkan call GetPhysicalDeviceImageFormatProperties failed: %d", result);
        return {};
    }

    // Standard sample locations are not defined for more than 16 samples, and we don't need more
    // than 16. Omit 32 and 64.
    VkSampleCountFlags sampleCounts = properties.sampleCounts &
                    (VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT |
                     VK_SAMPLE_COUNT_8_BIT | VK_SAMPLE_COUNT_16_BIT);

    // Disable MSAA if driver workaround requires it, by pretending the format does not support any
    // sample count other than 1.
    if (this->avoidMSAA()) {
        sampleCounts &= VK_SAMPLE_COUNT_1_BIT;
    }

    // If VK_EXT_multisampled_render_to_single_sampled is used, verify that the
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT flag does not alter the
    // supported sample counts. If it does, it's not against the spec but it also doesn't make
    // practical sense (the extension is all about load (unresolve) and store (resolve) ops, it
    // shouldn't affect multisampled rendering itself). In that case, issue a warning and mask out
    // unsupported bits.
    if (this->msaaRenderToSingleSampledSupport() && sampleCounts > VK_SAMPLE_COUNT_1_BIT) {
        properties.sampleCounts = VK_SAMPLE_COUNT_1_BIT;
        VULKAN_CALL_RESULT_NOCHECK(
                interface,
                result,
                GetPhysicalDeviceImageFormatProperties(
                        physDev,
                        format,
                        VK_IMAGE_TYPE_2D,
                        VK_IMAGE_TILING_OPTIMAL,
                        usage,
                        VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT,
                        &properties));
        if (result != VK_SUCCESS && result != VK_ERROR_FORMAT_NOT_SUPPORTED) {
            SKIA_LOG_W("Vulkan call GetPhysicalDeviceImageFormatProperties failed: %d", result);
            return {};
        }
        if (result == VK_ERROR_FORMAT_NOT_SUPPORTED ||
            properties.sampleCounts <= VK_SAMPLE_COUNT_1_BIT ||
            (sampleCounts & properties.sampleCounts) != sampleCounts) {
            SKIA_LOG_W("Inconsistent MSAA rendering support in the presence of "
                        "VK_EXT_multisampled_render_to_single_sampled (Supported MSAA bits: %#X vs "
                        "with MSRTSS: %#X)",
                        sampleCounts,
                        result == VK_ERROR_FORMAT_NOT_SUPPORTED ? 0 : properties.sampleCounts);

            // Mask out the unsupported bits
            if (result == VK_SUCCESS) {
                sampleCounts &= (properties.sampleCounts | VK_SAMPLE_COUNT_1_BIT);
            } else {
                sampleCounts &= VK_SAMPLE_COUNT_1_BIT;
            }
        }
    }

    // VkSampleCount is bit equal to SampleCount, so VkSampleCountFlags will be bit-equal to
    // SkEnumBitMask<SampleCount>, but given the type wrapping we have to cast to SampleCount.
    return static_cast<SampleCount>(sampleCounts);
}

/*
 * The VK_IMAGE_USAGE_HOST_TRANSFER_BIT flag may cause the image to be put in a suboptimal physical
 * layout.  In practice, images that could have had framebuffer compression end up with framebuffer
 * compression disabled.  Using `VkHostImageCopyDevicePerformanceQuery`, we can determine if the
 * layout is going to be suboptimal and avoid this flag.
 *
 * `fIsEfficientWithHostImageCopy` indicates whether the VK_IMAGE_USAGE_HOST_TRANSFER_BIT is
 * efficient for this format with the following assumptions:
 *
 * - Image tiling is VK_IMAGE_TILING_OPTIMAL (note that VK_IMAGE_TILING_LINEAR is always
 *   efficient for host image copy).
 * - Image type is 2D.
 * - Image create flags is 0.
 * - Image usage flags is a subset of VK_IMAGE_USAGE_SAMPLED_BIT |
 *                                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
 *                                    VK_IMAGE_USAGE_TRANSFER_DST_BIT
 */
bool VulkanCaps::isEfficientWithHostCopy(const skgpu::VulkanInterface* interface,
                                         VkPhysicalDevice physDev,
                                         VkFormat format) const {
    if (!this->supportsHostImageCopy() || format == VK_FORMAT_UNDEFINED) {
        return false;
    }

    VkHostImageCopyDevicePerformanceQuery perfQuery = {};
    perfQuery.sType = VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY_EXT;

    VkPhysicalDeviceImageFormatInfo2 imageFormatInfo = {};
    imageFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
    imageFormatInfo.format = format;
    imageFormatInfo.type = VK_IMAGE_TYPE_2D;
    imageFormatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageFormatInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
    imageFormatInfo.flags = 0;

    VkImageFormatProperties2 imageFormatProperties2 = {};
    imageFormatProperties2.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
    imageFormatProperties2.pNext = &perfQuery;

    bool isEfficient = false;
    if (VULKAN_CALL(interface,
                    GetPhysicalDeviceImageFormatProperties2(
                            physDev, &imageFormatInfo, &imageFormatProperties2)) == VK_SUCCESS) {
        // There are two results returned in `perfQuery`:
        //
        // * `identicalMemoryLayout` indicates that the added flag does not affect the physical
        //   layout of the image. We can definitely add the flag in this case.
        // * `optimalDeviceAccess` indicates that the added flag _does_ change the physical layout
        //   of the image, but that according to the driver authors the fallback layout is still
        //   "pretty good, you won't know the difference".
        //
        // For now, host image copy is only used if `identicalMemoryLayout` is true, but we could
        // consider enabling it when only `optimalDeviceAccess` is true based on experimenting on
        // different vendors.
        isEfficient = perfQuery.identicalMemoryLayout;
    }
    return isEfficient;
}

// 4 uint32s for the render step id, paint id, compatible render pass description, and write
// swizzle.
static constexpr uint16_t kPipelineKeyData32Count = 4;

static constexpr int kPipelineKeyRenderStepIDIndex = 0;
static constexpr int kPipelineKeyPaintParamsIDIndex = 1;
static constexpr int kPipelineKeyRenderPassDescIndex = 2;
static constexpr int kPipelineKeyWriteSwizzleIndex = 3;

UniqueKey VulkanCaps::makeGraphicsPipelineKey(const GraphicsPipelineDesc& pipelineDesc,
                                              const RenderPassDesc& renderPassDesc) const {
    UniqueKey pipelineKey;
    {
        UniqueKey::Builder builder(
                &pipelineKey, get_pipeline_domain(), kPipelineKeyData32Count, "GraphicsPipeline");

        // Add GraphicsPipelineDesc information
        builder[kPipelineKeyRenderStepIDIndex] = static_cast<uint32_t>(pipelineDesc.renderStepID());
        builder[kPipelineKeyPaintParamsIDIndex] = pipelineDesc.paintParamsID().asUInt();
        // Add RenderPassDesc information
        builder[kPipelineKeyRenderPassDescIndex] = VulkanRenderPass::GetRenderPassKey(
                renderPassDesc, /*compatibleForPipelineKey=*/true);
        // Add RenderPass info relevant for pipeline creation that's not captured in RenderPass keys
        builder[kPipelineKeyWriteSwizzleIndex] = renderPassDesc.fWriteSwizzle.asKey();

        builder.finish();
    }

    return pipelineKey;
}

bool VulkanCaps::extractGraphicsDescs(const UniqueKey& key,
                                      GraphicsPipelineDesc* pipelineDesc,
                                      RenderPassDesc* renderPassDesc,
                                      const RendererProvider* rendererProvider) const {
    SkASSERT(key.domain() == get_pipeline_domain());
    SkASSERT(key.dataSize() == 4 * kPipelineKeyData32Count);

    const uint32_t* rawKeyData = key.data();

    SkASSERT(RenderStep::IsValidRenderStepID(rawKeyData[kPipelineKeyRenderStepIDIndex]));
    RenderStep::RenderStepID renderStepID =
            static_cast<RenderStep::RenderStepID>(rawKeyData[kPipelineKeyRenderStepIDIndex]);

    *pipelineDesc =
            GraphicsPipelineDesc(renderStepID,
                                 UniquePaintParamsID(rawKeyData[kPipelineKeyPaintParamsIDIndex]));

    const uint32_t rpDescBits = rawKeyData[kPipelineKeyRenderPassDescIndex];
    VulkanRenderPass::ExtractRenderPassDesc(
            rpDescBits,
            SwizzleCtorAccessor::Make(rawKeyData[kPipelineKeyWriteSwizzleIndex]),
            this->getDstReadStrategy(),
            renderPassDesc);

    return true;
}

void VulkanCaps::buildKeyForTexture(SkISize dimensions,
                                    const TextureInfo& info,
                                    ResourceType type,
                                    GraphiteResourceKey* key) const {
    SkASSERT(!dimensions.isEmpty());

    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(info);
    // We expect that the VkFormat enum is at most a 32-bit value.
    static_assert(VK_FORMAT_MAX_ENUM == 0x7FFFFFFF);
    // We should either be using a known VkFormat or have a valid ycbcr conversion.
    SkASSERT(vkInfo.fFormat != VK_FORMAT_UNDEFINED || vkInfo.fYcbcrConversionInfo.isValid());

    uint32_t format = static_cast<uint32_t>(vkInfo.fFormat);
    uint32_t samples = SamplesToKey(info.sampleCount());
    // We don't have to key the number of mip levels because it is inherit in the combination of
    // isMipped and dimensions.
    bool isMipped = info.mipmapped() == Mipmapped::kYes;
    Protected isProtected = info.isProtected();

    // Confirm all the below parts of the key can fit in a single uint32_t. The sum of the shift
    // amounts in the asserts must be less than or equal to 32. vkInfo.fFlags and
    // vkInfo.fImageUsageFlags will go into their own 32-bit block.
    SkASSERT(samples                            < (1u << 3));  // sample key is first 3 bits
    SkASSERT(static_cast<uint32_t>(isMipped)    < (1u << 1));  // isMapped is 4th bit
    SkASSERT(static_cast<uint32_t>(isProtected) < (1u << 1));  // isProtected is 5th bit
    SkASSERT(vkInfo.fImageTiling                < (1u << 1));  // imageTiling is 6th bit
    SkASSERT(vkInfo.fSharingMode                < (1u << 1));  // sharingMode is 7th bit
    SkASSERT(vkInfo.fAspectMask                 < (1u << 11)); // aspectMask is bits 8 - 19

    // We need two uint32_ts for dimensions and 3 for miscellaneous information.
    static constexpr uint16_t kNum32DimensionDataCnt = 2;
    static constexpr uint16_t kNum32MiscDataCnt = 3;
    // Non-YCbCr formats need 1 int for format.
    // YCbCr conversion needs 1 int for non-format flags, and a 64-bit format (external or regular).
    static constexpr uint16_t kNum32FormatDataCntNoYcbcr = 1;
    static constexpr uint16_t kNum32FormatDataCntYcbcr = 3;

    const VulkanYcbcrConversionInfo& ycbcrInfo = vkInfo.fYcbcrConversionInfo;
    const uint16_t num32DataCnt =
            kNum32DimensionDataCnt + kNum32MiscDataCnt +
            (ycbcrInfo.isValid() ? kNum32FormatDataCntYcbcr : kNum32FormatDataCntNoYcbcr);

    GraphiteResourceKey::Builder builder(key, type, num32DataCnt);

    int i = 0;
    builder[i++] = dimensions.width();
    builder[i++] = dimensions.height();

    if (ycbcrInfo.isValid()) {
        SkASSERT(ycbcrInfo.format() != VK_FORMAT_UNDEFINED || ycbcrInfo.hasExternalFormat());
        ImmutableSamplerInfo packedInfo = VulkanYcbcrConversion::ToImmutableSamplerInfo(ycbcrInfo);

        builder[i++] = packedInfo.fNonFormatYcbcrConversionInfo;
        builder[i++] = (uint32_t) packedInfo.fFormat;
        builder[i++] = (uint32_t) (packedInfo.fFormat >> 32);
    } else {
        builder[i++] = format;
    }

    builder[i++] = static_cast<uint32_t>(vkInfo.fFlags);
    builder[i++] = static_cast<uint32_t>(vkInfo.fImageUsageFlags);
    builder[i++] = (samples                                    << 0) |
                   (static_cast<uint32_t>(isMipped)            << kNumSampleKeyBits) |
                   (static_cast<uint32_t>(isProtected)         << 4) |
                   (static_cast<uint32_t>(vkInfo.fImageTiling) << 5) |
                   (static_cast<uint32_t>(vkInfo.fSharingMode) << 6) |
                   (static_cast<uint32_t>(vkInfo.fAspectMask)  << 7);
    SkASSERT(i == num32DataCnt);
}

DstReadStrategy VulkanCaps::getDstReadStrategy() const {
    // We know the graphite Vulkan backend does not support frame buffer fetch, so make sure it is
    // not marked as supported and skip checking for it.
    SkASSERT(!this->shaderCaps()->fFBFetchSupport);

    // All render target textures are expected to have VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT.
    return DstReadStrategy::kReadFromInput;
}

ImmutableSamplerInfo VulkanCaps::getImmutableSamplerInfo(const TextureInfo& textureInfo) const {
    const skgpu::VulkanYcbcrConversionInfo& ycbcrConversionInfo =
            TextureInfoPriv::Get<VulkanTextureInfo>(textureInfo).fYcbcrConversionInfo;

    if (ycbcrConversionInfo.isValid()) {
        return VulkanYcbcrConversion::ToImmutableSamplerInfo(ycbcrConversionInfo);
    }

    // If the YCbCr conversion for the TextureInfo is invalid, then return a default
    // ImmutableSamplerInfo struct.
    return {};
}

std::string VulkanCaps::toString(const ImmutableSamplerInfo& immutableSamplerInfo) const {
    return VulkanYcbcrConversion::InfoToString(
            VulkanYcbcrConversion::FromImmutableSamplerInfo(immutableSamplerInfo));
}

} // namespace skgpu::graphite
