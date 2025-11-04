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
#include "include/private/base/SkMath.h"
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
    reqs.fRenderStepBufferBinding =
            VulkanGraphicsPipeline::kRenderStepUniformBufferIndex;
    reqs.fPaintParamsBufferBinding =  VulkanGraphicsPipeline::kPaintUniformBufferIndex;
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

    // Graphite requires Vulkan version 1.1 or later, which always has protected support. The
    // protectedMemory feature is assumed enabled if isProtected is true.
    if (isProtected == Protected::kYes) {
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
            VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME, 2)) {
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
    this->initDepthStencilFormatTable(vkInterface, physDev, deviceProperties.fBase.properties);

    this->finishInitialization(contextOptions);
}

// Walk the feature chain once and extract any enabled features that Graphite cares about.
VulkanCaps::EnabledFeatures VulkanCaps::getEnabledFeatures(
        const VkPhysicalDeviceFeatures2* features, uint32_t physicalDeviceVersion) {
    EnabledFeatures enabled;
    if (features) {
        // Base features:
        enabled.fDualSrcBlend = features->features.dualSrcBlend;

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
        SKGPU_LOG_W("VK_KHR_driver_properties is not enabled, driver workarounds cannot "
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
    // but the issue hasn't appeared yet in Graphite. It may just have occured on older Arm drivers
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
    VK_FORMAT_B8G8R8A8_SRGB,
    VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
    VK_FORMAT_BC1_RGB_UNORM_BLOCK,
    VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    VK_FORMAT_R16_UNORM,
    VK_FORMAT_R16G16_UNORM,
    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
    VK_FORMAT_R16G16B16A16_UNORM,
    VK_FORMAT_R16G16_SFLOAT,
    VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16
};
// These are all the valid depth/stencil formats that we support in Skia.
static constexpr VkFormat kDepthStencilVkFormats[] = {
    VK_FORMAT_S8_UINT,
    VK_FORMAT_D16_UNORM,
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
};

bool VulkanCaps::isSampleCountSupported(TextureFormat format, uint8_t requestedSampleCount) const {
    VkFormat vkFormat = TextureFormatToVkFormat(format);
    const SupportedSampleCounts* sampleCounts;

    // TODO(b/390473370): When Caps stores the format tables, the color format and depth stencil
    // format infos will be combined and this will be simplified.
    if (TextureFormatIsDepthOrStencil(format)) {
        const DepthStencilFormatInfo& formatInfo = this->getDepthStencilFormatInfo(vkFormat);
        if (!formatInfo.isDepthStencilSupported(
                formatInfo.fFormatProperties.optimalTilingFeatures)) {
            return false;
        }
        sampleCounts = &formatInfo.fSupportedSampleCounts;
    } else {
        const FormatInfo& formatInfo = this->getFormatInfo(vkFormat);
        if (!formatInfo.isRenderable(VK_IMAGE_TILING_OPTIMAL, 1)) {
            return false;
        }
        sampleCounts = &formatInfo.fSupportedSampleCounts;
    }

    return sampleCounts->isSampleCountSupported(requestedSampleCount);
}

TextureFormat VulkanCaps::getDepthStencilFormat(SkEnumBitMask<DepthStencilFlags> flags) const {
    VkFormat format = fDepthStencilFlagsToFormatTable[flags.value()];
    return VkFormatToTextureFormat(format);
}

TextureInfo VulkanCaps::getDefaultAttachmentTextureInfo(AttachmentDesc desc,
                                                        Protected isProtected,
                                                        Discardable discardable) const {
    if ((isProtected == Protected::kYes && !this->protectedSupport()) ||
         !this->isSampleCountSupported(desc.fFormat, desc.fSampleCount)) {
        return {};
    }

    const bool isDepthStencil = TextureFormatIsDepthOrStencil(desc.fFormat);

    /**
     * Graphite, unlike ganesh, does not require a dedicated MSAA attachment on every surface.
     * MSAA textures now get resolved within the scope of a render pass, which can be done simply
     * with the color attachment usage flag. So we no longer require transfer src/dst usage flags.
     * All renderable textures in Vulkan are made with input attachment usage.
     */
    VkImageCreateFlags createFlags =
            (isProtected == Protected::kYes) ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    VkImageUsageFlags usageFlags = isDepthStencil ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                                  : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                                            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

    if (discardable == Discardable::kYes && fSupportsMemorylessAttachments) {
        usageFlags = usageFlags | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    }

    /*
     * When VK_EXT_multisampled_render_to_single_sampled is supported, proactively use the
     * VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT flag. This flag is expected to
     * be harmless (if not, it's a driver bug).
     */
    if (desc.fSampleCount == 1 && this->msaaRenderToSingleSampledSupport()) {
        createFlags |= VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
    }

    VulkanTextureInfo info;
    info.fSampleCount = desc.fSampleCount;
    info.fMipmapped = Mipmapped::kNo;
    info.fFlags = createFlags;
    info.fFormat = TextureFormatToVkFormat(desc.fFormat);
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = usageFlags;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return TextureInfos::MakeVulkan(info);
}

TextureInfo VulkanCaps::getDefaultSampledTextureInfo(SkColorType ct,
                                                     Mipmapped mipmapped,
                                                     Protected isProtected,
                                                     Renderable isRenderable) const {
    VkFormat format = this->getFormatFromColorType(ct);
    const FormatInfo& formatInfo = this->getFormatInfo(format);

    static constexpr int kSingleSampled = 1;
    if ((isProtected == Protected::kYes && !this->protectedSupport()) ||
        !formatInfo.isTexturable(VK_IMAGE_TILING_OPTIMAL) ||
        (isRenderable == Renderable::kYes &&
         !formatInfo.isRenderable(VK_IMAGE_TILING_OPTIMAL, kSingleSampled)) ) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = kSingleSampled;
    info.fMipmapped = mipmapped;
    info.fFlags = (isProtected == Protected::kYes) ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fFormat = format;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (isRenderable == Renderable::kYes) {
        // We make all renderable images support being used as input attachment
        info.fImageUsageFlags = info.fImageUsageFlags |
                                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        // Proactively prepare the image for multisampled rendering.
        if (this->msaaRenderToSingleSampledSupport()) {
            info.fFlags |= VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
        }
    } else {
        // On every known driver where VK_EXT_host_image_copy is used by Skia, it is known that
        // using the host-image-copy flag reduces the performance of renderable images. So, we don't
        // even bother with a query in the `Renderable::kYes` case.
        if (formatInfo.isEfficientWithHostImageCopy(info.fImageTiling, isProtected)) {
            info.fImageUsageFlags |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
        }
    }
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return TextureInfos::MakeVulkan(info);
}

TextureInfo VulkanCaps::getTextureInfoForSampledCopy(const TextureInfo& textureInfo,
                                                     Mipmapped mipmapped) const {
    VulkanTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = mipmapped;
    info.fFormat = TextureInfoPriv::Get<VulkanTextureInfo>(textureInfo).fFormat;
    info.fFlags = (textureInfo.isProtected() == Protected::kYes) ?
            VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    const FormatInfo& formatInfo = this->getFormatInfo(info.fFormat);
    if (formatInfo.isEfficientWithHostImageCopy(info.fImageTiling, textureInfo.isProtected())) {
        info.fImageUsageFlags |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
    }
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return TextureInfos::MakeVulkan(info);
}

namespace {
VkFormat format_from_compression(SkTextureCompressionType compression) {
    switch (compression) {
        case SkTextureCompressionType::kETC2_RGB8_UNORM:
            return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case SkTextureCompressionType::kBC1_RGB8_UNORM:
            return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case SkTextureCompressionType::kBC1_RGBA8_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        default:
            return VK_FORMAT_UNDEFINED;
    }
}
}

TextureInfo VulkanCaps::getDefaultCompressedTextureInfo(SkTextureCompressionType compression,
                                                        Mipmapped mipmapped,
                                                        Protected isProtected) const {
    VkFormat format = format_from_compression(compression);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    static constexpr int defaultSampleCount = 1;
    if ((isProtected == Protected::kYes && !this->protectedSupport()) ||
        !formatInfo.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = defaultSampleCount;
    info.fMipmapped = mipmapped;
    info.fFlags = (isProtected == Protected::kYes) ? VK_IMAGE_CREATE_PROTECTED_BIT : 0;
    info.fFormat = format;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    info.fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (formatInfo.isEfficientWithHostImageCopy(info.fImageTiling, isProtected)) {
        info.fImageUsageFlags |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
    }
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return TextureInfos::MakeVulkan(info);
}

TextureInfo VulkanCaps::getDefaultStorageTextureInfo(SkColorType colorType) const {
    VkFormat format = this->getFormatFromColorType(colorType);
    const FormatInfo& formatInfo = this->getFormatInfo(format);
    if (!formatInfo.isTexturable(VK_IMAGE_TILING_OPTIMAL) ||
        !formatInfo.isStorage(VK_IMAGE_TILING_OPTIMAL)) {
        return {};
    }

    VulkanTextureInfo info;
    info.fSampleCount = 1;
    info.fMipmapped = Mipmapped::kNo;
    info.fFlags = 0;
    info.fFormat = format;
    info.fImageTiling = VK_IMAGE_TILING_OPTIMAL;
    // Storage textures are currently always sampleable from a shader
    info.fImageUsageFlags = VK_IMAGE_USAGE_STORAGE_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    info.fSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.fAspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    return TextureInfos::MakeVulkan(info);
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
    static_assert(std::size(kVkFormats) == VulkanCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");

    std::fill_n(fColorTypeToFormatTable, kSkColorTypeCnt, VK_FORMAT_UNDEFINED);

    // NOTE: VkFormat's naming convention orders channels from low address to high address when
    // interpreting unpacked formats. For packed formats, the channels are ordered most significant
    // to least significant (making them opposite of the unpacked).

    // Go through all the formats and init their support surface and data ColorTypes.
    // Format: VK_FORMAT_R8G8B8A8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGBA_8888
            {
                constexpr SkColorType ct = SkColorType::kRGBA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8G8B8A8_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 3;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8_UNORM, Surface: kR_8
            {
                constexpr SkColorType ct = SkColorType::kR8_unorm_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kAlpha_8
            {
                constexpr SkColorType ct = SkColorType::kAlpha_8_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
            // Format: VK_FORMAT_R8_UNORM, Surface: kGray_8
            {
                constexpr SkColorType ct = SkColorType::kGray_8_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_B8G8R8A8_UNORM, Surface: kBGRA_8888
            {
                constexpr SkColorType ct = SkColorType::kBGRA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // TODO: This is currently brittle, but add kExternalFormatColorType as a valid color
            // type for VK_FORMAT_B8G8R8A8_UNORM in order to pass texture format + color type
            // compatibility checks.
            //
            // b/431290055 exposed an issue where we could end up using an SkColorType that is not
            // compatible with the BackendTexture's VkFormat. In this case, the driver reported that
            // an AHardwareBuffer with the format AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM (only
            // present in Android framework builds) could be treated as VK_FORMAT_B8G8R8A8_UNORM.
            // This is not an officially supported equivalency per the spec (
            // https://docs.vulkan.org/spec/latest/chapters/memory.html#memory-external-android-hardware-buffer-formats),
            // but clients would benefit from having the driver's suggested VkFormat be used rather
            // than falling back to importing the AHwBuf using its external format (which is less
            // performant).
            //
            // Using the driver-recommended VkFormat led to failing Skia checks which rely upon
            // surfaces + images having an SkColorType that is compatible with a texture's native
            // format. When *creating* a Surface or Image from an AHwBuf-based BackendTexture, its
            // color type is determined by the *AHwBuf format* (using
            // `AHardwareBufferUtils::GetSkColorTypeFromBufferFormat`). However, when
            // *validating* the surface/image, Skia checks whether the color type is compatible with
            // the texture's *VkFormat* (which is distinct from an AndroidHardwareBuffer format).
            //
            // Ideally, we would simply add the AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM -->
            // VK_FORMAT_B8G8R8A8_UNORM mapping to `GetSkColorTypeFromBufferFormat`. However, this
            // utility is public and shared b/w both ganesh and graphite. Adding this introduced
            // nontrivial complexity in ganesh (e.g., its SkImages::DeferredFromAHardwareBuffer API
            // does not know the AHwBuf format at creation). This could be worked around, but the
            // change would be more invasive and no clients have requested that we allow the use of
            // a driver's non-spec reported VkFormat equivalency in ganesh. Therefore, we simply
            // default to assigning kExternalFormatColorType for
            // AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM and add it as a supported color type for
            // VK_FORMAT_B8G8R8A8_UNORM here in the Caps table. This should allow BackendTextures
            // based on AHwBufs with this format to pass validity checks and theoretically still be
            // used normally (see below for explanation as to why this works).
            //
            // Format: VK_FORMAT_B8G8R8A8_UNORM, Surface: kRGBA_8888
            {
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                // `Caps::areColorTypeAndTextureInfoCompatible` consults the fColorType field, so
                // make sure it aligns with the color type we expect to see for AHardwareBuffers
                // that use AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM (kExternalFormatColorType).
                ctInfo.fColorType = kExternalFormatColorType;
                // fTransferColorType is currently not referenced, but the actual color type
                // (e.g. for readbacks) should be kBGRA_8888_SkColorType so use that here. Simply
                // omit assigning any read/write swizzles because we actually already know the
                // texture format is compatible with kBGRA_8888_SkColorType.
                constexpr SkColorType transferColorType = SkColorType::kBGRA_8888_SkColorType;
                ctInfo.fTransferColorType = transferColorType;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R5G6B5_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R5G6B5_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R5G6B5_UNORM_PACK16, Surface: kRGB_565_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kRGB_565_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: kRGBA_F16_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kRGBA_F16_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_R16G16B16A16_SFLOAT, Surface: kRGB_F16F16F16x_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kRGB_F16F16F16x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle::RGB1();
            }
        }
    }
    // Format: VK_FORMAT_R16_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_R16_SFLOAT;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_SFLOAT, Surface: kAlpha_F16
            {
                constexpr SkColorType ct = SkColorType::kA16_float_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                // This SkColorType is a lie, but we don't have a kRGB_888_SkColorType. The Vulkan
                // format is 3 bpp so we must manualy convert to/from this and kRGB_888x when doing
                // transfers. We signal this need for manual conversions in the
                // supportedRead/WriteColorType calls.
                ctInfo.fTransferColorType = SkColorType::kRGB_888x_SkColorType;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R8G8_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R8G8_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8_UNORM, Surface: kR8G8_unorm
            {
                constexpr SkColorType ct = SkColorType::kR8G8_unorm_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32, Surface: kRGBA_1010102
            {
                constexpr SkColorType ct = SkColorType::kRGBA_1010102_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
            // Format: VK_FORMAT_A2B10G10R10_UNORM_PACK32, Surface: kRGB_101010x
            {
                constexpr SkColorType ct = SkColorType::kRGB_101010x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle::RGB1();
            }
        }
    }
    // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32
    {
        constexpr VkFormat format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_A2R10G10B10_UNORM_PACK32, Surface: kBGRA_1010102
            {
                constexpr SkColorType ct = SkColorType::kBGRA_1010102_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_B4G4R4A4_UNORM_PACK16, Surface: kARGB_4444_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kARGB_4444_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                // The color type is misnamed and really stores ABGR data, but there is no
                // SkColorType that matches this actual ARGB VkFormat data. Swapping R and B when
                // rendering into it has it match the reported transfer color type, but we have to
                // swap R and B when sampling as well. This only works so long as we don't present
                // textures of this format to a screen that would not know about this swap.
                ctInfo.fReadSwizzle = skgpu::Swizzle::BGRA();
                ctInfo.fWriteSwizzle = skgpu::Swizzle::BGRA();
            }
        }
    }

    // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16
    {
        constexpr VkFormat format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R4G4B4A4_UNORM_PACK16, Surface: kARGB_4444_SkColorType
            {
                constexpr SkColorType ct = SkColorType::kARGB_4444_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R8G8B8A8_SRGB, Surface: kRGBA_8888_SRGB
            {
                constexpr SkColorType ct = SkColorType::kSRGBA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = SkColorType::kSRGBA_8888_SkColorType;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_B8G8R8A8_SRGB
    {
        constexpr VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_B8G8R8A8_SRGB, Surface: kRGBA_8888_SRGB
            {
                constexpr SkColorType ct = SkColorType::kSRGBA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                // Since the B and R channels are swapped and there's no BGRA sRGB color type,
                // just disable read/writes back to the CPU.
                ctInfo.fTransferColorType = SkColorType::kUnknown_SkColorType;
                ctInfo.fFlags = ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 2;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16_UNORM, Surface: kAlpha_16
            {
                constexpr SkColorType ct = SkColorType::kA16_unorm_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
                ctInfo.fReadSwizzle = skgpu::Swizzle("000r");
                ctInfo.fWriteSwizzle = skgpu::Swizzle("a000");
            }
            // Format: VK_FORMAT_R16_UNORM, Surface: kR16_unorm
            {
                constexpr SkColorType ct = SkColorType::kR16_unorm_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }
    // Format: VK_FORMAT_R16G16_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_R16G16_UNORM;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_UNORM, Surface: kRG_1616
            {
                constexpr SkColorType ct = SkColorType::kR16G16_unorm_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16B16A16_UNORM, Surface: kRGBA_16161616
            {
                constexpr SkColorType ct = SkColorType::kR16G16B16A16_unorm_SkColorType;
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
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R16G16_SFLOAT, Surface: kRG_F16
            {
                constexpr SkColorType ct = SkColorType::kR16G16_float_SkColorType;
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
            info.init(interface, *this, physDev, format);
        }
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
            SkDEBUGCODE(info.fIsWrappedOnly = true;)
        }
    }
    // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM
    {
        constexpr VkFormat format = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
        auto& info = this->getFormatInfo(format);
        if (fSupportsYcbcrConversion) {
            info.init(interface, *this, physDev, format);
        }
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_G8_B8R8_2PLANE_420_UNORM, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
            SkDEBUGCODE(info.fIsWrappedOnly = true;)
        }
    }
    // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }

    // Format: VK_FORMAT_BC1_RGB_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_BC1_RGB_UNORM_BLOCK, Surface: kRGB_888x
            {
                constexpr SkColorType ct = SkColorType::kRGB_888x_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }

    // Format: VK_FORMAT_BC1_RGBA_UNORM_BLOCK
    {
        constexpr VkFormat format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_BC1_RGBA_UNORM_BLOCK, Surface: kRGBA_8888
            {
                constexpr SkColorType ct = SkColorType::kRGBA_8888_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag;
            }
        }
    }

    // Format: VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16
    // Technically without this extension and enabled feature we could still use this format to
    // sample with a ycbcr sampler. But for simplicity until we have clients requesting that, we
    // limit the use of this format to cases where we have the extension supported.
    if (enabledFeatures.fFormatRGBA10x6WithoutYCbCrSampler) {
        constexpr VkFormat format = VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;
        auto& info = this->getFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.isTexturable(VK_IMAGE_TILING_OPTIMAL)) {
            info.fColorTypeInfoCount = 1;
            info.fColorTypeInfos = std::make_unique<ColorTypeInfo[]>(info.fColorTypeInfoCount);
            int ctIdx = 0;
            // Format: VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16, Surface: kRGBA_10x6
            {
                constexpr SkColorType ct = SkColorType::kRGBA_10x6_SkColorType;
                auto& ctInfo = info.fColorTypeInfos[ctIdx++];
                ctInfo.fColorType = ct;
                ctInfo.fTransferColorType = ct;
                ctInfo.fFlags = ColorTypeInfo::kUploadData_Flag | ColorTypeInfo::kRenderable_Flag;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Map SkColorType (used for creating Surfaces) to VkFormats. The order in which the formats are
    // passed into the setColorType function indicates the priority in selecting which format we use
    // for a given SkColorType.
    typedef SkColorType ct;

    this->setColorType(ct::kAlpha_8_SkColorType,            { VK_FORMAT_R8_UNORM                 });
    this->setColorType(ct::kRGB_565_SkColorType,            { VK_FORMAT_R5G6B5_UNORM_PACK16      });
    this->setColorType(ct::kARGB_4444_SkColorType,          { VK_FORMAT_R4G4B4A4_UNORM_PACK16,
                                                              VK_FORMAT_B4G4R4A4_UNORM_PACK16    });
    this->setColorType(ct::kRGBA_8888_SkColorType,          { VK_FORMAT_R8G8B8A8_UNORM           });
    this->setColorType(ct::kSRGBA_8888_SkColorType,         { VK_FORMAT_R8G8B8A8_SRGB,
                                                              VK_FORMAT_B8G8R8A8_SRGB            });
    this->setColorType(ct::kRGB_888x_SkColorType,           { VK_FORMAT_R8G8B8_UNORM,
                                                              VK_FORMAT_R8G8B8A8_UNORM           });
    this->setColorType(ct::kR8G8_unorm_SkColorType,         { VK_FORMAT_R8G8_UNORM               });
    this->setColorType(ct::kBGRA_8888_SkColorType,          { VK_FORMAT_B8G8R8A8_UNORM           });
    this->setColorType(ct::kRGBA_1010102_SkColorType,       { VK_FORMAT_A2B10G10R10_UNORM_PACK32 });
    this->setColorType(ct::kBGRA_1010102_SkColorType,       { VK_FORMAT_A2R10G10B10_UNORM_PACK32 });
    this->setColorType(ct::kRGB_101010x_SkColorType,        { VK_FORMAT_A2B10G10R10_UNORM_PACK32 });
    this->setColorType(ct::kGray_8_SkColorType,             { VK_FORMAT_R8_UNORM                 });
    this->setColorType(ct::kA16_float_SkColorType,          { VK_FORMAT_R16_SFLOAT               });
    this->setColorType(ct::kRGBA_F16_SkColorType,           { VK_FORMAT_R16G16B16A16_SFLOAT      });
    this->setColorType(ct::kRGB_F16F16F16x_SkColorType,     { VK_FORMAT_R16G16B16A16_SFLOAT      });
    this->setColorType(ct::kA16_unorm_SkColorType,          { VK_FORMAT_R16_UNORM                });
    this->setColorType(ct::kR16_unorm_SkColorType,          { VK_FORMAT_R16_UNORM                });
    this->setColorType(ct::kR16G16_unorm_SkColorType,       { VK_FORMAT_R16G16_UNORM             });
    this->setColorType(ct::kR16G16B16A16_unorm_SkColorType, { VK_FORMAT_R16G16B16A16_UNORM       });
    this->setColorType(ct::kR16G16_float_SkColorType,       { VK_FORMAT_R16G16_SFLOAT            });
    this->setColorType(ct::kRGBA_10x6_SkColorType,          { VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 });
}

namespace {
void set_ds_flags_to_format(VkFormat& slot, VkFormat format) {
    if (slot == VK_FORMAT_UNDEFINED) {
        slot = format;
    }
}
} // namespace

void VulkanCaps::initDepthStencilFormatTable(const skgpu::VulkanInterface* interface,
                                             VkPhysicalDevice physDev,
                                             const VkPhysicalDeviceProperties& properties) {
    static_assert(std::size(kDepthStencilVkFormats) == VulkanCaps::kNumDepthStencilVkFormats,
                  "Size of DepthStencilVkFormats array must match static value in header");

    using DSFlags = SkEnumBitMask<DepthStencilFlags>;
    constexpr DSFlags stencilFlags = DepthStencilFlags::kStencil;
    constexpr DSFlags depthFlags = DepthStencilFlags::kDepth;
    constexpr DSFlags dsFlags = DepthStencilFlags::kDepthStencil;

    std::fill_n(fDepthStencilFlagsToFormatTable, kNumDepthStencilFlags, VK_FORMAT_UNDEFINED);
    // Format: VK_FORMAT_S8_UINT
    {
        constexpr VkFormat format = VK_FORMAT_S8_UINT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[stencilFlags.value()], format);
        }
    }
    // Format: VK_FORMAT_D16_UNORM
    {
        // Qualcomm drivers will report OUT_OF_HOST_MEMORY when binding memory to a VkImage with
        // D16_UNORM in a protected context. Using D32_SFLOAT succeeds, so clearly it's not actually
        // out of memory. D16_UNORM appears to function correctly in unprotected contexts.
        const bool disableD16InProtected =
                this->protectedSupport() && skgpu::kQualcomm_VkVendor == properties.vendorID;
        if (!disableD16InProtected) {
            constexpr VkFormat format = VK_FORMAT_D16_UNORM;
            auto& info = this->getDepthStencilFormatInfo(format);
            info.init(interface, *this, physDev, format);
            if (info.fFormatProperties.optimalTilingFeatures &
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
            }
        }
    }
    // Format: VK_FORMAT_D32_SFLOAT
    {
        constexpr VkFormat format = VK_FORMAT_D32_SFLOAT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
        }
    }
    // Format: VK_FORMAT_D24_UNORM_S8_UINT
    {
        constexpr VkFormat format = VK_FORMAT_D24_UNORM_S8_UINT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[stencilFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[dsFlags.value()], format);
        }
    }
    // Format: VK_FORMAT_D32_SFLOAT_S8_UINT
    {
        constexpr VkFormat format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        auto& info = this->getDepthStencilFormatInfo(format);
        info.init(interface, *this, physDev, format);
        if (info.fFormatProperties.optimalTilingFeatures &
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[stencilFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[depthFlags.value()], format);
            set_ds_flags_to_format(fDepthStencilFlagsToFormatTable[dsFlags.value()], format);
        }
    }
}

void VulkanCaps::SupportedSampleCounts::initSampleCounts(const skgpu::VulkanInterface* interface,
                                                         const VulkanCaps& caps,
                                                         VkPhysicalDevice physDev,
                                                         VkFormat format,
                                                         VkImageUsageFlags usage) {
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
        SKGPU_LOG_W("Vulkan call GetPhysicalDeviceImageFormatProperties failed: %d", result);
        return;
    }

    // Standard sample locations are not defined for more than 16 samples, and we don't need more
    // than 16. Omit 32 and 64.
    fSampleCounts = properties.sampleCounts &
                    (VK_SAMPLE_COUNT_1_BIT | VK_SAMPLE_COUNT_2_BIT | VK_SAMPLE_COUNT_4_BIT |
                     VK_SAMPLE_COUNT_8_BIT | VK_SAMPLE_COUNT_16_BIT);

    // Disable MSAA if driver workaround requires it, by pretending the format does not support any
    // sample count other than 1.
    if (caps.avoidMSAA()) {
        fSampleCounts &= VK_SAMPLE_COUNT_1_BIT;
    }

    // If VK_EXT_multisampled_render_to_single_sampled is used, verify that the
    // VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT flag does not alter the
    // supported sample counts. If it does, it's not against the spec but it also doesn't make
    // practical sense (the extension is all about load (unresolve) and store (resolve) ops, it
    // shouldn't affect multisampled rendering itself). In that case, issue a warning and mask out
    // unsupported bits.
    if (caps.msaaRenderToSingleSampledSupport() && fSampleCounts > VK_SAMPLE_COUNT_1_BIT) {
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
            SKGPU_LOG_W("Vulkan call GetPhysicalDeviceImageFormatProperties failed: %d", result);
            return;
        }
        if (result == VK_ERROR_FORMAT_NOT_SUPPORTED ||
            properties.sampleCounts <= VK_SAMPLE_COUNT_1_BIT ||
            (fSampleCounts & properties.sampleCounts) != fSampleCounts) {
            SKGPU_LOG_W(
                    "Inconsistent MSAA rendering support in the presence of "
                    "VK_EXT_multisampled_render_to_single_sampled (Supported MSAA bits: %#X vs "
                    "with MSRTSS: %#X)",
                    fSampleCounts,
                    result == VK_ERROR_FORMAT_NOT_SUPPORTED ? 0 : properties.sampleCounts);

            // Mask out the unsupported bits
            if (result == VK_SUCCESS) {
                fSampleCounts &= (properties.sampleCounts | VK_SAMPLE_COUNT_1_BIT);
            } else {
                fSampleCounts &= VK_SAMPLE_COUNT_1_BIT;
            }
        }
    }
}

bool VulkanCaps::SupportedSampleCounts::isSampleCountSupported(int requestedCount) const {
    requestedCount = std::max(1, requestedCount);
    // Non-power-of-two sample counts are never supported (but practically also never expected to be
    // requested)
    if (!SkIsPow2(requestedCount)) {
        return false;
    }

    return (fSampleCounts & requestedCount) != 0;
}


namespace {
bool is_texturable(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT & flags) &&
           SkToBool(VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT & flags);
}

bool is_renderable(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT & flags);
}

bool is_storage(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & flags);
}

bool is_transfer_src(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_TRANSFER_SRC_BIT & flags);
}

bool is_transfer_dst(VkFormatFeatureFlags flags) {
    return SkToBool(VK_FORMAT_FEATURE_TRANSFER_DST_BIT & flags);
}
}

void VulkanCaps::FormatInfo::init(const skgpu::VulkanInterface* interface,
                                  const VulkanCaps& caps,
                                  VkPhysicalDevice physDev,
                                  VkFormat format) {
    fFormatProperties = {};
    VULKAN_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &fFormatProperties));

    if (is_renderable(fFormatProperties.optimalTilingFeatures)) {
        // We make all renderable images support being used as input attachment
        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                       VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                       VK_IMAGE_USAGE_SAMPLED_BIT |
                                       VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                       VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        this->fSupportedSampleCounts.initSampleCounts(interface, caps, physDev, format, usageFlags);
    }

    fIsEfficientWithHostImageCopy = false;
    if (caps.supportsHostImageCopy()) {
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

        if (VULKAN_CALL(interface,
                        GetPhysicalDeviceImageFormatProperties2(
                                physDev, &imageFormatInfo, &imageFormatProperties2)) ==
            VK_SUCCESS) {
            // There are two results returned in `perfQuery`:
            //
            // * `identicalMemoryLayout` indicates that the added flag does not affect the physical
            //   layout of the image. We can definitely add the flag in this case.
            // * `optimalDeviceAccess` indicates that the added flag _does_ change the physical
            //   layout of the image, but that according to the driver authors the fallback layout
            //   is still "pretty good, you won't know the difference".
            //
            // For now, host image copy is only used if `identicalMemoryLayout` is true, but we
            // could consider enabling it when only `optimalDeviceAccess` is true based on
            // experimenting on different vendors.
            fIsEfficientWithHostImageCopy = perfQuery.identicalMemoryLayout;
        }
    }
}

bool VulkanCaps::FormatInfo::isTexturable(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_texturable(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_texturable(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isRenderable(VkImageTiling imageTiling,
                                          uint32_t sampleCount) const {
    if (!fSupportedSampleCounts.isSampleCountSupported(sampleCount)) {
        return false;
    }
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_renderable(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_renderable(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isStorage(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_storage(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_storage(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isTransferSrc(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_transfer_src(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_transfer_src(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isTransferDst(VkImageTiling imageTiling) const {
    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return is_transfer_dst(fFormatProperties.optimalTilingFeatures);
        case VK_IMAGE_TILING_LINEAR:
            return is_transfer_dst(fFormatProperties.linearTilingFeatures);
        default:
            return false;
    }
    SkUNREACHABLE;
}

bool VulkanCaps::FormatInfo::isEfficientWithHostImageCopy(VkImageTiling imageTiling,
                                                          Protected isProtected) const {
    if (isProtected == Protected::kYes) {
        // Currently, we don't query whether protected textures can be used with host image copy;
        // that is unlikely to be the case.
        return false;
    }

    switch (imageTiling) {
        case VK_IMAGE_TILING_OPTIMAL:
            return fIsEfficientWithHostImageCopy;
        case VK_IMAGE_TILING_LINEAR:
            // Host-image-copy is always efficient with linear tiling, as it's just a series of
            // `memcpy`s.
            return true;
        default:
            break;
    }

    return false;
}

void VulkanCaps::setColorType(SkColorType colorType, std::initializer_list<VkFormat> formats) {
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

VkFormat VulkanCaps::getFormatFromColorType(SkColorType colorType) const {
    int idx = static_cast<int>(colorType);
    return fColorTypeToFormatTable[idx];
}

VulkanCaps::FormatInfo& VulkanCaps::getFormatInfo(VkFormat format) {
    static_assert(std::size(kVkFormats) == VulkanCaps::kNumVkFormats,
                  "Size of VkFormats array must match static value in header");

    static FormatInfo kInvalidFormat;
    if (format == VK_FORMAT_UNDEFINED) {
        return kInvalidFormat;
    }

    for (size_t i = 0; i < std::size(kVkFormats); ++i) {
        if (kVkFormats[i] == format) {
            return fFormatTable[i];
        }
    }

   return kInvalidFormat;
}

const VulkanCaps::FormatInfo& VulkanCaps::getFormatInfo(VkFormat format) const {
    VulkanCaps* nonConstThis = const_cast<VulkanCaps*>(this);
    return nonConstThis->getFormatInfo(format);
}

void VulkanCaps::DepthStencilFormatInfo::init(const skgpu::VulkanInterface* interface,
                                              const VulkanCaps& caps,
                                              VkPhysicalDevice physDev,
                                              VkFormat format) {
    fFormatProperties = {};
    VULKAN_CALL(interface, GetPhysicalDeviceFormatProperties(physDev, format, &fFormatProperties));

    if (this->isDepthStencilSupported(fFormatProperties.optimalTilingFeatures)) {
        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        fSupportedSampleCounts.initSampleCounts(interface, caps, physDev, format, usageFlags);
    }
}

bool VulkanCaps::DepthStencilFormatInfo::isDepthStencilSupported(VkFormatFeatureFlags flags) const {
    return SkToBool(VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT & flags);
}

VulkanCaps::DepthStencilFormatInfo& VulkanCaps::getDepthStencilFormatInfo(VkFormat format) {
    static_assert(std::size(kDepthStencilVkFormats) == VulkanCaps::kNumDepthStencilVkFormats,
                  "Size of VkFormats array must match static value in header");
    for (size_t i = 0; i < std::size(kDepthStencilVkFormats); ++i) {
        if (kVkFormats[i] == format) {
            return fDepthStencilFormatTable[i];
        }
    }
    static DepthStencilFormatInfo kInvalidFormat;
    return kInvalidFormat;
}

const VulkanCaps::DepthStencilFormatInfo& VulkanCaps::getDepthStencilFormatInfo(VkFormat format)
        const {
    VulkanCaps* nonConstThis = const_cast<VulkanCaps*>(this);
    return nonConstThis->getDepthStencilFormatInfo(format);
}

const Caps::ColorTypeInfo* VulkanCaps::getColorTypeInfo(SkColorType ct,
                                                        const TextureInfo& textureInfo) const {
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(textureInfo);
    VkFormat vkFormat = vkInfo.fFormat;
    if (vkFormat == VK_FORMAT_UNDEFINED) {
        // If VkFormat is undefined but there is a valid YCbCr conversion associated with the
        // texture, then we know we are using an external format and can return color type
        // info representative of external format color information.
        return vkInfo.fYcbcrConversionInfo.isValid() ? &fExternalFormatColorTypeInfo : nullptr;
    }

    const FormatInfo& info = this->getFormatInfo(vkFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const ColorTypeInfo& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == ct) {
            return &ctInfo;
        }
    }

    return nullptr;
}

bool VulkanCaps::onIsTexturable(const TextureInfo& texInfo) const {
    return texInfo.isValid() &&
           this->isTexturable(TextureInfoPriv::Get<VulkanTextureInfo>(texInfo));
}

bool VulkanCaps::isRenderable(const TextureInfo& texInfo) const {
    return texInfo.isValid() &&
           this->isRenderable(TextureInfoPriv::Get<VulkanTextureInfo>(texInfo));
}

bool VulkanCaps::isStorage(const TextureInfo& texInfo) const {
    if (!texInfo.isValid()) {
        return false;
    }
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(texInfo);

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isStorage(vkInfo.fImageTiling);
}

bool VulkanCaps::isFormatSupported(VkFormat format) const {
    const FormatInfo& formatInfo = this->getFormatInfo(format);

    // If Skia claims support for a VkFormat we should have a nonzero fColorTypeInfoCount and valid
    // fColorTypeInfos ptr. Therefore, just checking these should be more than sufficient to confirm
    // that the format is supported by Skia.
    return formatInfo.fColorTypeInfoCount != 0 && formatInfo.fColorTypeInfos != nullptr;
}

bool VulkanCaps::isTexturable(const VulkanTextureInfo& vkInfo) const {
    // All images using external formats are required to be able to be sampled per Vulkan spec.
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAndroidHardwareBufferFormatPropertiesANDROID.html#_description
    if (vkInfo.fFormat == VK_FORMAT_UNDEFINED && vkInfo.fYcbcrConversionInfo.isValid()) {
        return true;
    }

    // Otherwise, we are working with a known format and can simply reference the format table info.
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isTexturable(vkInfo.fImageTiling);
}

bool VulkanCaps::isRenderable(const VulkanTextureInfo& vkInfo) const {
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    // All renderable vulkan textures within graphite must also support input attachment usage
    return info.isRenderable(vkInfo.fImageTiling, vkInfo.fSampleCount) &&
           SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
}

bool VulkanCaps::isTransferSrc(const VulkanTextureInfo& vkInfo) const {
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isTransferSrc(vkInfo.fImageTiling);
}

bool VulkanCaps::isTransferDst(const VulkanTextureInfo& vkInfo) const {
    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    return info.isTransferDst(vkInfo.fImageTiling);
}

bool VulkanCaps::supportsWritePixels(const TextureInfo& texInfo) const {
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(texInfo);

    // Can't write if it needs a YCbCr sampler
    if (VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return false;
    }

    if (vkInfo.fSampleCount > 1) {
        return false;
    }

    if (!SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        return false;
    }

    return true;
}

bool VulkanCaps::supportsReadPixels(const TextureInfo& texInfo) const {
    if (texInfo.isProtected() == Protected::kYes) {
        return false;
    }

    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(texInfo);

    // Can't read if it needs a YCbCr sampler
    if (VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return false;
    }

    if (VkFormatIsCompressed(vkInfo.fFormat)) {
        return false;
    }

    if (vkInfo.fSampleCount > 1) {
        return false;
    }

    if (!SkToBool(vkInfo.fImageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)) {
        return false;
    }

    return true;
}

std::pair<SkColorType, bool /*isRGBFormat*/> VulkanCaps::supportedWritePixelsColorType(
        SkColorType dstColorType,
        const TextureInfo& dstTextureInfo,
        SkColorType srcColorType) const {
    if (!dstTextureInfo.isValid()) {
        return {kUnknown_SkColorType, false};
    }
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(dstTextureInfo);

    // Can't write to external / YCbCr formats
    if (vkInfo.fFormat == VK_FORMAT_UNDEFINED || VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return {kUnknown_SkColorType, false};
    }

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == dstColorType) {
            return {ctInfo.fTransferColorType, vkInfo.fFormat == VK_FORMAT_R8G8B8_UNORM};
        }
    }

    return {kUnknown_SkColorType, false};
}

std::pair<SkColorType, bool /*isRGBFormat*/> VulkanCaps::supportedReadPixelsColorType(
        SkColorType srcColorType,
        const TextureInfo& srcTextureInfo,
        SkColorType dstColorType) const {
    if (!srcTextureInfo.isValid()) {
        return {kUnknown_SkColorType, false};
    }
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(srcTextureInfo);

    // Can't read from YCbCr formats
    // TODO: external formats?
    if (VkFormatNeedsYcbcrSampler(vkInfo.fFormat)) {
        return {kUnknown_SkColorType, false};
    }

    // TODO: handle compressed formats
    if (VkFormatIsCompressed(vkInfo.fFormat)) {
        SkASSERT(this->isTexturable(vkInfo));
        return {kUnknown_SkColorType, false};
    }

    const FormatInfo& info = this->getFormatInfo(vkInfo.fFormat);
    for (int i = 0; i < info.fColorTypeInfoCount; ++i) {
        const auto& ctInfo = info.fColorTypeInfos[i];
        if (ctInfo.fColorType == srcColorType) {
            return {ctInfo.fTransferColorType, vkInfo.fFormat == VK_FORMAT_R8G8B8_UNORM};
        }
    }

    return {kUnknown_SkColorType, false};
}

bool VulkanCaps::msaaTextureRenderToSingleSampledSupport(const TextureInfo& info) const {
    const auto& vkInfo = TextureInfoPriv::Get<VulkanTextureInfo>(info);
    return vkInfo.fFlags & VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
}

// 4 uint32s for the render step id, paint id, compatible render pass description, and write
// swizzle.
static constexpr int kPipelineKeyData32Count = 4;

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
    uint32_t samples = SamplesToKey(info.numSamples());
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
    static constexpr int kNum32DimensionDataCnt = 2;
    static constexpr int kNum32MiscDataCnt = 3;
    // Non-YCbCr formats need 1 int for format.
    // YCbCr conversion needs 1 int for non-format flags, and a 64-bit format (external or regular).
    static constexpr int kNum32FormatDataCntNoYcbcr = 1;
    static constexpr int kNum32FormatDataCntYcbcr = 3;

    const VulkanYcbcrConversionInfo& ycbcrInfo = vkInfo.fYcbcrConversionInfo;
    const int num32DataCnt =
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
    builder[i++] = (samples                                            << 0) |
                   (static_cast<uint32_t>(isMipped)                    << 3) |
                   (static_cast<uint32_t>(isProtected)                 << 4) |
                   (static_cast<uint32_t>(vkInfo.fImageTiling)         << 5) |
                   (static_cast<uint32_t>(vkInfo.fSharingMode)         << 6) |
                   (static_cast<uint32_t>(vkInfo.fAspectMask)          << 7);
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

static constexpr const char* vk_chromafilter_to_str(VkFilter f) {
    switch (f) {
        case VK_FILTER_NEAREST:   return "nearest";
        case VK_FILTER_LINEAR:    return "linear";
        case VK_FILTER_CUBIC_EXT: return "cubic";
        default:                  return "unknown";
    }
    SkUNREACHABLE;
}

std::string VulkanCaps::toString(const ImmutableSamplerInfo& immutableSamplerInfo) const {
    const skgpu::VulkanYcbcrConversionInfo info =
            VulkanYcbcrConversion::FromImmutableSamplerInfo(immutableSamplerInfo);
    if (!info.isValid()) {
        return "";
    }

    std::string result;

    if (info.hasExternalFormat()) {
        result += 'x';
        result += std::to_string(info.externalFormat());
    } else {
        result += std::to_string(info.format());
    }

    result += " ";
    result += VkModelToStr(info.model());
    result += "+";
    result += VkRangeToStr(info.range());
    result += info.xChromaOffset() ? " mid"  : " cos";  // midpoint or cosited-even
    result += info.yChromaOffset() ? " mid " : " cos "; // midpoint or cosited-even
    result += vk_chromafilter_to_str(info.chromaFilter());
    result += info.forceExplicitReconstruction() ? " T " : " F ";
    result += VkSwizzleToStr(info.components().r, 'r');
    result += VkSwizzleToStr(info.components().g, 'g');
    result += VkSwizzleToStr(info.components().b, 'b');
    result += VkSwizzleToStr(info.components().a, 'a');
    result += " cf";
    result += info.samplerFilterMustMatchChromaFilter() ? '1' : '0';
    result += "lf";
    result += info.supportsLinearFilter() ? '1' : '0';
    return result;
}

} // namespace skgpu::graphite
