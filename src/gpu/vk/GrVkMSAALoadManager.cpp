/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkMSAALoadManager.h"

#include "src/core/SkTraceEvent.h"
#include "src/gpu/vk/GrVkCommandBuffer.h"
#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkImageView.h"
#include "src/gpu/vk/GrVkMSAALoadPipeline.h"
#include "src/gpu/vk/GrVkMeshBuffer.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkResourceProvider.h"
#include "src/gpu/vk/GrVkUniformBuffer.h"
#include "src/gpu/vk/GrVkUtil.h"

GrVkMSAALoadManager::GrVkMSAALoadManager()
        : fVertShaderModule(VK_NULL_HANDLE)
        , fFragShaderModule(VK_NULL_HANDLE)
        , fPipelineLayout(nullptr) {}

GrVkMSAALoadManager::~GrVkMSAALoadManager() {}

bool GrVkMSAALoadManager::createMSAALoadProgram(GrVkGpu* gpu) {
    TRACE_EVENT0("skia", TRACE_FUNC);

    SkSL::String vertShaderText;
    vertShaderText.append(
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"

            "layout(set = 0, binding = 0) uniform vertexUniformBuffer {"
            "half4 uPosXform;"
            "};"
            "layout(location = 0) in float2 inPosition;"

            "// MSAA Load Program VS\n"
            "void main() {"
            "sk_Position.xy = inPosition * uPosXform.xy + uPosXform.zw;"
            "sk_Position.zw = half2(0, 1);"
            "}");

    SkSL::String fragShaderText;
    fragShaderText.append(
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"

            "layout(input_attachment_index = 0, set = 2, binding = 0) uniform subpassInput uInput;"

            "// MSAA Load Program FS\n"
            "void main() {"
            "sk_FragColor = subpassLoad(uInput);"
            "}");

    SkSL::Program::Settings settings;
    SkSL::String spirv;
    SkSL::Program::Inputs inputs;
    if (!GrCompileVkShaderModule(gpu, vertShaderText, VK_SHADER_STAGE_VERTEX_BIT,
                                 &fVertShaderModule, &fShaderStageInfo[0], settings, &spirv,
                                 &inputs)) {
        this->destroyResources(gpu);
        return false;
    }
    SkASSERT(inputs.isEmpty());

    if (!GrCompileVkShaderModule(gpu, fragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 &fFragShaderModule, &fShaderStageInfo[1], settings, &spirv,
                                 &inputs)) {
        this->destroyResources(gpu);
        return false;
    }
    SkASSERT(inputs.isEmpty());

    VkDescriptorSetLayout dsLayout[GrVkUniformHandler::kDescSetCount];

    GrVkResourceProvider& resourceProvider = gpu->resourceProvider();

    dsLayout[GrVkUniformHandler::kUniformBufferDescSet] = resourceProvider.getUniformDSLayout();

    GrVkDescriptorSetManager::Handle samplerHandle;
    resourceProvider.getZeroSamplerDescriptorSetHandle(&samplerHandle);

    dsLayout[GrVkUniformHandler::kSamplerDescSet] =
            resourceProvider.getSamplerDSLayout(samplerHandle);

    dsLayout[GrVkUniformHandler::kInputDescSet] = resourceProvider.getInputDSLayout();

    // Create the VkPipelineLayout
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = 0;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.setLayoutCount = GrVkUniformHandler::kDescSetCount;
    layoutCreateInfo.pSetLayouts = dsLayout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    VkResult err = GR_VK_CALL(
            gpu->vkInterface(),
            CreatePipelineLayout(gpu->device(), &layoutCreateInfo, nullptr, &fPipelineLayout));
    if (err) {
        this->destroyResources(gpu);
        return false;
    }

    static const float vdata[] = {0, 0, 0, 1, 1, 0, 1, 1};
    fVertexBuffer = GrVkMeshBuffer::Make(gpu, GrGpuBufferType::kVertex, sizeof(vdata), true);
    SkASSERT(fVertexBuffer.get());
    fVertexBuffer->updateData(vdata, sizeof(vdata));

    // We use 2 float4's for uniforms
    fUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, 8 * sizeof(float)));
    SkASSERT(fUniformBuffer.get());

    return true;
}

bool GrVkMSAALoadManager::loadMSAAFromResolve(GrVkGpu* gpu,
                                              GrVkCommandBuffer* commandBuffer,
                                              const GrVkRenderPass& renderPass,
                                              GrSurface* dst,
                                              GrSurface* src,
                                              const SkIRect& rect) {
    GrVkRenderTarget* dstRt = static_cast<GrVkRenderTarget*>(dst->asRenderTarget());
    if (!dstRt) {
        return false;
    }

    GrVkRenderTarget* srcRT = static_cast<GrVkRenderTarget*>(src->asRenderTarget());
    if (!srcRT) {
        return false;
    }

    if (!srcRT->supportsInputAttachmentUsage()) {
        return false;
    }

    if (VK_NULL_HANDLE == fVertShaderModule) {
        SkASSERT(VK_NULL_HANDLE == fFragShaderModule && nullptr == fPipelineLayout &&
                 nullptr == fVertexBuffer.get() && nullptr == fUniformBuffer.get());
        if (!this->createMSAALoadProgram(gpu)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }
    SkASSERT(fPipelineLayout);

    GrVkResourceProvider& resourceProv = gpu->resourceProvider();

    GrVkMSAALoadPipeline* pipeline =
            resourceProv.findOrCreateMSAALoadPipeline(renderPass, dstRt, fShaderStageInfo,
                                                      fPipelineLayout);
    if (!pipeline) {
        return false;
    }
    commandBuffer->bindPipeline(gpu, pipeline);
    // Release ref to pipeline. The command buffer is holding a ref.
    pipeline->unref();

    // Set Dynamic viewport and stencil
    // We always use one viewport the size of the RT
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = SkIntToScalar(dst->width());
    viewport.height = SkIntToScalar(dst->height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer->setViewport(gpu, 0, 1, &viewport);

    // We assume the scissor is not enabled so just set it to the whole RT
    VkRect2D scissor;
    scissor.extent.width = dst->width();
    scissor.extent.height = dst->height();
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer->setScissor(gpu, 0, 1, &scissor);

    // Bind vertex buffer
    commandBuffer->bindInputBuffer(gpu, 0, fVertexBuffer);

    // Update and bind uniform descriptor set
    int w = rect.width();
    int h = rect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    float dx0 = 2.f * rect.fLeft / dw - 1.f;
    float dx1 = 2.f * (rect.fLeft + w) / dw - 1.f;
    float dy0 = 2.f * rect.fTop / dh - 1.f;
    float dy1 = 2.f * (rect.fTop + h) / dh - 1.f;

    float uniData[] = {dx1 - dx0, dy1 - dy0, dx0, dy0};  // posXform

    fUniformBuffer->updateData(gpu, uniData, sizeof(uniData), nullptr);
    static_assert(GrVkUniformHandler::kUniformBufferDescSet < GrVkUniformHandler::kInputDescSet);
    commandBuffer->bindDescriptorSets(gpu, fPipelineLayout,
                                      GrVkUniformHandler::kUniformBufferDescSet,
                                      /*setCount=*/1, fUniformBuffer->descriptorSet(),
                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);
    commandBuffer->addRecycledResource(fUniformBuffer->resource());

    // Update the input descriptor set
    const GrVkDescriptorSet* inputDS = srcRT->inputDescSet(gpu, /*forResolve=*/true);
    if (!inputDS) {
        return false;
    }
    commandBuffer->bindDescriptorSets(gpu, fPipelineLayout,
                                      GrVkUniformHandler::kInputDescSet, /*setCount=*/1,
                                      inputDS->descriptorSet(),
                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);

    // We don't need to add the srd and dst resources here since those are all tracked by the main
    // render pass code out in GrVkOpsRenderPass and GrVkRenderTarget::adResources.
    commandBuffer->addRecycledResource(inputDS);

    commandBuffer->draw(gpu, 4, 1, 0, 0);

    return true;
}

void GrVkMSAALoadManager::destroyResources(GrVkGpu* gpu) {
    if (fVertShaderModule != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyShaderModule(gpu->device(), fVertShaderModule, nullptr));
        fVertShaderModule = VK_NULL_HANDLE;
    }

    if (fFragShaderModule != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyShaderModule(gpu->device(), fFragShaderModule, nullptr));
        fFragShaderModule = VK_NULL_HANDLE;
    }

    if (fPipelineLayout != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyPipelineLayout(gpu->device(), fPipelineLayout, nullptr));
        fPipelineLayout = VK_NULL_HANDLE;
    }

    if (fUniformBuffer) {
        fUniformBuffer->release(gpu);
        fUniformBuffer.reset();
    }
}

