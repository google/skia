/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_VulkanPreferredFeatures_DEFINED
#define skgpu_VulkanPreferredFeatures_DEFINED

#include "include/private/base/SkAPI.h"
#include "include/private/gpu/vk/SkiaVulkan.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace skgpu {

class SK_API VulkanPreferredFeatures {
public:
    ~VulkanPreferredFeatures();

    /*
     * VulkanPreferredFeatures is used by Skia to add extensions and features on top of what the app
     * wants to enable. The flow of Vulkan initialization for an app that wants to use Skia would
     * be:
     *
     * ```
     * // Query the loader for instance information, decide on API version to use
     *
     * // Prepare for Skia-enabled extensions and features
     * skgpu::VulkanPreferredFeatures skiaFeatures;
     * skiaFeatures.init(apiVersion);
     *
     * // Decide on instance extensions to use in the app.
     *
     * // Allow Skia to add to the instance extension list.
     * skiaFeatures.addToInstanceExtensions(...);
     *
     * // Create the instance, choose physical device, query available extensions, decide on
     * // device features to query
     *
     * // Allow Skia to add to the feature query list.
     * skiaFeatures.addFeaturesToQuery(...);
     *
     * // Query features, decide on extensions and features to enable.
     *
     * // Allow Skia to add to the device extension and feature list.
     * skiaFeatures.addFeaturesToEnable(...);
     *
     * // Create the Vulkan device.
     * ```
     *
     * Parameters:
     *
     * * appAPIVersion: The API version the app has specified in VkApplicationInfo::apiVersion (and
     *   will later provide in VulkanBackendContext::fMaxAPIVersion). The minimum supported version
     *   is VK_API_VERSION_1_1, maximum is VK_API_VERSION_1_4.
     */
    void init(uint32_t appAPIVersion);

    /*
     * Before creating a Vulkan instance, call addToInstanceExtensions to give Skia a chance to add
     * instance extensions it may take advantage of. Extensions are only added to appExtensions if
     * caller hasn't already included them to be enabled.
     *
     * Parameters:
     *
     * * instanceExtensions, instanceExtensionCount: The list of available instance extensions as
     *   queried from the loader.
     * * appExtensions: The list of extensions to be enabled on the instance; more extensions may be
     *   added to this list by this call.
     */
    void addToInstanceExtensions(const VkExtensionProperties* instanceExtensions,
                                 size_t instanceExtensionCount,
                                 std::vector<const char*>& appExtensions);

    /*
     * Before querying Vulkan device features, call addFeaturesToQuery to give Skia a chance to add
     * device extension features it may take advantage of to the query. Features are only added to
     * appFeatures if caller hasn't already included them in the query.
     *
     * Beware that the structs that get chained to appFeatures are member variables of this class,
     * so this object must not leave scope until feature query and device creation is complete.
     *
     * Parameters:
     *
     * * deviceExtensions, deviceExtensionCount: The list of available device extensions as queried
     *   from the physical device.
     * * appFeatures: The features to be queried from the physical device; more features may be
     *   added to the pNext chain by this call.
     */
    void addFeaturesToQuery(const VkExtensionProperties* deviceExtensions,
                            size_t deviceExtensionCount,
                            VkPhysicalDeviceFeatures2& appFeatures);

    /*
     * Before creating the Vulkan device, call addFeaturesToEnable to give Skia a chance to add
     * device extensions and features it may take advantage of. Extensions and features are only
     * added to appExtensions and appFeatures respectively if caller hasn't already included them to
     * be enabled. This function may replace chained features with VkPhysicalDeviceVulkanNNFeatures
     * structs instead in order to add features. Features that are already enabled by the
     * application are retained in that case in the new struct.
     *
     * Parameters:
     *
     * * appExtensions: The list of extensions to be enabled on the device; more extensions may be
     *   added to this list by this call.
     * * appFeatures: The features to be enabled on the device; more features may be
     *   added to the pNext chain by this call.
     */
    void addFeaturesToEnable(std::vector<const char*>& appExtensions,
                             VkPhysicalDeviceFeatures2& appFeatures);

private:
    uint32_t fAPIVersion = 0;
    // Track what the application did, so warnings can be generated if the class is not
    // fully/correctly used.
    bool fHasAddedToInstanceExtensions = false;
    bool fHasAddedFeaturesToQuery = false;
    bool fHasAddedFeaturesToEnable = false;

    // The list of device features Skia is interested in. If any are included in
    // VkPhysicalDeviceFeatures2::pNext by the app, it is not included again by Skia. However, to
    // take best advantage of the Vulkan API, the app should not intentionally include these feature
    // structs only to forcefully disable the feature - this blocks Skia from leveraging it.
    //
    // When ambiguous, the extension name is included to determine which extension a feature struct
    // has come from. This is usually needed when addFeaturesToQuery has included a feature struct
    // that is present in multiple extensions (e.g. due to extension promotion). It is later used by
    // addFeaturesToEnable to know which extension to enable.

    // Available since Vulkan 1.2
    VkPhysicalDeviceVulkan11Features fVulkan11 = {};

    // Available since Vulkan 1.2
    VkPhysicalDeviceVulkan12Features fVulkan12 = {};

    // Available since Vulkan 1.3
    VkPhysicalDeviceVulkan13Features fVulkan13 = {};

    // Available since Vulkan 1.4
    VkPhysicalDeviceVulkan14Features fVulkan14 = {};

    // Feature of VK_EXT_rasterization_order_attachment_access or
    // VK_ARM_rasterization_order_attachment_access.
    VkPhysicalDeviceRasterizationOrderAttachmentAccessFeaturesEXT
            fRasterizationOrderAttachmentAccess = {};
    const char* fRasterizationOrderAttachmentAccessExtension = nullptr;

    // Feature of VK_EXT_blend_operation_advanced
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT fBlendOperationAdvanced = {};

    // Feature of VK_EXT_extended_dynamic_state
    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT fExtendedDynamicState = {};

    // Feature of VK_EXT_extended_dynamic_state2
    VkPhysicalDeviceExtendedDynamicState2FeaturesEXT fExtendedDynamicState2 = {};

    // Feature of VK_EXT_vertex_input_dynamic_state
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT fVertexInputDynamicState = {};

    // Feature of VK_EXT_graphics_pipeline_library
    VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT fGraphicsPipelineLibrary = {};

    // Feature of VK_KHR_sampler_ycbcr_conversion or Vulkan 1.1
    VkPhysicalDeviceSamplerYcbcrConversionFeatures fSamplerYcbcrConversion = {};

    // Feature of VK_EXT_rgba10x6_formats
    VkPhysicalDeviceRGBA10X6FormatsFeaturesEXT fRGBA10x6Formats = {};

    // Feature of VK_KHR_synchronization2 or Vulkan 1.3
    VkPhysicalDeviceDynamicRenderingFeatures fSynchronization2 = {};

    // Feature of VK_KHR_dynamic_rendering or Vulkan 1.3
    VkPhysicalDeviceDynamicRenderingFeatures fDynamicRendering = {};

    // Feature of VK_KHR_dynamic_rendering_local_read or Vulkan 1.4
    VkPhysicalDeviceDynamicRenderingLocalReadFeatures fDynamicRenderingLocalRead = {};

    // Feature of VK_EXT_multisampled_render_to_single_sampled
    VkPhysicalDeviceMultisampledRenderToSingleSampledFeaturesEXT
            fMultisampledRenderToSingleSampled = {};

    // Feature of VK_EXT_host_image_copy or Vulkan 1.4
    VkPhysicalDeviceHostImageCopyFeatures fHostImageCopy = {};

    // Feature of VK_EXT_pipeline_creation_cache_control or Vulkan 1.3
    VkPhysicalDevicePipelineCreationCacheControlFeatures fPipelineCreationCacheControl = {};

    // Extensions that don't have a feature:
    // VK_KHR_driver_properties or Vulkan 1.2
    const char* fDriverPropertiesExtension = nullptr;
    // VK_KHR_create_renderpass2 or Vulkan 1.2
    const char* fCreateRenderpass2Extension = nullptr;
    // VK_EXT_load_store_op_none, VK_KHR_load_store_op_none or Vulkan 1.4
    const char* fLoadStoreOpNoneExtension = nullptr;
    // VK_EXT_conservative_rasterization
    const char* fConservativeRasterizationExtension = nullptr;

    // Extensions that the other extensions above depend on:
    // Dependency of VK_EXT_graphics_pipeline_library: VK_KHR_pipeline_library
    const char* fPipelineLibraryExtension = nullptr;
    // Dependencies of VK_EXT_host_image_copy: VK_KHR_copy_commands2, VK_KHR_format_feature_flags2
    const char* fCopyCommands2Extension = nullptr;
    const char* fFormatFeatureFlags2Extension = nullptr;
    // Dependency of VK_EXT_multisampled_render_to_single_sampled: VK_KHR_depth_stencil_resolve
    const char* fDepthStencilResolveExtension = nullptr;
};

}  // namespace skgpu

#endif  // skgpu_VulkanPreferredFeatures_DEFINED
