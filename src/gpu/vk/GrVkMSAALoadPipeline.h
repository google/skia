/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkMSAALoadPipeline_DEFINED
#define GrVkMSAALoadPipeline_DEFINED

#include "src/gpu/vk/GrVkPipeline.h"

class GrVkMSAALoadPipeline : public GrVkPipeline {
public:
    // We expect the passed in renderPass to be stored on the GrVkResourceProvider and not a local
    // object of the client.
    static GrVkMSAALoadPipeline* Create(GrVkGpu* gpu,
                                        VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                        VkPipelineLayout pipelineLayout,
                                        int numSamples,
                                        const GrVkRenderPass& renderPass,
                                        VkPipelineCache cache);

    bool isCompatible(const GrVkRenderPass& rp) const;

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkMSAALoadPipeline: %d (%d refs)\n", fPipeline, this->getRefCnt());
    }
#endif

private:
    GrVkMSAALoadPipeline(const GrVkGpu* gpu, VkPipeline pipeline, const GrVkRenderPass* renderPass)
        : INHERITED(gpu, pipeline, /*layout=*/VK_NULL_HANDLE)
        , fRenderPass(renderPass) {
    }

    const GrVkRenderPass* fRenderPass;

    typedef GrVkPipeline INHERITED;
};

#endif
