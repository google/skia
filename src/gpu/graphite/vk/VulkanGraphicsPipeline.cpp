/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
#include "src/gpu/graphite/vk/VulkanResourceProvider.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace skgpu::graphite {

static inline VkFormat attrib_type_to_vkformat(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return VK_FORMAT_R32_SFLOAT;
        case VertexAttribType::kFloat2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexAttribType::kFloat3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexAttribType::kFloat4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexAttribType::kHalf:
            return VK_FORMAT_R16_SFLOAT;
        case VertexAttribType::kHalf2:
            return VK_FORMAT_R16G16_SFLOAT;
        case VertexAttribType::kHalf4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case VertexAttribType::kInt2:
            return VK_FORMAT_R32G32_SINT;
        case VertexAttribType::kInt3:
            return VK_FORMAT_R32G32B32_SINT;
        case VertexAttribType::kInt4:
            return VK_FORMAT_R32G32B32A32_SINT;
        case VertexAttribType::kUInt2:
            return VK_FORMAT_R32G32_UINT;
        case VertexAttribType::kByte:
            return VK_FORMAT_R8_SINT;
        case VertexAttribType::kByte2:
            return VK_FORMAT_R8G8_SINT;
        case VertexAttribType::kByte4:
            return VK_FORMAT_R8G8B8A8_SINT;
        case VertexAttribType::kUByte:
            return VK_FORMAT_R8_UINT;
        case VertexAttribType::kUByte2:
            return VK_FORMAT_R8G8_UINT;
        case VertexAttribType::kUByte4:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VertexAttribType::kUByte_norm:
            return VK_FORMAT_R8_UNORM;
        case VertexAttribType::kUByte4_norm:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case VertexAttribType::kShort2:
            return VK_FORMAT_R16G16_SINT;
        case VertexAttribType::kShort4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VertexAttribType::kUShort2:
            return VK_FORMAT_R16G16_UINT;
        case VertexAttribType::kUShort2_norm:
            return VK_FORMAT_R16G16_UNORM;
        case VertexAttribType::kInt:
            return VK_FORMAT_R32_SINT;
        case VertexAttribType::kUInt:
            return VK_FORMAT_R32_UINT;
        case VertexAttribType::kUShort_norm:
            return VK_FORMAT_R16_UNORM;
        case VertexAttribType::kUShort4_norm:
            return VK_FORMAT_R16G16B16A16_UNORM;
    }
    SK_ABORT("Unknown vertex attrib type");
}

static void setup_vertex_input_state(
        VkVertexInputRate appendVertexRate,
        const SkSpan<const Attribute>& staticAttrs,
        const SkSpan<const Attribute>& appendAttrs,
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
        skia_private::STArray<2, VkVertexInputBindingDescription, true>* bindingDescs,
        skia_private::STArray<16, VkVertexInputAttributeDescription>* attributeDescs) {
    // Setup attribute & binding descriptions
    int attribIndex = 0;
    size_t staticAttributeOffset = 0;
    for (auto attrib : staticAttrs) {
        VkVertexInputAttributeDescription vkAttrib;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kStaticDataBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = staticAttributeOffset;
        staticAttributeOffset += attrib.sizeAlign4();
        attributeDescs->push_back(vkAttrib);
    }

    size_t appendAttributeOffset = 0;
    for (auto attrib : appendAttrs) {
        VkVertexInputAttributeDescription vkAttrib;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kAppendDataBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = appendAttributeOffset;
        appendAttributeOffset += attrib.sizeAlign4();
        attributeDescs->push_back(vkAttrib);
    }

    if (bindingDescs && !staticAttrs.empty()) {
        bindingDescs->push_back() = {
                VulkanGraphicsPipeline::kStaticDataBufferIndex,
                (uint32_t) staticAttributeOffset,
                VK_VERTEX_INPUT_RATE_VERTEX
        };
    }
    if (bindingDescs && !appendAttrs.empty()) {
        bindingDescs->push_back() = {
                VulkanGraphicsPipeline::kAppendDataBufferIndex,
                (uint32_t) appendAttributeOffset,
                appendVertexRate
        };
    }

    *vertexInputInfo = {};
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->vertexBindingDescriptionCount = bindingDescs ? bindingDescs->size() : 0;
    vertexInputInfo->pVertexBindingDescriptions =
            bindingDescs && !bindingDescs->empty() ? bindingDescs->begin() : VK_NULL_HANDLE;
    vertexInputInfo->vertexAttributeDescriptionCount = attributeDescs ? attributeDescs->size() : 0;
    vertexInputInfo->pVertexAttributeDescriptions =
            attributeDescs && !attributeDescs->empty() ? attributeDescs->begin() : VK_NULL_HANDLE;
}

static VkPrimitiveTopology primitive_type_to_vk_topology(PrimitiveType primitiveType) {
    switch (primitiveType) {
        case PrimitiveType::kTriangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveType::kTriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveType::kPoints:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    }
    SkUNREACHABLE;
}

static void setup_input_assembly_state(PrimitiveType primitiveType,
                                       VkPipelineInputAssemblyStateCreateInfo* inputAssemblyInfo) {
    *inputAssemblyInfo = {};
    inputAssemblyInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->primitiveRestartEnable = false;
    inputAssemblyInfo->topology = primitive_type_to_vk_topology(primitiveType);
}

static VkStencilOp stencil_op_to_vk_stencil_op(StencilOp op) {
    static const VkStencilOp gTable[] = {
        VK_STENCIL_OP_KEEP,                 // kKeep
        VK_STENCIL_OP_ZERO,                 // kZero
        VK_STENCIL_OP_REPLACE,              // kReplace
        VK_STENCIL_OP_INVERT,               // kInvert
        VK_STENCIL_OP_INCREMENT_AND_WRAP,   // kIncWrap
        VK_STENCIL_OP_DECREMENT_AND_WRAP,   // kDecWrap
        VK_STENCIL_OP_INCREMENT_AND_CLAMP,  // kIncClamp
        VK_STENCIL_OP_DECREMENT_AND_CLAMP,  // kDecClamp
    };
    static_assert(std::size(gTable) == kStencilOpCount);
    static_assert(0 == (int)StencilOp::kKeep);
    static_assert(1 == (int)StencilOp::kZero);
    static_assert(2 == (int)StencilOp::kReplace);
    static_assert(3 == (int)StencilOp::kInvert);
    static_assert(4 == (int)StencilOp::kIncWrap);
    static_assert(5 == (int)StencilOp::kDecWrap);
    static_assert(6 == (int)StencilOp::kIncClamp);
    static_assert(7 == (int)StencilOp::kDecClamp);
    SkASSERT(op < (StencilOp)kStencilOpCount);
    return gTable[(int)op];
}

static VkCompareOp compare_op_to_vk_compare_op(CompareOp op) {
    static const VkCompareOp gTable[] = {
        VK_COMPARE_OP_ALWAYS,              // kAlways
        VK_COMPARE_OP_NEVER,               // kNever
        VK_COMPARE_OP_GREATER,             // kGreater
        VK_COMPARE_OP_GREATER_OR_EQUAL,    // kGEqual
        VK_COMPARE_OP_LESS,                // kLess
        VK_COMPARE_OP_LESS_OR_EQUAL,       // kLEqual
        VK_COMPARE_OP_EQUAL,               // kEqual
        VK_COMPARE_OP_NOT_EQUAL,           // kNotEqual
    };
    static_assert(std::size(gTable) == kCompareOpCount);
    static_assert(0 == (int)CompareOp::kAlways);
    static_assert(1 == (int)CompareOp::kNever);
    static_assert(2 == (int)CompareOp::kGreater);
    static_assert(3 == (int)CompareOp::kGEqual);
    static_assert(4 == (int)CompareOp::kLess);
    static_assert(5 == (int)CompareOp::kLEqual);
    static_assert(6 == (int)CompareOp::kEqual);
    static_assert(7 == (int)CompareOp::kNotEqual);
    SkASSERT(op < (CompareOp)kCompareOpCount);

    return gTable[(int)op];
}

static void setup_stencil_op_state(VkStencilOpState* opState,
                                   const DepthStencilSettings::Face& face,
                                   uint32_t referenceValue) {
    opState->failOp = stencil_op_to_vk_stencil_op(face.fStencilFailOp);
    opState->passOp = stencil_op_to_vk_stencil_op(face.fDepthStencilPassOp);
    opState->depthFailOp = stencil_op_to_vk_stencil_op(face.fDepthFailOp);
    opState->compareOp = compare_op_to_vk_compare_op(face.fCompareOp);
    opState->compareMask = face.fReadMask; // TODO - check this.
    opState->writeMask = face.fWriteMask;
    opState->reference = referenceValue;
}

static void setup_depth_stencil_state(const DepthStencilSettings& stencilSettings,
                                      VkPipelineDepthStencilStateCreateInfo* stencilInfo) {
    SkASSERT(stencilSettings.fDepthTestEnabled ||
             stencilSettings.fDepthCompareOp == CompareOp::kAlways);

    *stencilInfo = {};
    stencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stencilInfo->depthTestEnable = stencilSettings.fDepthTestEnabled;
    stencilInfo->depthWriteEnable = stencilSettings.fDepthWriteEnabled;
    stencilInfo->depthCompareOp = compare_op_to_vk_compare_op(stencilSettings.fDepthCompareOp);
    stencilInfo->depthBoundsTestEnable = VK_FALSE; // Default value TODO - Confirm
    stencilInfo->stencilTestEnable = stencilSettings.fStencilTestEnabled;
    if (stencilSettings.fStencilTestEnabled) {
        setup_stencil_op_state(&stencilInfo->front,
                               stencilSettings.fFrontStencil,
                               stencilSettings.fStencilReferenceValue);
        setup_stencil_op_state(&stencilInfo->back,
                               stencilSettings.fBackStencil,
                               stencilSettings.fStencilReferenceValue);
    }
    stencilInfo->minDepthBounds = 0.0f;
    stencilInfo->maxDepthBounds = 1.0f;
}

static void setup_viewport_scissor_state(VkPipelineViewportStateCreateInfo* viewportInfo) {
    *viewportInfo = {};
    viewportInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

    // The viewport and scissor are set dyanmically with draw pass commands
    viewportInfo->viewportCount = 1;
    viewportInfo->scissorCount = 1;

    SkASSERT(viewportInfo->viewportCount == viewportInfo->scissorCount);
}

static void setup_multisample_state(int numSamples,
                                    VkPipelineMultisampleStateCreateInfo* multisampleInfo) {
    *multisampleInfo = {};
    multisampleInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    SkAssertResult(skgpu::SampleCountToVkSampleCount(numSamples,
                                                     &multisampleInfo->rasterizationSamples));
    multisampleInfo->sampleShadingEnable = VK_FALSE;
    multisampleInfo->minSampleShading = 0.0f;
    multisampleInfo->pSampleMask = nullptr;
    multisampleInfo->alphaToCoverageEnable = VK_FALSE;
    multisampleInfo->alphaToOneEnable = VK_FALSE;
}

static VkBlendFactor blend_coeff_to_vk_blend(skgpu::BlendCoeff coeff) {
    switch (coeff) {
        case skgpu::BlendCoeff::kZero:
            return VK_BLEND_FACTOR_ZERO;
        case skgpu::BlendCoeff::kOne:
            return VK_BLEND_FACTOR_ONE;
        case skgpu::BlendCoeff::kSC:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case skgpu::BlendCoeff::kISC:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case skgpu::BlendCoeff::kDC:
            return VK_BLEND_FACTOR_DST_COLOR;
        case skgpu::BlendCoeff::kIDC:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case skgpu::BlendCoeff::kSA:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case skgpu::BlendCoeff::kISA:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case skgpu::BlendCoeff::kDA:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case skgpu::BlendCoeff::kIDA:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case skgpu::BlendCoeff::kConstC:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case skgpu::BlendCoeff::kIConstC:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case skgpu::BlendCoeff::kS2C:
            return VK_BLEND_FACTOR_SRC1_COLOR;
        case skgpu::BlendCoeff::kIS2C:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case skgpu::BlendCoeff::kS2A:
            return VK_BLEND_FACTOR_SRC1_ALPHA;
        case skgpu::BlendCoeff::kIS2A:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
        case skgpu::BlendCoeff::kIllegal:
            return VK_BLEND_FACTOR_ZERO;
    }
    SkUNREACHABLE;
}

static VkBlendOp blend_equation_to_vk_blend_op(skgpu::BlendEquation equation) {
    static const VkBlendOp gTable[] = {
        // Basic blend ops
        VK_BLEND_OP_ADD,
        VK_BLEND_OP_SUBTRACT,
        VK_BLEND_OP_REVERSE_SUBTRACT,

        // Advanced blend ops
        VK_BLEND_OP_SCREEN_EXT,
        VK_BLEND_OP_OVERLAY_EXT,
        VK_BLEND_OP_DARKEN_EXT,
        VK_BLEND_OP_LIGHTEN_EXT,
        VK_BLEND_OP_COLORDODGE_EXT,
        VK_BLEND_OP_COLORBURN_EXT,
        VK_BLEND_OP_HARDLIGHT_EXT,
        VK_BLEND_OP_SOFTLIGHT_EXT,
        VK_BLEND_OP_DIFFERENCE_EXT,
        VK_BLEND_OP_EXCLUSION_EXT,
        VK_BLEND_OP_MULTIPLY_EXT,
        VK_BLEND_OP_HSL_HUE_EXT,
        VK_BLEND_OP_HSL_SATURATION_EXT,
        VK_BLEND_OP_HSL_COLOR_EXT,
        VK_BLEND_OP_HSL_LUMINOSITY_EXT,

        // Illegal.
        VK_BLEND_OP_ADD,
    };
    static_assert(0 == (int)skgpu::BlendEquation::kAdd);
    static_assert(1 == (int)skgpu::BlendEquation::kSubtract);
    static_assert(2 == (int)skgpu::BlendEquation::kReverseSubtract);
    static_assert(3 == (int)skgpu::BlendEquation::kScreen);
    static_assert(4 == (int)skgpu::BlendEquation::kOverlay);
    static_assert(5 == (int)skgpu::BlendEquation::kDarken);
    static_assert(6 == (int)skgpu::BlendEquation::kLighten);
    static_assert(7 == (int)skgpu::BlendEquation::kColorDodge);
    static_assert(8 == (int)skgpu::BlendEquation::kColorBurn);
    static_assert(9 == (int)skgpu::BlendEquation::kHardLight);
    static_assert(10 == (int)skgpu::BlendEquation::kSoftLight);
    static_assert(11 == (int)skgpu::BlendEquation::kDifference);
    static_assert(12 == (int)skgpu::BlendEquation::kExclusion);
    static_assert(13 == (int)skgpu::BlendEquation::kMultiply);
    static_assert(14 == (int)skgpu::BlendEquation::kHSLHue);
    static_assert(15 == (int)skgpu::BlendEquation::kHSLSaturation);
    static_assert(16 == (int)skgpu::BlendEquation::kHSLColor);
    static_assert(17 == (int)skgpu::BlendEquation::kHSLLuminosity);
    static_assert(std::size(gTable) == skgpu::kBlendEquationCnt);

    SkASSERT((unsigned)equation < skgpu::kBlendEquationCnt);
    return gTable[(int)equation];
}

static void setup_color_blend_state(const skgpu::BlendInfo& blendInfo,
                                    VkPipelineColorBlendStateCreateInfo* colorBlendInfo,
                                    VkPipelineColorBlendAttachmentState* attachmentState) {
    skgpu::BlendEquation equation = blendInfo.fEquation;
    skgpu::BlendCoeff srcCoeff = blendInfo.fSrcBlend;
    skgpu::BlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = skgpu::BlendShouldDisable(equation, srcCoeff, dstCoeff);

    *attachmentState = {};
    attachmentState->blendEnable = !blendOff;
    if (!blendOff) {
        attachmentState->srcColorBlendFactor = blend_coeff_to_vk_blend(srcCoeff);
        attachmentState->dstColorBlendFactor = blend_coeff_to_vk_blend(dstCoeff);
        attachmentState->colorBlendOp = blend_equation_to_vk_blend_op(equation);
        attachmentState->srcAlphaBlendFactor = blend_coeff_to_vk_blend(srcCoeff);
        attachmentState->dstAlphaBlendFactor = blend_coeff_to_vk_blend(dstCoeff);
        attachmentState->alphaBlendOp = blend_equation_to_vk_blend_op(equation);
    }

    if (!blendInfo.fWritesColor) {
        attachmentState->colorWriteMask = 0;
    } else {
        attachmentState->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    *colorBlendInfo = {};
    colorBlendInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo->logicOpEnable = VK_FALSE;
    colorBlendInfo->attachmentCount = 1;
    colorBlendInfo->pAttachments = attachmentState;
    // colorBlendInfo->blendConstants is set dynamically
}

static void setup_raster_state(bool isWireframe,
                               VkPipelineRasterizationStateCreateInfo* rasterInfo) {
    *rasterInfo = {};
    rasterInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo->depthClampEnable = VK_FALSE;
    rasterInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterInfo->polygonMode = isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterInfo->cullMode = VK_CULL_MODE_NONE;
    rasterInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterInfo->depthBiasEnable = VK_FALSE;
    rasterInfo->depthBiasConstantFactor = 0.0f;
    rasterInfo->depthBiasClamp = 0.0f;
    rasterInfo->depthBiasSlopeFactor = 0.0f;
    rasterInfo->lineWidth = 1.0f;
}

static void setup_shader_stage_info(VkShaderStageFlagBits stage,
                                    VkShaderModule shaderModule,
                                    VkPipelineShaderStageCreateInfo* shaderStageInfo) {
    *shaderStageInfo = {};
    shaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo->stage = stage;
    shaderStageInfo->module = shaderModule;
    shaderStageInfo->pName = "main";
}

static VkDescriptorSetLayout descriptor_data_to_layout(
        const VulkanSharedContext* sharedContext, const SkSpan<DescriptorData>& descriptorData) {
    // descriptorData can be empty to indicate that we should create a mock placeholder layout
    // with no descriptors.
    VkDescriptorSetLayout setLayout;
    DescriptorDataToVkDescSetLayout(sharedContext, descriptorData, &setLayout);
    if (setLayout == VK_NULL_HANDLE) {
        SKGPU_LOG_E("Failed to create descriptor set layout; pipeline creation will fail.\n");
        return VK_NULL_HANDLE;
    }
    return setLayout;
}

static void destroy_desc_set_layouts(const VulkanSharedContext* sharedContext,
                                     skia_private::TArray<VkDescriptorSetLayout>& setLayouts) {
    for (int i = 0; i < setLayouts.size(); i++) {
        if (setLayouts[i] != VK_NULL_HANDLE) {
            VULKAN_CALL(sharedContext->interface(),
            DestroyDescriptorSetLayout(sharedContext->device(),
                                       setLayouts[i],
                                       nullptr));
        }
    }
}

static bool input_attachment_desc_set_layout(VkDescriptorSetLayout& outLayout,
                                             const VulkanSharedContext* sharedContext,
                                             bool mockOnly) {
    skia_private::STArray<1, DescriptorData> inputAttachmentDesc;

    if (!mockOnly) {
        inputAttachmentDesc.push_back(VulkanGraphicsPipeline::kInputAttachmentDescriptor);
    }

    // If mockOnly is true (meaning no input attachment descriptor is actually needed), then still
    // request a mock VkDescriptorSetLayout handle by passing in the unpopulated span.
    outLayout = descriptor_data_to_layout(sharedContext, {inputAttachmentDesc});
    return outLayout != VK_NULL_HANDLE;
}

static bool uniform_desc_set_layout(VkDescriptorSetLayout& outLayout,
                                    const VulkanSharedContext* sharedContext,
                                    bool hasStepUniforms,
                                    bool hasPaintUniforms,
                                    bool hasGradientBuffer) {
    // Define a container with size reserved for up to kNumUniformBuffers descriptors. Only add
    // DescriptorData for uniforms that actually are used and need to be included in the layout.
    skia_private::STArray<
            VulkanGraphicsPipeline::kNumUniformBuffers, DescriptorData> uniformDescriptors;

    DescriptorType uniformBufferType =
            sharedContext->caps()->storageBufferSupport() ? DescriptorType::kStorageBuffer
                                                          : DescriptorType::kUniformBuffer;
    if (hasStepUniforms) {
        uniformDescriptors.push_back({
                uniformBufferType, /*count=*/1,
                VulkanGraphicsPipeline::kRenderStepUniformBufferIndex,
                PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader});
    }
    if (hasPaintUniforms) {
        uniformDescriptors.push_back({
                uniformBufferType, /*count=*/1,
                VulkanGraphicsPipeline::kPaintUniformBufferIndex,
                PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader});
    }
    if (hasGradientBuffer) {
        uniformDescriptors.push_back({
                DescriptorType::kStorageBuffer,
                /*count=*/1,
                VulkanGraphicsPipeline::kGradientBufferIndex,
                PipelineStageFlags::kFragmentShader});
    }

    // If no uniforms are used, still request a mock VkDescriptorSetLayout handle by passing in the
    // unpopulated span of uniformDescriptors to descriptor set layout creation.
    outLayout = descriptor_data_to_layout(sharedContext, {uniformDescriptors});
    return true;
}

static bool texture_sampler_desc_set_layout(VkDescriptorSetLayout& outLayout,
                                            const VulkanSharedContext* sharedContext,
                                            const int numTextureSamplers,
                                            SkSpan<sk_sp<VulkanSampler>> immutableSamplers) {
    SkASSERT(numTextureSamplers >= 0);
    // The immutable sampler span size must be = the total number of texture/samplers such that
    // we can use the index of a sampler as its binding index (or we just have none, which
    // enables us to skip some of this logic entirely).
    SkASSERT(immutableSamplers.empty() ||
             SkTo<int>(immutableSamplers.size()) == numTextureSamplers);

    skia_private::TArray<DescriptorData> textureSamplerDescs(numTextureSamplers);
    for (int i = 0; i < numTextureSamplers; i++) {
        Sampler* immutableSampler = nullptr;
        if (!immutableSamplers.empty() && immutableSamplers[i]) {
            immutableSampler = immutableSamplers[i].get();
        }
        textureSamplerDescs.push_back({DescriptorType::kCombinedTextureSampler,
                                       /*count=*/1,
                                       /*bindingIdx=*/i,
                                       PipelineStageFlags::kFragmentShader,
                                       immutableSampler});
    }

    // If no texture/samplers are used, a mock VkDescriptorSetLayout handle by passing in the
    // unpopulated span of textureSamplerDescs to descriptor set layout creation.
    outLayout = descriptor_data_to_layout(sharedContext, {textureSamplerDescs});
    return outLayout != VK_NULL_HANDLE;
}

static VkPipelineLayout setup_pipeline_layout(const VulkanSharedContext* sharedContext,
                                              uint32_t pushConstantSize,
                                              VkShaderStageFlagBits pushConstantPipelineStageFlags,
                                              bool hasStepUniforms,
                                              bool hasPaintUniforms,
                                              bool hasGradientBuffer,
                                              int numTextureSamplers,
                                              bool loadMsaaFromResolve,
                                              SkSpan<sk_sp<VulkanSampler>> immutableSamplers) {
    // Create a container with the max anticipated amount (kMaxNumDescSets) of VkDescriptorSetLayout
    // handles which will be used to create the pipeline layout.
    skia_private::STArray<
            VulkanGraphicsPipeline::kMaxNumDescSets, VkDescriptorSetLayout> setLayouts;
    setLayouts.push_back_n(VulkanGraphicsPipeline::kMaxNumDescSets, VkDescriptorSetLayout());

    // Populate the container with actual descriptor set layout handles. Each index should contain
    // either a valid/real or a mock/placehodler layout handle. Mock VkDescriptorSetLayouts do not
    // actually contain any descriptors, but are needed as placeholders to maintain expected
    // descriptor set binding indices. This is because VK_NULL_HANDLE is a valid
    // VkDescriptorSetLayout value iff the graphicsPipelineLibrary feature is enabled, which is not
    // the case for all targeted devices (see
    // VUID-VkPipelineLayoutCreateInfo-graphicsPipelineLibrary-06753). If any of the helpers
    // encounter an error (i.e., return false), return a null VkPipelineLayout.
    if (!input_attachment_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kDstAsInputDescSetIndex],
                sharedContext,
                /*mockOnly=*/false) || // We always add an input attachment descriptor
        !uniform_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kUniformBufferDescSetIndex],
                sharedContext,
                hasStepUniforms,
                hasPaintUniforms,
                hasGradientBuffer) ||
        !texture_sampler_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kTextureBindDescSetIndex],
                sharedContext,
                numTextureSamplers,
                immutableSamplers) ||
        !input_attachment_desc_set_layout(
                setLayouts[VulkanGraphicsPipeline::kLoadMsaaFromResolveInputDescSetIndex],
                sharedContext,
                /*mockOnly=*/!loadMsaaFromResolve)) { // Actual descriptor needed iff loading MSAA
        destroy_desc_set_layouts(sharedContext, setLayouts);
        return VK_NULL_HANDLE;
    }

    // Generate a pipeline layout using the now-populated descriptor set layout array
    VkPushConstantRange pushConstantRange;
    if (pushConstantSize) {
        pushConstantRange.offset = 0;
        pushConstantRange.size = pushConstantSize;
        pushConstantRange.stageFlags = pushConstantPipelineStageFlags;
    }
    VkPipelineLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.setLayoutCount = setLayouts.size();
    layoutCreateInfo.pSetLayouts = setLayouts.begin();
    layoutCreateInfo.pushConstantRangeCount = pushConstantSize ? 1 : 0;
    layoutCreateInfo.pPushConstantRanges = pushConstantSize ? &pushConstantRange : nullptr;

    VkResult result;
    VkPipelineLayout layout;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreatePipelineLayout(sharedContext->device(),
                                            &layoutCreateInfo,
                                            /*const VkAllocationCallbacks*=*/nullptr,
                                            &layout));

    // DescriptorSetLayouts can be deleted after the pipeline layout is created.
    destroy_desc_set_layouts(sharedContext, setLayouts);

    return result == VK_SUCCESS ? layout : VK_NULL_HANDLE;
}

static void setup_dynamic_state(VkPipelineDynamicStateCreateInfo* dynamicInfo,
                                VkDynamicState* dynamicStates) {
    *dynamicInfo = {};
    dynamicInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
    dynamicStates[2] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    dynamicInfo->dynamicStateCount = 3;
    dynamicInfo->pDynamicStates = dynamicStates;
}

VulkanProgramInfo::~VulkanProgramInfo() {
    if (fVS != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyShaderModule(fSharedContext->device(), fVS, nullptr));
        fVS = VK_NULL_HANDLE;
    }
    if (fFS != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyShaderModule(fSharedContext->device(), fFS, nullptr));
        fFS = VK_NULL_HANDLE;
    }
    if (fLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(fSharedContext->interface(),
                    DestroyPipelineLayout(fSharedContext->device(),
                                          fLayout,
                                          nullptr));
        fLayout = VK_NULL_HANDLE;
    }
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(
        const VulkanSharedContext* sharedContext,
        VulkanResourceProvider* rsrcProvider,
        const RuntimeEffectDictionary* runtimeDict,
        const UniqueKey& pipelineKey,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
        uint32_t compilationID) {
    SkASSERT(rsrcProvider);

    SkSL::ProgramSettings settings;
    settings.fSharpenTextures = true;
    settings.fForceNoRTFlip = true;

    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    const RenderStep* step = sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());
    const bool useStorageBuffers = sharedContext->caps()->storageBufferSupport();

    if (step->staticAttributes().size() + step->appendAttributes().size() >
        sharedContext->vulkanCaps().maxVertexAttributes()) {
        SKGPU_LOG_W("Requested more than the supported number of vertex attributes");
        return nullptr;
    }

    skia_private::TArray<SamplerDesc> descContainer {};
    std::unique_ptr<ShaderInfo> shaderInfo =
            ShaderInfo::Make(sharedContext->caps(),
                             sharedContext->shaderCodeDictionary(),
                             runtimeDict,
                             step,
                             pipelineDesc.paintParamsID(),
                             useStorageBuffers,
                             renderPassDesc.fWriteSwizzle,
                             renderPassDesc.fDstReadStrategy,
                             &descContainer);

    // Populate an array of sampler ptrs where a sampler's index within the array indicates their
    // binding index within the descriptor set. Initialize all values to nullptr, which represents a
    // "regular", dynamic sampler at that index.
    skia_private::TArray<sk_sp<VulkanSampler>> immutableSamplers;
    immutableSamplers.push_back_n(shaderInfo->numFragmentTexturesAndSamplers());

    // This logic relies upon Vulkan using combined texture/sampler bindings, which is necessary for
    // ycbcr samplers per the Vulkan spec.
    SkASSERT(!sharedContext->caps()->resourceBindingRequirements().fSeparateTextureAndSamplerBinding
             && shaderInfo->numFragmentTexturesAndSamplers() == descContainer.size());
    for (int i = 0; i < descContainer.size(); i++) {
        // If a SamplerDesc is not equivalent to the default-initialized SamplerDesc, that indicates
        // the usage of an immutable sampler. That sampler desc should then be used to obtain an
        // actual immutable sampler from the resource provider and added at the proper index within
        // immutableSamplers for inclusion in the pipeline layout.
        if (descContainer.at(i) != SamplerDesc()) {
            sk_sp<Sampler> immutableSampler =
                    rsrcProvider->findOrCreateCompatibleSampler(descContainer.at(i));
            sk_sp<VulkanSampler> vulkanSampler =
                    sk_ref_sp<VulkanSampler>(static_cast<VulkanSampler*>(immutableSampler.get()));
            SkASSERT(vulkanSampler);
            immutableSamplers[i] = std::move(vulkanSampler);
        }
    }

    auto program = VulkanProgramInfo::Make(sharedContext);

    const std::string& fsSkSL = shaderInfo->fragmentSkSL();
    const bool hasFragmentSkSL = !fsSkSL.empty();
    std::string vsSPIRV, fsSPIRV;
    SkSL::Program::Interface vsInterface, fsInterface;
    if (hasFragmentSkSL) {
        if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                                fsSkSL,
                                SkSL::ProgramKind::kGraphiteFragment,
                                settings,
                                &fsSPIRV,
                                &fsInterface,
                                errorHandler)) {
            return nullptr;
        }
        if(!program->setFragmentShader(CreateVulkanShaderModule(
                sharedContext, fsSPIRV, VK_SHADER_STAGE_FRAGMENT_BIT))) {
            return nullptr;
        }
    }

    const std::string& vsSkSL = shaderInfo->vertexSkSL();
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                           vsSkSL,
                           SkSL::ProgramKind::kGraphiteVertex,
                           settings,
                           &vsSPIRV,
                           &vsInterface,
                           errorHandler)) {
        return nullptr;
    }
    if (!program->setVertexShader(CreateVulkanShaderModule(
                sharedContext, vsSPIRV, VK_SHADER_STAGE_VERTEX_BIT))) {
        return nullptr;
    }

    // TODO: Query RenderPassDesc for input attachment information. For now, we only use one for
    // loading MSAA from resolve so we can simply pass in 0 when not doing that.
    if (!program->setLayout(setup_pipeline_layout(
                sharedContext,
                VulkanResourceProvider::kIntrinsicConstantSize,
                VulkanResourceProvider::kIntrinsicConstantStageFlags,
                !step->uniforms().empty(),
                shaderInfo->hasPaintUniforms(),
                shaderInfo->hasGradientBuffer(),
                shaderInfo->numFragmentTexturesAndSamplers(),
                /*loadMsaaFromResolve=*/false,
                SkSpan<sk_sp<VulkanSampler>>(immutableSamplers)))) {
        return nullptr;
    }

    // This pipeline factory is for regular pipelines that run in the main subpass. Depending on
    // if there will be a load-MSAA subpass, the index changes. Ideally with
    // VK_dynamic_rendering_local_read, we can reuse pipelines across subpasses for layer elision.
    int subpassIndex = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc) ? 1 : 0;
    VkPipeline vkPipeline = MakePipeline(sharedContext,
                                         rsrcProvider,
                                         *program,
                                         subpassIndex,
                                         step->primitiveType(),
                                         step->appendsVertices() ? VK_VERTEX_INPUT_RATE_VERTEX :
                                                                   VK_VERTEX_INPUT_RATE_INSTANCE,
                                         step->staticAttributes(),
                                         step->appendAttributes(),
                                         step->depthStencilSettings(),
                                         shaderInfo->blendInfo(),
                                         renderPassDesc);

    sk_sp<VulkanGraphicsPipeline> pipeline;
    if (vkPipeline) {
        PipelineInfo pipelineInfo{ *shaderInfo, pipelineCreationFlags,
                                    pipelineKey.hash(), compilationID };
#if defined(GPU_TEST_UTILS)
        pipelineInfo.fNativeVertexShader   = "SPIR-V disassembly not available";
        pipelineInfo.fNativeFragmentShader = "SPIR-V disassmebly not available";
#endif

        pipeline = sk_sp<VulkanGraphicsPipeline>(
            new VulkanGraphicsPipeline(sharedContext,
                                       pipelineInfo,
                                       program->releaseLayout(),
                                       vkPipeline,
                                       /*ownsPipelineLayout=*/true,
                                       std::move(immutableSamplers)));
    }

    return pipeline;
}

VkPipeline VulkanGraphicsPipeline::MakePipeline(const VulkanSharedContext* sharedContext,
                                                VulkanResourceProvider* rsrcProvider,
                                                const VulkanProgramInfo& program,
                                                int subpassIndex,
                                                PrimitiveType primitiveType,
                                                VkVertexInputRate appendInputRate,
                                                SkSpan<const Attribute> staticAttrs,
                                                SkSpan<const Attribute> appendAttrs,
                                                const DepthStencilSettings& depthStencilSettings,
                                                const BlendInfo& blendInfo,
                                                const RenderPassDesc& renderPassDesc) {
    SkASSERT(program.layout() && program.vs()); // but a fragment shader isn't required

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    skia_private::STArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    skia_private::STArray<16, VkVertexInputAttributeDescription> attributeDescs;
    setup_vertex_input_state(appendInputRate,
                             staticAttrs,
                             appendAttrs,
                             &vertexInputInfo,
                             &bindingDescs,
                             &attributeDescs);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(primitiveType, &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(depthStencilSettings, &depthStencilInfo);

    VkPipelineViewportStateCreateInfo viewportInfo;
    setup_viewport_scissor_state(&viewportInfo);

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(renderPassDesc.fColorAttachment.fSampleCount, &multisampleInfo);

    // We will only have one color blend attachment per pipeline.
    VkPipelineColorBlendAttachmentState attachmentStates[1];
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    setup_color_blend_state(blendInfo, &colorBlendInfo, attachmentStates);

    VkPipelineRasterizationStateCreateInfo rasterInfo;
    // TODO: Check for wire frame mode once that is an available context option within graphite.
    setup_raster_state(/*isWireframe=*/false, &rasterInfo);

    VkPipelineShaderStageCreateInfo pipelineShaderStages[2];
    setup_shader_stage_info(VK_SHADER_STAGE_VERTEX_BIT,
                            program.vs(),
                            &pipelineShaderStages[0]);
    if (program.fs()) {
        setup_shader_stage_info(VK_SHADER_STAGE_FRAGMENT_BIT,
                                program.fs(),
                                &pipelineShaderStages[1]);
    }

    VkDynamicState dynamicStates[3];
    VkPipelineDynamicStateCreateInfo dynamicInfo;
    setup_dynamic_state(&dynamicInfo, dynamicStates);

    sk_sp<VulkanRenderPass> compatibleRenderPass =
            rsrcProvider->findOrCreateRenderPass(renderPassDesc, /*compatibleOnly=*/true);
    if (!compatibleRenderPass) {
        SKGPU_LOG_E("Failed to create compatible renderpass for pipeline");
        return VK_NULL_HANDLE;
    }
    SkDEBUGCODE(int subpassCount = RenderPassDescWillLoadMSAAFromResolve(renderPassDesc) ? 2 : 1;)
    SkASSERT(subpassIndex >= 0 && subpassIndex < subpassCount);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stageCount = program.fs() ? 2 : 1;
    pipelineCreateInfo.pStages = &pipelineShaderStages[0];
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = &viewportInfo;
    pipelineCreateInfo.pRasterizationState = &rasterInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    pipelineCreateInfo.pDynamicState = &dynamicInfo;
    pipelineCreateInfo.layout = program.layout();
    pipelineCreateInfo.renderPass = compatibleRenderPass->renderPass();
    pipelineCreateInfo.subpass = subpassIndex;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline vkPipeline;
    VkResult result;
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "VkCreateGraphicsPipeline");
        VULKAN_CALL_RESULT(sharedContext,
                           result,
                           CreateGraphicsPipelines(sharedContext->device(),
                                                   rsrcProvider->pipelineCache(),
                                                   /*createInfoCount=*/1,
                                                   &pipelineCreateInfo,
                                                   /*pAllocator=*/nullptr,
                                                   &vkPipeline));
    }
    if (result != VK_SUCCESS) {
        SKGPU_LOG_E("Failed to create pipeline. Error: %d\n", result);
        return VK_NULL_HANDLE;
    }

    return vkPipeline;
}

std::unique_ptr<VulkanProgramInfo> VulkanGraphicsPipeline::CreateLoadMSAAProgram(
        const VulkanSharedContext* sharedContext) {
    SkSL::ProgramSettings settings;
    settings.fForceNoRTFlip = true;
    std::string vsSPIRV, fsSPIRV;
    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    std::string vertShaderText;
    vertShaderText.append(
            "layout(vulkan,  push_constant) uniform vertexUniformBuffer {"
                "half4 uPosXform;"
            "};"

            // MSAA Load Program VS
            "void main() {"
                "float2 position = float2(sk_VertexID >> 1, sk_VertexID & 1);"
                "sk_Position.xy = position * uPosXform.xy + uPosXform.zw;"
                "sk_Position.zw = half2(0, 1);"
            "}");

    std::string fragShaderText;
    fragShaderText.append(
            "layout(vulkan, input_attachment_index=0, set=" +
            std::to_string(VulkanGraphicsPipeline::kLoadMsaaFromResolveInputDescSetIndex) +
            ", binding=0) subpassInput uInput;"

            // MSAA Load Program FS
            "void main() {"
                "sk_FragColor = subpassLoad(uInput);"
            "}");

    auto program = VulkanProgramInfo::Make(sharedContext);

    SkSL::Program::Interface vsInterface, fsInterface;
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            vertShaderText,
                            SkSL::ProgramKind::kGraphiteVertex,
                            settings,
                            &vsSPIRV,
                            &vsInterface,
                            errorHandler)) {
        return nullptr;
    }
    if (!program->setVertexShader(CreateVulkanShaderModule(
                sharedContext, vsSPIRV, VK_SHADER_STAGE_VERTEX_BIT))) {
        return nullptr;
    }

    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            fragShaderText,
                            SkSL::ProgramKind::kGraphiteFragment,
                            settings,
                            &fsSPIRV,
                            &fsInterface,
                            errorHandler)) {
        return nullptr;
    }
    if (!program->setFragmentShader(CreateVulkanShaderModule(
                sharedContext, fsSPIRV, VK_SHADER_STAGE_FRAGMENT_BIT))) {
        return nullptr;
    }

    // The load msaa pipeline takes no step or paint uniforms and no instance attributes. It only
    // references one input attachment texture (which does not require a sampler) and one vertex
    // attribute (NDC position)
    skia_private::TArray<DescriptorData> inputAttachmentDescriptors(1);
    inputAttachmentDescriptors.push_back(VulkanGraphicsPipeline::kInputAttachmentDescriptor);
    // This pipeline is used to read from the resolve attachment to a color attachment. We should
    // never require an immutable sampler for this, since that would imply that we are rendering to
    // a surface with an external format.
    if (!program->setLayout(setup_pipeline_layout(
                sharedContext,
                /*pushConstantSize=*/32,
                (VkShaderStageFlagBits)VK_SHADER_STAGE_VERTEX_BIT,
                /*hasStepUniforms=*/false,
                /*hasPaintUniforms=*/false,
                /*hasGradientBuffer=*/false,
                /*numTextureSamplers=*/0,
                /*loadMsaaFromResolve=*/true,
                /*immutableSamplers=*/{}))) {
        return nullptr;
    }

    return program;
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::MakeLoadMSAAPipeline(
        const VulkanSharedContext* sharedContext,
        VulkanResourceProvider* rsrcProvider,
        const VulkanProgramInfo& loadMSAAProgram,
        const RenderPassDesc& renderPassDesc) {
    // The load MSAA pipeline does not have any vertex or instance attributes, does not use the
    // depth or stencil attachments. Unlike the general Make factory, we do not destroy any of the
    // shader modules or layout. The pipeline layout will not be owned by the created
    // VulkanGraphicsPipeline either, as it's owned by the resource provider.
    SkASSERT(RenderPassDescWillLoadMSAAFromResolve(renderPassDesc));
    VkPipeline vkPipeline = MakePipeline(sharedContext,
                                         rsrcProvider,
                                         loadMSAAProgram,
                                         /*subpassIndex=*/0, // loading to MSAA is always first
                                         PrimitiveType::kTriangleStrip,
                                         /*appendInputRate=*/{},
                                         /*staticAttrs=*/{},
                                         /*appendAttrs=*/{},
                                         /*depthStencilSettings=*/{},
                                         /*blendInfo=*/{},
                                         renderPassDesc);
    if (!vkPipeline) {
        return nullptr;
    }

    return sk_sp<VulkanGraphicsPipeline>(new VulkanGraphicsPipeline(
        sharedContext,
        /*pipelineInfo=*/{}, // leave empty for an internal pipeline
        loadMSAAProgram.layout(),
        vkPipeline,
        /*ownsPipelineLayout=*/false,
        /*immutableSamplers=*/{}));
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(
        const VulkanSharedContext* sharedContext,
        const PipelineInfo& pipelineInfo,
        VkPipelineLayout pipelineLayout,
        VkPipeline pipeline,
        bool ownsPipelineLayout,
        skia_private::TArray<sk_sp<VulkanSampler>>&& immutableSamplers)
    : GraphicsPipeline(sharedContext, pipelineInfo)
    , fPipelineLayout(pipelineLayout)
    , fPipeline(pipeline)
    , fOwnsPipelineLayout(ownsPipelineLayout)
    , fImmutableSamplers(std::move(immutableSamplers)) {}

void VulkanGraphicsPipeline::freeGpuData() {
    auto sharedCtxt = static_cast<const VulkanSharedContext*>(this->sharedContext());
    if (fPipeline != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
            DestroyPipeline(sharedCtxt->device(), fPipeline, nullptr));
    }
    if (fOwnsPipelineLayout && fPipelineLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyPipelineLayout(sharedCtxt->device(), fPipelineLayout, nullptr));
    }
}

} // namespace skgpu::graphite
