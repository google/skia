/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkPipelineState_DEFINED
#define GrVkPipelineState_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/ganesh/GrRefCnt.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorSet.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorSetManager.h"
#include "src/gpu/ganesh/vk/GrVkPipelineStateDataManager.h"

class GrPipeline;
class GrStencilSettings;
class GrVkBuffer;
class GrVkCommandBuffer;
class GrVkDescriptorPool;
class GrVkGpu;
class GrVkImageView;
class GrVkPipeline;
class GrVkRenderTarget;
class GrVkSampler;
class GrVkTexture;

/**
 * This class holds onto a GrVkPipeline object that we use for draws. Besides storing the acutal
 * GrVkPipeline object, this class is also responsible handling all uniforms, descriptors, samplers,
 * and other similar objects that are used along with the VkPipeline in the draw. This includes both
 * allocating and freeing these objects, as well as updating their values.
 */
class GrVkPipelineState {
public:
    using UniformInfoArray = GrVkPipelineStateDataManager::UniformInfoArray;
    using UniformHandle = GrGLSLProgramDataManager::UniformHandle;

    GrVkPipelineState(GrVkGpu*,
                      sk_sp<const GrVkPipeline>,
                      const GrVkDescriptorSetManager::Handle& samplerDSHandle,
                      const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
                      const UniformInfoArray& uniforms,
                      uint32_t uniformSize,
                      bool usePushConstants,
                      const UniformInfoArray& samplers,
                      std::unique_ptr<GrGeometryProcessor::ProgramImpl>,
                      std::unique_ptr<GrXferProcessor::ProgramImpl>,
                      std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fpImpls);

    ~GrVkPipelineState();

    bool setAndBindUniforms(GrVkGpu*, SkISize colorAttachmentDimensions, const GrProgramInfo&,
                            GrVkCommandBuffer*);
    /**
     * This must be called after setAndBindUniforms() since that function invalidates texture
     * bindings.
     */
    bool setAndBindTextures(GrVkGpu*,
                            const GrGeometryProcessor&,
                            const GrPipeline&,
                            const GrSurfaceProxy* const geomProcTextures[],
                            GrVkCommandBuffer*);

    bool setAndBindInputAttachment(GrVkGpu*, gr_rp<const GrVkDescriptorSet> inputDescSet,
                                   GrVkCommandBuffer*);

    void bindPipeline(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer);

    void freeGPUResources(GrVkGpu* gpu);

private:
    /**
     * We use the RT's size and origin to adjust from Skia device space to vulkan normalized device
     * space and to make device space positions have the correct origin for processors that require
     * them.
     */
    struct RenderTargetState {
        SkISize         fRenderTargetSize;
        GrSurfaceOrigin fRenderTargetOrigin;

        RenderTargetState() { this->invalidate(); }
        void invalidate() {
            fRenderTargetSize.fWidth = -1;
            fRenderTargetSize.fHeight = -1;
            fRenderTargetOrigin = (GrSurfaceOrigin)-1;
        }
    };

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setRenderTargetState(SkISize colorAttachmentDimensions, GrSurfaceOrigin);

    // GrManagedResources
    sk_sp<const GrVkPipeline> fPipeline;

    const GrVkDescriptorSetManager::Handle fSamplerDSHandle;

    skia_private::STArray<4, const GrVkSampler*> fImmutableSamplers;

    // Tracks the current render target uniforms stored in the vertex buffer.
    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;

    // Processors in the GrVkPipelineState
    std::unique_ptr<GrGeometryProcessor::ProgramImpl>              fGPImpl;
    std::unique_ptr<GrXferProcessor::ProgramImpl>                  fXPImpl;
    std::vector<std::unique_ptr<GrFragmentProcessor::ProgramImpl>> fFPImpls;

    GrVkPipelineStateDataManager fDataManager;

    int fNumSamplers;
};

#endif
