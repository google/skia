/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#if defined(SK_VULKAN)

#include "include/gpu/vk/GrVkVulkan.h"

#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/vk/GrVkCopyPipeline.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkRenderTarget.h"
#include "src/gpu/vk/GrVkUtil.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

using sk_gpu_test::GrContextFactory;

class TestVkCopyProgram {
public:
    TestVkCopyProgram()
            : fVertShaderModule(VK_NULL_HANDLE)
            , fFragShaderModule(VK_NULL_HANDLE)
            , fPipelineLayout(VK_NULL_HANDLE) {}

    void test(GrVkGpu* gpu, skiatest::Reporter* reporter) {
        const char vertShaderText[] =
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"

            "layout(set = 0, binding = 0) uniform vertexUniformBuffer {"
            "half4 uPosXform;"
            "half4 uTexCoordXform;"
            "};"
            "layout(location = 0) in float2 inPosition;"
            "layout(location = 1) out half2 vTexCoord;"

            "// Copy Program VS\n"
            "void main() {"
            "vTexCoord = half2(inPosition * uTexCoordXform.xy + uTexCoordXform.zw);"
            "sk_Position.xy = inPosition * uPosXform.xy + uPosXform.zw;"
            "sk_Position.zw = half2(0, 1);"
            "}";

        const char fragShaderText[] =
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"

            "layout(set = 1, binding = 0) uniform sampler2D uTextureSampler;"
            "layout(location = 1) in half2 vTexCoord;"

            "// Copy Program FS\n"
            "void main() {"
            "sk_FragColor = texture(uTextureSampler, vTexCoord);"
            "}";

        SkSL::Program::Settings settings;
        SkSL::String spirv;
        SkSL::Program::Inputs inputs;
        if (!GrCompileVkShaderModule(gpu, vertShaderText, VK_SHADER_STAGE_VERTEX_BIT,
                                     &fVertShaderModule, &fShaderStageInfo[0], settings,
                                     &spirv, &inputs)) {
            this->destroyResources(gpu);
            REPORTER_ASSERT(reporter, false);
            return;
        }
        SkASSERT(inputs.isEmpty());

        if (!GrCompileVkShaderModule(gpu, fragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT,
                                     &fFragShaderModule, &fShaderStageInfo[1], settings,
                                     &spirv, &inputs)) {
            this->destroyResources(gpu);
            REPORTER_ASSERT(reporter, false);
            return;
        }

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
            REPORTER_ASSERT(reporter, false);
            return;
        }

        GrSurfaceDesc surfDesc;
        surfDesc.fFlags = kRenderTarget_GrSurfaceFlag;
        surfDesc.fWidth = 16;
        surfDesc.fHeight = 16;
        surfDesc.fConfig = kRGBA_8888_GrPixelConfig;
        surfDesc.fSampleCnt = 1;
        sk_sp<GrTexture> tex = gpu->createTexture(surfDesc, SkBudgeted::kNo);
        if (!tex) {
            this->destroyResources(gpu);
            REPORTER_ASSERT(reporter, tex.get());
            return;

        }
        GrRenderTarget* rt = tex->asRenderTarget();
        REPORTER_ASSERT(reporter, rt);
        GrVkRenderTarget* vkRT = static_cast<GrVkRenderTarget*>(rt);

        GrVkCopyPipeline* copyPipeline = GrVkCopyPipeline::Create(gpu,
                                                                  fShaderStageInfo,
                                                                  fPipelineLayout,
                                                                  1,
                                                                  *vkRT->simpleRenderPass(),
                                                                  VK_NULL_HANDLE);

        REPORTER_ASSERT(reporter, copyPipeline);
        if (copyPipeline) {
            copyPipeline->unref(gpu);
        }

        this->destroyResources(gpu);
    }

    void destroyResources(GrVkGpu* gpu) {
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
    }

    VkShaderModule fVertShaderModule;
    VkShaderModule fFragShaderModule;
    VkPipelineShaderStageCreateInfo fShaderStageInfo[2];

    GrVkDescriptorSetManager::Handle fSamplerDSHandle;
    VkPipelineLayout fPipelineLayout;

};

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkMakeCopyPipelineTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->priv().getGpu());

    TestVkCopyProgram copyProgram;
    copyProgram.test(gpu, reporter);
}

#endif
