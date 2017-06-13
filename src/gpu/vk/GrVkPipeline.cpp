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
#include "GrVkRenderTarget.h"
#include "GrVkUtil.h"

static inline VkFormat attrib_type_to_vkformat(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return VK_FORMAT_R32_SFLOAT;
        case kVec2f_GrVertexAttribType:
            return VK_FORMAT_R32G32_SFLOAT;
        case kVec3f_GrVertexAttribType:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case kVec4f_GrVertexAttribType:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case kVec2i_GrVertexAttribType:
            return VK_FORMAT_R32G32_SINT;
        case kVec3i_GrVertexAttribType:
            return VK_FORMAT_R32G32B32_SINT;
        case kVec4i_GrVertexAttribType:
            return VK_FORMAT_R32G32B32A32_SINT;
        case kUByte_GrVertexAttribType:
            return VK_FORMAT_R8_UNORM;
        case kVec4ub_GrVertexAttribType:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case kVec2us_GrVertexAttribType:
            return VK_FORMAT_R16G16_UNORM;
        case kInt_GrVertexAttribType:
            return VK_FORMAT_R32_SINT;
        case kUint_GrVertexAttribType:
            return VK_FORMAT_R32_UINT;
    }
    SkFAIL("Unknown vertex attrib type");
    return VK_FORMAT_UNDEFINED;
}

static void setup_vertex_input_state(const GrPrimitiveProcessor& primProc,
                                  VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
                                  SkSTArray<2, VkVertexInputBindingDescription, true>* bindingDescs,
                                  VkVertexInputAttributeDescription* attributeDesc) {
    uint32_t vertexBinding, instanceBinding;

    if (primProc.hasVertexAttribs()) {
        vertexBinding = bindingDescs->count();
        bindingDescs->push_back() = {
            vertexBinding,
            (uint32_t) primProc.getVertexStride(),
            VK_VERTEX_INPUT_RATE_VERTEX
        };
    }

    if (primProc.hasInstanceAttribs()) {
        instanceBinding = bindingDescs->count();
        bindingDescs->push_back() = {
            instanceBinding,
            (uint32_t) primProc.getInstanceStride(),
            VK_VERTEX_INPUT_RATE_INSTANCE
        };
    }

    // setup attribute descriptions
    int vaCount = primProc.numAttribs();
    if (vaCount > 0) {
        for (int attribIndex = 0; attribIndex < vaCount; attribIndex++) {
            using InputRate = GrPrimitiveProcessor::Attribute::InputRate;
            const GrGeometryProcessor::Attribute& attrib = primProc.getAttrib(attribIndex);
            VkVertexInputAttributeDescription& vkAttrib = attributeDesc[attribIndex];
            vkAttrib.location = attribIndex; // for now assume location = attribIndex
            vkAttrib.binding = InputRate::kPerInstance == attrib.fInputRate ? instanceBinding
                                                                            : vertexBinding;
            vkAttrib.format = attrib_type_to_vkformat(attrib.fType);
            vkAttrib.offset = attrib.fOffsetInRecord;
        }
    }

    memset(vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->pNext = nullptr;
    vertexInputInfo->flags = 0;
    vertexInputInfo->vertexBindingDescriptionCount = bindingDescs->count();
    vertexInputInfo->pVertexBindingDescriptions = bindingDescs->begin();
    vertexInputInfo->vertexAttributeDescriptionCount = vaCount;
    vertexInputInfo->pVertexAttributeDescriptions = attributeDesc;
}

static VkPrimitiveTopology gr_primitive_type_to_vk_topology(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case GrPrimitiveType::kTriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case GrPrimitiveType::kTriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case GrPrimitiveType::kPoints:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case GrPrimitiveType::kLines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case GrPrimitiveType::kLineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case GrPrimitiveType::kLinesAdjacency:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    }
    SkFAIL("invalid GrPrimitiveType");
    return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

static void setup_input_assembly_state(GrPrimitiveType primitiveType,
                                       VkPipelineInputAssemblyStateCreateInfo* inputAssemblyInfo) {
    memset(inputAssemblyInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    inputAssemblyInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyInfo->pNext = nullptr;
    inputAssemblyInfo->flags = 0;
    inputAssemblyInfo->primitiveRestartEnable = false;
    inputAssemblyInfo->topology = gr_primitive_type_to_vk_topology(primitiveType);
}


static VkStencilOp stencil_op_to_vk_stencil_op(GrStencilOp op) {
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
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kGrStencilOpCount);
    GR_STATIC_ASSERT(0 == (int)GrStencilOp::kKeep);
    GR_STATIC_ASSERT(1 == (int)GrStencilOp::kZero);
    GR_STATIC_ASSERT(2 == (int)GrStencilOp::kReplace);
    GR_STATIC_ASSERT(3 == (int)GrStencilOp::kInvert);
    GR_STATIC_ASSERT(4 == (int)GrStencilOp::kIncWrap);
    GR_STATIC_ASSERT(5 == (int)GrStencilOp::kDecWrap);
    GR_STATIC_ASSERT(6 == (int)GrStencilOp::kIncClamp);
    GR_STATIC_ASSERT(7 == (int)GrStencilOp::kDecClamp);
    SkASSERT(op < (GrStencilOp)kGrStencilOpCount);
    return gTable[(int)op];
}

static VkCompareOp stencil_func_to_vk_compare_op(GrStencilTest test) {
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
    GR_STATIC_ASSERT(SK_ARRAY_COUNT(gTable) == kGrStencilTestCount);
    GR_STATIC_ASSERT(0 == (int)GrStencilTest::kAlways);
    GR_STATIC_ASSERT(1 == (int)GrStencilTest::kNever);
    GR_STATIC_ASSERT(2 == (int)GrStencilTest::kGreater);
    GR_STATIC_ASSERT(3 == (int)GrStencilTest::kGEqual);
    GR_STATIC_ASSERT(4 == (int)GrStencilTest::kLess);
    GR_STATIC_ASSERT(5 == (int)GrStencilTest::kLEqual);
    GR_STATIC_ASSERT(6 == (int)GrStencilTest::kEqual);
    GR_STATIC_ASSERT(7 == (int)GrStencilTest::kNotEqual);
    SkASSERT(test < (GrStencilTest)kGrStencilTestCount);

    return gTable[(int)test];
}

static void setup_depth_stencil_state(const GrStencilSettings& stencilSettings,
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
        const GrStencilSettings::Face& front = stencilSettings.front();
        stencilInfo->front.failOp = stencil_op_to_vk_stencil_op(front.fFailOp);
        stencilInfo->front.passOp = stencil_op_to_vk_stencil_op(front.fPassOp);
        stencilInfo->front.depthFailOp = stencilInfo->front.failOp;
        stencilInfo->front.compareOp = stencil_func_to_vk_compare_op(front.fTest);
        stencilInfo->front.compareMask = front.fTestMask;
        stencilInfo->front.writeMask = front.fWriteMask;
        stencilInfo->front.reference = front.fRef;

        // Set back face
        if (!stencilSettings.isTwoSided()) {
            stencilInfo->back = stencilInfo->front;
        } else {
            const GrStencilSettings::Face& back = stencilSettings.back();
            stencilInfo->back.failOp = stencil_op_to_vk_stencil_op(back.fFailOp);
            stencilInfo->back.passOp = stencil_op_to_vk_stencil_op(back.fPassOp);
            stencilInfo->back.depthFailOp = stencilInfo->front.failOp;
            stencilInfo->back.compareOp = stencil_func_to_vk_compare_op(back.fTest);
            stencilInfo->back.compareMask = back.fTestMask;
            stencilInfo->back.writeMask = back.fWriteMask;
            stencilInfo->back.reference = back.fRef;
        }
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
    viewportInfo->pViewports = nullptr; // This is set dynamically

    viewportInfo->scissorCount = 1;
    viewportInfo->pScissors = nullptr; // This is set dynamically

    SkASSERT(viewportInfo->viewportCount == viewportInfo->scissorCount);
}

static void setup_multisample_state(const GrPipeline& pipeline,
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

static bool blend_coeff_refs_constant(GrBlendCoeff coeff) {
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

static void setup_color_blend_state(const GrPipeline& pipeline,
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

static void setup_raster_state(const GrPipeline& pipeline,
                               const GrCaps* caps,
                               VkPipelineRasterizationStateCreateInfo* rasterInfo) {
    memset(rasterInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo->pNext = nullptr;
    rasterInfo->flags = 0;
    rasterInfo->depthClampEnable = VK_FALSE;
    rasterInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterInfo->polygonMode = caps->wireframeMode() ? VK_POLYGON_MODE_LINE
                                                    : VK_POLYGON_MODE_FILL;
    rasterInfo->cullMode = VK_CULL_MODE_NONE;
    rasterInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterInfo->depthBiasEnable = VK_FALSE;
    rasterInfo->depthBiasConstantFactor = 0.0f;
    rasterInfo->depthBiasClamp = 0.0f;
    rasterInfo->depthBiasSlopeFactor = 0.0f;
    rasterInfo->lineWidth = 1.0f;
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

GrVkPipeline* GrVkPipeline::Create(GrVkGpu* gpu, const GrPipeline& pipeline,
                                   const GrStencilSettings& stencil,
                                   const GrPrimitiveProcessor& primProc,
                                   VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                   int shaderStageCount,
                                   GrPrimitiveType primitiveType,
                                   const GrVkRenderPass& renderPass,
                                   VkPipelineLayout layout,
                                   VkPipelineCache cache) {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    SkSTArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    SkSTArray<16, VkVertexInputAttributeDescription> attributeDesc;
    SkASSERT(primProc.numAttribs() <= gpu->vkCaps().maxVertexAttributes());
    VkVertexInputAttributeDescription* pAttribs = attributeDesc.push_back_n(primProc.numAttribs());
    setup_vertex_input_state(primProc, &vertexInputInfo, &bindingDescs, pAttribs);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(primitiveType, &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(stencil, &depthStencilInfo);

    VkPipelineViewportStateCreateInfo viewportInfo;
    setup_viewport_scissor_state(&viewportInfo);

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(pipeline, primProc, gpu->caps(), &multisampleInfo);

    // We will only have one color attachment per pipeline.
    VkPipelineColorBlendAttachmentState attachmentStates[1];
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    setup_color_blend_state(pipeline, &colorBlendInfo, attachmentStates);

    VkPipelineRasterizationStateCreateInfo rasterInfo;
    setup_raster_state(pipeline, gpu->caps(), &rasterInfo);

    VkDynamicState dynamicStates[3];
    VkPipelineDynamicStateCreateInfo dynamicInfo;
    setup_dynamic_state(&dynamicInfo, dynamicStates);

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

void GrVkPipeline::SetDynamicScissorRectState(GrVkGpu* gpu,
                                              GrVkCommandBuffer* cmdBuffer,
                                              const GrRenderTarget* renderTarget,
                                              SkIRect scissorRect) {
    if (!scissorRect.intersect(SkIRect::MakeWH(renderTarget->width(), renderTarget->height()))) {
        scissorRect.setEmpty();
    }

    VkRect2D scissor;
    scissor.offset.x = scissorRect.fLeft;
    scissor.extent.width = scissorRect.width();
    if (kTopLeft_GrSurfaceOrigin == renderTarget->origin()) {
        scissor.offset.y = scissorRect.fTop;
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == renderTarget->origin());
        scissor.offset.y = renderTarget->height() - scissorRect.fBottom;
    }
    scissor.extent.height = scissorRect.height();

    SkASSERT(scissor.offset.x >= 0);
    SkASSERT(scissor.offset.y >= 0);
    cmdBuffer->setScissor(gpu, 0, 1, &scissor);
}

void GrVkPipeline::SetDynamicViewportState(GrVkGpu* gpu,
                                           GrVkCommandBuffer* cmdBuffer,
                                           const GrRenderTarget* renderTarget) {
    // We always use one viewport the size of the RT
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = SkIntToScalar(renderTarget->width());
    viewport.height = SkIntToScalar(renderTarget->height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdBuffer->setViewport(gpu, 0, 1, &viewport);
}

void GrVkPipeline::SetDynamicBlendConstantState(GrVkGpu* gpu,
                                                GrVkCommandBuffer* cmdBuffer,
                                                GrPixelConfig pixelConfig,
                                                const GrXferProcessor& xferProcessor) {
    GrXferProcessor::BlendInfo blendInfo;
    xferProcessor.getBlendInfo(&blendInfo);
    GrBlendCoeff srcCoeff = blendInfo.fSrcBlend;
    GrBlendCoeff dstCoeff = blendInfo.fDstBlend;
    float floatColors[4];
    if (blend_coeff_refs_constant(srcCoeff) || blend_coeff_refs_constant(dstCoeff)) {
        // Swizzle the blend to match what the shader will output.
        const GrSwizzle& swizzle = gpu->caps()->shaderCaps()->configOutputSwizzle(pixelConfig);
        GrColor blendConst = swizzle.applyTo(blendInfo.fBlendConstant);
        GrColorToRGBAFloat(blendConst, floatColors);
    } else {
        memset(floatColors, 0, 4 * sizeof(float));
    }
    cmdBuffer->setBlendConstants(gpu, floatColors);
}
