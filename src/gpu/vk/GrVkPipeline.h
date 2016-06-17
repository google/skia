/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipeline_DEFINED
#define GrVkPipeline_DEFINED

#include "GrTypes.h"

#include "GrVkResource.h"

#include "vk/GrVkDefines.h"

class GrNonInstancedVertices;
class GrPipeline;
class GrPrimitiveProcessor;
class GrVkCommandBuffer;
class GrVkGpu;
class GrVkRenderPass;

class GrVkPipeline : public GrVkResource {
public:
    static GrVkPipeline* Create(GrVkGpu* gpu,
                                const GrPipeline& pipeline,
                                const GrPrimitiveProcessor& primProc,
                                VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                int shaderStageCount,
                                GrPrimitiveType primitiveType,
                                const GrVkRenderPass& renderPass,
                                VkPipelineLayout layout,
                                VkPipelineCache cache);

    VkPipeline pipeline() const { return fPipeline; }

    static void SetDynamicState(GrVkGpu*, GrVkCommandBuffer*, const GrPipeline&);


private:
    GrVkPipeline(VkPipeline pipeline) : INHERITED(), fPipeline(pipeline) {}

    GrVkPipeline(const GrVkPipeline&);
    GrVkPipeline& operator=(const GrVkPipeline&);

    void freeGPUData(const GrVkGpu* gpu) const override;

    VkPipeline  fPipeline;

    typedef GrVkResource INHERITED;
};

#endif
