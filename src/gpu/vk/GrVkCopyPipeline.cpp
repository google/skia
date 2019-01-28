/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkCopyPipeline.h"

#include "GrVkGpu.h"
#include "GrVkUtil.h"
#include "SkOnce.h"

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#include <sanitizer/lsan_interface.h>
#endif

static void setup_multisample_state(int numSamples,
                                    VkPipelineMultisampleStateCreateInfo* multisampleInfo) {
    memset(multisampleInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    multisampleInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo->pNext = nullptr;
    multisampleInfo->flags = 0;
    SkAssertResult(GrSampleCountToVkSampleCount(numSamples,
                                                &multisampleInfo->rasterizationSamples));
    multisampleInfo->sampleShadingEnable = VK_FALSE;
    multisampleInfo->minSampleShading = 0.0f;
    multisampleInfo->pSampleMask = nullptr;
    multisampleInfo->alphaToCoverageEnable = VK_FALSE;
    multisampleInfo->alphaToOneEnable = VK_FALSE;
}

GrVkCopyPipeline* GrVkCopyPipeline::Create(GrVkGpu* gpu,
                                           VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                           VkPipelineLayout pipelineLayout,
                                           int numSamples,
                                           const GrVkRenderPass& renderPass,
                                           VkPipelineCache cache) {

    static const VkVertexInputAttributeDescription attributeDesc = {
        0,                        // location
        0,                        // binding
        VK_FORMAT_R32G32_SFLOAT,  // format
        0,                        // offset
    };

    static const VkVertexInputBindingDescription bindingDesc = {
        0,                           // binding
        2 * sizeof(float),           // stride
        VK_VERTEX_INPUT_RATE_VERTEX  // inputRate
    };

    static const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,  // sType
        nullptr,                                                    // pNext
        0,                                                          // flags
        1,                                                          // vertexBindingDescriptionCount
        &bindingDesc,                                               // pVertexBindingDescriptions
        1,                                                          // vertexAttributeDescriptionCnt
        &attributeDesc,                                             // pVertexAttributeDescriptions
    };

    static const VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,  // sType
        nullptr,                                                      // pNext
        0,                                                            // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,                         // topology
        VK_FALSE                                                      // primitiveRestartEnable
    };

    static const VkStencilOpState dummyStencilState = {
        VK_STENCIL_OP_KEEP,   // failOp
        VK_STENCIL_OP_KEEP,   // passOp
        VK_STENCIL_OP_KEEP,   // depthFailOp
        VK_COMPARE_OP_NEVER,  // compareOp
        0,                    // compareMask
        0,                    // writeMask
        0                     // reference
    };

    static const VkPipelineDepthStencilStateCreateInfo stencilInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,  // sType
        nullptr,                                                     // pNext
        0,                                                           // flags
        VK_FALSE,                                                    // depthTestEnable
        VK_FALSE,                                                    // depthWriteEnable
        VK_COMPARE_OP_ALWAYS,                                        // depthCompareOp
        VK_FALSE,                                                    // depthBoundsTestEnable
        VK_FALSE,                                                    // stencilTestEnable
        dummyStencilState,                                           // front
        dummyStencilState,                                           // bakc
        0.0f,                                                        // minDepthBounds
        1.0f                                                         // maxDepthBounds
    };

    static const VkPipelineViewportStateCreateInfo viewportInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,  // sType
        nullptr,                                                // pNext
        0,                                                      // flags
        1,                                                      // viewportCount
        nullptr,                                                // pViewports
        1,                                                      // scissorCount
        nullptr                                                 // pScissors
    };

    static const VkPipelineColorBlendAttachmentState attachmentState = {
        VK_FALSE,                                             // blendEnable
        VK_BLEND_FACTOR_ONE,                                  // srcColorBlendFactor
        VK_BLEND_FACTOR_ZERO,                                 // dstColorBlendFactor
        VK_BLEND_OP_ADD,                                      // colorBlendOp
        VK_BLEND_FACTOR_ONE,                                  // srcAlphaBlendFactor
        VK_BLEND_FACTOR_ZERO,                                 // dstAlphaBlendFactor
        VK_BLEND_OP_ADD,                                      // alphaBlendOp
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | // colorWriteMask
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT   // colorWriteMask
    };

    static const VkPipelineColorBlendStateCreateInfo colorBlendInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,  // sType
        nullptr,                                                   // pNext
        0,                                                         // flags
        VK_FALSE,                                                  // logicOpEnable
        VK_LOGIC_OP_CLEAR,                                         // logicOp
        1,                                                         // attachmentCount
        &attachmentState,                                          // pAttachments
        { 0.f, 0.f, 0.f, 0.f }                                       // blendConstants[4]
    };

    static const VkPipelineRasterizationStateCreateInfo rasterInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,  // sType
        nullptr,                                                     // pNext
        0,                                                           // flags
        VK_FALSE,                                                    // depthClampEnable
        VK_FALSE,                                                    // rasterizerDiscardEnabled
        VK_POLYGON_MODE_FILL,                                        // polygonMode
        VK_CULL_MODE_NONE,                                           // cullMode
        VK_FRONT_FACE_COUNTER_CLOCKWISE,                             // frontFace
        VK_FALSE,                                                    // depthBiasEnable
        0.0f,                                                        // depthBiasConstantFactor
        0.0f,                                                        // depthBiasClamp
        0.0f,                                                        // depthBiasSlopeFactor
        1.0f                                                         // lineWidth
    };

    static const VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR };
    static const VkPipelineDynamicStateCreateInfo dynamicInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,  // sType
        nullptr,                                               // pNext
        0,                                                     // flags
        2,                                                     // dynamicStateCount
        dynamicStates                                          // pDynamicStates
    };

    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    setup_multisample_state(numSamples, &multisampleInfo);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    memset(&pipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = nullptr;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStageInfo;
    pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineCreateInfo.pTessellationState = nullptr;
    pipelineCreateInfo.pViewportState = &viewportInfo;
    pipelineCreateInfo.pRasterizationState = &rasterInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleInfo;
    pipelineCreateInfo.pDepthStencilState = &stencilInfo;
    pipelineCreateInfo.pColorBlendState = &colorBlendInfo;
    pipelineCreateInfo.pDynamicState = &dynamicInfo;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass.vkRenderPass();
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex = -1;

    VkPipeline vkPipeline;
    VkResult err;
    {
#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skia:8712
        __lsan::ScopedDisabler lsanDisabler;
#endif
        err = GR_VK_CALL(gpu->vkInterface(), CreateGraphicsPipelines(gpu->device(),
                                                                     cache, 1,
                                                                     &pipelineCreateInfo,
                                                                     nullptr, &vkPipeline));
    }
    if (err) {
        SkDebugf("Failed to create copy pipeline. Error: %d\n", err);
        return nullptr;
    }

    return new GrVkCopyPipeline(vkPipeline, &renderPass);
}

bool GrVkCopyPipeline::isCompatible(const GrVkRenderPass& rp) const {
    return rp.isCompatible(*fRenderPass);
}
