/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a Vulkan specific test, ensuring VulkanPreferredFeatures enables extensions and features
// correctly.

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)
#include "include/gpu/vk/VulkanPreferredFeatures.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <vulkan/vulkan_core.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

// A pretend Vulkan device configuration to expose a subset of Vulkan extensions
//
// Note: Skia doesn't actually enable all of these yet. The test expectations below are commented
// out for the extensions that are planned for the future. Once any extension is enabled, the
// corresponding expectations should be uncommented.
struct VulkanExts {
    bool fRasterizationOrderAttachmentAccessARM = true;
    bool fRasterizationOrderAttachmentAccessEXT = true;
    bool fBlendOperationAdvancedEXT = true;
    bool fExtendedDynamicStateEXT = true;
    bool fExtendedDynamicState2EXT = true;
    bool fVertexInputDynamicStateEXT = true;
    bool fGraphicsPipelineLibraryEXT = true;
    bool fSamplerYcbcrConversionKHR = true;
    bool fRGBA10x6FormatsEXT = true;
    bool fSynchronization2KHR = true;
    bool fDynamicRenderingKHR = true;
    bool fDynamicRenderingLocalReadKHR = true;
    bool fMultisampledRenderToSingleSampledEXT = true;
    bool fHostImageCopyEXT = true;
    bool fPipelineCreationCacheControlEXT = true;
    bool fDriverPropertiesKHR = true;
    bool fCreateRenderpass2KHR = true;
    bool fLoadStoreOpNoneEXT = true;
    bool fLoadStoreOpNoneKHR = true;
    bool fConservativeRasterizationEXT = true;
    bool fPipelineLibraryKHR = true;
    bool fCopyCommands2KHR = true;
    bool fFormatFeatureFlags2KHR = true;
    bool fDepthStencilResolveKHR = true;
    bool fShaderDrawParametersKHR = true;
    bool fDrawIndirectCountKHR = true;
    bool fSamplerMirrorClampToEdgeKHR = true;
    bool fDescriptorIndexingEXT = true;
    bool fSamplerFilterMinmaxEXT = true;
    bool fShaderViewportIndexLayerEXT = true;
    bool fPushDescriptorKHR = true;
    bool fFrameBoundaryEXT = true;
};

static std::vector<VkExtensionProperties> get_device_exts(const VulkanExts& config) {
    std::vector<VkExtensionProperties> exts;
#define ADD_EXT(fFlag, EXTNAME)                                  \
    do {                                                         \
        if (config.fFlag) {                                      \
            VkExtensionProperties ext = {};                      \
            strcpy(ext.extensionName, EXTNAME##_EXTENSION_NAME); \
            ext.specVersion = 1;                                 \
            exts.push_back(ext);                                 \
        }                                                        \
    } while (0)
    ADD_EXT(fRasterizationOrderAttachmentAccessARM, VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    ADD_EXT(fRasterizationOrderAttachmentAccessEXT, VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    ADD_EXT(fBlendOperationAdvancedEXT, VK_EXT_BLEND_OPERATION_ADVANCED);
    ADD_EXT(fExtendedDynamicStateEXT, VK_EXT_EXTENDED_DYNAMIC_STATE);
    ADD_EXT(fExtendedDynamicState2EXT, VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    ADD_EXT(fVertexInputDynamicStateEXT, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    ADD_EXT(fGraphicsPipelineLibraryEXT, VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    ADD_EXT(fSamplerYcbcrConversionKHR, VK_KHR_SAMPLER_YCBCR_CONVERSION);
    ADD_EXT(fRGBA10x6FormatsEXT, VK_EXT_RGBA10X6_FORMATS);
    ADD_EXT(fSynchronization2KHR, VK_KHR_SYNCHRONIZATION_2);
    ADD_EXT(fDynamicRenderingKHR, VK_KHR_DYNAMIC_RENDERING);
    ADD_EXT(fDynamicRenderingLocalReadKHR, VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    ADD_EXT(fMultisampledRenderToSingleSampledEXT, VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    ADD_EXT(fHostImageCopyEXT, VK_EXT_HOST_IMAGE_COPY);
    ADD_EXT(fPipelineCreationCacheControlEXT, VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    ADD_EXT(fDriverPropertiesKHR, VK_KHR_DRIVER_PROPERTIES);
    ADD_EXT(fCreateRenderpass2KHR, VK_KHR_CREATE_RENDERPASS_2);
    ADD_EXT(fLoadStoreOpNoneEXT, VK_EXT_LOAD_STORE_OP_NONE);
    ADD_EXT(fLoadStoreOpNoneKHR, VK_KHR_LOAD_STORE_OP_NONE);
    ADD_EXT(fConservativeRasterizationEXT, VK_EXT_CONSERVATIVE_RASTERIZATION);
    ADD_EXT(fPipelineLibraryKHR, VK_KHR_PIPELINE_LIBRARY);
    ADD_EXT(fCopyCommands2KHR, VK_KHR_COPY_COMMANDS_2);
    ADD_EXT(fFormatFeatureFlags2KHR, VK_KHR_FORMAT_FEATURE_FLAGS_2);
    ADD_EXT(fDepthStencilResolveKHR, VK_KHR_DEPTH_STENCIL_RESOLVE);
    ADD_EXT(fShaderDrawParametersKHR, VK_KHR_SHADER_DRAW_PARAMETERS);
    ADD_EXT(fDrawIndirectCountKHR, VK_KHR_DRAW_INDIRECT_COUNT);
    ADD_EXT(fSamplerMirrorClampToEdgeKHR, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    ADD_EXT(fDescriptorIndexingEXT, VK_EXT_DESCRIPTOR_INDEXING);
    ADD_EXT(fSamplerFilterMinmaxEXT, VK_EXT_SAMPLER_FILTER_MINMAX);
    ADD_EXT(fShaderViewportIndexLayerEXT, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    ADD_EXT(fPushDescriptorKHR, VK_KHR_PUSH_DESCRIPTOR);
    ADD_EXT(fFrameBoundaryEXT, VK_EXT_FRAME_BOUNDARY);
#undef ADD_EXT
    return exts;
}

static void enable_device_features(const VulkanExts& config, void* basePNext) {
#define SET_IF(fFlag, name)           \
    do {                              \
        if (config.fFlag) {           \
            features->name = VK_TRUE; \
        }                             \
    } while (0)
#define SET_FEATURES(STRUCT_NAME, StructName, set)                               \
    case VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME: {                      \
        auto* features = reinterpret_cast<VkPhysicalDevice##StructName*>(pNext); \
        set;                                                                     \
        break;                                                                   \
    }

    VkBaseOutStructure* pNext = static_cast<VkBaseOutStructure*>(basePNext);
    while (pNext) {
        switch (pNext->sType) {
            SET_FEATURES(VULKAN_1_1_FEATURES,
                         Vulkan11Features,
                         SET_IF(fSamplerYcbcrConversionKHR, samplerYcbcrConversion);
                         SET_IF(fShaderDrawParametersKHR, shaderDrawParameters))

            SET_FEATURES(VULKAN_1_2_FEATURES,
                         Vulkan12Features,
                         SET_IF(fSamplerMirrorClampToEdgeKHR, samplerMirrorClampToEdge);
                         SET_IF(fDrawIndirectCountKHR, drawIndirectCount);
                         SET_IF(fDescriptorIndexingEXT, descriptorIndexing);
                         SET_IF(fSamplerFilterMinmaxEXT, samplerFilterMinmax);
                         SET_IF(fShaderViewportIndexLayerEXT, shaderOutputViewportIndex);
                         SET_IF(fShaderViewportIndexLayerEXT, shaderOutputLayer))

            SET_FEATURES(VULKAN_1_3_FEATURES,
                         Vulkan13Features,
                         SET_IF(fPipelineCreationCacheControlEXT, pipelineCreationCacheControl);
                         SET_IF(fSynchronization2KHR, synchronization2);
                         SET_IF(fDynamicRenderingKHR, dynamicRendering))

            SET_FEATURES(VULKAN_1_4_FEATURES,
                         Vulkan14Features,
                         SET_IF(fDynamicRenderingLocalReadKHR, dynamicRenderingLocalRead);
                         SET_IF(fHostImageCopyEXT, hostImageCopy))

            SET_FEATURES(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                         RasterizationOrderAttachmentAccessFeaturesEXT,
                         SET_IF(fRasterizationOrderAttachmentAccessARM,
                                rasterizationOrderColorAttachmentAccess);
                         SET_IF(fRasterizationOrderAttachmentAccessARM,
                                rasterizationOrderDepthAttachmentAccess);
                         SET_IF(fRasterizationOrderAttachmentAccessARM,
                                rasterizationOrderStencilAttachmentAccess);
                         SET_IF(fRasterizationOrderAttachmentAccessEXT,
                                rasterizationOrderColorAttachmentAccess);
                         SET_IF(fRasterizationOrderAttachmentAccessEXT,
                                rasterizationOrderDepthAttachmentAccess);
                         SET_IF(fRasterizationOrderAttachmentAccessEXT,
                                rasterizationOrderStencilAttachmentAccess))

            SET_FEATURES(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                         BlendOperationAdvancedFeaturesEXT,
                         SET_IF(fBlendOperationAdvancedEXT, advancedBlendCoherentOperations))

            SET_FEATURES(EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                         ExtendedDynamicStateFeaturesEXT,
                         SET_IF(fExtendedDynamicStateEXT, extendedDynamicState))

            SET_FEATURES(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                         ExtendedDynamicState2FeaturesEXT,
                         SET_IF(fExtendedDynamicState2EXT, extendedDynamicState2);
                         SET_IF(fExtendedDynamicState2EXT, extendedDynamicState2LogicOp);
                         SET_IF(fExtendedDynamicState2EXT, extendedDynamicState2PatchControlPoints))

            SET_FEATURES(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                         VertexInputDynamicStateFeaturesEXT,
                         SET_IF(fVertexInputDynamicStateEXT, vertexInputDynamicState))

            SET_FEATURES(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                         GraphicsPipelineLibraryFeaturesEXT,
                         SET_IF(fGraphicsPipelineLibraryEXT, graphicsPipelineLibrary))

            SET_FEATURES(SAMPLER_YCBCR_CONVERSION_FEATURES,
                         SamplerYcbcrConversionFeatures,
                         SET_IF(fSamplerYcbcrConversionKHR, samplerYcbcrConversion))

            SET_FEATURES(RGBA10X6_FORMATS_FEATURES_EXT,
                         RGBA10X6FormatsFeaturesEXT,
                         SET_IF(fRGBA10x6FormatsEXT, formatRgba10x6WithoutYCbCrSampler))

            SET_FEATURES(SYNCHRONIZATION_2_FEATURES,
                         Synchronization2Features,
                         SET_IF(fSynchronization2KHR, synchronization2))

            SET_FEATURES(DYNAMIC_RENDERING_FEATURES,
                         DynamicRenderingFeatures,
                         SET_IF(fDynamicRenderingKHR, dynamicRendering))

            SET_FEATURES(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
                         DynamicRenderingLocalReadFeatures,
                         SET_IF(fDynamicRenderingLocalReadKHR, dynamicRenderingLocalRead))

            SET_FEATURES(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                         MultisampledRenderToSingleSampledFeaturesEXT,
                         SET_IF(fMultisampledRenderToSingleSampledEXT,
                                multisampledRenderToSingleSampled))

            SET_FEATURES(HOST_IMAGE_COPY_FEATURES,
                         HostImageCopyFeatures,
                         SET_IF(fHostImageCopyEXT, hostImageCopy))

            SET_FEATURES(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
                         PipelineCreationCacheControlFeatures,
                         SET_IF(fPipelineCreationCacheControlEXT, pipelineCreationCacheControl))

            SET_FEATURES(FRAME_BOUNDARY_FEATURES_EXT,
                         FrameBoundaryFeaturesEXT,
                         SET_IF(fFrameBoundaryEXT, frameBoundary))

            default:
                break;
        }
        pNext = pNext->pNext;
    }
#undef SET_FEATURES
#undef SET_IF
}

static size_t count_ext(const std::vector<const char*> enabledExts, const char* to_find) {
    return std::count_if(enabledExts.begin(), enabledExts.end(), [to_find](const char* ext) {
        return strcmp(ext, to_find) == 0;
    });
}

static size_t count_struct(const void* basePNext, VkStructureType structureType) {
    size_t count = 0;

    const VkBaseInStructure* pNext = static_cast<const VkBaseInStructure*>(basePNext);
    while (pNext) {
        if (pNext->sType == structureType) {
            ++count;
        }

        pNext = pNext->pNext;
    }

    return count;
}

template <typename VulkanStruct, VkBool32 VulkanStruct::* Feature>
static bool get_feature(skiatest::Reporter* reporter,
                        const void* basePNext,
                        VkStructureType structureType) {
    bool foundBefore = false;
    bool enabled = false;

    const VkBaseInStructure* pNext = static_cast<const VkBaseInStructure*>(basePNext);
    while (pNext) {
        if (pNext->sType == structureType) {
            // Verify no duplicate instances of the struct.
            REPORTER_ASSERT(reporter, !foundBefore);
            foundBefore = true;

            const auto* feature = reinterpret_cast<const VulkanStruct*>(pNext);
            enabled = feature->*Feature;
        }

        pNext = pNext->pNext;
    }

    return enabled;
}

static bool check_exclusive(skiatest::Reporter* reporter,
                            const void* basePNext,
                            VkStructureType structureType1,
                            VkStructureType structureType2) {
    bool foundStructureType1 = false;
    bool foundStructureType2 = false;

    const VkBaseInStructure* pNext = static_cast<const VkBaseInStructure*>(basePNext);
    while (pNext) {
        if (pNext->sType == structureType1) {
            // Verify no duplicate instances of the struct.
            REPORTER_ASSERT(reporter, !foundStructureType1);
            foundStructureType1 = true;
        } else if (pNext->sType == structureType2) {
            // Verify no duplicate instances of the struct.
            REPORTER_ASSERT(reporter, !foundStructureType2);
            foundStructureType2 = true;
        }

        pNext = pNext->pNext;
    }

    return !foundStructureType1 || !foundStructureType2;
}

#define CHECK_EXT_ENABLED(EXTNAME) \
    REPORTER_ASSERT(reporter, count_ext(enabledExts, EXTNAME##_EXTENSION_NAME) == 1)
#define CHECK_EXT_DISABLED(EXTNAME) \
    REPORTER_ASSERT(reporter, count_ext(enabledExts, EXTNAME##_EXTENSION_NAME) == 0)

#define CHECK_CHAINED(STRUCT_NAME) \
    REPORTER_ASSERT(               \
            reporter,              \
            count_struct(features.pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME) == 1)
#define CHECK_NOT_CHAINED(STRUCT_NAME) \
    REPORTER_ASSERT(                   \
            reporter,                  \
            count_struct(features.pNext, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME) == 0)

#define CHECK_EXCLUSIVE(STRUCT_NAME1, STRUCT_NAME2)                                   \
    REPORTER_ASSERT(reporter,                                                         \
                    check_exclusive(reporter,                                         \
                                    features.pNext,                                   \
                                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME1, \
                                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME2))

#define CHECK_FEATURE_ENABLED(STRUCT_NAME, StructName, member)                                    \
    do {                                                                                          \
        const bool member##Enabled =                                                              \
                get_feature<VkPhysicalDevice##StructName, &VkPhysicalDevice##StructName::member>( \
                        reporter,                                                                 \
                        features.pNext,                                                           \
                        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME);                         \
        REPORTER_ASSERT(reporter, member##Enabled);                                               \
    } while (0)

#define CHECK_FEATURE_DISABLED(STRUCT_NAME, StructName, member)                                   \
    do {                                                                                          \
        const bool member##Enabled =                                                              \
                get_feature<VkPhysicalDevice##StructName, &VkPhysicalDevice##StructName::member>( \
                        reporter,                                                                 \
                        features.pNext,                                                           \
                        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME);                         \
        REPORTER_ASSERT(reporter, !member##Enabled);                                              \
    } while (0)

// A test where the application adds no features and extensions and lets Skia choose them. Uses
// Vulkan 1.1 as the API version.
DEF_TEST(VkPreferredFeaturesTest_BasicVulkan11, reporter) {
    VulkanExts config;
    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_1);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    // Ask Skia to enable available features
    std::vector<const char*> enabledExts;
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_NOT_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_ENABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_ENABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_ENABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_ENABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_ENABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    //CHECK_EXT_ENABLED(VK_KHR_SYNCHRONIZATION_2);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_ENABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_ENABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_ENABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_ENABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_ENABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_ENABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_ENABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_ENABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    CHECK_EXT_DISABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_ENABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                          BlendOperationAdvancedFeaturesEXT,
                          advancedBlendCoherentOperations);

    CHECK_FEATURE_ENABLED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                          ExtendedDynamicStateFeaturesEXT,
                          extendedDynamicState);

    CHECK_FEATURE_ENABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                          ExtendedDynamicState2FeaturesEXT,
                          extendedDynamicState2);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2LogicOp);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2PatchControlPoints);

    CHECK_FEATURE_ENABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                          VertexInputDynamicStateFeaturesEXT,
                          vertexInputDynamicState);

    CHECK_FEATURE_ENABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                          GraphicsPipelineLibraryFeaturesEXT,
                          graphicsPipelineLibrary);

    CHECK_FEATURE_ENABLED(SAMPLER_YCBCR_CONVERSION_FEATURES,
                          SamplerYcbcrConversionFeatures,
                          samplerYcbcrConversion);

    CHECK_FEATURE_ENABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                          RGBA10X6FormatsFeaturesEXT,
                          formatRgba10x6WithoutYCbCrSampler);

    //CHECK_FEATURE_ENABLED(SYNCHRONIZATION_2_FEATURES, Synchronization2Features, synchronization2);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
    //                      DynamicRenderingLocalReadFeatures,
    //                      dynamicRenderingLocalRead);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    CHECK_FEATURE_ENABLED(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    CHECK_FEATURE_ENABLED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
                          PipelineCreationCacheControlFeatures,
                          pipelineCreationCacheControl);

    CHECK_FEATURE_ENABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

// A test where the application adds no features and extensions and lets Skia choose them. Uses
// Vulkan 1.2 as the API version.
DEF_TEST(VkPreferredFeaturesTest_BasicVulkan12, reporter) {
    VulkanExts config;
    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_2);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    // Ask Skia to enable available features
    std::vector<const char*> enabledExts;
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_ENABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_ENABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_ENABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_ENABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    //CHECK_EXT_ENABLED(VK_KHR_SYNCHRONIZATION_2);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_ENABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_ENABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_DISABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_ENABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_ENABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_ENABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    CHECK_EXT_DISABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, 16BIT_STORAGE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, MULTIVIEW_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, VARIABLE_POINTERS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, PROTECTED_MEMORY_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, SHADER_DRAW_PARAMETERS_FEATURES);

    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, 8BIT_STORAGE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_ATOMIC_INT64_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_FLOAT16_INT8_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, DESCRIPTOR_INDEXING_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SCALAR_BLOCK_LAYOUT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, IMAGELESS_FRAMEBUFFER_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, HOST_QUERY_RESET_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, TIMELINE_SEMAPHORE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, BUFFER_DEVICE_ADDRESS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, VULKAN_MEMORY_MODEL_FEATURES);

    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, samplerYcbcrConversion);
    CHECK_FEATURE_DISABLED(VULKAN_1_1_FEATURES, Vulkan11Features, shaderDrawParameters);

    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerMirrorClampToEdge);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, drawIndirectCount);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorIndexing);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerFilterMinmax);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputViewportIndex);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputLayer);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_ENABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                          BlendOperationAdvancedFeaturesEXT,
                          advancedBlendCoherentOperations);

    CHECK_FEATURE_ENABLED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                          ExtendedDynamicStateFeaturesEXT,
                          extendedDynamicState);

    CHECK_FEATURE_ENABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                          ExtendedDynamicState2FeaturesEXT,
                          extendedDynamicState2);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2LogicOp);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2PatchControlPoints);

    CHECK_FEATURE_ENABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                          VertexInputDynamicStateFeaturesEXT,
                          vertexInputDynamicState);

    CHECK_FEATURE_ENABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                          GraphicsPipelineLibraryFeaturesEXT,
                          graphicsPipelineLibrary);

    CHECK_FEATURE_DISABLED(SAMPLER_YCBCR_CONVERSION_FEATURES,
                           SamplerYcbcrConversionFeatures,
                           samplerYcbcrConversion);

    CHECK_FEATURE_ENABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                          RGBA10X6FormatsFeaturesEXT,
                          formatRgba10x6WithoutYCbCrSampler);

    //CHECK_FEATURE_ENABLED(SYNCHRONIZATION_2_FEATURES, Synchronization2Features, synchronization2);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
    //                      DynamicRenderingLocalReadFeatures,
    //                      dynamicRenderingLocalRead);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    CHECK_FEATURE_ENABLED(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    CHECK_FEATURE_ENABLED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
                          PipelineCreationCacheControlFeatures,
                          pipelineCreationCacheControl);

    CHECK_FEATURE_ENABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

// A test where the application adds no features and extensions and lets Skia choose them. Uses
// Vulkan 1.3 as the API version.
DEF_TEST(VkPreferredFeaturesTest_BasicVulkan13, reporter) {
    VulkanExts config;
    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_3);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    // Ask Skia to enable available features
    std::vector<const char*> enabledExts;
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_ENABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_ENABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    CHECK_EXT_DISABLED(VK_KHR_SYNCHRONIZATION_2);
    CHECK_EXT_DISABLED(VK_KHR_DYNAMIC_RENDERING);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_ENABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_DISABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_DISABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_ENABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_DISABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    CHECK_EXT_DISABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, 16BIT_STORAGE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, MULTIVIEW_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, VARIABLE_POINTERS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, PROTECTED_MEMORY_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, SHADER_DRAW_PARAMETERS_FEATURES);

    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, 8BIT_STORAGE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_ATOMIC_INT64_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_FLOAT16_INT8_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, DESCRIPTOR_INDEXING_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SCALAR_BLOCK_LAYOUT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, IMAGELESS_FRAMEBUFFER_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, HOST_QUERY_RESET_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, TIMELINE_SEMAPHORE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, BUFFER_DEVICE_ADDRESS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, VULKAN_MEMORY_MODEL_FEATURES);

    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, IMAGE_ROBUSTNESS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, INLINE_UNIFORM_BLOCK_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, MAINTENANCE_4_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, PRIVATE_DATA_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SHADER_INTEGER_DOT_PRODUCT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SHADER_TERMINATE_INVOCATION_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SUBGROUP_SIZE_CONTROL_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, TEXTURE_COMPRESSION_ASTC_HDR_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES);

    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, samplerYcbcrConversion);
    CHECK_FEATURE_DISABLED(VULKAN_1_1_FEATURES, Vulkan11Features, shaderDrawParameters);

    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerMirrorClampToEdge);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, drawIndirectCount);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorIndexing);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerFilterMinmax);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputViewportIndex);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputLayer);

    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, pipelineCreationCacheControl);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, synchronization2);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, dynamicRendering);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_ENABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                          BlendOperationAdvancedFeaturesEXT,
                          advancedBlendCoherentOperations);

    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                           ExtendedDynamicStateFeaturesEXT,
                           extendedDynamicState);

    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2LogicOp);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2PatchControlPoints);

    CHECK_FEATURE_ENABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                          VertexInputDynamicStateFeaturesEXT,
                          vertexInputDynamicState);

    CHECK_FEATURE_ENABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                          GraphicsPipelineLibraryFeaturesEXT,
                          graphicsPipelineLibrary);

    CHECK_FEATURE_DISABLED(SAMPLER_YCBCR_CONVERSION_FEATURES,
                           SamplerYcbcrConversionFeatures,
                           samplerYcbcrConversion);

    CHECK_FEATURE_ENABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                          RGBA10X6FormatsFeaturesEXT,
                          formatRgba10x6WithoutYCbCrSampler);

    CHECK_FEATURE_DISABLED(SYNCHRONIZATION_2_FEATURES, Synchronization2Features, synchronization2);

    CHECK_FEATURE_DISABLED(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
    //                      DynamicRenderingLocalReadFeatures,
    //                      dynamicRenderingLocalRead);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    CHECK_FEATURE_ENABLED(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    CHECK_FEATURE_DISABLED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
                           PipelineCreationCacheControlFeatures,
                           pipelineCreationCacheControl);

    CHECK_FEATURE_ENABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

// A test where the application adds no features and extensions and lets Skia choose them. Uses
// Vulkan 1.4 as the API version.
DEF_TEST(VkPreferredFeaturesTest_BasicVulkan14, reporter) {
    VulkanExts config;
    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_4);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    // Ask Skia to enable available features
    std::vector<const char*> enabledExts;
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_ENABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_ENABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    CHECK_EXT_DISABLED(VK_KHR_SYNCHRONIZATION_2);
    CHECK_EXT_DISABLED(VK_KHR_DYNAMIC_RENDERING);
    CHECK_EXT_DISABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_DISABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_DISABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_DISABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_DISABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_ENABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_DISABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    CHECK_EXT_DISABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, 16BIT_STORAGE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, MULTIVIEW_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, VARIABLE_POINTERS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, PROTECTED_MEMORY_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_1_FEATURES, SHADER_DRAW_PARAMETERS_FEATURES);

    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, 8BIT_STORAGE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_ATOMIC_INT64_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_FLOAT16_INT8_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, DESCRIPTOR_INDEXING_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SCALAR_BLOCK_LAYOUT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, IMAGELESS_FRAMEBUFFER_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, HOST_QUERY_RESET_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, TIMELINE_SEMAPHORE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, BUFFER_DEVICE_ADDRESS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_2_FEATURES, VULKAN_MEMORY_MODEL_FEATURES);

    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, IMAGE_ROBUSTNESS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, INLINE_UNIFORM_BLOCK_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, MAINTENANCE_4_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, PRIVATE_DATA_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SHADER_INTEGER_DOT_PRODUCT_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SHADER_TERMINATE_INVOCATION_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, SUBGROUP_SIZE_CONTROL_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, TEXTURE_COMPRESSION_ASTC_HDR_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_3_FEATURES, ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES);

    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, GLOBAL_PRIORITY_QUERY_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, SHADER_SUBGROUP_ROTATE_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, SHADER_FLOAT_CONTROLS_2_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, SHADER_EXPECT_ASSUME_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, LINE_RASTERIZATION_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, VERTEX_ATTRIBUTE_DIVISOR_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, INDEX_TYPE_UINT8_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, MAINTENANCE_5_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, MAINTENANCE_6_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, PIPELINE_PROTECTED_ACCESS_FEATURES);
    CHECK_EXCLUSIVE(VULKAN_1_4_FEATURES, PIPELINE_ROBUSTNESS_FEATURES);

    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, samplerYcbcrConversion);
    CHECK_FEATURE_DISABLED(VULKAN_1_1_FEATURES, Vulkan11Features, shaderDrawParameters);

    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerMirrorClampToEdge);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, drawIndirectCount);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorIndexing);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerFilterMinmax);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputViewportIndex);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputLayer);

    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, pipelineCreationCacheControl);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, synchronization2);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, dynamicRendering);

    CHECK_FEATURE_ENABLED(VULKAN_1_4_FEATURES, Vulkan14Features, dynamicRenderingLocalRead);
    CHECK_FEATURE_ENABLED(VULKAN_1_4_FEATURES, Vulkan14Features, hostImageCopy);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_ENABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                          BlendOperationAdvancedFeaturesEXT,
                          advancedBlendCoherentOperations);

    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                           ExtendedDynamicStateFeaturesEXT,
                           extendedDynamicState);

    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2LogicOp);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2PatchControlPoints);

    CHECK_FEATURE_ENABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                          VertexInputDynamicStateFeaturesEXT,
                          vertexInputDynamicState);

    CHECK_FEATURE_ENABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                          GraphicsPipelineLibraryFeaturesEXT,
                          graphicsPipelineLibrary);

    CHECK_FEATURE_DISABLED(SAMPLER_YCBCR_CONVERSION_FEATURES,
                           SamplerYcbcrConversionFeatures,
                           samplerYcbcrConversion);

    CHECK_FEATURE_ENABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                          RGBA10X6FormatsFeaturesEXT,
                          formatRgba10x6WithoutYCbCrSampler);

    CHECK_FEATURE_DISABLED(SYNCHRONIZATION_2_FEATURES, Synchronization2Features, synchronization2);

    CHECK_FEATURE_DISABLED(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);

    CHECK_FEATURE_DISABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
                           DynamicRenderingLocalReadFeatures,
                           dynamicRenderingLocalRead);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    CHECK_FEATURE_DISABLED(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    CHECK_FEATURE_DISABLED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
                           PipelineCreationCacheControlFeatures,
                           pipelineCreationCacheControl);

    CHECK_FEATURE_ENABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

#define CHAIN(STRUCT_NAME, StructName, Feature)                              \
    VkPhysicalDevice##StructName app##StructName = {};                       \
    app##StructName.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_##STRUCT_NAME; \
    app##StructName.pNext = features.pNext;                                  \
    app##StructName.Feature = VK_TRUE;                                       \
    features.pNext = &app##StructName

#define ADDEXT(EXTNAME) enabledExts.push_back(EXTNAME##_EXTENSION_NAME)

// A test where the application adds some features and extensions and lets Skia add to them. At the
// same time, support for some extensions is disabled. Uses Vulkan 1.1 as the API version.
DEF_TEST(VkPreferredFeaturesTest_CustomVulkan11, reporter) {
    VulkanExts config;
    config.fSamplerFilterMinmaxEXT = false;
    config.fDynamicRenderingKHR = false;
    config.fDynamicRenderingLocalReadKHR = false;
    config.fRasterizationOrderAttachmentAccessARM = false;
    config.fSamplerYcbcrConversionKHR = false;
    config.fLoadStoreOpNoneKHR = false;
    config.fPushDescriptorKHR = false;
    config.fDepthStencilResolveKHR = false;
    config.fMultisampledRenderToSingleSampledEXT = false;
    config.fGraphicsPipelineLibraryEXT = false;
    config.fExtendedDynamicState2EXT = false;
    config.fCreateRenderpass2KHR = false;
    config.fFrameBoundaryEXT = false;

    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_1);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    CHAIN(SUBGROUP_SIZE_CONTROL_FEATURES, SubgroupSizeControlFeatures, subgroupSizeControl);
    CHAIN(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHAIN(MAINTENANCE_5_FEATURES, Maintenance5Features, maintenance5);
    CHAIN(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    std::vector<const char*> enabledExts;
    ADDEXT(VK_EXT_SUBGROUP_SIZE_CONTROL);
    ADDEXT(VK_KHR_MAINTENANCE_4);
    ADDEXT(VK_KHR_PUSH_DESCRIPTOR);

    // Ask Skia to enable available features
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_NOT_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_ENABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_ENABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    //CHECK_EXT_ENABLED(VK_KHR_SYNCHRONIZATION_2);
    CHECK_EXT_DISABLED(VK_KHR_DYNAMIC_RENDERING);
    CHECK_EXT_DISABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_DISABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_ENABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_ENABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_ENABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_ENABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_DISABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_DISABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_ENABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_ENABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    // Extensions enabled by the app must remain enabled
    CHECK_EXT_ENABLED(VK_EXT_SUBGROUP_SIZE_CONTROL);
    CHECK_EXT_ENABLED(VK_KHR_MAINTENANCE_4);
    CHECK_EXT_ENABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_DISABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_ENABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                          BlendOperationAdvancedFeaturesEXT,
                          advancedBlendCoherentOperations);

    CHECK_FEATURE_ENABLED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                          ExtendedDynamicStateFeaturesEXT,
                          extendedDynamicState);

    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2LogicOp);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2PatchControlPoints);

    CHECK_FEATURE_ENABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                          VertexInputDynamicStateFeaturesEXT,
                          vertexInputDynamicState);

    CHECK_FEATURE_DISABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                           GraphicsPipelineLibraryFeaturesEXT,
                           graphicsPipelineLibrary);

    CHECK_FEATURE_DISABLED(SAMPLER_YCBCR_CONVERSION_FEATURES,
                           SamplerYcbcrConversionFeatures,
                           samplerYcbcrConversion);

    CHECK_FEATURE_ENABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                          RGBA10X6FormatsFeaturesEXT,
                          formatRgba10x6WithoutYCbCrSampler);

    //CHECK_FEATURE_ENABLED(SYNCHRONIZATION_2_FEATURES, Synchronization2Features, synchronization2);

    CHECK_FEATURE_DISABLED(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);

    CHECK_FEATURE_DISABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
                           DynamicRenderingLocalReadFeatures,
                           dynamicRenderingLocalRead);

    CHECK_FEATURE_DISABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                           MultisampledRenderToSingleSampledFeaturesEXT,
                           multisampledRenderToSingleSampled);

    CHECK_FEATURE_ENABLED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
                          PipelineCreationCacheControlFeatures,
                          pipelineCreationCacheControl);

    // Features enabled by the app must remain enabled
    CHECK_FEATURE_ENABLED(
            SUBGROUP_SIZE_CONTROL_FEATURES, SubgroupSizeControlFeatures, subgroupSizeControl);
    CHECK_FEATURE_ENABLED(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHECK_FEATURE_ENABLED(MAINTENANCE_5_FEATURES, Maintenance5Features, maintenance5);
    CHECK_FEATURE_ENABLED(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    CHECK_FEATURE_DISABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

// A test where the application adds some features and extensions and lets Skia add to them. At the
// same time, support for some extensions is disabled. Uses Vulkan 1.2 as the API version.
DEF_TEST(VkPreferredFeaturesTest_CustomVulkan12, reporter) {
    VulkanExts config;
    config.fSamplerFilterMinmaxEXT = false;
    config.fShaderViewportIndexLayerEXT = false;
    config.fRGBA10x6FormatsEXT = false;
    config.fHostImageCopyEXT = false;
    config.fVertexInputDynamicStateEXT = false;
    config.fExtendedDynamicStateEXT = false;
    config.fExtendedDynamicState2EXT = false;
    config.fFrameBoundaryEXT = false;

    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_2);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    CHAIN(SUBGROUP_SIZE_CONTROL_FEATURES, SubgroupSizeControlFeatures, subgroupSizeControl);
    CHAIN(DESCRIPTOR_INDEXING_FEATURES,
          DescriptorIndexingFeatures,
          shaderUniformTexelBufferArrayNonUniformIndexing);
    appDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
    CHAIN(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHAIN(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
          PipelineCreationCacheControlFeatures,
          pipelineCreationCacheControl);
    CHAIN(SHADER_ATOMIC_INT64_FEATURES, ShaderAtomicInt64Features, shaderSharedInt64Atomics);

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    std::vector<const char*> enabledExts;
    ADDEXT(VK_EXT_SHADER_OBJECT);
    ADDEXT(VK_EXT_RGBA10X6_FORMATS);
    ADDEXT(VK_KHR_PUSH_DESCRIPTOR);
    ADDEXT(VK_KHR_SHADER_DRAW_PARAMETERS);
    ADDEXT(VK_EXT_DESCRIPTOR_INDEXING);
    ADDEXT(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);

    // Ask Skia to enable available features
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_NOT_CHAINED(SAMPLER_YCBCR_CONVERSION_FEATURES);
    CHECK_NOT_CHAINED(DESCRIPTOR_INDEXING_FEATURES);
    CHECK_NOT_CHAINED(SHADER_ATOMIC_INT64_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_DISABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_ENABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    //CHECK_EXT_ENABLED(VK_KHR_SYNCHRONIZATION_2);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_DISABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_ENABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_DISABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_ENABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_ENABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_ENABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    // Extensions enabled by the app must remain enabled
    CHECK_EXT_ENABLED(VK_EXT_SHADER_OBJECT);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    CHECK_EXT_ENABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_ENABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_ENABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, samplerYcbcrConversion);
    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, shaderDrawParameters);

    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerMirrorClampToEdge);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, drawIndirectCount);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderInputAttachmentArrayDynamicIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderUniformTexelBufferArrayDynamicIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageTexelBufferArrayDynamicIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderUniformBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderSampledImageArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageImageArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderInputAttachmentArrayNonUniformIndexing);
    CHECK_FEATURE_ENABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderUniformTexelBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageTexelBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingUniformBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingSampledImageUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingStorageImageUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingStorageBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES,
                           Vulkan12Features,
                           descriptorBindingUniformTexelBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES,
                           Vulkan12Features,
                           descriptorBindingStorageTexelBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingUpdateUnusedWhilePending);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingPartiallyBound);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingVariableDescriptorCount);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, runtimeDescriptorArray);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderBufferInt64Atomics);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderSharedInt64Atomics);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerFilterMinmax);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputViewportIndex);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputLayer);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_ENABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                          BlendOperationAdvancedFeaturesEXT,
                          advancedBlendCoherentOperations);

    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
                           ExtendedDynamicStateFeaturesEXT,
                           extendedDynamicState);

    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2LogicOp);
    CHECK_FEATURE_DISABLED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
                           ExtendedDynamicState2FeaturesEXT,
                           extendedDynamicState2PatchControlPoints);

    CHECK_FEATURE_DISABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                           VertexInputDynamicStateFeaturesEXT,
                           vertexInputDynamicState);

    CHECK_FEATURE_ENABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                          GraphicsPipelineLibraryFeaturesEXT,
                          graphicsPipelineLibrary);

    CHECK_FEATURE_DISABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                           RGBA10X6FormatsFeaturesEXT,
                           formatRgba10x6WithoutYCbCrSampler);

    //CHECK_FEATURE_ENABLED(SYNCHRONIZATION_2_FEATURES, Synchronization2Features, synchronization2);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
    //                      DynamicRenderingLocalReadFeatures,
    //                      dynamicRenderingLocalRead);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    CHECK_FEATURE_DISABLED(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    // Features enabled by the app must remain enabled
    CHECK_FEATURE_ENABLED(
            SUBGROUP_SIZE_CONTROL_FEATURES, SubgroupSizeControlFeatures, subgroupSizeControl);
    CHECK_FEATURE_ENABLED(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHECK_FEATURE_ENABLED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
                          PipelineCreationCacheControlFeatures,
                          pipelineCreationCacheControl);

    CHECK_FEATURE_DISABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

// A test where the application adds some features and extensions and lets Skia add to them. At the
// same time, support for some extensions is disabled. Uses Vulkan 1.3 as the API version.
DEF_TEST(VkPreferredFeaturesTest_CustomVulkan13, reporter) {
    VulkanExts config;
    config.fDrawIndirectCountKHR = false;
    config.fSamplerYcbcrConversionKHR = false;
    config.fGraphicsPipelineLibraryEXT = false;
    config.fRGBA10x6FormatsEXT = false;
    config.fHostImageCopyEXT = false;
    config.fVertexInputDynamicStateEXT = false;

    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_3);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    CHAIN(VULKAN_1_1_FEATURES, Vulkan11Features, multiview);
    CHAIN(VULKAN_1_2_FEATURES, Vulkan12Features, vulkanMemoryModel);
    appVulkan12Features.bufferDeviceAddress = VK_TRUE;
    appVulkan12Features.shaderFloat16 = VK_TRUE;
    appVulkan12Features.scalarBlockLayout = VK_TRUE;
    CHAIN(VULKAN_1_3_FEATURES, Vulkan13Features, shaderZeroInitializeWorkgroupMemory);
    CHAIN(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);
    CHAIN(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
          DynamicRenderingLocalReadFeatures,
          dynamicRenderingLocalRead);
    CHAIN(SYNCHRONIZATION_2_FEATURES, Synchronization2Features, synchronization2);
    CHAIN(SHADER_INTEGER_DOT_PRODUCT_FEATURES,
          ShaderIntegerDotProductFeatures,
          shaderIntegerDotProduct);
    CHAIN(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHAIN(PIPELINE_CREATION_CACHE_CONTROL_FEATURES,
          PipelineCreationCacheControlFeatures,
          pipelineCreationCacheControl);

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    std::vector<const char*> enabledExts;
    ADDEXT(VK_EXT_SHADER_OBJECT);
    ADDEXT(VK_KHR_PIPELINE_LIBRARY);
    ADDEXT(VK_KHR_PUSH_DESCRIPTOR);
    ADDEXT(VK_KHR_DRIVER_PROPERTIES);
    ADDEXT(VK_KHR_SHADER_DRAW_PARAMETERS);
    ADDEXT(VK_EXT_DESCRIPTOR_INDEXING);
    ADDEXT(VK_KHR_DYNAMIC_RENDERING);

    // Ask Skia to enable available features
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_NOT_CHAINED(SAMPLER_YCBCR_CONVERSION_FEATURES);
    CHECK_NOT_CHAINED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT);
    CHECK_NOT_CHAINED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT);
    CHECK_NOT_CHAINED(DYNAMIC_RENDERING_FEATURES);
    CHECK_NOT_CHAINED(SYNCHRONIZATION_2_FEATURES);
    CHECK_NOT_CHAINED(SHADER_INTEGER_DOT_PRODUCT_FEATURES);
    CHECK_NOT_CHAINED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_DISABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_DISABLED(VK_EXT_RGBA10X6_FORMATS);
    CHECK_EXT_DISABLED(VK_KHR_SYNCHRONIZATION_2);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_DISABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_DISABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_ENABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_DISABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    // Extensions enabled by the app must remain enabled
    CHECK_EXT_ENABLED(VK_EXT_SHADER_OBJECT);
    CHECK_EXT_ENABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_ENABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_ENABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING);
    CHECK_EXT_ENABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, multiview);
    CHECK_FEATURE_DISABLED(VULKAN_1_1_FEATURES, Vulkan11Features, samplerYcbcrConversion);
    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, shaderDrawParameters);

    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerMirrorClampToEdge);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, drawIndirectCount);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderInputAttachmentArrayDynamicIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderUniformTexelBufferArrayDynamicIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageTexelBufferArrayDynamicIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderUniformBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderSampledImageArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageImageArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderInputAttachmentArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderUniformTexelBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, shaderStorageTexelBufferArrayNonUniformIndexing);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingUniformBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingSampledImageUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingStorageImageUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingStorageBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES,
                           Vulkan12Features,
                           descriptorBindingUniformTexelBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES,
                           Vulkan12Features,
                           descriptorBindingStorageTexelBufferUpdateAfterBind);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingUpdateUnusedWhilePending);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingPartiallyBound);
    CHECK_FEATURE_DISABLED(
            VULKAN_1_2_FEATURES, Vulkan12Features, descriptorBindingVariableDescriptorCount);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, runtimeDescriptorArray);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderBufferInt64Atomics);
    CHECK_FEATURE_DISABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderSharedInt64Atomics);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerFilterMinmax);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputViewportIndex);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputLayer);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, vulkanMemoryModel);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, bufferDeviceAddress);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderFloat16);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, scalarBlockLayout);

    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, dynamicRendering);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, synchronization2);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, pipelineCreationCacheControl);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, shaderIntegerDotProduct);
    CHECK_FEATURE_ENABLED(
            VULKAN_1_3_FEATURES, Vulkan13Features, shaderZeroInitializeWorkgroupMemory);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_ENABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                          BlendOperationAdvancedFeaturesEXT,
                          advancedBlendCoherentOperations);

    CHECK_FEATURE_DISABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                           VertexInputDynamicStateFeaturesEXT,
                           vertexInputDynamicState);

    CHECK_FEATURE_DISABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                           GraphicsPipelineLibraryFeaturesEXT,
                           graphicsPipelineLibrary);

    CHECK_FEATURE_DISABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                           RGBA10X6FormatsFeaturesEXT,
                           formatRgba10x6WithoutYCbCrSampler);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
    //                      DynamicRenderingLocalReadFeatures,
    //                      dynamicRenderingLocalRead);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    CHECK_FEATURE_DISABLED(HOST_IMAGE_COPY_FEATURES, HostImageCopyFeatures, hostImageCopy);

    // Features enabled by the app must remain enabled
    CHECK_FEATURE_ENABLED(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);

    CHECK_FEATURE_ENABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

// A test where the application adds some features and extensions and lets Skia add to them. At the
// same time, support for some extensions is disabled. Uses Vulkan 1.4 as the API version.
DEF_TEST(VkPreferredFeaturesTest_CustomVulkan14, reporter) {
    VulkanExts config;
    config.fConservativeRasterizationEXT = false;
    config.fRasterizationOrderAttachmentAccessEXT = false;
    config.fGraphicsPipelineLibraryEXT = false;
    config.fPipelineLibraryKHR = false;
    config.fBlendOperationAdvancedEXT = false;

    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_4);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    CHAIN(VULKAN_1_2_FEATURES, Vulkan12Features, subgroupBroadcastDynamicId);
    appVulkan12Features.uniformAndStorageBuffer8BitAccess = VK_TRUE;
    CHAIN(VULKAN_1_4_FEATURES, Vulkan14Features, stippledRectangularLines);
    CHAIN(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
          DynamicRenderingLocalReadFeatures,
          dynamicRenderingLocalRead);
    CHAIN(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHAIN(MAINTENANCE_5_FEATURES, Maintenance5Features, maintenance5);
    CHAIN(MAINTENANCE_6_FEATURES, Maintenance6Features, maintenance6);

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    std::vector<const char*> enabledExts;
    ADDEXT(VK_KHR_MAINTENANCE_5);
    ADDEXT(VK_KHR_PUSH_DESCRIPTOR);
    ADDEXT(VK_EXT_SHADER_OBJECT);
    ADDEXT(VK_EXT_DESCRIPTOR_INDEXING);
    ADDEXT(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);

    // Ask Skia to enable available features
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_NOT_CHAINED(SAMPLER_YCBCR_CONVERSION_FEATURES);
    CHECK_NOT_CHAINED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT);
    CHECK_NOT_CHAINED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT);
    CHECK_NOT_CHAINED(DYNAMIC_RENDERING_FEATURES);
    CHECK_NOT_CHAINED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES);
    CHECK_NOT_CHAINED(SYNCHRONIZATION_2_FEATURES);
    CHECK_NOT_CHAINED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES);
    CHECK_NOT_CHAINED(HOST_IMAGE_COPY_FEATURES);

    CHECK_EXT_ENABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_DISABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_DISABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_ENABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    CHECK_EXT_DISABLED(VK_KHR_SYNCHRONIZATION_2);
    CHECK_EXT_DISABLED(VK_KHR_DYNAMIC_RENDERING);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    CHECK_EXT_DISABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_DISABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_DISABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_DISABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_DISABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_DISABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_DISABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    // Extensions enabled by the app must remain enabled
    CHECK_EXT_ENABLED(VK_KHR_MAINTENANCE_5);
    CHECK_EXT_ENABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_EXT_SHADER_OBJECT);
    CHECK_EXT_ENABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, samplerYcbcrConversion);
    CHECK_FEATURE_DISABLED(VULKAN_1_1_FEATURES, Vulkan11Features, shaderDrawParameters);

    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerMirrorClampToEdge);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, drawIndirectCount);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorIndexing);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerFilterMinmax);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputViewportIndex);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputLayer);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, subgroupBroadcastDynamicId);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, uniformAndStorageBuffer8BitAccess);

    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, dynamicRendering);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, synchronization2);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, pipelineCreationCacheControl);

    CHECK_FEATURE_ENABLED(VULKAN_1_4_FEATURES, Vulkan14Features, maintenance5);
    CHECK_FEATURE_ENABLED(VULKAN_1_4_FEATURES, Vulkan14Features, maintenance6);
    CHECK_FEATURE_ENABLED(VULKAN_1_4_FEATURES, Vulkan14Features, dynamicRenderingLocalRead);
    CHECK_FEATURE_ENABLED(VULKAN_1_4_FEATURES, Vulkan14Features, hostImageCopy);
    CHECK_FEATURE_ENABLED(VULKAN_1_4_FEATURES, Vulkan14Features, pushDescriptor);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_DISABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                           BlendOperationAdvancedFeaturesEXT,
                           advancedBlendCoherentOperations);

    CHECK_FEATURE_ENABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                          VertexInputDynamicStateFeaturesEXT,
                          vertexInputDynamicState);

    CHECK_FEATURE_DISABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                           GraphicsPipelineLibraryFeaturesEXT,
                           graphicsPipelineLibrary);

    CHECK_FEATURE_ENABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                          RGBA10X6FormatsFeaturesEXT,
                          formatRgba10x6WithoutYCbCrSampler);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    // Features enabled by the app must remain enabled
    CHECK_FEATURE_ENABLED(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);


    CHECK_FEATURE_ENABLED(FRAME_BOUNDARY_FEATURES_EXT,
                          FrameBoundaryFeaturesEXT,
                          frameBoundary);
}

// A similar test to VkPreferredFeaturesTest_CustomVulkan*, except the chain passed to
// addFeaturesToEnable does not include all the structs passed to addFeaturesToQuery.
// addFeaturesToEnable should automatically add back the structs that addFeaturesToQuery had decided
// to chain.
DEF_TEST(VkPreferredFeaturesTest_EmptyEnableChain, reporter) {
    VulkanExts config;
    config.fGraphicsPipelineLibraryEXT = false;
    config.fPipelineLibraryKHR = false;
    config.fBlendOperationAdvancedEXT = false;
    config.fFrameBoundaryEXT = false;

    const std::vector<VkExtensionProperties> exts = get_device_exts(config);

    skgpu::VulkanPreferredFeatures preferred;
    preferred.init(VK_API_VERSION_1_3);

    {
        std::vector<const char*> instanceExts;
        preferred.addToInstanceExtensions(nullptr, 0, instanceExts);
    }

    VkPhysicalDeviceFeatures2 features = {};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    CHAIN(VULKAN_1_2_FEATURES, Vulkan12Features, subgroupBroadcastDynamicId);
    CHAIN(DYNAMIC_RENDERING_FEATURES, DynamicRenderingFeatures, dynamicRendering);
    CHAIN(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHAIN(MAINTENANCE_5_FEATURES, Maintenance5Features, maintenance5);

    // Ask Skia to add its features to query
    preferred.addFeaturesToQuery(exts.data(), exts.size(), features);
    // Populate the requested features
    enable_device_features(config, features.pNext);

    std::vector<const char*> enabledExts;
    ADDEXT(VK_KHR_MAINTENANCE_5);
    ADDEXT(VK_KHR_PUSH_DESCRIPTOR);
    ADDEXT(VK_EXT_SHADER_OBJECT);

    // Start a new chain of features, only the ones the app has enabled.
    features.pNext = &appVulkan12Features;
    appVulkan12Features.pNext = &appDynamicRenderingFeatures;
    appDynamicRenderingFeatures.pNext = &appShaderObjectFeaturesEXT;
    appShaderObjectFeaturesEXT.pNext = &appMaintenance5Features;
    appMaintenance5Features.pNext = nullptr;

    // Ask Skia to enable available features
    preferred.addFeaturesToEnable(enabledExts, features);

    CHECK_CHAINED(VULKAN_1_1_FEATURES);
    CHECK_CHAINED(VULKAN_1_2_FEATURES);
    CHECK_CHAINED(VULKAN_1_3_FEATURES);
    CHECK_NOT_CHAINED(VULKAN_1_4_FEATURES);

    CHECK_NOT_CHAINED(SAMPLER_YCBCR_CONVERSION_FEATURES);
    CHECK_NOT_CHAINED(EXTENDED_DYNAMIC_STATE_FEATURES_EXT);
    CHECK_NOT_CHAINED(EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT);
    CHECK_NOT_CHAINED(DYNAMIC_RENDERING_FEATURES);
    CHECK_NOT_CHAINED(SYNCHRONIZATION_2_FEATURES);
    CHECK_NOT_CHAINED(PIPELINE_CREATION_CACHE_CONTROL_FEATURES);

    CHECK_EXT_DISABLED(VK_ARM_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_ENABLED(VK_EXT_RASTERIZATION_ORDER_ATTACHMENT_ACCESS);
    CHECK_EXT_DISABLED(VK_EXT_BLEND_OPERATION_ADVANCED);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_EXTENDED_DYNAMIC_STATE_2);
    CHECK_EXT_ENABLED(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE);
    CHECK_EXT_DISABLED(VK_EXT_GRAPHICS_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_YCBCR_CONVERSION);
    CHECK_EXT_ENABLED(VK_EXT_RGBA10X6_FORMATS);
    CHECK_EXT_DISABLED(VK_KHR_SYNCHRONIZATION_2);
    CHECK_EXT_DISABLED(VK_KHR_DYNAMIC_RENDERING);
    //CHECK_EXT_ENABLED(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ);
    CHECK_EXT_ENABLED(VK_EXT_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED);
    //CHECK_EXT_ENABLED(VK_EXT_HOST_IMAGE_COPY);
    CHECK_EXT_DISABLED(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL);
    CHECK_EXT_DISABLED(VK_KHR_DRIVER_PROPERTIES);
    CHECK_EXT_DISABLED(VK_KHR_CREATE_RENDERPASS_2);
    CHECK_EXT_DISABLED(VK_EXT_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_KHR_LOAD_STORE_OP_NONE);
    CHECK_EXT_ENABLED(VK_EXT_CONSERVATIVE_RASTERIZATION);
    CHECK_EXT_DISABLED(VK_KHR_PIPELINE_LIBRARY);
    CHECK_EXT_DISABLED(VK_KHR_COPY_COMMANDS_2);
    CHECK_EXT_DISABLED(VK_KHR_FORMAT_FEATURE_FLAGS_2);
    CHECK_EXT_DISABLED(VK_KHR_DEPTH_STENCIL_RESOLVE);
    CHECK_EXT_DISABLED(VK_KHR_SHADER_DRAW_PARAMETERS);
    CHECK_EXT_DISABLED(VK_KHR_DRAW_INDIRECT_COUNT);
    CHECK_EXT_DISABLED(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE);
    CHECK_EXT_DISABLED(VK_EXT_DESCRIPTOR_INDEXING);
    CHECK_EXT_DISABLED(VK_EXT_SAMPLER_FILTER_MINMAX);
    CHECK_EXT_DISABLED(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER);
    // Extensions enabled by the app must remain enabled
    CHECK_EXT_ENABLED(VK_KHR_MAINTENANCE_5);
    CHECK_EXT_ENABLED(VK_KHR_PUSH_DESCRIPTOR);
    CHECK_EXT_ENABLED(VK_EXT_SHADER_OBJECT);
    CHECK_EXT_DISABLED(VK_EXT_FRAME_BOUNDARY);

    CHECK_FEATURE_ENABLED(VULKAN_1_1_FEATURES, Vulkan11Features, samplerYcbcrConversion);
    CHECK_FEATURE_DISABLED(VULKAN_1_1_FEATURES, Vulkan11Features, shaderDrawParameters);

    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerMirrorClampToEdge);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, drawIndirectCount);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, descriptorIndexing);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, samplerFilterMinmax);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputViewportIndex);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, shaderOutputLayer);
    CHECK_FEATURE_ENABLED(VULKAN_1_2_FEATURES, Vulkan12Features, subgroupBroadcastDynamicId);

    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, dynamicRendering);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, synchronization2);
    CHECK_FEATURE_ENABLED(VULKAN_1_3_FEATURES, Vulkan13Features, pipelineCreationCacheControl);

    CHECK_FEATURE_ENABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                          RasterizationOrderAttachmentAccessFeaturesEXT,
                          rasterizationOrderColorAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderDepthAttachmentAccess);
    CHECK_FEATURE_DISABLED(RASTERIZATION_ORDER_ATTACHMENT_ACCESS_FEATURES_EXT,
                           RasterizationOrderAttachmentAccessFeaturesEXT,
                           rasterizationOrderStencilAttachmentAccess);

    CHECK_FEATURE_DISABLED(BLEND_OPERATION_ADVANCED_FEATURES_EXT,
                           BlendOperationAdvancedFeaturesEXT,
                           advancedBlendCoherentOperations);

    CHECK_FEATURE_ENABLED(VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
                          VertexInputDynamicStateFeaturesEXT,
                          vertexInputDynamicState);

    CHECK_FEATURE_DISABLED(GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT,
                           GraphicsPipelineLibraryFeaturesEXT,
                           graphicsPipelineLibrary);

    CHECK_FEATURE_ENABLED(RGBA10X6_FORMATS_FEATURES_EXT,
                          RGBA10X6FormatsFeaturesEXT,
                          formatRgba10x6WithoutYCbCrSampler);

    //CHECK_FEATURE_ENABLED(DYNAMIC_RENDERING_LOCAL_READ_FEATURES,
    //                      DynamicRenderingLocalReadFeatures,
    //                      dynamicRenderingLocalRead);

    CHECK_FEATURE_ENABLED(MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_FEATURES_EXT,
                          MultisampledRenderToSingleSampledFeaturesEXT,
                          multisampledRenderToSingleSampled);

    // Features enabled by the app must remain enabled
    CHECK_FEATURE_ENABLED(SHADER_OBJECT_FEATURES_EXT, ShaderObjectFeaturesEXT, shaderObject);
    CHECK_FEATURE_ENABLED(MAINTENANCE_5_FEATURES, Maintenance5Features, maintenance5);

    CHECK_FEATURE_DISABLED(FRAME_BOUNDARY_FEATURES_EXT,
                           FrameBoundaryFeaturesEXT,
                           frameBoundary);
}

#endif
