/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/vk/VulkanPreferredFeatures.h"

#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace skgpu {

/*
 * # Overview
 *
 * Skia requires the application to create the Vulkan instance and device. This operation involves
 * enabling extensions and device features that the application would like to take advantage of.
 * However, the application is not necessarily aware of all the extensions and features that _Skia_
 * would like to use (and it's too onerous to require them to keep up). If the application does not
 * enable some extensions, Skia's performance may degrade.
 *
 * VulkanPreferredFeatures is used to let Skia modify the list of extensions and device features
 * before the application creates the device. The application thus never needs to worry about what
 * Skia might or might not use.
 *
 * # How It Works
 *
 * There are three main operations:
 *
 * - Add instance extensions. Before creating the Vulkan instance, Skia could add more extensions to
 *   enable.
 * - Add device features to query. This lets Skia discover what features are available.
 * - Enable device features.
 *
 * Device features and extensions are where the complexity is. In particular, there is:
 *
 * - The list of _available_ extensions. When adding features to query, this information is
 *   assembled in DeviceExtensions for faster look up.
 * - The list of _enabled_ extensions. When enabling features, this information is also assembled in
 *   DeviceExtensions.
 * - The list of device features. This is a chain of VkPhysicalDeviceFooFeatures structs, starting
 *   with VkPhysicalDeviceFeatures2. FeaturesToAdd is used to track whether any of these feature
 *   structs need to be chained or not. The choice of feature structs depends on the API version
 *   used by the application, following the extension promotions to core.
 *
 * See addFeaturesToQuery() for details of how feature structs are chosen and chained to be queried
 * in addition to the application's own set of feature structs.
 *
 * See addFeaturesToEnable() for details of how the application-provided chain of feature structs is
 * manipulated to include the previously-queried features. Based on Vulkan rules, some structs are
 * mutually exclusive, which is handled by this function as well.
 *
 * # How to Add Support for New Extensions
 *
 * When adding a new extension, the following changes must be made:
 *
 * - Add a flag corresponding to the extension to DeviceExtensions
 *   - Detect the extension and set it in mark_device_extensions().
 * - Add the extensions's feature struct to VulkanPreferredFeatures. This provides storage for the
 *   struct so it lives long enough (and not go out of scope too early).
 * - Add a flag corresponding to the feature struct to FeaturesToAdd.
 *   - In addFeaturesToQuery(), initalize the sType of the feature struct based on the flag in
 *     DeviceExtensions. If extension is promoted to core in the API version used by the
 *     application, the VkPhysicalDeviceVulkan??Features struct should be used for the same feature
 *     instead, if available.
 *   - In get_features_to_query(), if the extension is promoted to a core version, set the
 *     FeaturesToAdd flag to false as the VkPhysicalDeviceVulkan??Features struct would be used to
 *     query the same feature.
 *   - In get_features_to_query() additionally, set the FeaturesToAdd flag to false if the
 *     application has already included the corresponding feature struct in its own chain.
 *   - Back in addFeaturesToQuery(), chain the feature struct if the FeaturesToAdd flag is set.
 * - In addFeaturesToEnable():
 *   - Set the FeaturesToAdd flag to true iff the sType of the feature struct is set.
 *   - If the extension is not enabled by the app but the FeaturesToAdd flag is set, add the
 *     extension to appExtensions.
 *   - Disable optional features in the struct if not going to be used by Skia. Note that this
 *     modifies the struct owned by VulkanPreferredFeatures; if that's the one that's chained, the
 *     application didn't care about these features. If the application has chained its own struct
 *     of this type, it's unaffected by this operation.
 *   - Detect the struct in the big switch. If promoted to core, make sure to drop the struct. If
 *     not, chain it if the extension is enabled.
 *   - In the end, chain the feature struct if still needed to be added.
 * - Make sure to update the existing tests in tests/VkPreferredFeaturesTest.cpp to make sure the
 *   extension and its feature is enabled appropropriately.
 *
 * # How to Add Support for New Vulkan Versions
 *
 * When adding a support for a new Vulkan version, the following changes must be made:
 *
 * - Add the VkPhysicalDeviceVulkan??Features struct to VulkanPreferredFeatures, and a flag for it
 *   in FeaturesToAdd.
 * - In get_features_to_query(), check for the Vulkan version and decided if that struct should be
 *   used, or the individual feature structs from extensions that were promoted to core in that
 *   version.
 *   - Check in the same function if the application has already included
 *     VkPhysicalDeviceVulkan??Features.
 * - In addFeaturesToQuery(), similarly check for the Vulkan version and set the sType of either the
 *   VkPhysicalDeviceVulkan??Features struct or the individual feature structs from promoted
 *   extensions.
 *   - Chain the VkPhysicalDeviceVulkan??Features struct if the FeaturesToAdd flag is set.
 * - In addFeaturesToEnable():
 *   - Set the FeaturesToAdd flag to true iff the sType of the feature struct is set.
 *   - Disable optional features in the struct if not going to be used by Skia.
 *   - Detect the struct in the big switch. If pointer does not point to VulkanPreferredFeatures's
 *     own struct, copy the application-enabled features to VulkanPreferredFeatures's struct. Note
 *     that for simplicity, Skia always chains its own instance of VkPhysicalDeviceVulkan??Features.
 *   - Based on valid usage that forbids including VkPhysicalDeviceVulkan??Features and feature
 *     structs from promoted extensions, detect the latter structs in the big switch, copy their
 *     features to VkPhysicalDeviceVulkan??Features and drop them from the chain.
 *   - In the end, chain the VkPhysicalDeviceVulkan??Features feature struct.
 * - Make sure to update tests/VkPreferredFeaturesTest.cpp with a new test that ensures the
 *   promotion rules were followed.
 */

namespace {
struct DeviceExtensions {
    // Extensions that Skia may benefit from enabling if available.
    bool fRasterizationOrderAttachmentAccessARM = false;
    bool fRasterizationOrderAttachmentAccessEXT = false;
    bool fBlendOperationAdvancedEXT = false;
    bool fExtendedDynamicStateEXT = false;
    bool fExtendedDynamicState2EXT = false;
    bool fVertexInputDynamicStateEXT = false;
    bool fGraphicsPipelineLibraryEXT = false;
    bool fRGBA10x6FormatsEXT = false;
    bool fSynchronization2KHR = false;
    bool fDynamicRenderingKHR = false;
    bool fDynamicRenderingLocalReadKHR = false;
    bool fMultisampledRenderToSingleSampledEXT = false;
    bool fHostImageCopyEXT = false;
    bool fPipelineCreationCacheControlEXT = false;
    bool fDriverPropertiesKHR = false;
    bool fCreateRenderpass2KHR = false;
    bool fLoadStoreOpNoneEXT = false;
    bool fLoadStoreOpNoneKHR = false;
    bool fConservativeRasterizationEXT = false;
#if defined(SK_BUILD_FOR_ANDROID)
    bool fExternalMemoryAHardwareBufferEXT = false;
#endif
    bool fPipelineLibraryKHR = false;
    bool fCopyCommands2KHR = false;
    bool fFormatFeatureFlags2KHR = false;
    bool fDepthStencilResolveKHR = false;
#if defined(SK_BUILD_FOR_ANDROID)
    bool fQueueFamilyForeignEXT = false;
#endif

    // Featureless extensions that the application may have enabled and that Skia may need to enable
    // via a VkPhysicalDeviceVulkanNNFeatures struct. See "Table 1. Extension Feature Aliases" in
    // the Features chapter of the spec:
    //
    //    Extension:                            Features on promotion to core:
    //    VK_KHR_shader_draw_parameters         shaderDrawParameters
    //    VK_KHR_draw_indirect_count            drawIndirectCount
    //    VK_KHR_sampler_mirror_clamp_to_edge   samplerMirrorClampToEdge
    //    VK_EXT_descriptor_indexing            descriptorIndexing
    //    VK_EXT_sampler_filter_minmax          samplerFilterMinmax
    //    VK_EXT_shader_viewport_index_layer    shaderOutputViewportIndex, shaderOutputLayer
    //    VK_KHR_push_descriptor                pushDescriptor
    //    VK_EXT_frame_boundary                 frameBoundaryEXT
    bool fShaderDrawParametersKHR = false;
    bool fDrawIndirectCountKHR = false;
    bool fSamplerMirrorClampToEdgeKHR = false;
    bool fDescriptorIndexingEXT = false;
    bool fSamplerFilterMinmaxEXT = false;
    bool fShaderViewportIndexLayerEXT = false;
    bool fPushDescriptorKHR = false;
    bool fFrameBoundaryEXT = false;
};

// Given a list of device extensions, mark which ones are present.
void mark_device_extensions(DeviceExtensions& exts, const char* name) {
    if (strcmp(name, VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME) == 0) {
        exts.fRasterizationOrderAttachmentAccessARM = true;
    } else if (strcmp(name, VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME) == 0) {
        exts.fRasterizationOrderAttachmentAccessEXT = true;
    } else if (strcmp(name, VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME) == 0) {
        exts.fBlendOperationAdvancedEXT = true;
    } else if (strcmp(name, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
        exts.fExtendedDynamicStateEXT = true;
    } else if (strcmp(name, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME) == 0) {
        exts.fExtendedDynamicState2EXT = true;
    } else if (strcmp(name, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
        exts.fVertexInputDynamicStateEXT = true;
    } else if (strcmp(name, VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME) == 0) {
        exts.fGraphicsPipelineLibraryEXT = true;
    } else if (strcmp(name, VK_EXT_RGBA10X6_FORMATS_EXTENSION_NAME) == 0) {
        exts.fRGBA10x6FormatsEXT = true;
    } else if (strcmp(name, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0) {
        exts.fSynchronization2KHR = true;
    } else if (strcmp(name, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0) {
        exts.fDynamicRenderingKHR = true;
    } else if (strcmp(name, VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME) == 0) {
        exts.fDynamicRenderingLocalReadKHR = true;
    } else if (strcmp(name, VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME) == 0) {
        exts.fMultisampledRenderToSingleSampledEXT = true;
    } else if (strcmp(name, VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME) == 0) {
        exts.fHostImageCopyEXT = true;
    } else if (strcmp(name, VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME) == 0) {
        exts.fPipelineCreationCacheControlEXT = true;
    } else if (strcmp(name, VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME) == 0) {
        exts.fDriverPropertiesKHR = true;
    } else if (strcmp(name, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0) {
        exts.fCreateRenderpass2KHR = true;
    } else if (strcmp(name, VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME) == 0) {
        exts.fLoadStoreOpNoneEXT = true;
    } else if (strcmp(name, VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME) == 0) {
        exts.fLoadStoreOpNoneKHR = true;
    } else if (strcmp(name, VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME) == 0) {
        exts.fConservativeRasterizationEXT = true;
#if defined(SK_BUILD_FOR_ANDROID)
    } else if (
            strcmp(name, VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME) == 0) {
        exts.fExternalMemoryAHardwareBufferEXT = true;
#endif
    } else if (strcmp(name, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME) == 0) {
        exts.fPipelineLibraryKHR = true;
    } else if (strcmp(name, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME) == 0) {
        exts.fCopyCommands2KHR = true;
    } else if (strcmp(name, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME) == 0) {
        exts.fFormatFeatureFlags2KHR = true;
    } else if (strcmp(name, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME) == 0) {
        exts.fDepthStencilResolveKHR = true;
#if defined(SK_BUILD_FOR_ANDROID)
    } else if (strcmp(name, VK_EXT_QUEUE_FAMILY_FOREIGN_EXTENSION_NAME) == 0) {
        exts.fQueueFamilyForeignEXT = true;
#endif
    } else if (strcmp(name, VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME) == 0) {
        exts.fShaderDrawParametersKHR = true;
    } else if (strcmp(name, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME) == 0) {
        exts.fDrawIndirectCountKHR = true;
    } else if (strcmp(name, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME) == 0) {
        exts.fSamplerMirrorClampToEdgeKHR = true;
    } else if (strcmp(name, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0) {
        exts.fDescriptorIndexingEXT = true;
    } else if (strcmp(name, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME) == 0) {
        exts.fSamplerFilterMinmaxEXT = true;
    } else if (strcmp(name, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0) {
        exts.fShaderViewportIndexLayerEXT = true;
    } else if (strcmp(name, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME) == 0) {
        exts.fPushDescriptorKHR = true;
    } else if (strcmp(name, VK_EXT_FRAME_BOUNDARY_EXTENSION_NAME) == 0) {
        exts.fFrameBoundaryEXT = true;
    }
}

DeviceExtensions get_supported_device_extensions(const VkExtensionProperties* deviceExtensions,
                                                 size_t deviceExtensionCount) {
    DeviceExtensions supportedExts;

    for (size_t i = 0; i < deviceExtensionCount; ++i) {
        mark_device_extensions(supportedExts, deviceExtensions[i].extensionName);
    }

    return supportedExts;
}

struct FeaturesToAdd {
    bool fVulkan11 = true;
    bool fVulkan12 = true;
    bool fVulkan13 = true;
    bool fVulkan14 = true;
    bool fRasterizationOrderAttachmentAccess = true;
    bool fBlendOperationAdvanced = true;
    bool fExtendedDynamicState = true;
    bool fExtendedDynamicState2 = true;
    bool fVertexInputDynamicState = true;
    bool fGraphicsPipelineLibrary = true;
    bool fRGBA10x6Formats = true;
    bool fSynchronization2 = true;
    bool fDynamicRendering = true;
    bool fDynamicRenderingLocalRead = true;
    bool fMultisampledRenderToSingleSampled = true;
    bool fHostImageCopy = true;
    bool fPipelineCreationCacheControl = true;
    bool fFrameBoundary = true;
    bool fSamplerYcbcrConversion = true;
};

FeaturesToAdd get_features_to_query(uint32_t apiVersion,
                                    const DeviceExtensions& exts,
                                    const VkPhysicalDeviceFeatures2& appFeatures) {
    FeaturesToAdd toQuery;

    // First, exclude feature structs that are not available / are unnecessary based on apiVersion
    // and available extensions.
    if (apiVersion >= VK_API_VERSION_1_4) {
        // VK_KHR_dynamic_rendering_local_read's feature is included in
        // VkPhysicalDeviceVulkan14Features
        toQuery.fDynamicRenderingLocalRead = false;
        // VK_EXT_host_image_copy's feature is included in VkPhysicalDeviceVulkan14Features
        toQuery.fHostImageCopy = false;
    } else {
        toQuery.fVulkan14 = false;
        // Check for support via individual extensions' feature structs
        toQuery.fDynamicRenderingLocalRead = exts.fDynamicRenderingLocalReadKHR;
        toQuery.fHostImageCopy = exts.fHostImageCopyEXT;
    }
    if (apiVersion >= VK_API_VERSION_1_3) {
        // VK_KHR_synchronization2's feature is included in VkPhysicalDeviceVulkan13Features
        toQuery.fSynchronization2 = false;
        // VK_KHR_dynamic_rendering's feature is included in VkPhysicalDeviceVulkan13Features
        toQuery.fDynamicRendering = false;
        // VK_EXT_pipeline_creation_cache_control's feature is included in
        // VkPhysicalDeviceVulkan13Features
        toQuery.fPipelineCreationCacheControl = false;
        // VK_EXT_extended_dynamic_state is core in Vulkan 1.3, but no feature for it is present in
        // VkPhysicalDeviceVulkan13Features; it's assumed supported / enabled.
        toQuery.fExtendedDynamicState = false;
        // Note: VK_EXT_extended_dynamic_state2 was partially promoted to Vulkan 1.3. Only dynamic
        // state from the main feature is used by Skia, so it's ok to rely on Vulkan 1.3 to provide
        // them, but if the optional features of this extension are needed, the extension struct
        // should still be used to query and enable those features.
        toQuery.fExtendedDynamicState2 = false;
    } else {
        toQuery.fVulkan13 = false;
        // Check for support via individual extensions' feature structs
        toQuery.fSynchronization2 = exts.fSynchronization2KHR;
        toQuery.fDynamicRendering = exts.fDynamicRenderingKHR;
        toQuery.fPipelineCreationCacheControl = exts.fPipelineCreationCacheControlEXT;
        toQuery.fExtendedDynamicState = exts.fExtendedDynamicStateEXT;
        toQuery.fExtendedDynamicState2 = exts.fExtendedDynamicState2EXT;
    }
    if (apiVersion >= VK_API_VERSION_1_2) {
        // VkPhysicalDeviceSamplerYcbcrConversionFeature's feature is included within
        // VkPhysicalDeviceVulkan11Features. Note that this struct was introduced in Vulkan 1.2.
        toQuery.fSamplerYcbcrConversion = false;
        // No feature from VkPhysicalDeviceVulkan12Features is used by Skia currently.
    } else {
        toQuery.fVulkan12 = false;
        toQuery.fVulkan11 = false;
        // For Vulkan 1.1, always query for YCbCr conversion feature support (if the app doesn't
        // already do so independently).
        toQuery.fSamplerYcbcrConversion = true;
    }
    // Check for support via individual extensions' feature structs (none of which have been
    // promoted to core).
    toQuery.fRasterizationOrderAttachmentAccess = exts.fRasterizationOrderAttachmentAccessARM ||
                                                  exts.fRasterizationOrderAttachmentAccessEXT;
    toQuery.fBlendOperationAdvanced = exts.fBlendOperationAdvancedEXT;
    toQuery.fVertexInputDynamicState = exts.fVertexInputDynamicStateEXT;
    toQuery.fGraphicsPipelineLibrary = exts.fGraphicsPipelineLibraryEXT;
    toQuery.fRGBA10x6Formats = exts.fRGBA10x6FormatsEXT;
    toQuery.fMultisampledRenderToSingleSampled = exts.fMultisampledRenderToSingleSampledEXT;
    toQuery.fFrameBoundary = exts.fFrameBoundaryEXT;

    // Then, go over appFeatures::pNext and exclude features that are already being queried by the
    // app.
    const VkBaseInStructure* pNext = static_cast<const VkBaseInStructure*>(appFeatures.pNext);
    while (pNext) {
        switch (pNext->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES:
                toQuery.fVulkan11 = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES:
                toQuery.fVulkan12 = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES:
                toQuery.fVulkan13 = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES:
                toQuery.fVulkan14 = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT:
                toQuery.fRasterizationOrderAttachmentAccess = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT:
                toQuery.fBlendOperationAdvanced = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT:
                toQuery.fExtendedDynamicState = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT:
                toQuery.fExtendedDynamicState2 = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT:
                toQuery.fVertexInputDynamicState = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT:
                toQuery.fGraphicsPipelineLibrary = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES:
                toQuery.fSamplerYcbcrConversion = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT:
                toQuery.fRGBA10x6Formats = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES:
                toQuery.fSynchronization2 = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES:
                toQuery.fDynamicRendering = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES:
                toQuery.fDynamicRenderingLocalRead = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT:
                toQuery.fMultisampledRenderToSingleSampled = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES:
                toQuery.fHostImageCopy = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES:
                toQuery.fPipelineCreationCacheControl = false;
                break;
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT:
                toQuery.fFrameBoundary = false;
                break;
            default:
                break;
        }

        pNext = pNext->pNext;
    }

    return toQuery;
}

template <typename VulkanStruct> void chain(void**& chainEnd, VulkanStruct* toAdd) {
    *chainEnd = toAdd;
    chainEnd = (void**)&toAdd->pNext;
}
}  // anonymous namespace

VulkanPreferredFeatures::~VulkanPreferredFeatures() {
    if (fHasAddedToInstanceExtensions && !fHasAddedFeaturesToQuery) {
        SkDebugf("WARNING: VulkanPreferredFeatures::addFeaturesToQuery() was not called\n");
    }
    if (fHasAddedToInstanceExtensions && !fHasAddedFeaturesToEnable) {
        SkDebugf("WARNING: VulkanPreferredFeatures::addFeaturesToEnable() was not called\n");
    }
}

void VulkanPreferredFeatures::init(uint32_t appAPIVersion) {
    fAPIVersion = appAPIVersion;

    // Minimum required version is Vulkan 1.1
    SkASSERT(fAPIVersion >= VK_API_VERSION_1_1);
    // This class does not understand Vulkan versions higher than 1.4
    SkASSERT(fAPIVersion <= VK_API_VERSION_1_4);
}

void VulkanPreferredFeatures::addToInstanceExtensions(
        const VkExtensionProperties* instanceExtensions,
        size_t instanceExtensionCount,
        std::vector<const char*>& appExtensions) {
    // Nothing to do, currently Skia does not take advantage of any additional instance extensions.
    fHasAddedToInstanceExtensions = true;
}

void VulkanPreferredFeatures::addFeaturesToQuery(const VkExtensionProperties* deviceExtensions,
                                                 size_t deviceExtensionCount,
                                                 VkPhysicalDeviceFeatures2& appFeatures) {
    // First, inspect the list of device extensions and note which are available.
    DeviceExtensions exts =
            get_supported_device_extensions(deviceExtensions, deviceExtensionCount);

    // For now, pretend these extensions are not supported. This de-risks using this API by users
    // who didn't previously enable them. Those already in use by Skia will be re-enabled shortly
    // after. Those not yet in use by Skia will be enabled in the future when they start to get
    // used.
#if defined(SK_DISABLE_GRAPHICS_PIPELINE_LIBRARY)
    exts.fPipelineLibraryKHR = false;
    exts.fGraphicsPipelineLibraryEXT = false;
#endif
    exts.fSynchronization2KHR = false;
    exts.fDynamicRenderingKHR = false;
    exts.fDynamicRenderingLocalReadKHR = false;
#if defined(SK_DISABLE_HOST_IMAGE_COPY)
    exts.fHostImageCopyEXT = false;
    exts.fCopyCommands2KHR = false;
#endif
    exts.fShaderDrawParametersKHR = false;
    exts.fDrawIndirectCountKHR = false;
    exts.fSamplerMirrorClampToEdgeKHR = false;
    exts.fDescriptorIndexingEXT = false;
    exts.fSamplerFilterMinmaxEXT = false;
    exts.fShaderViewportIndexLayerEXT = false;
    exts.fPushDescriptorKHR = false;

    // Set the sType and extensions to enable. Later in addFeaturesToEnable, the availability of
    // device extensions is inferred from this. This is done irrespective of FeaturesToAdd
    // calculated below, because if an app includes a feature struct that has a desirable feature
    // disabled we can enable it later in addFeaturesToEnable. This is only possible if we know the
    // feature is supported (inferred from the sType being set here) and the feature is required by
    // the spec to be supported when the extension is available.
    if (fAPIVersion >= VK_API_VERSION_1_4) {
        fVulkan14.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES;
    } else {
        if (exts.fDynamicRenderingLocalReadKHR) {
            fDynamicRenderingLocalRead.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES;
        }
        if (exts.fHostImageCopyEXT) {
            fHostImageCopy.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES;
        }
        if (exts.fLoadStoreOpNoneKHR) {
            fLoadStoreOpNoneExtension = VK_KHR_LOAD_STORE_OP_NONE_EXTENSION_NAME;
        } else if (exts.fLoadStoreOpNoneEXT) {
            fLoadStoreOpNoneExtension = VK_EXT_LOAD_STORE_OP_NONE_EXTENSION_NAME;
        }
    }
    if (fAPIVersion >= VK_API_VERSION_1_3) {
        fVulkan13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    } else {
        if (exts.fSynchronization2KHR) {
            fSynchronization2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
        }
        if (exts.fDynamicRenderingKHR) {
            fDynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        }
        if (exts.fPipelineCreationCacheControlEXT) {
            fPipelineCreationCacheControl.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES;
        }
        if (exts.fExtendedDynamicStateEXT) {
            fExtendedDynamicState.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
        }
        if (exts.fExtendedDynamicState2EXT) {
            fExtendedDynamicState2.sType =
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
        }
        if (exts.fCopyCommands2KHR) {
            fCopyCommands2Extension = VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME;
        }
        if (exts.fFormatFeatureFlags2KHR) {
            fFormatFeatureFlags2Extension = VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME;
        }
    }
    if (fAPIVersion >= VK_API_VERSION_1_2) {
        fVulkan11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        fVulkan12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    } else {
        // Always query for YCbCr support if we are using Vulkan 1.1. It is technically possible for
        // it to be supported even if the associated extension, VK_KHR_sampler_ycbcr_conversion
        // (which was promoted to 1.1), is not.
        fSamplerYcbcrConversion.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
        if (exts.fDriverPropertiesKHR) {
            fDriverPropertiesExtension = VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME;
        }
        if (exts.fCreateRenderpass2KHR) {
            fCreateRenderpass2Extension = VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME;
        }
        if (exts.fDepthStencilResolveKHR) {
            fDepthStencilResolveExtension = VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME;
        }
    }
    SkASSERT(fAPIVersion >= VK_API_VERSION_1_1);

    if (exts.fRasterizationOrderAttachmentAccessARM) {
        fRasterizationOrderAttachmentAccess.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_ARM;
        fRasterizationOrderAttachmentAccessExtension =
                VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME;
    }
    if (exts.fRasterizationOrderAttachmentAccessEXT) {
        fRasterizationOrderAttachmentAccess.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT;
        fRasterizationOrderAttachmentAccessExtension =
                VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_EXTENSION_NAME;
    }
    if (exts.fBlendOperationAdvancedEXT) {
        fBlendOperationAdvanced.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
    }
    if (exts.fVertexInputDynamicStateEXT) {
        fVertexInputDynamicState.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
    }
    if (exts.fGraphicsPipelineLibraryEXT) {
        fGraphicsPipelineLibrary.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT;
        // Depends on VK_KHR_pipeline_library
        fPipelineLibraryExtension = VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME;
    }
    if (exts.fRGBA10x6FormatsEXT) {
        fRGBA10x6Formats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT;
    }
    if (exts.fMultisampledRenderToSingleSampledEXT) {
        fMultisampledRenderToSingleSampled.sType =
                VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT;
    }
    if (exts.fConservativeRasterizationEXT) {
        fConservativeRasterizationExtension = VK_EXT_CONSERVATIVE_RASTERIZATION_EXTENSION_NAME;
    }
#if defined(SK_BUILD_FOR_ANDROID)
    if (exts.fExternalMemoryAHardwareBufferEXT) {
        fExternalMemoryAHardwareBufferExtension =
                VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME;
    }
#endif
    if (exts.fFrameBoundaryEXT) {
        fFrameBoundary.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT;
    }

    // Inspect the list of features the app has already included and decide on which features to
    // add to the query.
    const FeaturesToAdd toQuery = get_features_to_query(fAPIVersion, exts, appFeatures);

    // Chain any new features we'd like to query.
    if (toQuery.fVulkan11) {
        SkASSERT(fVulkan11.sType != 0);
        AddToPNextChain(&appFeatures, &fVulkan11);
    }
    if (toQuery.fVulkan12) {
        SkASSERT(fVulkan12.sType != 0);
        AddToPNextChain(&appFeatures, &fVulkan12);
    }
    if (toQuery.fVulkan13) {
        SkASSERT(fVulkan13.sType != 0);
        AddToPNextChain(&appFeatures, &fVulkan13);
    }
    if (toQuery.fVulkan14) {
        SkASSERT(fVulkan14.sType != 0);
        AddToPNextChain(&appFeatures, &fVulkan14);
    }
    if (toQuery.fRasterizationOrderAttachmentAccess) {
        SkASSERT(fRasterizationOrderAttachmentAccess.sType != 0);
        AddToPNextChain(&appFeatures, &fRasterizationOrderAttachmentAccess);
    }
    if (toQuery.fBlendOperationAdvanced) {
        SkASSERT(fBlendOperationAdvanced.sType != 0);
        AddToPNextChain(&appFeatures, &fBlendOperationAdvanced);
    }
    if (toQuery.fExtendedDynamicState) {
        SkASSERT(fExtendedDynamicState.sType != 0);
        AddToPNextChain(&appFeatures, &fExtendedDynamicState);
    }
    if (toQuery.fExtendedDynamicState2) {
        SkASSERT(fExtendedDynamicState2.sType != 0);
        AddToPNextChain(&appFeatures, &fExtendedDynamicState2);
    }
    if (toQuery.fVertexInputDynamicState) {
        SkASSERT(fVertexInputDynamicState.sType != 0);
        AddToPNextChain(&appFeatures, &fVertexInputDynamicState);
    }
    if (toQuery.fGraphicsPipelineLibrary) {
        SkASSERT(fGraphicsPipelineLibrary.sType != 0);
        AddToPNextChain(&appFeatures, &fGraphicsPipelineLibrary);
    }
    if (toQuery.fSamplerYcbcrConversion) {
        SkASSERT(fSamplerYcbcrConversion.sType != 0);
        AddToPNextChain(&appFeatures, &fSamplerYcbcrConversion);
    }
    if (toQuery.fRGBA10x6Formats) {
        SkASSERT(fRGBA10x6Formats.sType != 0);
        AddToPNextChain(&appFeatures, &fRGBA10x6Formats);
    }
    if (toQuery.fSynchronization2) {
        SkASSERT(fSynchronization2.sType != 0);
        AddToPNextChain(&appFeatures, &fSynchronization2);
    }
    if (toQuery.fDynamicRendering) {
        SkASSERT(fDynamicRendering.sType != 0);
        AddToPNextChain(&appFeatures, &fDynamicRendering);
    }
    if (toQuery.fDynamicRenderingLocalRead) {
        SkASSERT(fDynamicRenderingLocalRead.sType != 0);
        AddToPNextChain(&appFeatures, &fDynamicRenderingLocalRead);
    }
    if (toQuery.fMultisampledRenderToSingleSampled) {
        SkASSERT(fMultisampledRenderToSingleSampled.sType != 0);
        AddToPNextChain(&appFeatures, &fMultisampledRenderToSingleSampled);
    }
    if (toQuery.fHostImageCopy) {
        SkASSERT(fHostImageCopy.sType != 0);
        AddToPNextChain(&appFeatures, &fHostImageCopy);
    }
    if (toQuery.fPipelineCreationCacheControl) {
        SkASSERT(fPipelineCreationCacheControl.sType != 0);
        AddToPNextChain(&appFeatures, &fPipelineCreationCacheControl);
    }
    if (toQuery.fFrameBoundary) {
        SkASSERT(fFrameBoundary.sType != 0);
        AddToPNextChain(&appFeatures, &fFrameBoundary);
    }

    fHasAddedFeaturesToQuery = true;
}

void VulkanPreferredFeatures::addFeaturesToEnable(std::vector<const char*>& appExtensions,
                                                  VkPhysicalDeviceFeatures2& appFeatures) {
    if (!fHasAddedFeaturesToQuery) {
        SkDebugf(
                "WARNING: VulkanPreferredFeatures::addFeaturesToQuery() was not called before "
                "addFeaturesToEnable, performance may degrade\n");
    }
    SkASSERT(fHasAddedFeaturesToQuery);

    FeaturesToAdd toEnable;
    toEnable.fVulkan11 = fVulkan11.sType != 0;
    toEnable.fVulkan12 = fVulkan12.sType != 0;
    toEnable.fVulkan13 = fVulkan13.sType != 0;
    toEnable.fVulkan14 = fVulkan14.sType != 0;
    toEnable.fRasterizationOrderAttachmentAccess = fRasterizationOrderAttachmentAccess.sType != 0;
    toEnable.fBlendOperationAdvanced = fBlendOperationAdvanced.sType != 0;
    toEnable.fExtendedDynamicState = fExtendedDynamicState.sType != 0;
    toEnable.fExtendedDynamicState2 = fExtendedDynamicState2.sType != 0;
    toEnable.fVertexInputDynamicState = fVertexInputDynamicState.sType != 0;
    toEnable.fGraphicsPipelineLibrary = fGraphicsPipelineLibrary.sType != 0;
    toEnable.fRGBA10x6Formats = fRGBA10x6Formats.sType != 0;
    toEnable.fSynchronization2 = fSynchronization2.sType != 0;
    toEnable.fDynamicRendering = fDynamicRendering.sType != 0;
    toEnable.fDynamicRenderingLocalRead = fDynamicRenderingLocalRead.sType != 0;
    toEnable.fMultisampledRenderToSingleSampled = fMultisampledRenderToSingleSampled.sType != 0;
    toEnable.fHostImageCopy = fHostImageCopy.sType != 0;
    toEnable.fPipelineCreationCacheControl = fPipelineCreationCacheControl.sType != 0;
    toEnable.fFrameBoundary = fFrameBoundary.sType != 0;

    // Note on enabling the YCbCr conversion feature: Regardless of whether Skia or the app added
    // VkPhysicalDeviceSamplerYcbcrConversionFeatures to the list of device features to query, we
    // expect that it is enabled OR the client has explicitly disabled it. In either case, we should
    // not override their selection.

    // Check which extensions are already enabled by the application.
    DeviceExtensions appEnabledExts;
    for (const char* name : appExtensions) {
        mark_device_extensions(appEnabledExts, name);
    }

    // Add extensions that are not enabled by the app.
    {
        if (!appEnabledExts.fRasterizationOrderAttachmentAccessARM &&
            !appEnabledExts.fRasterizationOrderAttachmentAccessEXT &&
            toEnable.fRasterizationOrderAttachmentAccess) {
            appExtensions.push_back(fRasterizationOrderAttachmentAccessExtension);
            appEnabledExts.fRasterizationOrderAttachmentAccessEXT = true;
        }
        if (!appEnabledExts.fBlendOperationAdvancedEXT && toEnable.fBlendOperationAdvanced) {
            appExtensions.push_back(VK_EXT_BLEND_OPERATION_ADVANCED_EXTENSION_NAME);
            appEnabledExts.fBlendOperationAdvancedEXT = true;
        }
        if (!appEnabledExts.fExtendedDynamicStateEXT && toEnable.fExtendedDynamicState) {
            appExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
            appEnabledExts.fExtendedDynamicStateEXT = true;
        }
        if (!appEnabledExts.fExtendedDynamicState2EXT && toEnable.fExtendedDynamicState2) {
            appExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
            appEnabledExts.fExtendedDynamicState2EXT = true;
        }
        if (!appEnabledExts.fVertexInputDynamicStateEXT && toEnable.fVertexInputDynamicState) {
            appExtensions.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
            appEnabledExts.fVertexInputDynamicStateEXT = true;
        }
        if (!appEnabledExts.fGraphicsPipelineLibraryEXT && toEnable.fGraphicsPipelineLibrary) {
            appExtensions.push_back(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME);
            appEnabledExts.fGraphicsPipelineLibraryEXT = true;
        }
        if (!appEnabledExts.fRGBA10x6FormatsEXT && toEnable.fRGBA10x6Formats) {
            appExtensions.push_back(VK_EXT_RGBA10X6_FORMATS_EXTENSION_NAME);
            appEnabledExts.fRGBA10x6FormatsEXT = true;
        }
        if (!appEnabledExts.fSynchronization2KHR && toEnable.fSynchronization2) {
            appExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
            appEnabledExts.fSynchronization2KHR = true;
        }
        if (!appEnabledExts.fDynamicRenderingKHR && toEnable.fDynamicRendering) {
            appExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
            appEnabledExts.fDynamicRenderingKHR = true;
        }
        if (!appEnabledExts.fDynamicRenderingLocalReadKHR && toEnable.fDynamicRenderingLocalRead) {
            appExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME);
            appEnabledExts.fDynamicRenderingLocalReadKHR = true;
        }
        if (!appEnabledExts.fMultisampledRenderToSingleSampledEXT &&
            toEnable.fMultisampledRenderToSingleSampled) {
            appExtensions.push_back(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_EXTENSION_NAME);
            appEnabledExts.fMultisampledRenderToSingleSampledEXT = true;
        }
        if (!appEnabledExts.fHostImageCopyEXT && toEnable.fHostImageCopy) {
            appExtensions.push_back(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME);
            appEnabledExts.fHostImageCopyEXT = true;
        }
        if (!appEnabledExts.fPipelineCreationCacheControlEXT &&
            toEnable.fPipelineCreationCacheControl) {
            appExtensions.push_back(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
            appEnabledExts.fPipelineCreationCacheControlEXT = true;
        }
        if (!appEnabledExts.fFrameBoundaryEXT && toEnable.fFrameBoundary) {
            appExtensions.push_back(VK_EXT_FRAME_BOUNDARY_EXTENSION_NAME);
            appEnabledExts.fFrameBoundaryEXT = true;
        }
        if (!appEnabledExts.fDriverPropertiesKHR && fDriverPropertiesExtension != nullptr) {
            appExtensions.push_back(fDriverPropertiesExtension);
            appEnabledExts.fDriverPropertiesKHR = true;
        }
        if (!appEnabledExts.fCreateRenderpass2KHR && fCreateRenderpass2Extension != nullptr) {
            appExtensions.push_back(fCreateRenderpass2Extension);
            appEnabledExts.fCreateRenderpass2KHR = true;
        }
        if (!appEnabledExts.fLoadStoreOpNoneKHR && !appEnabledExts.fLoadStoreOpNoneEXT &&
            fLoadStoreOpNoneExtension != nullptr) {
            appExtensions.push_back(fLoadStoreOpNoneExtension);
            appEnabledExts.fCreateRenderpass2KHR = true;
        }
        if (!appEnabledExts.fConservativeRasterizationEXT &&
            fConservativeRasterizationExtension != nullptr) {
            appExtensions.push_back(fConservativeRasterizationExtension);
            appEnabledExts.fConservativeRasterizationEXT = true;
        }
#if defined(SK_BUILD_FOR_ANDROID)
        if (!appEnabledExts.fExternalMemoryAHardwareBufferEXT &&
            fExternalMemoryAHardwareBufferExtension != nullptr) {
            appExtensions.push_back(fExternalMemoryAHardwareBufferExtension);
            appEnabledExts.fExternalMemoryAHardwareBufferEXT = true;
        }
#endif
        if (!appEnabledExts.fPipelineLibraryKHR && fPipelineLibraryExtension != nullptr) {
            appExtensions.push_back(fPipelineLibraryExtension);
            appEnabledExts.fPipelineLibraryKHR = true;
        }
        if (!appEnabledExts.fCopyCommands2KHR && fCopyCommands2Extension != nullptr) {
            appExtensions.push_back(fCopyCommands2Extension);
            appEnabledExts.fCopyCommands2KHR = true;
        }
        if (!appEnabledExts.fFormatFeatureFlags2KHR && fFormatFeatureFlags2Extension != nullptr) {
            appExtensions.push_back(fFormatFeatureFlags2Extension);
            appEnabledExts.fFormatFeatureFlags2KHR = true;
        }
        if (!appEnabledExts.fDepthStencilResolveKHR && fDepthStencilResolveExtension != nullptr) {
            appExtensions.push_back(fDepthStencilResolveExtension);
            appEnabledExts.fDepthStencilResolveKHR = true;
        }
#if defined(SK_BUILD_FOR_ANDROID)
        if (!appEnabledExts.fQueueFamilyForeignEXT && fQueueFamilyForeignExtension != nullptr) {
            appExtensions.push_back(fQueueFamilyForeignExtension);
            appEnabledExts.fQueueFamilyForeignEXT = true;
        }
#endif
    }

    // Go over the app features.  There are a couple of possibilities:
    //
    // - A struct is included by the app: In this case, if there is a feature Skia wants that
    //   is required to be supported when the extension is present, that feature is enabled.
    //   If an optional feature is supported but disabled by the app, Skia has no visibility into it
    //   right now, but no such feature is used by Skia currently.
    //
    // - A struct is included by addFeaturesToQuery: In this case, if there are features that Skia
    //   doesn't need, they are disabled since the app wasn't relying on those features.
    //
    // - If a Vulkan1X feature struct needs to be chained, then the individual feature structs that
    //   it subsumes are removed from the chain and their features are enabled in the Vulkan1X
    //   struct instead.
    //
    // Note that chaining the Vulkan1X feature struct is preferred where possible, because a 1.X
    // driver is not obliged to also expose the extensions that are promoted to core in that
    // version. As such, features are aggregated in Vulkan1X structs when possible and the
    // individual extension-specific structs are dropped. If Skia requires that Vulkan1X structs be
    // used when possible, the feature aggregation code below becomes unnecessary.
    //
    // In the following, if an extension of interest is enabled, its spec-required main feature is
    // enabled. Normally this is not needed, as the feature query would return VK_TRUE for that
    // feature. However, this is done to protect against the app having turned the feature off after
    // query.

    // Create a new chain of structs. AddToPNextChain is not used because the structs being chained
    // may already be present in appFeatures' pNext chain.
    void* newChainStart = nullptr;
    void** newChainEnd = &newChainStart;

    if (toEnable.fVulkan11) {
        // Disable all features not used by Skia
        fVulkan11.storageBuffer16BitAccess = VK_FALSE;
        fVulkan11.uniformAndStorageBuffer16BitAccess = VK_FALSE;
        fVulkan11.storagePushConstant16 = VK_FALSE;
        fVulkan11.storageInputOutput16 = VK_FALSE;
        fVulkan11.multiview = VK_FALSE;
        fVulkan11.multiviewGeometryShader = VK_FALSE;
        fVulkan11.multiviewTessellationShader = VK_FALSE;
        fVulkan11.variablePointersStorageBuffer = VK_FALSE;
        fVulkan11.variablePointers = VK_FALSE;
        fVulkan11.protectedMemory = VK_FALSE;
        fVulkan11.shaderDrawParameters = VK_FALSE;

        // samplerYcbcrConversion is required since Version 1.4.
        if (fAPIVersion >= VK_API_VERSION_1_4) {
            fVulkan11.samplerYcbcrConversion = VK_TRUE;
        }

        // The following are features from promoted extensions, where the original extension did not
        // have a feature.
        if (appEnabledExts.fShaderDrawParametersKHR) {
            fVulkan11.shaderDrawParameters = VK_TRUE;
        }
    }
    if (toEnable.fVulkan12) {
        // Disable all features not used by Skia
        fVulkan12.samplerMirrorClampToEdge = VK_FALSE;
        fVulkan12.drawIndirectCount = VK_FALSE;
        fVulkan12.storageBuffer8BitAccess = VK_FALSE;
        fVulkan12.uniformAndStorageBuffer8BitAccess = VK_FALSE;
        fVulkan12.storagePushConstant8 = VK_FALSE;
        fVulkan12.shaderBufferInt64Atomics = VK_FALSE;
        fVulkan12.shaderSharedInt64Atomics = VK_FALSE;
        fVulkan12.shaderFloat16 = VK_FALSE;
        fVulkan12.shaderInt8 = VK_FALSE;
        fVulkan12.descriptorIndexing = VK_FALSE;
        fVulkan12.shaderInputAttachmentArrayDynamicIndexing = VK_FALSE;
        fVulkan12.shaderUniformTexelBufferArrayDynamicIndexing = VK_FALSE;
        fVulkan12.shaderStorageTexelBufferArrayDynamicIndexing = VK_FALSE;
        fVulkan12.shaderUniformBufferArrayNonUniformIndexing = VK_FALSE;
        fVulkan12.shaderSampledImageArrayNonUniformIndexing = VK_FALSE;
        fVulkan12.shaderStorageBufferArrayNonUniformIndexing = VK_FALSE;
        fVulkan12.shaderStorageImageArrayNonUniformIndexing = VK_FALSE;
        fVulkan12.shaderInputAttachmentArrayNonUniformIndexing = VK_FALSE;
        fVulkan12.shaderUniformTexelBufferArrayNonUniformIndexing = VK_FALSE;
        fVulkan12.shaderStorageTexelBufferArrayNonUniformIndexing = VK_FALSE;
        fVulkan12.descriptorBindingUniformBufferUpdateAfterBind = VK_FALSE;
        fVulkan12.descriptorBindingSampledImageUpdateAfterBind = VK_FALSE;
        fVulkan12.descriptorBindingStorageImageUpdateAfterBind = VK_FALSE;
        fVulkan12.descriptorBindingStorageBufferUpdateAfterBind = VK_FALSE;
        fVulkan12.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_FALSE;
        fVulkan12.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_FALSE;
        fVulkan12.descriptorBindingUpdateUnusedWhilePending = VK_FALSE;
        fVulkan12.descriptorBindingPartiallyBound = VK_FALSE;
        fVulkan12.descriptorBindingVariableDescriptorCount = VK_FALSE;
        fVulkan12.runtimeDescriptorArray = VK_FALSE;
        fVulkan12.samplerFilterMinmax = VK_FALSE;
        fVulkan12.scalarBlockLayout = VK_FALSE;
        fVulkan12.imagelessFramebuffer = VK_FALSE;
        fVulkan12.uniformBufferStandardLayout = VK_FALSE;
        fVulkan12.shaderSubgroupExtendedTypes = VK_FALSE;
        fVulkan12.separateDepthStencilLayouts = VK_FALSE;
        fVulkan12.hostQueryReset = VK_FALSE;
        fVulkan12.timelineSemaphore = VK_FALSE;
        fVulkan12.bufferDeviceAddress = VK_FALSE;
        fVulkan12.bufferDeviceAddressCaptureReplay = VK_FALSE;
        fVulkan12.bufferDeviceAddressMultiDevice = VK_FALSE;
        fVulkan12.vulkanMemoryModel = VK_FALSE;
        fVulkan12.vulkanMemoryModelDeviceScope = VK_FALSE;
        fVulkan12.vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE;
        fVulkan12.shaderOutputViewportIndex = VK_FALSE;
        fVulkan12.shaderOutputLayer = VK_FALSE;
        fVulkan12.subgroupBroadcastDynamicId = VK_FALSE;

        // No required feature of Vulkan 1.2 is currently needed. The imagelessFramebuffer feature
        // is a candidate, but likely better to not bother and go directly with dynamic rendering.

        // The following are features from promoted extensions, where the original extension did not
        // have a feature.
        if (appEnabledExts.fDrawIndirectCountKHR) {
            fVulkan12.drawIndirectCount = VK_TRUE;
        }
        if (appEnabledExts.fSamplerMirrorClampToEdgeKHR) {
            fVulkan12.samplerMirrorClampToEdge = VK_TRUE;
        }
        if (appEnabledExts.fDescriptorIndexingEXT) {
            fVulkan12.descriptorIndexing = VK_TRUE;
        }
        if (appEnabledExts.fSamplerFilterMinmaxEXT) {
            fVulkan12.samplerFilterMinmax = VK_TRUE;
        }
        if (appEnabledExts.fShaderViewportIndexLayerEXT) {
            fVulkan12.shaderOutputViewportIndex = VK_TRUE;
            fVulkan12.shaderOutputLayer = VK_TRUE;
        }
    }
    if (toEnable.fVulkan13) {
        // Disable all features not used by Skia
        fVulkan13.robustImageAccess = VK_FALSE;
        fVulkan13.inlineUniformBlock = VK_FALSE;
        fVulkan13.descriptorBindingInlineUniformBlockUpdateAfterBind = VK_FALSE;
        fVulkan13.privateData = VK_FALSE;
        fVulkan13.shaderDemoteToHelperInvocation = VK_FALSE;
        fVulkan13.shaderTerminateInvocation = VK_FALSE;
        fVulkan13.subgroupSizeControl = VK_FALSE;
        fVulkan13.computeFullSubgroups = VK_FALSE;
        fVulkan13.textureCompressionASTC_HDR = VK_FALSE;
        fVulkan13.shaderZeroInitializeWorkgroupMemory = VK_FALSE;
        fVulkan13.shaderIntegerDotProduct = VK_FALSE;
        fVulkan13.maintenance4 = VK_FALSE;

        // synchronization2, dynamicRendering and pipelineCreationCacheControl are required features
        // of Vulkan 1.3.
        fVulkan13.synchronization2 = VK_TRUE;
        fVulkan13.dynamicRendering = VK_TRUE;
        fVulkan13.pipelineCreationCacheControl = VK_TRUE;
    }
    if (toEnable.fVulkan14) {
        // Disable all features not used by Skia
        fVulkan14.globalPriorityQuery = VK_FALSE;
        fVulkan14.shaderSubgroupRotate = VK_FALSE;
        fVulkan14.shaderSubgroupRotateClustered = VK_FALSE;
        fVulkan14.shaderFloatControls2 = VK_FALSE;
        fVulkan14.shaderExpectAssume = VK_FALSE;
        fVulkan14.rectangularLines = VK_FALSE;
        fVulkan14.bresenhamLines = VK_FALSE;
        fVulkan14.smoothLines = VK_FALSE;
        fVulkan14.stippledRectangularLines = VK_FALSE;
        fVulkan14.stippledBresenhamLines = VK_FALSE;
        fVulkan14.stippledSmoothLines = VK_FALSE;
        fVulkan14.vertexAttributeInstanceRateDivisor = VK_FALSE;
        fVulkan14.vertexAttributeInstanceRateZeroDivisor = VK_FALSE;
        fVulkan14.indexTypeUint8 = VK_FALSE;
        fVulkan14.maintenance5 = VK_FALSE;
        fVulkan14.maintenance6 = VK_FALSE;
        fVulkan14.pipelineProtectedAccess = VK_FALSE;
        fVulkan14.pipelineRobustness = VK_FALSE;
        fVulkan14.pushDescriptor = VK_FALSE;

        // dynamicRenderingLocalRead is a required feature of Vulkan 1.4.
        fVulkan14.dynamicRenderingLocalRead = VK_TRUE;

        // The following are features from promoted extensions, where the original extension did not
        // have a feature.
        if (appEnabledExts.fPushDescriptorKHR) {
            fVulkan14.pushDescriptor = VK_TRUE;
        }
    }

    bool hasVulkan11Features = toEnable.fVulkan11;
    bool hasVulkan12Features = toEnable.fVulkan12;
    bool hasVulkan13Features = toEnable.fVulkan13;
    bool hasVulkan14Features = toEnable.fVulkan14;

    // If chained by Skia, disable depth/stencil coherence even if supported, in case it comes with
    // a perf cost.
    fRasterizationOrderAttachmentAccess.rasterizationOrderDepthAttachmentAccess = VK_FALSE;
    fRasterizationOrderAttachmentAccess.rasterizationOrderStencilAttachmentAccess = VK_FALSE;

    // If chained by Skia, disable dynamic state that is unused by Skia.
    fExtendedDynamicState2.extendedDynamicState2LogicOp = VK_FALSE;
    fExtendedDynamicState2.extendedDynamicState2PatchControlPoints = VK_FALSE;

// Helper to enable a feature in the feature aggregate struct if it's enabled in the
// extension-specific struct.  It reduces code verbosity but more importantly ensures the feature
// name is identical and reduces chance of copy-paste mistakes.
#define DUP_INTO(aggregate, feature) aggregate.feature |= features->feature

    VkBaseOutStructure* pNext = static_cast<VkBaseOutStructure*>(appFeatures.pNext);
    while (pNext) {
        switch (pNext->sType) {
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES: {
                auto* features = reinterpret_cast<VkPhysicalDeviceVulkan11Features*>(pNext);
                // If this struct is not chained by Skia, copy enabled features.
                if (features != &fVulkan11) {
                    DUP_INTO(fVulkan11, storageBuffer16BitAccess);
                    DUP_INTO(fVulkan11, uniformAndStorageBuffer16BitAccess);
                    DUP_INTO(fVulkan11, storagePushConstant16);
                    DUP_INTO(fVulkan11, storageInputOutput16);
                    DUP_INTO(fVulkan11, multiview);
                    DUP_INTO(fVulkan11, multiviewGeometryShader);
                    DUP_INTO(fVulkan11, multiviewTessellationShader);
                    DUP_INTO(fVulkan11, variablePointersStorageBuffer);
                    DUP_INTO(fVulkan11, variablePointers);
                    DUP_INTO(fVulkan11, protectedMemory);
                    DUP_INTO(fVulkan11, samplerYcbcrConversion);
                    DUP_INTO(fVulkan11, shaderDrawParameters);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES: {
                auto* features = reinterpret_cast<VkPhysicalDeviceVulkan12Features*>(pNext);
                // If this struct is not chained by Skia, copy enabled features.
                if (features != &fVulkan12) {
                    DUP_INTO(fVulkan12, samplerMirrorClampToEdge);
                    DUP_INTO(fVulkan12, drawIndirectCount);
                    DUP_INTO(fVulkan12, storageBuffer8BitAccess);
                    DUP_INTO(fVulkan12, uniformAndStorageBuffer8BitAccess);
                    DUP_INTO(fVulkan12, storagePushConstant8);
                    DUP_INTO(fVulkan12, shaderBufferInt64Atomics);
                    DUP_INTO(fVulkan12, shaderSharedInt64Atomics);
                    DUP_INTO(fVulkan12, shaderFloat16);
                    DUP_INTO(fVulkan12, shaderInt8);
                    DUP_INTO(fVulkan12, descriptorIndexing);
                    DUP_INTO(fVulkan12, shaderInputAttachmentArrayDynamicIndexing);
                    DUP_INTO(fVulkan12, shaderUniformTexelBufferArrayDynamicIndexing);
                    DUP_INTO(fVulkan12, shaderStorageTexelBufferArrayDynamicIndexing);
                    DUP_INTO(fVulkan12, shaderUniformBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderSampledImageArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderStorageBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderStorageImageArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderInputAttachmentArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderUniformTexelBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderStorageTexelBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, descriptorBindingUniformBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingSampledImageUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingStorageImageUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingStorageBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingUniformTexelBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingStorageTexelBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingUpdateUnusedWhilePending);
                    DUP_INTO(fVulkan12, descriptorBindingPartiallyBound);
                    DUP_INTO(fVulkan12, descriptorBindingVariableDescriptorCount);
                    DUP_INTO(fVulkan12, runtimeDescriptorArray);
                    DUP_INTO(fVulkan12, samplerFilterMinmax);
                    DUP_INTO(fVulkan12, scalarBlockLayout);
                    DUP_INTO(fVulkan12, imagelessFramebuffer);
                    DUP_INTO(fVulkan12, uniformBufferStandardLayout);
                    DUP_INTO(fVulkan12, shaderSubgroupExtendedTypes);
                    DUP_INTO(fVulkan12, separateDepthStencilLayouts);
                    DUP_INTO(fVulkan12, hostQueryReset);
                    DUP_INTO(fVulkan12, timelineSemaphore);
                    DUP_INTO(fVulkan12, bufferDeviceAddress);
                    DUP_INTO(fVulkan12, bufferDeviceAddressCaptureReplay);
                    DUP_INTO(fVulkan12, bufferDeviceAddressMultiDevice);
                    DUP_INTO(fVulkan12, vulkanMemoryModel);
                    DUP_INTO(fVulkan12, vulkanMemoryModelDeviceScope);
                    DUP_INTO(fVulkan12, vulkanMemoryModelAvailabilityVisibilityChains);
                    DUP_INTO(fVulkan12, shaderOutputViewportIndex);
                    DUP_INTO(fVulkan12, shaderOutputLayer);
                    DUP_INTO(fVulkan12, subgroupBroadcastDynamicId);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES: {
                auto* features = reinterpret_cast<VkPhysicalDeviceVulkan13Features*>(pNext);
                // If this struct is not chained by Skia, copy enabled features.
                if (features != &fVulkan13) {
                    DUP_INTO(fVulkan13, robustImageAccess);
                    DUP_INTO(fVulkan13, inlineUniformBlock);
                    DUP_INTO(fVulkan13, descriptorBindingInlineUniformBlockUpdateAfterBind);
                    DUP_INTO(fVulkan13, pipelineCreationCacheControl);
                    DUP_INTO(fVulkan13, privateData);
                    DUP_INTO(fVulkan13, shaderDemoteToHelperInvocation);
                    DUP_INTO(fVulkan13, shaderTerminateInvocation);
                    DUP_INTO(fVulkan13, subgroupSizeControl);
                    DUP_INTO(fVulkan13, computeFullSubgroups);
                    DUP_INTO(fVulkan13, synchronization2);
                    DUP_INTO(fVulkan13, textureCompressionASTC_HDR);
                    DUP_INTO(fVulkan13, shaderZeroInitializeWorkgroupMemory);
                    DUP_INTO(fVulkan13, dynamicRendering);
                    DUP_INTO(fVulkan13, shaderIntegerDotProduct);
                    DUP_INTO(fVulkan13, maintenance4);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES: {
                auto* features = reinterpret_cast<VkPhysicalDeviceVulkan14Features*>(pNext);
                // If this struct is not chained by Skia, copy enabled features.
                if (features != &fVulkan14) {
                    DUP_INTO(fVulkan14, globalPriorityQuery);
                    DUP_INTO(fVulkan14, shaderSubgroupRotate);
                    DUP_INTO(fVulkan14, shaderSubgroupRotateClustered);
                    DUP_INTO(fVulkan14, shaderFloatControls2);
                    DUP_INTO(fVulkan14, shaderExpectAssume);
                    DUP_INTO(fVulkan14, rectangularLines);
                    DUP_INTO(fVulkan14, bresenhamLines);
                    DUP_INTO(fVulkan14, smoothLines);
                    DUP_INTO(fVulkan14, stippledRectangularLines);
                    DUP_INTO(fVulkan14, stippledBresenhamLines);
                    DUP_INTO(fVulkan14, stippledSmoothLines);
                    DUP_INTO(fVulkan14, vertexAttributeInstanceRateDivisor);
                    DUP_INTO(fVulkan14, vertexAttributeInstanceRateZeroDivisor);
                    DUP_INTO(fVulkan14, indexTypeUint8);
                    DUP_INTO(fVulkan14, dynamicRenderingLocalRead);
                    DUP_INTO(fVulkan14, maintenance5);
                    DUP_INTO(fVulkan14, maintenance6);
                    DUP_INTO(fVulkan14, pipelineProtectedAccess);
                    DUP_INTO(fVulkan14, pipelineRobustness);
                    DUP_INTO(fVulkan14, hostImageCopy);
                    DUP_INTO(fVulkan14, pushDescriptor);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT: {
                chain(newChainEnd, pNext);
                toEnable.fRasterizationOrderAttachmentAccess = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT: {
                chain(newChainEnd, pNext);
                toEnable.fBlendOperationAdvanced = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT: {
                // Drop this struct if Vulkan 1.3 is enabled
                if (!hasVulkan13Features) {
                    chain(newChainEnd, pNext);

                    // Enable the main feature
                    if (appEnabledExts.fExtendedDynamicStateEXT) {
                        auto* features =
                                reinterpret_cast<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*>(
                                        pNext);
                        features->extendedDynamicState = VK_TRUE;
                    }
                }
                toEnable.fExtendedDynamicState = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT: {
                // Drop this struct if Vulkan 1.3 is enabled and the extension's optional features
                // are not (they are not promoted to 1.3).
                auto* features =
                        reinterpret_cast<VkPhysicalDeviceExtendedDynamicState2FeaturesEXT*>(pNext);
                if (!hasVulkan13Features && !features->extendedDynamicState2LogicOp &&
                    !features->extendedDynamicState2PatchControlPoints) {
                    chain(newChainEnd, pNext);

                    // Enable the main feature
                    if (appEnabledExts.fExtendedDynamicState2EXT) {
                        features->extendedDynamicState2 = VK_TRUE;
                    }
                }
                toEnable.fExtendedDynamicState2 = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT: {
                chain(newChainEnd, pNext);
                // Enable the main feature
                if (appEnabledExts.fVertexInputDynamicStateEXT) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT*>(
                                    pNext);
                    features->vertexInputDynamicState = VK_TRUE;
                }
                toEnable.fVertexInputDynamicState = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT: {
                chain(newChainEnd, pNext);
                // Enable the main feature
                if (appEnabledExts.fGraphicsPipelineLibraryEXT) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT*>(
                                    pNext);
                    features->graphicsPipelineLibrary = VK_TRUE;
                }
                toEnable.fGraphicsPipelineLibrary = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES: {
                // Drop this struct if using the Vulkan 1.1 feature struct
                if (!hasVulkan11Features) {
                    chain(newChainEnd, pNext);

                    // Do not explicitly enable the YCbCr conversion feature even if we know
                    // we could by nature of the device supporting VK_KHR_sampler_ycbcr_conversion.
                    // Instead, always rely upon adding this feature struct to the list of
                    // features to query. If queried and supported, we expect that the feature will
                    // be enabled. If not, that means the client manually disabled it, which we also
                    // want to respect.
                }
                toEnable.fSamplerYcbcrConversion = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RGBA10X6_FORMATS_FEATURES_EXT: {
                chain(newChainEnd, pNext);
                toEnable.fRGBA10x6Formats = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES: {
                // Drop this struct if Vulkan 1.3 is enabled
                if (!hasVulkan13Features) {
                    chain(newChainEnd, pNext);
                    // Enable the main feature
                    if (appEnabledExts.fSynchronization2KHR) {
                        auto* features =
                                reinterpret_cast<VkPhysicalDeviceSynchronization2Features*>(pNext);
                        features->synchronization2 = VK_TRUE;
                    }
                }
                toEnable.fSynchronization2 = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES: {
                // Drop this struct if Vulkan 1.3 is enabled
                if (!hasVulkan13Features) {
                    chain(newChainEnd, pNext);
                    // Enable the main feature
                    if (appEnabledExts.fDynamicRenderingKHR) {
                        auto* features =
                                reinterpret_cast<VkPhysicalDeviceDynamicRenderingFeatures*>(pNext);
                        features->dynamicRendering = VK_TRUE;
                    }
                }
                toEnable.fDynamicRendering = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES: {
                // Drop this struct if Vulkan 1.4 is enabled
                if (!hasVulkan14Features) {
                    chain(newChainEnd, pNext);
                    // Enable the main feature
                    if (appEnabledExts.fDynamicRenderingLocalReadKHR) {
                        auto* features = reinterpret_cast<
                                VkPhysicalDeviceDynamicRenderingLocalReadFeatures*>(pNext);
                        features->dynamicRenderingLocalRead = VK_TRUE;
                    }
                    toEnable.fDynamicRenderingLocalRead = false;
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT: {
                chain(newChainEnd, pNext);
                // Enable the main feature
                if (appEnabledExts.fMultisampledRenderToSingleSampledEXT) {
                    auto* features = reinterpret_cast<
                            VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT*>(pNext);
                    features->multisampledRenderToSingleSampled = VK_TRUE;
                }
                toEnable.fMultisampledRenderToSingleSampled = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES: {
                // Drop this struct if Vulkan 1.4 is enabled
                if (!hasVulkan14Features) {
                    chain(newChainEnd, pNext);
                    // Enable the main feature
                    if (appEnabledExts.fHostImageCopyEXT) {
                        auto* features =
                                reinterpret_cast<VkPhysicalDeviceHostImageCopyFeatures*>(pNext);
                        features->hostImageCopy = VK_TRUE;
                    }
                }
                toEnable.fHostImageCopy = false;
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES: {
                // Drop this struct if Vulkan 1.3 is enabled
                if (!hasVulkan13Features) {
                    chain(newChainEnd, pNext);
                    // Enable the main feature
                    if (appEnabledExts.fPipelineCreationCacheControlEXT) {
                        auto* features = reinterpret_cast<
                                VkPhysicalDevicePipelineCreationCacheControlFeatures*>(pNext);
                        features->pipelineCreationCacheControl = VK_TRUE;
                    }
                }
                toEnable.fPipelineCreationCacheControl = false;
                break;
            }
            // Gather features promoted to Vulkan 1.1 and drop their structs per the following valid
            // usage statement:
            //
            // If the pNext chain includes a VkPhysicalDeviceVulkan11Features structure, then it
            // must not include a VkPhysicalDevice16BitStorageFeatures,
            // VkPhysicalDeviceMultiviewFeatures, VkPhysicalDeviceVariablePointersFeatures,
            // VkPhysicalDeviceProtectedMemoryFeatures,
            // VkPhysicalDeviceSamplerYcbcrConversionFeatures, or
            // VkPhysicalDeviceShaderDrawParametersFeatures structure
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES: {
                if (hasVulkan11Features) {
                    auto* features = reinterpret_cast<VkPhysicalDevice16BitStorageFeatures*>(pNext);
                    DUP_INTO(fVulkan11, storageBuffer16BitAccess);
                    DUP_INTO(fVulkan11, uniformAndStorageBuffer16BitAccess);
                    DUP_INTO(fVulkan11, storagePushConstant16);
                    DUP_INTO(fVulkan11, storageInputOutput16);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES: {
                if (hasVulkan11Features) {
                    auto* features = reinterpret_cast<VkPhysicalDeviceMultiviewFeatures*>(pNext);
                    DUP_INTO(fVulkan11, multiview);
                    DUP_INTO(fVulkan11, multiviewGeometryShader);
                    DUP_INTO(fVulkan11, multiviewTessellationShader);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES: {
                if (hasVulkan11Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceVariablePointersFeatures*>(pNext);
                    DUP_INTO(fVulkan11, variablePointersStorageBuffer);
                    DUP_INTO(fVulkan11, variablePointers);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES: {
                if (hasVulkan11Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceProtectedMemoryFeatures*>(pNext);
                    DUP_INTO(fVulkan11, protectedMemory);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES: {
                if (hasVulkan11Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderDrawParametersFeatures*>(pNext);
                    DUP_INTO(fVulkan11, shaderDrawParameters);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }

            // Gather features promoted to Vulkan 1.2 and drop their structs per the following valid
            // usage statement:
            //
            // If the pNext chain includes a VkPhysicalDeviceVulkan12Features structure, then it
            // must not include a VkPhysicalDevice8BitStorageFeatures,
            // VkPhysicalDeviceShaderAtomicInt64Features, VkPhysicalDeviceShaderFloat16Int8Features,
            // VkPhysicalDeviceDescriptorIndexingFeatures,
            // VkPhysicalDeviceScalarBlockLayoutFeatures,
            // VkPhysicalDeviceImagelessFramebufferFeatures,
            // VkPhysicalDeviceUniformBufferStandardLayoutFeatures,
            // VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures,
            // VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures,
            // VkPhysicalDeviceHostQueryResetFeatures, VkPhysicalDeviceTimelineSemaphoreFeatures,
            // VkPhysicalDeviceBufferDeviceAddressFeatures, or
            // VkPhysicalDeviceVulkanMemoryModelFeatures structure
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features = reinterpret_cast<VkPhysicalDevice8BitStorageFeatures*>(pNext);
                    DUP_INTO(fVulkan12, storageBuffer8BitAccess);
                    DUP_INTO(fVulkan12, uniformAndStorageBuffer8BitAccess);
                    DUP_INTO(fVulkan12, storagePushConstant8);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderAtomicInt64Features*>(pNext);
                    DUP_INTO(fVulkan12, shaderBufferInt64Atomics);
                    DUP_INTO(fVulkan12, shaderSharedInt64Atomics);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderFloat16Int8Features*>(pNext);
                    DUP_INTO(fVulkan12, shaderFloat16);
                    DUP_INTO(fVulkan12, shaderInt8);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceDescriptorIndexingFeatures*>(pNext);
                    DUP_INTO(fVulkan12, shaderInputAttachmentArrayDynamicIndexing);
                    DUP_INTO(fVulkan12, shaderUniformTexelBufferArrayDynamicIndexing);
                    DUP_INTO(fVulkan12, shaderStorageTexelBufferArrayDynamicIndexing);
                    DUP_INTO(fVulkan12, shaderUniformBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderSampledImageArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderStorageBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderStorageImageArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderInputAttachmentArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderUniformTexelBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, shaderStorageTexelBufferArrayNonUniformIndexing);
                    DUP_INTO(fVulkan12, descriptorBindingUniformBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingSampledImageUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingStorageImageUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingStorageBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingUniformTexelBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingStorageTexelBufferUpdateAfterBind);
                    DUP_INTO(fVulkan12, descriptorBindingUpdateUnusedWhilePending);
                    DUP_INTO(fVulkan12, descriptorBindingPartiallyBound);
                    DUP_INTO(fVulkan12, descriptorBindingVariableDescriptorCount);
                    DUP_INTO(fVulkan12, runtimeDescriptorArray);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceScalarBlockLayoutFeatures*>(pNext);
                    DUP_INTO(fVulkan12, scalarBlockLayout);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceImagelessFramebufferFeatures*>(pNext);
                    DUP_INTO(fVulkan12, imagelessFramebuffer);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceUniformBufferStandardLayoutFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan12, uniformBufferStandardLayout);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan12, shaderSubgroupExtendedTypes);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan12, separateDepthStencilLayouts);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceHostQueryResetFeatures*>(pNext);
                    DUP_INTO(fVulkan12, hostQueryReset);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceTimelineSemaphoreFeatures*>(pNext);
                    DUP_INTO(fVulkan12, timelineSemaphore);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceBufferDeviceAddressFeatures*>(pNext);
                    DUP_INTO(fVulkan12, bufferDeviceAddress);
                    DUP_INTO(fVulkan12, bufferDeviceAddressCaptureReplay);
                    DUP_INTO(fVulkan12, bufferDeviceAddressMultiDevice);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES: {
                if (hasVulkan12Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceVulkanMemoryModelFeatures*>(pNext);
                    DUP_INTO(fVulkan12, vulkanMemoryModel);
                    DUP_INTO(fVulkan12, vulkanMemoryModelDeviceScope);
                    DUP_INTO(fVulkan12, vulkanMemoryModelAvailabilityVisibilityChains);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }

            // Gather features promoted to Vulkan 1.3 and drop their structs per the following valid
            // usage statement:
            //
            // If the pNext chain includes a VkPhysicalDeviceVulkan13Features structure, then it
            // must not include a VkPhysicalDeviceDynamicRenderingFeatures,
            // VkPhysicalDeviceImageRobustnessFeatures, VkPhysicalDeviceInlineUniformBlockFeatures,
            // VkPhysicalDeviceMaintenance4Features,
            // VkPhysicalDevicePipelineCreationCacheControlFeatures,
            // VkPhysicalDevicePrivateDataFeatures,
            // VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures,
            // VkPhysicalDeviceShaderIntegerDotProductFeatures,
            // VkPhysicalDeviceShaderTerminateInvocationFeatures,
            // VkPhysicalDeviceSubgroupSizeControlFeatures,
            // VkPhysicalDeviceSynchronization2Features,
            // VkPhysicalDeviceTextureCompressionASTCHDRFeatures, or
            // VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures structure
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceImageRobustnessFeatures*>(pNext);
                    DUP_INTO(fVulkan13, robustImageAccess);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceInlineUniformBlockFeatures*>(pNext);
                    DUP_INTO(fVulkan13, inlineUniformBlock);
                    DUP_INTO(fVulkan13, descriptorBindingInlineUniformBlockUpdateAfterBind);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features = reinterpret_cast<VkPhysicalDeviceMaintenance4Features*>(pNext);
                    DUP_INTO(fVulkan13, maintenance4);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features = reinterpret_cast<VkPhysicalDevicePrivateDataFeatures*>(pNext);
                    DUP_INTO(fVulkan13, privateData);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features = reinterpret_cast<
                            VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures*>(pNext);
                    DUP_INTO(fVulkan13, shaderDemoteToHelperInvocation);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderIntegerDotProductFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan13, shaderIntegerDotProduct);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderTerminateInvocationFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan13, shaderTerminateInvocation);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceSubgroupSizeControlFeatures*>(pNext);
                    DUP_INTO(fVulkan13, subgroupSizeControl);
                    DUP_INTO(fVulkan13, computeFullSubgroups);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceTextureCompressionASTCHDRFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan13, textureCompressionASTC_HDR);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES: {
                if (hasVulkan13Features) {
                    auto* features = reinterpret_cast<
                            VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeatures*>(pNext);
                    DUP_INTO(fVulkan13, shaderZeroInitializeWorkgroupMemory);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }

            // Gather features promoted to Vulkan 1.4 and drop their structs per the following valid
            // usage statement:
            //
            // If the pNext chain includes a VkPhysicalDeviceVulkan14Features structure, then it
            // must not include a VkPhysicalDeviceGlobalPriorityQueryFeatures,
            // VkPhysicalDeviceShaderSubgroupRotateFeatures,
            // VkPhysicalDeviceShaderFloatControls2Features,
            // VkPhysicalDeviceShaderExpectAssumeFeatures,
            // VkPhysicalDeviceLineRasterizationFeatures,
            // VkPhysicalDeviceVertexAttributeDivisorFeatures,
            // VkPhysicalDeviceIndexTypeUint8Features,
            // VkPhysicalDeviceDynamicRenderingLocalReadFeatures,
            // VkPhysicalDeviceMaintenance5Features, VkPhysicalDeviceMaintenance6Features,
            // VkPhysicalDevicePipelineProtectedAccessFeatures,
            // VkPhysicalDevicePipelineRobustnessFeatures, or VkPhysicalDeviceHostImageCopyFeatures
            // structure
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GLOBAL_PRIORITY_QUERY_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceGlobalPriorityQueryFeatures*>(pNext);
                    DUP_INTO(fVulkan14, globalPriorityQuery);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_ROTATE_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderSubgroupRotateFeatures*>(pNext);
                    DUP_INTO(fVulkan14, shaderSubgroupRotate);
                    DUP_INTO(fVulkan14, shaderSubgroupRotateClustered);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT_CONTROLS_2_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderFloatControls2Features*>(pNext);
                    DUP_INTO(fVulkan14, shaderFloatControls2);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EXPECT_ASSUME_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceShaderExpectAssumeFeatures*>(pNext);
                    DUP_INTO(fVulkan14, shaderExpectAssume);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINE_RASTERIZATION_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceLineRasterizationFeatures*>(pNext);
                    DUP_INTO(fVulkan14, rectangularLines);
                    DUP_INTO(fVulkan14, bresenhamLines);
                    DUP_INTO(fVulkan14, smoothLines);
                    DUP_INTO(fVulkan14, stippledRectangularLines);
                    DUP_INTO(fVulkan14, stippledBresenhamLines);
                    DUP_INTO(fVulkan14, stippledSmoothLines);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_ATTRIBUTE_DIVISOR_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceVertexAttributeDivisorFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan14, vertexAttributeInstanceRateDivisor);
                    DUP_INTO(fVulkan14, vertexAttributeInstanceRateZeroDivisor);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INDEX_TYPE_UINT8_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceIndexTypeUint8Features*>(pNext);
                    DUP_INTO(fVulkan14, indexTypeUint8);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features = reinterpret_cast<VkPhysicalDeviceMaintenance5Features*>(pNext);
                    DUP_INTO(fVulkan14, maintenance5);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_6_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features = reinterpret_cast<VkPhysicalDeviceMaintenance6Features*>(pNext);
                    DUP_INTO(fVulkan14, maintenance6);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_PROTECTED_ACCESS_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDevicePipelineProtectedAccessFeatures*>(
                                    pNext);
                    DUP_INTO(fVulkan14, pipelineProtectedAccess);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_ROBUSTNESS_FEATURES: {
                if (hasVulkan14Features) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDevicePipelineRobustnessFeatures*>(pNext);
                    DUP_INTO(fVulkan14, pipelineRobustness);
                } else {
                    chain(newChainEnd, pNext);
                }
                break;
            }
            case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAME_BOUNDARY_FEATURES_EXT: {
                chain(newChainEnd, pNext);
                if (appEnabledExts.fFrameBoundaryEXT) {
                    auto* features =
                            reinterpret_cast<VkPhysicalDeviceFrameBoundaryFeaturesEXT*>(
                                    pNext);
                    features->frameBoundary = VK_TRUE;
                }
                toEnable.fFrameBoundary = false;
                break;
            }

            default:
                // Retain everything else that's chained.
                chain(newChainEnd, pNext);
                break;
        }

        pNext = pNext->pNext;
    }
#undef DUP_INTO

    // Chain any struct that was not found in appFeatures.
    if (hasVulkan11Features) {
        chain(newChainEnd, &fVulkan11);
    }
    if (hasVulkan12Features) {
        chain(newChainEnd, &fVulkan12);
    }
    if (hasVulkan13Features) {
        chain(newChainEnd, &fVulkan13);
    }
    if (hasVulkan14Features) {
        chain(newChainEnd, &fVulkan14);
    }
    if (toEnable.fRasterizationOrderAttachmentAccess) {
        chain(newChainEnd, &fRasterizationOrderAttachmentAccess);
    }
    if (toEnable.fBlendOperationAdvanced) {
        chain(newChainEnd, &fBlendOperationAdvanced);
    }
    if (toEnable.fExtendedDynamicState) {
        chain(newChainEnd, &fExtendedDynamicState);
    }
    if (toEnable.fExtendedDynamicState2) {
        chain(newChainEnd, &fExtendedDynamicState2);
    }
    if (toEnable.fVertexInputDynamicState) {
        chain(newChainEnd, &fVertexInputDynamicState);
    }
    if (toEnable.fGraphicsPipelineLibrary) {
        chain(newChainEnd, &fGraphicsPipelineLibrary);
    }
    if (toEnable.fSamplerYcbcrConversion) {
        chain(newChainEnd, &fSamplerYcbcrConversion);
    }
    if (toEnable.fRGBA10x6Formats) {
        chain(newChainEnd, &fRGBA10x6Formats);
    }
    if (toEnable.fSynchronization2) {
        chain(newChainEnd, &fSynchronization2);
    }
    if (toEnable.fDynamicRendering) {
        chain(newChainEnd, &fDynamicRendering);
    }
    if (toEnable.fDynamicRenderingLocalRead) {
        chain(newChainEnd, &fDynamicRenderingLocalRead);
    }
    if (toEnable.fMultisampledRenderToSingleSampled) {
        chain(newChainEnd, &fMultisampledRenderToSingleSampled);
    }
    if (toEnable.fHostImageCopy) {
        chain(newChainEnd, &fHostImageCopy);
    }
    if (toEnable.fPipelineCreationCacheControl) {
        chain(newChainEnd, &fPipelineCreationCacheControl);
    }
    if (toEnable.fFrameBoundary) {
        chain(newChainEnd, &fFrameBoundary);
    }

    // Replace appFeatures' pNext with the new chain.
    *newChainEnd = nullptr;
    appFeatures.pNext = newChainStart;

    fHasAddedFeaturesToEnable = true;
}

}  // namespace skgpu
