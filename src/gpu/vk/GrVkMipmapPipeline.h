/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkMipmapPipeline_DEFINED
#define GrVkMipmapPipeline_DEFINED

#include "GrVkPipeline.h"

class GrVkMipmapPipeline : public GrVkPipeline {
public:
    // We expect the passed in renderPass to be stored on the GrVkResourceProvider and not a local
    // object of the client.
    static GrVkMipmapPipeline* Create(GrVkGpu* gpu,
                                      VkPipelineShaderStageCreateInfo* shaderStageInfo,
                                      VkPipelineLayout pipelineLayout,
                                      int numSamples,
                                      const GrVkRenderPass& renderPass,
                                      VkPipelineCache cache);

    bool isCompatible(const GrVkRenderPass& rp) const;

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkMipmapPipeline: %d (%d refs)\n", fPipeline, this->getRefCnt());
    }
#endif

private:
    GrVkMipmapPipeline(VkPipeline pipeline, const GrVkRenderPass* renderPass)
        : INHERITED(pipeline)
        , fRenderPass(renderPass) {
    }

    const GrVkRenderPass* fRenderPass;

    typedef GrVkPipeline INHERITED;
};

#endif
