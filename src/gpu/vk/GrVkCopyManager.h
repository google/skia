/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
*/

#ifndef GrVkCopyManager_DEFINED
#define GrVkCopyManager_DEFINED

#include "GrTypes.h"
#include "GrVkDescriptorSetManager.h"

#include "vk/GrVkDefines.h"

class GrSurface;
class GrVkCopyPipeline;
class GrVkGpu;
class GrVkUniformBuffer;
class GrVkVertexBuffer;
struct SkIPoint;
struct SkIRect;

class GrVkCopyManager {
public:
    GrVkCopyManager();

    ~GrVkCopyManager();

    bool copySurfaceAsDraw(GrVkGpu* gpu,
                           GrSurface* dst, GrSurfaceOrigin dstOrigin,
                           GrSurface* src, GrSurfaceOrigin srcOrigin,
                           const SkIRect& srcRect, const SkIPoint& dstPoint,
                           bool canDiscardOutsideDstRect);

    void destroyResources(GrVkGpu* gpu);
    void abandonResources();

private:
    bool createCopyProgram(GrVkGpu* gpu);

    // Everything below is only created once and shared by all copy draws/pipelines
    VkShaderModule fVertShaderModule;
    VkShaderModule fFragShaderModule;
    VkPipelineShaderStageCreateInfo fShaderStageInfo[2];

    GrVkDescriptorSetManager::Handle fSamplerDSHandle;
    VkPipelineLayout fPipelineLayout;

    sk_sp<GrVkVertexBuffer> fVertexBuffer;
    std::unique_ptr<GrVkUniformBuffer> fUniformBuffer;
};

#endif
