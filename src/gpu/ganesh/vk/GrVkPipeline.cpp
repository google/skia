/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/vk/GrVkPipeline.h"

#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/vk/GrVkCommandBuffer.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkRenderTarget.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#include <sanitizer/lsan_interface.h>
#endif

static inline VkFormat attrib_type_to_vkformat(GrVertexAttribType type) {
    switch (type) {
        case kFloat_GrVertexAttribType:
            return VK_FORMAT_R32_SFLOAT;
        case kFloat2_GrVertexAttribType:
            return VK_FORMAT_R32G32_SFLOAT;
        case kFloat3_GrVertexAttribType:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case kFloat4_GrVertexAttribType:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case kHalf_GrVertexAttribType:
            return VK_FORMAT_R16_SFLOAT;
        case kHalf2_GrVertexAttribType:
            return VK_FORMAT_R16G16_SFLOAT;
        case kHalf4_GrVertexAttribType:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case kInt2_GrVertexAttribType:
            return VK_FORMAT_R32G32_SINT;
        case kInt3_GrVertexAttribType:
            return VK_FORMAT_R32G32B32_SINT;
        case kInt4_GrVertexAttribType:
            return VK_FORMAT_R32G32B32A32_SINT;
        case kByte_GrVertexAttribType:
            return VK_FORMAT_R8_SINT;
        case kByte2_GrVertexAttribType:
            return VK_FORMAT_R8G8_SINT;
        case kByte4_GrVertexAttribType:
            return VK_FORMAT_R8G8B8A8_SINT;
        case kUByte_GrVertexAttribType:
            return VK_FORMAT_R8_UINT;
        case kUByte2_GrVertexAttribType:
            return VK_FORMAT_R8G8_UINT;
        case kUByte4_GrVertexAttribType:
            return VK_FORMAT_R8G8B8A8_UINT;
        case kUByte_norm_GrVertexAttribType:
            return VK_FORMAT_R8_UNORM;
        case kUByte4_norm_GrVertexAttribType:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case kShort2_GrVertexAttribType:
            return VK_FORMAT_R16G16_SINT;
        case kShort4_GrVertexAttribType:
            return VK_FORMAT_R16G16B16A16_SINT;
        case kUShort2_GrVertexAttribType:
            return VK_FORMAT_R16G16_UINT;
        case kUShort2_norm_GrVertexAttribType:
            return VK_FORMAT_R16G16_UNORM;
        case kInt_GrVertexAttribType:
            return VK_FORMAT_R32_SINT;
        case kUInt_GrVertexAttribType:
            return VK_FORMAT_R32_UINT;
        case kUShort_norm_GrVertexAttribType:
            return VK_FORMAT_R16_UNORM;
        case kUShort4_norm_GrVertexAttribType:
            return VK_FORMAT_R16G16B16A16_UNORM;
    }
    SK_ABORT("Unknown vertex attrib type");
}

static void setup_vertex_input_state(
        const GrGeometryProcessor::AttributeSet& vertexAttribs,
        const GrGeometryProcessor::AttributeSet& instanceAttribs,
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
        SkSTArray<2, VkVertexInputBindingDescription, true>* bindingDescs,
        VkVertexInputAttributeDescription* attributeDesc) {
    int vaCount = vertexAttribs.count();
    int iaCount = instanceAttribs.count();

    uint32_t vertexBinding = 0, instanceBinding = 0;

    int nextBinding = bindingDescs->size();
    if (vaCount) {
        vertexBinding = nextBinding++;
    }

    if (iaCount) {
        instanceBinding = nextBinding;
    }

    // setup attribute descriptions
    int attribIndex = 0;
    for (auto attrib : vertexAttribs) {
        VkVertexInputAttributeDescription& vkAttrib = attributeDesc[attribIndex];
        vkAttrib.location = attribIndex++;  // for now assume location = attribIndex
        vkAttrib.binding = vertexBinding;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = *attrib.offset();
    }

    for (auto attrib : instanceAttribs) {
        VkVertexInputAttributeDescription& vkAttrib = attributeDesc[attribIndex];
        vkAttrib.location = attribIndex++;  // for now assume location = attribIndex
        vkAttrib.binding = instanceBinding;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = *attrib.offset();
    }

    if (vaCount) {
        bindingDescs->push_back() = {
                vertexBinding,
                (uint32_t) vertexAttribs.stride(),
                VK_VERTEX_INPUT_RATE_VERTEX
        };
    }
    if (iaCount) {
        bindingDescs->push_back() = {
                instanceBinding,
                (uint32_t) instanceAttribs.stride(),
                VK_VERTEX_INPUT_RATE_INSTANCE
        };
    }

    memset(vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->pNext = nullptr;
    vertexInputInfo->flags = 0;
    vertexInputInfo->vertexBindingDescriptionCount = bindingDescs->size();
    vertexInputInfo->pVertexBindingDescriptions = bindingDescs->begin();
    vertexInputInfo->vertexAttributeDescriptionCount = vaCount + iaCount;
    vertexInputInfo->pVertexAttributeDescriptions = attributeDesc;
}

static VkPrimitiveTopology gr_primitive_type_to_vk_topology(GrPrimitiveType primitiveType) {
    switch (primitiveType) {
        case GrPrimitiveType::kTriangles:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case GrPrimitiveType::kTriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case GrPrimitiveType::kPoints:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case GrPrimitiveType::kLines:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case GrPrimitiveType::kLineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    }
    SkUNREACHABLE;
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
    static_assert(std::size(gTable) == kGrStencilOpCount);
    static_assert(0 == (int)GrStencilOp::kKeep);
    static_assert(1 == (int)GrStencilOp::kZero);
    static_assert(2 == (int)GrStencilOp::kReplace);
    static_assert(3 == (int)GrStencilOp::kInvert);
    static_assert(4 == (int)GrStencilOp::kIncWrap);
    static_assert(5 == (int)GrStencilOp::kDecWrap);
    static_assert(6 == (int)GrStencilOp::kIncClamp);
    static_assert(7 == (int)GrStencilOp::kDecClamp);
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
    static_assert(std::size(gTable) == kGrStencilTestCount);
    static_assert(0 == (int)GrStencilTest::kAlways);
    static_assert(1 == (int)GrStencilTest::kNever);
    static_assert(2 == (int)GrStencilTest::kGreater);
    static_assert(3 == (int)GrStencilTest::kGEqual);
    static_assert(4 == (int)GrStencilTest::kLess);
    static_assert(5 == (int)GrStencilTest::kLEqual);
    static_assert(6 == (int)GrStencilTest::kEqual);
    static_assert(7 == (int)GrStencilTest::kNotEqual);
    SkASSERT(test < (GrStencilTest)kGrStencilTestCount);

    return gTable[(int)test];
}

static void setup_stencil_op_state(
        VkStencilOpState* opState, const GrStencilSettings::Face& stencilFace) {
    opState->failOp = stencil_op_to_vk_stencil_op(stencilFace.fFailOp);
    opState->passOp = stencil_op_to_vk_stencil_op(stencilFace.fPassOp);
    opState->depthFailOp = opState->failOp;
    opState->compareOp = stencil_func_to_vk_compare_op(stencilFace.fTest);
    opState->compareMask = stencilFace.fTestMask;
    opState->writeMask = stencilFace.fWriteMask;
    opState->reference = stencilFace.fRef;
}

static void setup_depth_stencil_state(
        const GrStencilSettings& stencilSettings,
        GrSurfaceOrigin origin,
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
        if (!stencilSettings.isTwoSided()) {
            setup_stencil_op_state(&stencilInfo->front, stencilSettings.singleSidedFace());
            stencilInfo->back = stencilInfo->front;
        } else {
            setup_stencil_op_state(&stencilInfo->front, stencilSettings.postOriginCCWFace(origin));
            setup_stencil_op_state(&stencilInfo->back, stencilSettings.postOriginCWFace(origin));
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

static void setup_multisample_state(int numSamples,
                                    const GrCaps* caps,
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
                               const GrCaps* caps,
                               VkPipelineRasterizationStateCreateInfo* rasterInfo) {
    memset(rasterInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    rasterInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterInfo->pNext = nullptr;
    rasterInfo->flags = 0;
    rasterInfo->depthClampEnable = VK_FALSE;
    rasterInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterInfo->polygonMode = (caps->wireframeMode() || isWireframe) ?
            VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    rasterInfo->cullMode = VK_CULL_MODE_NONE;
    rasterInfo->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterInfo->depthBiasEnable = VK_FALSE;
    rasterInfo->depthBiasConstantFactor = 0.0f;
    rasterInfo->depthBiasClamp = 0.0f;
    rasterInfo->depthBiasSlopeFactor = 0.0f;
    rasterInfo->lineWidth = 1.0f;
}

static void setup_conservative_raster_info(
        VkPipelineRasterizationConservativeStateCreateInfoEXT* conservativeRasterInfo) {
    memset(conservativeRasterInfo, 0,
           sizeof(VkPipelineRasterizationConservativeStateCreateInfoEXT));
    conservativeRasterInfo->sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT;
    conservativeRasterInfo->pNext = nullptr;
    conservativeRasterInfo->flags = 0;
    conservativeRasterInfo->conservativeRasterizationMode =
            VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT;
    conservativeRasterInfo->extraPrimitiveOverestimationSize = 0;
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

sk_sp<GrVkPipeline> GrVkPipeline::Make(GrVkGpu* gpu,
                                   const GrGeometryProcessor::AttributeSet& vertexAttribs,
                                   const GrGeometryProcessor::AttributeSet& instanceAttribs,
                                   GrPrimitiveType primitiveType,
                                   GrSurfaceOrigin origin,
                                   const GrStencilSettings& stencilSettings,
                                   int numSamples,
                                   bool isHWAntialiasState,
                                   const skgpu::BlendInfo& blendInfo,
                                   bool isWireframe,
                                   bool useConservativeRaster,
                                   uint32_t subpass,
                                   VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                   int shaderStageCount,
                                   VkRenderPass compatibleRenderPass,
                                   VkPipelineLayout layout,
                                   bool ownsLayout,
                                   VkPipelineCache cache) {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    SkSTArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    SkSTArray<16, VkVertexInputAttributeDescription> attributeDesc;
    int totalAttributeCnt = vertexAttribs.count() + instanceAttribs.count();
    SkASSERT(totalAttributeCnt <= gpu->vkCaps().maxVertexAttributes());
    VkVertexInputAttributeDescription* pAttribs = attributeDesc.push_back_n(totalAttributeCnt);
    setup_vertex_input_state(vertexAttribs, instanceAttribs, &vertexInputInfo, &bindingDescs,
                             pAttribs);

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    setup_input_assembly_state(primitiveType, &inputAssemblyInfo);

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    setup_depth_stencil_state(stencilSettings, origin, &depthStencilInfo);

    VkPipelineViewportStateCreateInfo viewportInfo;
    setup_viewport_scissor_state(&viewportInfo);

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(numSamples, gpu->caps(), &multisampleInfo);

    // We will only have one color attachment per pipeline.
    VkPipelineColorBlendAttachmentState attachmentStates[1];
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    setup_color_blend_state(blendInfo, &colorBlendInfo, attachmentStates);

    VkPipelineRasterizationStateCreateInfo rasterInfo;
    setup_raster_state(isWireframe, gpu->caps(), &rasterInfo);

    VkPipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterInfo;
    if (useConservativeRaster) {
        SkASSERT(gpu->caps()->conservativeRasterSupport());
        setup_conservative_raster_info(&conservativeRasterInfo);
        conservativeRasterInfo.pNext = rasterInfo.pNext;
        rasterInfo.pNext = &conservativeRasterInfo;
    }

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
    pipelineCreateInfo.renderPass = compatibleRenderPass;
    pipelineCreateInfo.subpass = subpass;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline vkPipeline;
    VkResult err;
    {
        TRACE_EVENT0_ALWAYS("skia.shaders", "CreateGraphicsPipeline");
#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skia:8712
        __lsan::ScopedDisabler lsanDisabler;
#endif
        GR_VK_CALL_RESULT(gpu, err, CreateGraphicsPipelines(gpu->device(), cache, 1,
                                                            &pipelineCreateInfo, nullptr,
                                                            &vkPipeline));
    }
    if (err) {
        SkDebugf("Failed to create pipeline. Error: %d\n", err);
        return nullptr;
    }

    if (!ownsLayout) {
        layout = VK_NULL_HANDLE;
    }
    return sk_sp<GrVkPipeline>(new GrVkPipeline(gpu, vkPipeline, layout));
}

sk_sp<GrVkPipeline> GrVkPipeline::Make(GrVkGpu* gpu,
                                       const GrProgramInfo& programInfo,
                                       VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                       int shaderStageCount,
                                       VkRenderPass compatibleRenderPass,
                                       VkPipelineLayout layout,
                                       VkPipelineCache cache,
                                       uint32_t subpass) {
    const GrGeometryProcessor& geomProc = programInfo.geomProc();
    const GrPipeline& pipeline = programInfo.pipeline();

    return Make(gpu,
                geomProc.vertexAttributes(),
                geomProc.instanceAttributes(),
                programInfo.primitiveType(),
                programInfo.origin(),
                programInfo.nonGLStencilSettings(),
                programInfo.numSamples(),
                programInfo.numSamples() > 1,
                pipeline.getXferProcessor().getBlendInfo(),
                pipeline.isWireframe(),
                pipeline.usesConservativeRaster(),
                subpass,
                shaderStageInfo,
                shaderStageCount,
                compatibleRenderPass,
                layout,
                /*ownsLayout=*/true,
                cache);
}

void GrVkPipeline::freeGPUData() const {
    GR_VK_CALL(fGpu->vkInterface(), DestroyPipeline(fGpu->device(), fPipeline, nullptr));
    if (fPipelineLayout != VK_NULL_HANDLE) {
        GR_VK_CALL(fGpu->vkInterface(),
                   DestroyPipelineLayout(fGpu->device(), fPipelineLayout, nullptr));
    }
}

void GrVkPipeline::SetDynamicScissorRectState(GrVkGpu* gpu,
                                              GrVkCommandBuffer* cmdBuffer,
                                              SkISize colorAttachmentDimensions,
                                              GrSurfaceOrigin rtOrigin,
                                              const SkIRect& scissorRect) {
    SkASSERT(scissorRect.isEmpty() ||
             SkIRect::MakeSize(colorAttachmentDimensions).contains(scissorRect));

    VkRect2D scissor;
    scissor.offset.x = scissorRect.fLeft;
    scissor.extent.width = scissorRect.width();
    if (kTopLeft_GrSurfaceOrigin == rtOrigin) {
        scissor.offset.y = scissorRect.fTop;
    } else {
        SkASSERT(kBottomLeft_GrSurfaceOrigin == rtOrigin);
        scissor.offset.y = colorAttachmentDimensions.height() - scissorRect.fBottom;
    }
    scissor.extent.height = scissorRect.height();

    SkASSERT(scissor.offset.x >= 0);
    SkASSERT(scissor.offset.y >= 0);
    cmdBuffer->setScissor(gpu, 0, 1, &scissor);
}

void GrVkPipeline::SetDynamicViewportState(GrVkGpu* gpu,
                                           GrVkCommandBuffer* cmdBuffer,
                                           SkISize colorAttachmentDimensions) {
    // We always use one viewport the size of the RT
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = SkIntToScalar(colorAttachmentDimensions.width());
    viewport.height = SkIntToScalar(colorAttachmentDimensions.height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdBuffer->setViewport(gpu, 0, 1, &viewport);
}

void GrVkPipeline::SetDynamicBlendConstantState(GrVkGpu* gpu,
                                                GrVkCommandBuffer* cmdBuffer,
                                                const skgpu::Swizzle& swizzle,
                                                const GrXferProcessor& xferProcessor) {
    const skgpu::BlendInfo& blendInfo = xferProcessor.getBlendInfo();
    skgpu::BlendCoeff srcCoeff = blendInfo.fSrcBlend;
    skgpu::BlendCoeff dstCoeff = blendInfo.fDstBlend;
    float floatColors[4];
    if (skgpu::BlendCoeffRefsConstant(srcCoeff) || skgpu::BlendCoeffRefsConstant(dstCoeff)) {
        // Swizzle the blend to match what the shader will output.
        SkPMColor4f blendConst = swizzle.applyTo(blendInfo.fBlendConstant);
        floatColors[0] = blendConst.fR;
        floatColors[1] = blendConst.fG;
        floatColors[2] = blendConst.fB;
        floatColors[3] = blendConst.fA;
        cmdBuffer->setBlendConstants(gpu, floatColors);
    }
}
