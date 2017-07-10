/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#include "GrVkCopyManager.h"

#include "GrSamplerParams.h"
#include "GrShaderCaps.h"
#include "GrSurface.h"
#include "GrTexturePriv.h"
#include "GrVkCommandBuffer.h"
#include "GrVkCopyPipeline.h"
#include "GrVkDescriptorSet.h"
#include "GrVkGpu.h"
#include "GrVkImageView.h"
#include "GrVkRenderTarget.h"
#include "GrVkResourceProvider.h"
#include "GrVkSampler.h"
#include "GrVkTexture.h"
#include "GrVkUniformBuffer.h"
#include "GrVkVertexBuffer.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkTraceEvent.h"

GrVkCopyManager::GrVkCopyManager()
    : fVertShaderModule(VK_NULL_HANDLE)
    , fFragShaderModule(VK_NULL_HANDLE)
    , fPipelineLayout(VK_NULL_HANDLE) {}

GrVkCopyManager::~GrVkCopyManager() {}

bool GrVkCopyManager::createCopyProgram(GrVkGpu* gpu) {
    TRACE_EVENT0(TRACE_DISABLED_BY_DEFAULT("skia"), "GrVkCopyManager::createCopyProgram()");

    const GrShaderCaps* shaderCaps = gpu->caps()->shaderCaps();
    const char* version = shaderCaps->versionDeclString();
    SkString vertShaderText(version);
    vertShaderText.append(
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"

        "layout(set = 0, binding = 0) uniform vertexUniformBuffer {"
            "mediump vec4 uPosXform;"
            "mediump vec4 uTexCoordXform;"
        "};"
        "layout(location = 0) in highp vec2 inPosition;"
        "layout(location = 1) out mediump vec2 vTexCoord;"

        "// Copy Program VS\n"
        "void main() {"
            "vTexCoord = inPosition * uTexCoordXform.xy + uTexCoordXform.zw;"
            "gl_Position.xy = inPosition * uPosXform.xy + uPosXform.zw;"
            "gl_Position.zw = vec2(0, 1);"
        "}"
    );

    SkString fragShaderText(version);
    fragShaderText.append(
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"

        "precision mediump float;"

        "layout(set = 1, binding = 0) uniform mediump sampler2D uTextureSampler;"
        "layout(location = 1) in mediump vec2 vTexCoord;"
        "layout(location = 0, index = 0) out mediump vec4 fsColorOut;"

        "// Copy Program FS\n"
        "void main() {"
            "fsColorOut = texture(uTextureSampler, vTexCoord);"
        "}"
    );

    SkSL::Program::Settings settings;
    SkSL::Program::Inputs inputs;
    if (!GrCompileVkShaderModule(gpu, vertShaderText.c_str(), VK_SHADER_STAGE_VERTEX_BIT,
                                 &fVertShaderModule, &fShaderStageInfo[0], settings, &inputs)) {
        this->destroyResources(gpu);
        return false;
    }
    SkASSERT(inputs.isEmpty());

    if (!GrCompileVkShaderModule(gpu, fragShaderText.c_str(), VK_SHADER_STAGE_FRAGMENT_BIT,
                                 &fFragShaderModule, &fShaderStageInfo[1], settings, &inputs)) {
        this->destroyResources(gpu);
        return false;
    }
    SkASSERT(inputs.isEmpty());

    VkDescriptorSetLayout dsLayout[2];

    GrVkResourceProvider& resourceProvider = gpu->resourceProvider();

    dsLayout[GrVkUniformHandler::kUniformBufferDescSet] = resourceProvider.getUniformDSLayout();

    uint32_t samplerVisibility = kFragment_GrShaderFlag;
    SkTArray<uint32_t> visibilityArray(&samplerVisibility, 1);

    resourceProvider.getSamplerDescriptorSetHandle(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                   visibilityArray, &fSamplerDSHandle);
    dsLayout[GrVkUniformHandler::kSamplerDescSet] =
        resourceProvider.getSamplerDSLayout(fSamplerDSHandle);

    // Create the VkPipelineLayout
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = 0;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.setLayoutCount = 2;
    layoutCreateInfo.pSetLayouts = dsLayout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreatePipelineLayout(gpu->device(),
                                                                       &layoutCreateInfo,
                                                                       nullptr,
                                                                       &fPipelineLayout));
    if (err) {
        this->destroyResources(gpu);
        return false;
    }

    static const float vdata[] = {
        0, 0,
        0, 1,
        1, 0,
        1, 1
    };
    fVertexBuffer.reset(GrVkVertexBuffer::Create(gpu, sizeof(vdata), false));
    SkASSERT(fVertexBuffer.get());
    fVertexBuffer->updateData(vdata, sizeof(vdata));

    // We use 2 vec4's for uniforms
    fUniformBuffer.reset(GrVkUniformBuffer::Create(gpu, 8 * sizeof(float)));
    SkASSERT(fUniformBuffer.get());

    return true;
}

bool GrVkCopyManager::copySurfaceAsDraw(GrVkGpu* gpu,
                                        GrSurface* dst,
                                        GrSurface* src,
                                        const SkIRect& srcRect,
                                        const SkIPoint& dstPoint) {
    // None of our copy methods can handle a swizzle. TODO: Make copySurfaceAsDraw handle the
    // swizzle.
    if (gpu->caps()->shaderCaps()->configOutputSwizzle(src->config()) !=
        gpu->caps()->shaderCaps()->configOutputSwizzle(dst->config())) {
        return false;
    }

    if (!gpu->vkCaps().supportsCopiesAsDraws()) {
        return false;
    }

    if (gpu->vkCaps().newCBOnPipelineChange()) {
        // We bind a new pipeline here for the copy so we must start a new command buffer.
        gpu->finishFlush();
    }

    GrVkRenderTarget* rt = static_cast<GrVkRenderTarget*>(dst->asRenderTarget());
    if (!rt) {
        return false;
    }

    GrVkTexture* srcTex = static_cast<GrVkTexture*>(src->asTexture());
    if (!srcTex) {
        return false;
    }

    if (VK_NULL_HANDLE == fVertShaderModule) {
        SkASSERT(VK_NULL_HANDLE == fFragShaderModule &&
                 VK_NULL_HANDLE == fPipelineLayout &&
                 nullptr == fVertexBuffer.get() &&
                 nullptr == fUniformBuffer.get());
        if (!this->createCopyProgram(gpu)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }

    GrVkResourceProvider& resourceProv = gpu->resourceProvider();

    GrVkCopyPipeline* pipeline = resourceProv.findOrCreateCopyPipeline(rt,
                                                                       fShaderStageInfo,
                                                                       fPipelineLayout);
    if (!pipeline) {
        return false;
    }

    // UPDATE UNIFORM DESCRIPTOR SET
    int w = srcRect.width();
    int h = srcRect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    float dx0 = 2.f * dstPoint.fX / dw - 1.f;
    float dx1 = 2.f * (dstPoint.fX + w) / dw - 1.f;
    float dy0 = 2.f * dstPoint.fY / dh - 1.f;
    float dy1 = 2.f * (dstPoint.fY + h) / dh - 1.f;
    if (kBottomLeft_GrSurfaceOrigin == dst->origin()) {
        dy0 = -dy0;
        dy1 = -dy1;
    }


    float sx0 = (float)srcRect.fLeft;
    float sx1 = (float)(srcRect.fLeft + w);
    float sy0 = (float)srcRect.fTop;
    float sy1 = (float)(srcRect.fTop + h);
    int sh = src->height();
    if (kBottomLeft_GrSurfaceOrigin == src->origin()) {
        sy0 = sh - sy0;
        sy1 = sh - sy1;
    }
    // src rect edges in normalized texture space (0 to 1).
    int sw = src->width();
    sx0 /= sw;
    sx1 /= sw;
    sy0 /= sh;
    sy1 /= sh;

    float uniData[] = { dx1 - dx0, dy1 - dy0, dx0, dy0,    // posXform
                        sx1 - sx0, sy1 - sy0, sx0, sy0 };  // texCoordXform

    fUniformBuffer->updateData(gpu, uniData, sizeof(uniData), nullptr);

    const GrVkDescriptorSet* uniformDS = resourceProv.getUniformDescriptorSet();
    SkASSERT(uniformDS);

    VkDescriptorBufferInfo uniBufferInfo;
    uniBufferInfo.buffer = fUniformBuffer->buffer();
    uniBufferInfo.offset = fUniformBuffer->offset();
    uniBufferInfo.range = fUniformBuffer->size();

    VkWriteDescriptorSet descriptorWrites;
    descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites.pNext = nullptr;
    descriptorWrites.dstSet = uniformDS->descriptorSet();
    descriptorWrites.dstBinding = GrVkUniformHandler::kGeometryBinding;
    descriptorWrites.dstArrayElement = 0;
    descriptorWrites.descriptorCount = 1;
    descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites.pImageInfo = nullptr;
    descriptorWrites.pBufferInfo = &uniBufferInfo;
    descriptorWrites.pTexelBufferView = nullptr;

    GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                        1,
                                                        &descriptorWrites,
                                                        0, nullptr));

    // UPDATE SAMPLER DESCRIPTOR SET
    const GrVkDescriptorSet* samplerDS =
        gpu->resourceProvider().getSamplerDescriptorSet(fSamplerDSHandle);

    GrSamplerParams params(SkShader::kClamp_TileMode, GrSamplerParams::kNone_FilterMode);

    GrVkSampler* sampler =
        resourceProv.findOrCreateCompatibleSampler(params, srcTex->texturePriv().maxMipMapLevel());

    VkDescriptorImageInfo imageInfo;
    memset(&imageInfo, 0, sizeof(VkDescriptorImageInfo));
    imageInfo.sampler = sampler->sampler();
    imageInfo.imageView = srcTex->textureView(true)->imageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeInfo;
    memset(&writeInfo, 0, sizeof(VkWriteDescriptorSet));
    writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeInfo.pNext = nullptr;
    writeInfo.dstSet = samplerDS->descriptorSet();
    writeInfo.dstBinding = 0;
    writeInfo.dstArrayElement = 0;
    writeInfo.descriptorCount = 1;
    writeInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeInfo.pImageInfo = &imageInfo;
    writeInfo.pBufferInfo = nullptr;
    writeInfo.pTexelBufferView = nullptr;

    GR_VK_CALL(gpu->vkInterface(), UpdateDescriptorSets(gpu->device(),
                                                        1,
                                                        &writeInfo,
                                                        0, nullptr));

    VkDescriptorSet vkDescSets[] = { uniformDS->descriptorSet(), samplerDS->descriptorSet() };

    GrVkRenderTarget* texRT = static_cast<GrVkRenderTarget*>(srcTex->asRenderTarget());
    if (texRT) {
        gpu->onResolveRenderTarget(texRT);
    }

    GrVkPrimaryCommandBuffer* cmdBuffer = gpu->currentCommandBuffer();

    // TODO: Make tighter bounds and then adjust bounds for origin and granularity if we see
    //       any perf issues with using the whole bounds
    SkIRect bounds = SkIRect::MakeWH(rt->width(), rt->height());

    // Change layouts of rt and texture
    GrVkImage* targetImage = rt->msaaImage() ? rt->msaaImage() : rt;
    targetImage->setImageLayout(gpu,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                false);

    srcTex->setImageLayout(gpu,
                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                           VK_ACCESS_SHADER_READ_BIT,
                           VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                           false);

    GrVkRenderPass::LoadStoreOps vkColorOps(VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                            VK_ATTACHMENT_STORE_OP_STORE);
    GrVkRenderPass::LoadStoreOps vkStencilOps(VK_ATTACHMENT_LOAD_OP_LOAD,
                                              VK_ATTACHMENT_STORE_OP_STORE);
    const GrVkRenderPass* renderPass;
    const GrVkResourceProvider::CompatibleRPHandle& rpHandle =
        rt->compatibleRenderPassHandle();
    if (rpHandle.isValid()) {
        renderPass = gpu->resourceProvider().findRenderPass(rpHandle,
                                                            vkColorOps,
                                                            vkStencilOps);
    } else {
        renderPass = gpu->resourceProvider().findRenderPass(*rt,
                                                            vkColorOps,
                                                            vkStencilOps);
    }

    SkASSERT(renderPass->isCompatible(*rt->simpleRenderPass()));


    cmdBuffer->beginRenderPass(gpu, renderPass, nullptr, *rt, bounds, false);
    cmdBuffer->bindPipeline(gpu, pipeline);

    // Uniform DescriptorSet, Sampler DescriptorSet, and vertex shader uniformBuffer
    SkSTArray<3, const GrVkRecycledResource*> descriptorRecycledResources;
    descriptorRecycledResources.push_back(uniformDS);
    descriptorRecycledResources.push_back(samplerDS);
    descriptorRecycledResources.push_back(fUniformBuffer->resource());

    // One sampler, texture view, and texture
    SkSTArray<3, const GrVkResource*> descriptorResources;
    descriptorResources.push_back(sampler);
    descriptorResources.push_back(srcTex->textureView(true));
    descriptorResources.push_back(srcTex->resource());

    cmdBuffer->bindDescriptorSets(gpu,
                                  descriptorRecycledResources,
                                  descriptorResources,
                                  fPipelineLayout,
                                  0,
                                  2,
                                  vkDescSets,
                                  0,
                                  nullptr);

    // Set Dynamic viewport and stencil
    // We always use one viewport the size of the RT
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = SkIntToScalar(rt->width());
    viewport.height = SkIntToScalar(rt->height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdBuffer->setViewport(gpu, 0, 1, &viewport);

    // We assume the scissor is not enabled so just set it to the whole RT
    VkRect2D scissor;
    scissor.extent.width = rt->width();
    scissor.extent.height = rt->height();
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    cmdBuffer->setScissor(gpu, 0, 1, &scissor);

    cmdBuffer->bindInputBuffer(gpu, 0, fVertexBuffer.get());
    cmdBuffer->draw(gpu, 4, 1, 0, 0);
    cmdBuffer->endRenderPass(gpu);

    // Release all temp resources which should now be reffed by the cmd buffer
    pipeline->unref(gpu);
    uniformDS->unref(gpu);
    samplerDS->unref(gpu);
    sampler->unref(gpu);
    renderPass->unref(gpu);

    return true;
}

void GrVkCopyManager::destroyResources(GrVkGpu* gpu) {
    if (VK_NULL_HANDLE != fVertShaderModule) {
        GR_VK_CALL(gpu->vkInterface(), DestroyShaderModule(gpu->device(), fVertShaderModule,
                                                           nullptr));
        fVertShaderModule = VK_NULL_HANDLE;
    }

    if (VK_NULL_HANDLE != fFragShaderModule) {
        GR_VK_CALL(gpu->vkInterface(), DestroyShaderModule(gpu->device(), fFragShaderModule,
                                                           nullptr));
        fFragShaderModule = VK_NULL_HANDLE;
    }

    if (VK_NULL_HANDLE != fPipelineLayout) {
        GR_VK_CALL(gpu->vkInterface(), DestroyPipelineLayout(gpu->device(), fPipelineLayout,
                                                             nullptr));
        fPipelineLayout = VK_NULL_HANDLE;
    }

    if (fUniformBuffer) {
        fUniformBuffer->release(gpu);
        fUniformBuffer.reset();
    }
}

void GrVkCopyManager::abandonResources() {
    fVertShaderModule = VK_NULL_HANDLE;
    fFragShaderModule = VK_NULL_HANDLE;
    fPipelineLayout = VK_NULL_HANDLE;

    if (fUniformBuffer) {
        fUniformBuffer->abandon();
        fUniformBuffer.reset();
    }
}
