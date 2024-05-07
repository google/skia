/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/TextureInfo.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkSLTypeShared.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/PipelineUtils.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanRenderPass.h"
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
        const SkSpan<const Attribute>& vertexAttrs,
        const SkSpan<const Attribute>& instanceAttrs,
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
        skia_private::STArray<2, VkVertexInputBindingDescription, true>* bindingDescs,
        skia_private::STArray<16, VkVertexInputAttributeDescription>* attributeDescs) {
    // Setup attribute & binding descriptions
    int attribIndex = 0;
    size_t vertexAttributeOffset = 0;
    for (auto attrib : vertexAttrs) {
        VkVertexInputAttributeDescription vkAttrib;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kVertexBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = vertexAttributeOffset;
        vertexAttributeOffset += attrib.sizeAlign4();
        attributeDescs->push_back(vkAttrib);
    }

    size_t instanceAttributeOffset = 0;
    for (auto attrib : instanceAttrs) {
        VkVertexInputAttributeDescription vkAttrib;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kInstanceBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = instanceAttributeOffset;
        instanceAttributeOffset += attrib.sizeAlign4();
        attributeDescs->push_back(vkAttrib);
    }

    if (bindingDescs && !vertexAttrs.empty()) {
        bindingDescs->push_back() = {
                VulkanGraphicsPipeline::kVertexBufferIndex,
                (uint32_t) vertexAttributeOffset,
                VK_VERTEX_INPUT_RATE_VERTEX
        };
    }
    if (bindingDescs && !instanceAttrs.empty()) {
        bindingDescs->push_back() = {
                VulkanGraphicsPipeline::kInstanceBufferIndex,
                (uint32_t) instanceAttributeOffset,
                VK_VERTEX_INPUT_RATE_INSTANCE
        };
    }

    memset(vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->pNext = nullptr;
    vertexInputInfo->flags = 0;
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
    memset(inputAssemblyInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    inputAssemblyInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->pNext = nullptr;
    inputAssemblyInfo->flags = 0;
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

    memset(stencilInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    stencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stencilInfo->pNext = nullptr;
    stencilInfo->flags = 0;
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
    memset(viewportInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    viewportInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo->pNext = nullptr;
    viewportInfo->flags = 0;

    viewportInfo->viewportCount = 1;
    viewportInfo->pViewports = nullptr; // This is set dynamically with a draw pass command

    viewportInfo->scissorCount = 1;
    viewportInfo->pScissors = nullptr; // This is set dynamically with a draw pass command

    SkASSERT(viewportInfo->viewportCount == viewportInfo->scissorCount);
}

static void setup_multisample_state(int numSamples,
                                    VkPipelineMultisampleStateCreateInfo* multisampleInfo) {
    memset(multisampleInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampleInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo->pNext = nullptr;
    multisampleInfo->flags = 0;
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

    memset(attachmentState, 0, sizeof(VkPipelineColorBlendAttachmentState));
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

    memset(colorBlendInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    colorBlendInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo->pNext = nullptr;
    colorBlendInfo->flags = 0;
    colorBlendInfo->logicOpEnable = VK_FALSE;
    colorBlendInfo->attachmentCount = 1;
    colorBlendInfo->pAttachments = attachmentState;
    // colorBlendInfo->blendConstants is set dynamically
}

static void setup_raster_state(bool isWireframe,
                               VkPipelineRasterizationStateCreateInfo* rasterInfo) {
    memset(rasterInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo->pNext = nullptr;
    rasterInfo->flags = 0;
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
    memset(shaderStageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    shaderStageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageInfo->pNext = nullptr;
    shaderStageInfo->flags = 0;
    shaderStageInfo->stage = stage;
    shaderStageInfo->module = shaderModule;
    shaderStageInfo->pName = "main";
    shaderStageInfo->pSpecializationInfo = nullptr;
}


static VkDescriptorSetLayout descriptor_data_to_layout(const VulkanSharedContext* sharedContext,
        const SkSpan<DescriptorData>& descriptorData) {
    if (descriptorData.size() == 0) { return VK_NULL_HANDLE; }

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

static VkPipelineLayout setup_pipeline_layout(const VulkanSharedContext* sharedContext,
                                              bool usesIntrinsicConstantUbo,
                                              bool hasStepUniforms,
                                              int numPaintUniforms,
                                              int numTextureSamplers,
                                              int numInputAttachments) {
    // Determine descriptor set layouts for this pipeline based upon render pass information.
    skia_private::STArray<3, VkDescriptorSetLayout> setLayouts;

    // Determine uniform descriptor set layout
    skia_private::STArray<VulkanGraphicsPipeline::kNumUniformBuffers, DescriptorData>
            uniformDescriptors;
    if (usesIntrinsicConstantUbo) {
        uniformDescriptors.push_back(VulkanGraphicsPipeline::kIntrinsicUniformBufferDescriptor);
    }
    if (hasStepUniforms) {
        uniformDescriptors.push_back(VulkanGraphicsPipeline::kRenderStepUniformDescriptor);
    }
    if (numPaintUniforms > 0) {
        uniformDescriptors.push_back(VulkanGraphicsPipeline::kPaintUniformDescriptor);
    }

    if (!uniformDescriptors.empty()) {
        VkDescriptorSetLayout uniformSetLayout =
                descriptor_data_to_layout(sharedContext, {uniformDescriptors});
        if (uniformSetLayout == VK_NULL_HANDLE) { return VK_NULL_HANDLE; }
        setLayouts.push_back(uniformSetLayout);
    }

    // Determine input attachment descriptor set layout
    if (numInputAttachments > 0) {
        // For now, we only expect to have up to 1 input attachment. We also share that descriptor
        // set number with uniform descriptors for normal graphics pipeline usages, so verify that
        // we are not using any uniform descriptors to avoid conflicts.
        SkASSERT(numInputAttachments == 1 && uniformDescriptors.empty());
        skia_private::TArray<DescriptorData> inputAttachmentDescriptors(numInputAttachments);
        inputAttachmentDescriptors.push_back(VulkanGraphicsPipeline::kInputAttachmentDescriptor);

        VkDescriptorSetLayout inputAttachmentDescSetLayout =
                descriptor_data_to_layout(sharedContext, {inputAttachmentDescriptors});

        if (inputAttachmentDescSetLayout == VK_NULL_HANDLE) {
            destroy_desc_set_layouts(sharedContext, setLayouts);
            return VK_NULL_HANDLE;
        }
        setLayouts.push_back(inputAttachmentDescSetLayout);
    }

    // Determine texture/sampler descriptor set layout
    if (numTextureSamplers > 0) {
        skia_private::TArray<DescriptorData> textureSamplerDescs(numTextureSamplers);
        for (int i = 0; i < numTextureSamplers; i++) {
            textureSamplerDescs.push_back({DescriptorType::kCombinedTextureSampler,
                                            /*count=*/1,
                                            /*bindingIdx=*/i,
                                            PipelineStageFlags::kFragmentShader});
        }
        VkDescriptorSetLayout textureSamplerDescSetLayout =
                descriptor_data_to_layout(sharedContext, {textureSamplerDescs});

        if (textureSamplerDescSetLayout == VK_NULL_HANDLE) {
            destroy_desc_set_layouts(sharedContext, setLayouts);
            return VK_NULL_HANDLE;
        }
        setLayouts.push_back(textureSamplerDescSetLayout);
    }

    // Generate a pipeline layout using the now-populated descriptor set layout array
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.setLayoutCount = setLayouts.size();
    layoutCreateInfo.pSetLayouts = setLayouts.begin();
    // TODO: Add support for push constants.
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

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

static void destroy_shader_modules(const VulkanSharedContext* sharedContext,
                                   VkShaderModule vsModule,
                                   VkShaderModule fsModule) {
    if (vsModule != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyShaderModule(sharedContext->device(), vsModule, nullptr));
    }
    if (fsModule != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyShaderModule(sharedContext->device(), fsModule, nullptr));
    }
}

static void setup_dynamic_state(VkPipelineDynamicStateCreateInfo* dynamicInfo,
                                VkDynamicState* dynamicStates) {
    memset(dynamicInfo, 0, sizeof(VkPipelineDynamicStateCreateInfo));
    dynamicInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo->pNext = VK_NULL_HANDLE;
    dynamicInfo->flags = 0;
    dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
    dynamicStates[2] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    dynamicInfo->dynamicStateCount = 3;
    dynamicInfo->pDynamicStates = dynamicStates;
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(
        const VulkanSharedContext* sharedContext,
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc,
        const sk_sp<VulkanRenderPass>& compatibleRenderPass,
        VkPipelineCache pipelineCache) {

    SkSL::Program::Interface vsInterface, fsInterface;
    SkSL::ProgramSettings settings;
    settings.fForceNoRTFlip = true; // TODO: Confirm
    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    const RenderStep* step = sharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());
    const bool useStorageBuffers = sharedContext->caps()->storageBufferPreferred();

    if (step->vertexAttributes().size() + step->instanceAttributes().size() >
        sharedContext->vulkanCaps().maxVertexAttributes()) {
        SKGPU_LOG_W("Requested more than the supported number of vertex attributes");
        return nullptr;
    }

    FragSkSLInfo fsSkSLInfo = BuildFragmentSkSL(sharedContext->caps(),
                                                sharedContext->shaderCodeDictionary(),
                                                runtimeDict,
                                                step,
                                                pipelineDesc.paintParamsID(),
                                                useStorageBuffers,
                                                renderPassDesc.fWriteSwizzle);
    std::string& fsSkSL = fsSkSLInfo.fSkSL;
    const bool localCoordsNeeded = fsSkSLInfo.fRequiresLocalCoords;

    bool hasFragmentSkSL = !fsSkSL.empty();
    std::string vsSPIRV, fsSPIRV;
    VkShaderModule fsModule = VK_NULL_HANDLE, vsModule = VK_NULL_HANDLE;

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

        fsModule = createVulkanShaderModule(sharedContext, fsSPIRV, VK_SHADER_STAGE_FRAGMENT_BIT);
        if (!fsModule) {
            return nullptr;
        }
    }

    VertSkSLInfo vsSkSLInfo = BuildVertexSkSL(sharedContext->caps()->resourceBindingRequirements(),
                                              step,
                                              useStorageBuffers,
                                              localCoordsNeeded);
    const std::string& vsSkSL = vsSkSLInfo.fSkSL;
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            vsSkSL,
                            SkSL::ProgramKind::kGraphiteVertex,
                            settings,
                            &vsSPIRV,
                            &vsInterface,
                            errorHandler)) {
        return nullptr;
    }

    vsModule = createVulkanShaderModule(sharedContext, vsSPIRV, VK_SHADER_STAGE_VERTEX_BIT);
    if (!vsModule) {
        // Clean up the other shader module before returning.
        destroy_shader_modules(sharedContext, VK_NULL_HANDLE, fsModule);
        return nullptr;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    skia_private::STArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    skia_private::STArray<16, VkVertexInputAttributeDescription> attributeDescs;
    setup_vertex_input_state(step->vertexAttributes(),
                             step->instanceAttributes(),
                             &vertexInputInfo,
                             &bindingDescs,
                             &attributeDescs);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(step->primitiveType(), &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(step->depthStencilSettings(), &depthStencilInfo);

    VkPipelineViewportStateCreateInfo viewportInfo;
    setup_viewport_scissor_state(&viewportInfo);

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(renderPassDesc.fColorAttachment.fTextureInfo.numSamples(),
                            &multisampleInfo);

    // We will only have one color blend attachment per pipeline.
    VkPipelineColorBlendAttachmentState attachmentStates[1];
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    setup_color_blend_state(fsSkSLInfo.fBlendInfo, &colorBlendInfo, attachmentStates);

    VkPipelineRasterizationStateCreateInfo rasterInfo;
    // TODO: Check for wire frame mode once that is an available context option within graphite.
    setup_raster_state(/*isWireframe=*/false, &rasterInfo);

    VkPipelineShaderStageCreateInfo pipelineShaderStages[2];
    setup_shader_stage_info(VK_SHADER_STAGE_VERTEX_BIT,
                            vsModule,
                            &pipelineShaderStages[0]);
    if (hasFragmentSkSL) {
        setup_shader_stage_info(VK_SHADER_STAGE_FRAGMENT_BIT,
                                fsModule,
                                &pipelineShaderStages[1]);
    }

    // TODO: Query RenderPassDesc for input attachment information. For now, we only use one for
    // loading MSAA from resolve so we can simply pass in 0 when not doing that.
    VkPipelineLayout pipelineLayout = setup_pipeline_layout(sharedContext,
                                                            /*usesIntrinsicConstantUbo=*/true,
                                                            !step->uniforms().empty(),
                                                            fsSkSLInfo.fNumPaintUniforms,
                                                            fsSkSLInfo.fNumTexturesAndSamplers,
                                                            /*numInputAttachments=*/0);
    if (pipelineLayout == VK_NULL_HANDLE) {
        destroy_shader_modules(sharedContext, vsModule, fsModule);
        return nullptr;
    }

    VkDynamicState dynamicStates[3];
    VkPipelineDynamicStateCreateInfo dynamicInfo;
    setup_dynamic_state(&dynamicInfo, dynamicStates);

    bool loadMsaaFromResolve = renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
                               renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    memset(&pipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = hasFragmentSkSL ? 2 : 1;
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
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = compatibleRenderPass->renderPass();
    pipelineCreateInfo.subpass = loadMsaaFromResolve ? 1 : 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline vkPipeline;
    VkResult result;
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "VkCreateGraphicsPipeline");
        VULKAN_CALL_RESULT(sharedContext,
                           result,
                           CreateGraphicsPipelines(sharedContext->device(),
                                                   pipelineCache,
                                                   /*createInfoCount=*/1,
                                                   &pipelineCreateInfo,
                                                   /*pAllocator=*/nullptr,
                                                   &vkPipeline));
    }
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to create pipeline. Error: %d\n", result);
        return nullptr;
    }

    // After creating the pipeline object, we can clean up the VkShaderModule(s).
    destroy_shader_modules(sharedContext, vsModule, fsModule);

#if defined(GRAPHITE_TEST_UTILS)
    GraphicsPipeline::PipelineInfo pipelineInfo = {pipelineDesc.renderStepID(),
                                                   pipelineDesc.paintParamsID(),
                                                   std::move(vsSkSL),
                                                   std::move(fsSkSL),
                                                   "SPIR-V disassembly not available",
                                                   "SPIR-V disassembly not available"};
    GraphicsPipeline::PipelineInfo* pipelineInfoPtr = &pipelineInfo;
#else
    GraphicsPipeline::PipelineInfo* pipelineInfoPtr = nullptr;
#endif

    return sk_sp<VulkanGraphicsPipeline>(
            new VulkanGraphicsPipeline(sharedContext,
                                       pipelineInfoPtr,
                                       pipelineLayout,
                                       vkPipeline,
                                       fsSkSLInfo.fNumPaintUniforms > 0,
                                       !step->uniforms().empty(),
                                       fsSkSLInfo.fNumTexturesAndSamplers,
                                       /*ownsPipelineLayout=*/true));
}

bool VulkanGraphicsPipeline::InitializeMSAALoadPipelineStructs(
        const VulkanSharedContext* sharedContext,
        VkShaderModule* outVertexShaderModule,
        VkShaderModule* outFragShaderModule,
        VkPipelineShaderStageCreateInfo* outShaderStageInfo,
        VkPipelineLayout* outPipelineLayout) {
    SkSL::Program::Interface vsInterface, fsInterface;
    SkSL::ProgramSettings settings;
    settings.fForceNoRTFlip = true;
    std::string vsSPIRV, fsSPIRV;
    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();

    std::string vertShaderText;
    vertShaderText.append(
            "// MSAA Load Program VS\n"
            "layout(vulkan, location=0) in float2 ndc_position;"

            "void main() {"
            "sk_Position.xy = ndc_position;"
            "sk_Position.zw = half2(0, 1);"
            "}");

    std::string fragShaderText;
    fragShaderText.append(
            "layout(vulkan, input_attachment_index=0, set=0, binding=0) subpassInput uInput;"

            "// MSAA Load Program FS\n"
            "void main() {"
            "sk_FragColor = subpassLoad(uInput);"
            "}");

    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            vertShaderText,
                            SkSL::ProgramKind::kGraphiteVertex,
                            settings,
                            &vsSPIRV,
                            &vsInterface,
                            errorHandler)) {
        return false;
    }
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            fragShaderText,
                            SkSL::ProgramKind::kGraphiteFragment,
                            settings,
                            &fsSPIRV,
                            &fsInterface,
                            errorHandler)) {
        return false;
    }
    *outFragShaderModule =
            createVulkanShaderModule(sharedContext, fsSPIRV, VK_SHADER_STAGE_FRAGMENT_BIT);
    if (*outFragShaderModule == VK_NULL_HANDLE) {
        return false;
    }

    *outVertexShaderModule =
            createVulkanShaderModule(sharedContext, vsSPIRV, VK_SHADER_STAGE_VERTEX_BIT);
    if (*outVertexShaderModule == VK_NULL_HANDLE) {
        destroy_shader_modules(sharedContext, VK_NULL_HANDLE, *outFragShaderModule);
        return false;
    }

    setup_shader_stage_info(VK_SHADER_STAGE_VERTEX_BIT,
                            *outVertexShaderModule,
                            &outShaderStageInfo[0]);

    setup_shader_stage_info(VK_SHADER_STAGE_FRAGMENT_BIT,
                            *outFragShaderModule,
                            &outShaderStageInfo[1]);

    // The load msaa pipeline takes no step or paint uniforms and no instance attributes. It only
    // references one input attachment texture (which does not require a sampler) and one vertex
    // attribute (NDC position)
    skia_private::TArray<DescriptorData> inputAttachmentDescriptors(1);
    inputAttachmentDescriptors.push_back(VulkanGraphicsPipeline::kInputAttachmentDescriptor);
    *outPipelineLayout = setup_pipeline_layout(sharedContext,
                                               /*usesIntrinsicConstantUbo=*/false,
                                               /*hasStepUniforms=*/false,
                                               /*numPaintUniforms=*/0,
                                               /*numTextureSamplers=*/0,
                                               /*numInputAttachments=*/1);

    if (*outPipelineLayout == VK_NULL_HANDLE) {
        destroy_shader_modules(sharedContext, *outVertexShaderModule, *outFragShaderModule);
        return false;
    }
    return true;
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::MakeLoadMSAAPipeline(
        const VulkanSharedContext* sharedContext,
        VkShaderModule vsModule,
        VkShaderModule fsModule,
        VkPipelineShaderStageCreateInfo* pipelineShaderStages,
        VkPipelineLayout pipelineLayout,
        sk_sp<VulkanRenderPass> compatibleRenderPass,
        VkPipelineCache pipelineCache,
        const TextureInfo& dstColorAttachmentTexInfo) {

    int numSamples = dstColorAttachmentTexInfo.numSamples();

    // Create vertex attribute list
    Attribute vertexAttrib[1] = {{"ndc_position", VertexAttribType::kFloat2, SkSLType::kFloat2}};
    SkSpan<const Attribute> loadMSAAVertexAttribs = {vertexAttrib};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    skia_private::STArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    skia_private::STArray<16, VkVertexInputAttributeDescription> attributeDescs;
    setup_vertex_input_state(loadMSAAVertexAttribs,
                             /*instanceAttrs=*/{}, // Load msaa pipeline takes no instance attribs
                             &vertexInputInfo,
                             &bindingDescs,
                             &attributeDescs);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(PrimitiveType::kTriangleStrip, &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(/*stencilSettings=*/{}, &depthStencilInfo);

    VkPipelineViewportStateCreateInfo viewportInfo;
    setup_viewport_scissor_state(&viewportInfo);

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(numSamples, &multisampleInfo);

    // We will only have one color blend attachment per pipeline.
    VkPipelineColorBlendAttachmentState attachmentStates[1];
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    setup_color_blend_state({}, &colorBlendInfo, attachmentStates);

    VkPipelineRasterizationStateCreateInfo rasterInfo;
    // TODO: Check for wire frame mode once that is an available context option within graphite.
    setup_raster_state(/*isWireframe=*/false, &rasterInfo);

    VkDynamicState dynamicStates[3];
    VkPipelineDynamicStateCreateInfo dynamicInfo;
    setup_dynamic_state(&dynamicInfo, dynamicStates);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    memset(&pipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = pipelineShaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = &viewportInfo;
    pipelineCreateInfo.pRasterizationState = &rasterInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    pipelineCreateInfo.pDynamicState = &dynamicInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = compatibleRenderPass->renderPass();

    VkPipeline vkPipeline;
    VkResult result;
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline");
        SkASSERT(pipelineCache != VK_NULL_HANDLE);
        VULKAN_CALL_RESULT(sharedContext,
                           result,
                           CreateGraphicsPipelines(sharedContext->device(),
                                                   pipelineCache,
                                                   /*createInfoCount=*/1,
                                                   &pipelineCreateInfo,
                                                   /*pAllocator=*/nullptr,
                                                   &vkPipeline));
    }
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to create pipeline. Error: %d\n", result);
        return nullptr;
    }

    // TODO: If we want to track GraphicsPipeline::PipelineInfo in debug, we'll need to add
    // PipelineDesc as an argument for this method. For now, just pass in nullptr.
    return sk_sp<VulkanGraphicsPipeline>(
            new VulkanGraphicsPipeline(sharedContext,
                                       /*pipelineInfo=*/nullptr,
                                       pipelineLayout,
                                       vkPipeline,
                                       /*hasFragmentUniforms=*/false,
                                       /*hasStepUniforms=*/false,
                                       /*numTextureSamplers*/0,
                                       /*ownsPipelineLayout=*/false));
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                                               PipelineInfo* pipelineInfo,
                                               VkPipelineLayout pipelineLayout,
                                               VkPipeline pipeline,
                                               bool hasFragmentUniforms,
                                               bool hasStepUniforms,
                                               int numTextureSamplers,
                                               bool ownsPipelineLayout)
        : GraphicsPipeline(sharedContext, pipelineInfo)
        , fPipelineLayout(pipelineLayout)
        , fPipeline(pipeline)
        , fHasFragmentUniforms(hasFragmentUniforms)
        , fHasStepUniforms(hasStepUniforms)
        , fNumTextureSamplers(numTextureSamplers)
        , fOwnsPipelineLayout(ownsPipelineLayout) {}

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
