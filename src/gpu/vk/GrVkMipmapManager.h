/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrVkMipmapManager_DEFINED
#define GrVkMipmapManager_DEFINED

#include "GrVkDescriptorSetManager.h"

#include "vk/GrVkDefines.h"

class GrVkGpu;
class GrVkMipmapPipeline;
class GrVkTexture;
class GrVkUniformBuffer;
class GrVkVertexBuffer;
struct SkIPoint;
struct SkIRect;

class GrVkMipmapManager {
public:
    GrVkMipmapManager()
        : fVertShaderModule(VK_NULL_HANDLE)
        , fFragShaderModule(VK_NULL_HANDLE)
        , fPipelineLayout(VK_NULL_HANDLE)
        , fUniformBuffer(nullptr) {}

    bool generateMipmap(GrVkGpu* gpu, GrVkTexture* texture, bool gammaCorrect);

    void destroyResources(GrVkGpu* gpu);
    void abandonResources();

private:
    bool createMipmapProgram(GrVkGpu* gpu);

    // Everything below is only created once and shared by all mipmap draws/pipelines
    VkShaderModule fVertShaderModule;
    VkShaderModule fFragShaderModule;
    VkPipelineShaderStageCreateInfo fShaderStageInfo[2];

    GrVkDescriptorSetManager::Handle fSamplerDSHandle;
    VkPipelineLayout fPipelineLayout;

    sk_sp<GrVkVertexBuffer> fVertexBuffer;
    GrVkUniformBuffer* fUniformBuffer;
};

#endif
