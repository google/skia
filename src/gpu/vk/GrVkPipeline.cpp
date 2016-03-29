/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkPipeline.h"

#include "GrGeometryProcessor.h"
#include "GrPipeline.h"
#include "GrVkCommandBuffer.h"
#include "GrVkGpu.h"
#include "GrVkProgramDesc.h"
#include "GrVkRenderTarget.h"
#include "GrVkUtil.h"

static inline const VkFormat& attrib_type_to_vkformat(GrVertexAttribType type) {
    SkASSERT(type >= 0 && type < kGrVertexAttribTypeCount);
    static const VkFormat kFormats[kGrVertexAttribTypeCount] = {
        VK_FORMAT_R32_SFLOAT,          // kFloat_GrVertexAttribType
        VK_FORMAT_R32G32_SFLOAT,       // kVec2f_GrVertexAttribType
        VK_FORMAT_R32G32B32_SFLOAT,    // kVec3f_GrVertexAttribType
        VK_FORMAT_R32G32B32A32_SFLOAT, // kVec4f_GrVertexAttribType
        VK_FORMAT_R8_UNORM,            // kUByte_GrVertexAttribType
        VK_FORMAT_R8G8B8A8_UNORM,      // kVec4ub_GrVertexAttribType
        VK_FORMAT_R16G16_UNORM,        // kVec2us_GrVertexAttribType
    };
    GR_STATIC_ASSERT(0 == kFloat_GrVertexAttribType);
    GR_STATIC_ASSERT(1 == kVec2f_GrVertexAttribType);
    GR_STATIC_ASSERT(2 == kVec3f_GrVertexAttribType);
    GR_STATIC_ASSERT(3 == kVec4f_GrVertexAttribType);
    GR_STATIC_ASSERT(4 == kUByte_GrVertexAttribType);
    GR_STATIC_ASSERT(5 == kVec4ub_GrVertexAttribType);
    GR_STATIC_ASSERT(6 == kVec2us_GrVertexAttribType);
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(kFormats) == kGrVertexAttribTypeCount);
    return kFormats[type];
}

static void setup_vertex_input_state(const GrPrimitiveProcessor& primProc,
                                     VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
                                     VkVertexInputBindingDescription* bindingDesc,
                                     int maxBindingDescCount,
                                     VkVertexInputAttributeDescription* attributeDesc,
                                     int maxAttributeDescCount) {
    // for now we have only one vertex buffer and one binding
    memset(bindingDesc, 0, sizeof(VkVertexInputBindingDescription));
    bindingDesc->binding = 0;
    bindingDesc->stride = (uint32_t)primProc.getVertexStride();
    bindingDesc->inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // setup attribute descriptions
    int vaCount = primProc.numAttribs();
    SkASSERT(vaCount < maxAttributeDescCount);
    if (vaCount > 0) {
        size_t offset = 0;
        for (int attribIndex = 0; attribIndex < vaCount; attribIndex++) {
            const GrGeometryProcessor::Attribute& attrib = primProc.getAttrib(attribIndex);
            GrVertexAttribType attribType = attrib.fType;

            VkVertexInputAttributeDescription& vkAttrib = attributeDesc[attribIndex];
            vkAttrib.location = attribIndex; // for now assume location = attribIndex
            vkAttrib.binding = 0; // for now only one vertex buffer & binding
            vkAttrib.format = attrib_type_to_vkformat(attribType);
            vkAttrib.offset = static_cast<uint32_t>(offset);
            offset += attrib.fOffset;
        }
    }

    memset(vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->pNext = nullptr;
    vertexInputInfo->flags = 0;
    vertexInputInfo->vertexBindingDescriptionCount = 1;
    vertexInputInfo->pVertexBindingDescriptions = bindingDesc;
    vertexInputInfo->vertexAttributeDescriptionCount = vaCount;
    vertexInputInfo->pVertexAttributeDescriptions = attributeDesc;
}


static void setup_input_assembly_state(GrPrimitiveType primitiveType,
                                       VkPipelineInputAssemblyStateCreateInfo* inputAssemblyInfo) {
    static const VkPrimitiveTopology gPrimitiveType2VkTopology[] = {
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
        VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
        VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
    };

    memset(inputAssemblyInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    inputAssemblyInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->pNext = nullptr;
    inputAssemblyInfo->flags = 0;
    inputAssemblyInfo->primitiveRestartEnable = false;
    inputAssemblyInfo->topology = gPrimitiveType2VkTopology[primitiveType];
}


VkStencilOp stencil_op_to_vk_stencil_op(GrStencilOp op) {
    static const VkStencilOp gTable[] = {
        VK_STENCIL_OP_KEEP,                 // kKeep_StencilOp
        VK_STENCIL_OP_REPLACE,              // kReplace_StencilOp
        VK_STENCIL_OP_INCREMENT_AND_WRAP,   // kIncWrap_StencilOp
        VK_STENCIL_OP_INCREMENT_AND_CLAMP,  // kIncClamp_StencilOp
        VK_STENCIL_OP_DECREMENT_AND_WRAP,   // kDecWrap_StencilOp
        VK_STENCIL_OP_DECREMENT_AND_CLAMP,  // kDecClamp_StencilOp
        VK_STENCIL_OP_ZERO,                 // kZero_StencilOp
        VK_STENCIL_OP_INVERT,               // kInvert_StencilOp
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kStencilOpCount);
    GR_STATIC_ASSERT(0 == kKeep_StencilOp);
    GR_STATIC_ASSERT(1 == kReplace_StencilOp);
    GR_STATIC_ASSERT(2 == kIncWrap_StencilOp);
    GR_STATIC_ASSERT(3 == kIncClamp_StencilOp);
    GR_STATIC_ASSERT(4 == kDecWrap_StencilOp);
    GR_STATIC_ASSERT(5 == kDecClamp_StencilOp);
    GR_STATIC_ASSERT(6 == kZero_StencilOp);
    GR_STATIC_ASSERT(7 == kInvert_StencilOp);
    SkASSERT((unsigned)op < kStencilOpCount);
    return gTable[op];
}

VkCompareOp stencil_func_to_vk_compare_op(GrStencilFunc basicFunc) {
    static const VkCompareOp gTable[] = {
        VK_COMPARE_OP_ALWAYS,              // kAlways_StencilFunc
        VK_COMPARE_OP_NEVER,               // kNever_StencilFunc
        VK_COMPARE_OP_GREATER,             // kGreater_StencilFunc
        VK_COMPARE_OP_GREATER_OR_EQUAL,    // kGEqual_StencilFunc
        VK_COMPARE_OP_LESS,                // kLess_StencilFunc
        VK_COMPARE_OP_LESS_OR_EQUAL,       // kLEqual_StencilFunc,
        VK_COMPARE_OP_EQUAL,               // kEqual_StencilFunc,
        VK_COMPARE_OP_NOT_EQUAL,           // kNotEqual_StencilFunc,
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kBasicStencilFuncCount);
    GR_STATIC_ASSERT(0 == kAlways_StencilFunc);
    GR_STATIC_ASSERT(1 == kNever_StencilFunc);
    GR_STATIC_ASSERT(2 == kGreater_StencilFunc);
    GR_STATIC_ASSERT(3 == kGEqual_StencilFunc);
    GR_STATIC_ASSERT(4 == kLess_StencilFunc);
    GR_STATIC_ASSERT(5 == kLEqual_StencilFunc);
    GR_STATIC_ASSERT(6 == kEqual_StencilFunc);
    GR_STATIC_ASSERT(7 == kNotEqual_StencilFunc);
    SkASSERT((unsigned)basicFunc < kBasicStencilFuncCount);

    return gTable[basicFunc];
}

void setup_depth_stencil_state(const GrVkGpu* gpu,
                               const GrStencilSettings& stencilSettings,
                               VkPipelineDepthStencilStateCreateInfo* stencilInfo) {
    memset(stencilInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    stencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    stencilInfo->pNext = nullptr;
    stencilInfo->flags = 0;
    // set depth testing defaults
    stencilInfo->depthTestEnable = VK_FALSE;
    stencilInfo->depthWriteEnable = VK_FALSE;
    stencilInfo->depthCompareOp = VK_COMPARE_OP_ALWAYS;
    stencilInfo->depthBoundsTestEnable = VK_FALSE;
    stencilInfo->stencilTestEnable = !stencilSettings.isDisabled();
    if (!stencilSettings.isDisabled()) {
        // Set front face
        GrStencilSettings::Face face = GrStencilSettings::kFront_Face;
        stencilInfo->front.failOp = stencil_op_to_vk_stencil_op(stencilSettings.failOp(face));
        stencilInfo->front.passOp = stencil_op_to_vk_stencil_op(stencilSettings.passOp(face));
        stencilInfo->front.depthFailOp = stencilInfo->front.failOp;
        stencilInfo->front.compareOp = stencil_func_to_vk_compare_op(stencilSettings.func(face));
        stencilInfo->front.compareMask = stencilSettings.funcMask(face);
        stencilInfo->front.writeMask = stencilSettings.writeMask(face);
        stencilInfo->front.reference = stencilSettings.funcRef(face);

        // Set back face
        face = GrStencilSettings::kBack_Face;
        stencilInfo->back.failOp = stencil_op_to_vk_stencil_op(stencilSettings.failOp(face));
        stencilInfo->back.passOp = stencil_op_to_vk_stencil_op(stencilSettings.passOp(face));
        stencilInfo->back.depthFailOp = stencilInfo->front.failOp;
        stencilInfo->back.compareOp = stencil_func_to_vk_compare_op(stencilSettings.func(face));
        stencilInfo->back.compareMask = stencilSettings.funcMask(face);
        stencilInfo->back.writeMask = stencilSettings.writeMask(face);
        stencilInfo->back.reference = stencilSettings.funcRef(face);
    }
    stencilInfo->minDepthBounds = 0.0f;
    stencilInfo->maxDepthBounds = 1.0f;
}

void setup_viewport_scissor_state(const GrVkGpu* gpu,
                                  const GrPipeline& pipeline,
                                  const GrVkRenderTarget* vkRT,
                                  VkPipelineViewportStateCreateInfo* viewportInfo) {
    memset(viewportInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    viewportInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo->pNext = nullptr;
    viewportInfo->flags = 0;

    viewportInfo->viewportCount = 1;
    viewportInfo->pViewports = nullptr; // This is set dynamically

    viewportInfo->scissorCount = 1;
    viewportInfo->pScissors = nullptr; // This is set dynamically

    SkASSERT(viewportInfo->viewportCount == viewportInfo->scissorCount);
}

void setup_multisample_state(const GrPipeline& pipeline,
                             const GrPrimitiveProcessor& primProc,
                             const GrCaps* caps,
                             VkPipelineMultisampleStateCreateInfo* multisampleInfo) {
    memset(multisampleInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampleInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo->pNext = nullptr;
    multisampleInfo->flags = 0;
    int numSamples = pipeline.getRenderTarget()->numColorSamples();
    SkAssertResult(GrSampleCountToVkSampleCount(numSamples,
                   &multisampleInfo->rasterizationSamples));
    float sampleShading = primProc.getSampleShading();
    SkASSERT(sampleShading == 0.0f || caps->sampleShadingSupport());
    multisampleInfo->sampleShadingEnable = sampleShading > 0.0f;
    multisampleInfo->minSampleShading = sampleShading;
    multisampleInfo->pSampleMask = nullptr;
    multisampleInfo->alphaToCoverageEnable = VK_FALSE;
    multisampleInfo->alphaToOneEnable = VK_FALSE;
}

static VkBlendFactor blend_coeff_to_vk_blend(GrBlendCoeff coeff) {
    static const VkBlendFactor gTable[] = {
        VK_BLEND_FACTOR_ZERO,                      // kZero_GrBlendCoeff
        VK_BLEND_FACTOR_ONE,                       // kOne_GrBlendCoeff
        VK_BLEND_FACTOR_SRC_COLOR,                 // kSC_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,       // kISC_GrBlendCoeff
        VK_BLEND_FACTOR_DST_COLOR,                 // kDC_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,       // kIDC_GrBlendCoeff
        VK_BLEND_FACTOR_SRC_ALPHA,                 // kSA_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,       // kISA_GrBlendCoeff
        VK_BLEND_FACTOR_DST_ALPHA,                 // kDA_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,       // kIDA_GrBlendCoeff
        VK_BLEND_FACTOR_CONSTANT_COLOR,            // kConstC_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,  // kIConstC_GrBlendCoeff
        VK_BLEND_FACTOR_CONSTANT_ALPHA,            // kConstA_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,  // kIConstA_GrBlendCoeff
        VK_BLEND_FACTOR_SRC1_COLOR,                // kS2C_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,      // kIS2C_GrBlendCoeff
        VK_BLEND_FACTOR_SRC1_ALPHA,                // kS2A_GrBlendCoeff
        VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,      // kIS2A_GrBlendCoeff

    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kGrBlendCoeffCnt);
    GR_STATIC_ASSERT(0 == kZero_GrBlendCoeff);
    GR_STATIC_ASSERT(1 == kOne_GrBlendCoeff);
    GR_STATIC_ASSERT(2 == kSC_GrBlendCoeff);
    GR_STATIC_ASSERT(3 == kISC_GrBlendCoeff);
    GR_STATIC_ASSERT(4 == kDC_GrBlendCoeff);
    GR_STATIC_ASSERT(5 == kIDC_GrBlendCoeff);
    GR_STATIC_ASSERT(6 == kSA_GrBlendCoeff);
    GR_STATIC_ASSERT(7 == kISA_GrBlendCoeff);
    GR_STATIC_ASSERT(8 == kDA_GrBlendCoeff);
    GR_STATIC_ASSERT(9 == kIDA_GrBlendCoeff);
    GR_STATIC_ASSERT(10 == kConstC_GrBlendCoeff);
    GR_STATIC_ASSERT(11 == kIConstC_GrBlendCoeff);
    GR_STATIC_ASSERT(12 == kConstA_GrBlendCoeff);
    GR_STATIC_ASSERT(13 == kIConstA_GrBlendCoeff);
    GR_STATIC_ASSERT(14 == kS2C_GrBlendCoeff);
    GR_STATIC_ASSERT(15 == kIS2C_GrBlendCoeff);
    GR_STATIC_ASSERT(16 == kS2A_GrBlendCoeff);
    GR_STATIC_ASSERT(17 == kIS2A_GrBlendCoeff);

    SkASSERT((unsigned)coeff < kGrBlendCoeffCnt);
    return gTable[coeff];
}


static VkBlendOp blend_equation_to_vk_blend_op(GrBlendEquation equation) {
    static const VkBlendOp gTable[] = {
        VK_BLEND_OP_ADD,               // kAdd_GrBlendEquation
        VK_BLEND_OP_SUBTRACT,          // kSubtract_GrBlendEquation
        VK_BLEND_OP_REVERSE_SUBTRACT,  // kReverseSubtract_GrBlendEquation
    };
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kFirstAdvancedGrBlendEquation);
    GR_STATIC_ASSERT(0 == kAdd_GrBlendEquation);
    GR_STATIC_ASSERT(1 == kSubtract_GrBlendEquation);
    GR_STATIC_ASSERT(2 == kReverseSubtract_GrBlendEquation);

    SkASSERT((unsigned)equation < kGrBlendCoeffCnt);
    return gTable[equation];
}

bool blend_coeff_refs_constant(GrBlendCoeff coeff) {
    static const bool gCoeffReferencesBlendConst[] = {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        true,
        true,
        true,
        true,

        // extended blend coeffs
        false,
        false,
        false,
        false,
    };
    return gCoeffReferencesBlendConst[coeff];
    GR_STATIC_ASSERT(kGrBlendCoeffCnt == SK_ARRAY_COUNT(gCoeffReferencesBlendConst));
    // Individual enum asserts already made in blend_coeff_to_vk_blend
}

void setup_color_blend_state(const GrVkGpu* gpu,
                             const GrPipeline& pipeline,
                             VkPipelineColorBlendStateCreateInfo* colorBlendInfo,
                             VkPipelineColorBlendAttachmentState* attachmentState) {
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);

    GrBlendEquation equation = blendInfo.fEquation;
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    bool blendOff = (kAdd_GrBlendEquation == equation || kSubtract_GrBlendEquation == equation) &&
                    kOne_GrBlendCoeff == srcCoeff && kZero_GrBlendCoeff == dstCoeff;

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

    if (!blendInfo.fWriteColor) {
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

VkCullModeFlags draw_face_to_vk_cull_mode(GrPipelineBuilder::DrawFace drawFace) {
    // Assumes that we've set the front face to be ccw
    static const VkCullModeFlags gTable[] = {
        VK_CULL_MODE_NONE,              // kBoth_DrawFace
        VK_CULL_MODE_BACK_BIT,          // kCCW_DrawFace, cull back face
        VK_CULL_MODE_FRONT_BIT,         // kCW_DrawFace, cull front face
    };
    GR_STATIC_ASSERT(0 == GrPipelineBuilder::kBoth_DrawFace);
    GR_STATIC_ASSERT(1 == GrPipelineBuilder::kCCW_DrawFace);
    GR_STATIC_ASSERT(2 == GrPipelineBuilder::kCW_DrawFace);
    SkASSERT((unsigned)drawFace <= 2);

    return gTable[drawFace];
}

void setup_raster_state(const GrVkGpu* gpu,
    const GrPipeline& pipeline,
    VkPipelineRasterizationStateCreateInfo* rasterInfo) {
    memset(rasterInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo->pNext = nullptr;
    rasterInfo->flags = 0;
    rasterInfo->depthClampEnable = VK_FALSE;
    rasterInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterInfo->polygonMode = VK_POLYGON_MODE_FILL;
    rasterInfo->cullMode = draw_face_to_vk_cull_mode(pipeline.getDrawFace());
    rasterInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterInfo->depthBiasEnable = VK_FALSE;
    rasterInfo->depthBiasConstantFactor = 0.0f;
    rasterInfo->depthBiasClamp = 0.0f;
    rasterInfo->depthBiasSlopeFactor = 0.0f;
    rasterInfo->lineWidth = 1.0f;
}

void setup_dynamic_state(const GrVkGpu* gpu,
                         const GrPipeline& pipeline,
                         VkPipelineDynamicStateCreateInfo* dynamicInfo,
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

GrVkPipeline* GrVkPipeline::Create(GrVkGpu* gpu, const GrPipeline& pipeline,
                                   const GrPrimitiveProcessor& primProc,
                                   VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                   int shaderStageCount,
                                   GrPrimitiveType primitiveType,
                                   const GrVkRenderPass& renderPass,
                                   VkPipelineLayout layout,
                                   VkPipelineCache cache) {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    VkVertexInputBindingDescription bindingDesc;
    // TODO: allocate this based on VkPhysicalDeviceLimits::maxVertexInputAttributes
    static const int kMaxVertexAttributes = 16;
    static VkVertexInputAttributeDescription attributeDesc[kMaxVertexAttributes];
    setup_vertex_input_state(primProc, &vertexInputInfo, &bindingDesc, 1,
                             attributeDesc, kMaxVertexAttributes);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(primitiveType, &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(gpu, pipeline.getStencil(), &depthStencilInfo);

    GrRenderTarget* rt = pipeline.getRenderTarget();
    GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);
    VkPipelineViewportStateCreateInfo viewportInfo;
    setup_viewport_scissor_state(gpu, pipeline, vkRT, &viewportInfo);

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(pipeline, primProc, gpu->caps(), &multisampleInfo);

    // We will only have one color attachment per pipeline.
    VkPipelineColorBlendAttachmentState attachmentStates[1];
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    setup_color_blend_state(gpu, pipeline, &colorBlendInfo, attachmentStates);

    VkPipelineRasterizationStateCreateInfo rasterInfo;
    setup_raster_state(gpu, pipeline, &rasterInfo);

    VkDynamicState dynamicStates[3];
    VkPipelineDynamicStateCreateInfo dynamicInfo;
    setup_dynamic_state(gpu, pipeline, &dynamicInfo, dynamicStates);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    memset(&pipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = shaderStageCount;
    pipelineCreateInfo.pStages = shaderStageInfo;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = &viewportInfo;
    pipelineCreateInfo.pRasterizationState = &rasterInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    pipelineCreateInfo.pDepthStencilState = &depthStencilInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    pipelineCreateInfo.pDynamicState = &dynamicInfo;
    pipelineCreateInfo.layout = layout;
    pipelineCreateInfo.renderPass = renderPass.vkRenderPass();
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline vkPipeline;
    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateGraphicsPipelines(gpu->device(),
                                                                          cache, 1,
                                                                          &pipelineCreateInfo,
                                                                          nullptr, &vkPipeline));
    if (err) {
        return nullptr;
    }

    return new GrVkPipeline(vkPipeline);
}

void GrVkPipeline::freeGPUData(const GrVkGpu* gpu) const {
    GR_VK_CALL(gpu->vkInterface(), DestroyPipeline(gpu->device(), fPipeline, nullptr));
}

void set_dynamic_scissor_state(GrVkGpu* gpu,
                               GrVkCommandBuffer* cmdBuffer,
                               const GrPipeline& pipeline,
                               const GrRenderTarget& target) {
    // We always use one scissor and if it is disabled we just make it the size of the RT
    const GrScissorState& scissorState = pipeline.getScissorState();
    VkRect2D scissor;
    if (scissorState.enabled() &&
        !scissorState.rect().contains(0, 0, target.width(), target.height())) {
        // This all assumes the scissorState has previously been clipped to the device space render
        // target.
        scissor.offset.x = scissorState.rect().fLeft;
        scissor.extent.width = scissorState.rect().width();
        if (kTopLeft_GrSurfaceOrigin == target.origin()) {
            scissor.offset.y = scissorState.rect().fTop;
        } else {
            SkASSERT(kBottomLeft_GrSurfaceOrigin == target.origin());
            scissor.offset.y = target.height() - scissorState.rect().fBottom;
        }
        scissor.extent.height = scissorState.rect().height();

        SkASSERT(scissor.offset.x >= 0);
        SkASSERT(scissor.offset.x + scissor.extent.width <= (uint32_t)target.width());
        SkASSERT(scissor.offset.y >= 0);
        SkASSERT(scissor.offset.y + scissor.extent.height <= (uint32_t)target.height());
    } else {
        scissor.extent.width = target.width();
        scissor.extent.height = target.height();
        scissor.offset.x = 0;
        scissor.offset.y = 0;
    }
    cmdBuffer->setScissor(gpu, 0, 1, &scissor);
}

void set_dynamic_viewport_state(GrVkGpu* gpu,
                                GrVkCommandBuffer* cmdBuffer,
                                const GrRenderTarget& target) {
    // We always use one viewport the size of the RT
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = SkIntToScalar(target.width());
    viewport.height = SkIntToScalar(target.height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdBuffer->setViewport(gpu, 0, 1, &viewport);
}

void set_dynamic_blend_constant_state(GrVkGpu* gpu,
                                     GrVkCommandBuffer* cmdBuffer,
                                     const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    float floatColors[4];
    if (blend_coeff_refs_constant(srcCoeff) || blend_coeff_refs_constant(dstCoeff)) {
        GrColorToRGBAFloat(blendInfo.fBlendConstant, floatColors);
    } else {
        memset(floatColors, 0, 4 * sizeof(float));
    }
    cmdBuffer->setBlendConstants(gpu, floatColors);
}

void GrVkPipeline::SetDynamicState(GrVkGpu* gpu,
                                   GrVkCommandBuffer* cmdBuffer,
                                   const GrPipeline& pipeline) {
    const GrRenderTarget& target = *pipeline.getRenderTarget();
    set_dynamic_scissor_state(gpu, cmdBuffer, pipeline, target);
    set_dynamic_viewport_state(gpu, cmdBuffer, target);
    set_dynamic_blend_constant_state(gpu, cmdBuffer, pipeline);
}
