/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkPipelineState_DEFINED
#define GrVkPipelineState_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/gpu/vk/GrVkDescriptorSetManager.h"
#include "src/gpu/vk/GrVkPipelineStateDataManager.h"

class GrPipeline;
class GrStencilSettings;
class GrVkBuffer;
class GrVkCommandBuffer;
class GrVkDescriptorPool;
class GrVkDescriptorSet;
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

    GrVkPipelineState(
            GrVkGpu* gpu,
            sk_sp<const GrVkPipeline> pipeline,
            const GrVkDescriptorSetManager::Handle& samplerDSHandle,
            const GrGLSLBuiltinUniformHandles& builtinUniformHandles,
            const UniformInfoArray& uniforms,
            uint32_t uniformSize,
            bool usePushConstants,
            const UniformInfoArray& samplers,
            std::unique_ptr<GrGLSLPrimitiveProcessor> geometryProcessor,
            std::unique_ptr<GrGLSLXferProcessor> xferProcessor,
            std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fpImpls);

    ~GrVkPipelineState();

    bool setAndBindUniforms(GrVkGpu*, const GrRenderTarget*, const GrProgramInfo&,
                            GrVkCommandBuffer*);
    /**
     * This must be called after setAndBindUniforms() since that function invalidates texture
     * bindings.
     */
    bool setAndBindTextures(GrVkGpu*, const GrPrimitiveProcessor&, const GrPipeline&,
                            const GrSurfaceProxy* const primitiveProcessorTextures[],
                            GrVkCommandBuffer*);

    bool setAndBindInputAttachment(GrVkGpu*, GrVkRenderTarget* renderTarget, GrVkCommandBuffer*);

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

        /**
        * Gets a float4 that adjusts the position from Skia device coords to Vulkans normalized device
        * coords. Assuming the transformed position, pos, is a homogeneous float3, the vec, v, is
        * applied as such:
        * pos.x = dot(v.xy, pos.xz)
        * pos.y = dot(v.zw, pos.yz)
        */
        void getRTAdjustmentVec(float* destVec) {
            destVec[0] = 2.f / fRenderTargetSize.fWidth;
            destVec[1] = -1.f;
            if (kBottomLeft_GrSurfaceOrigin == fRenderTargetOrigin) {
                destVec[2] = -2.f / fRenderTargetSize.fHeight;
                destVec[3] = 1.f;
            } else {
                destVec[2] = 2.f / fRenderTargetSize.fHeight;
                destVec[3] = -1.f;
            }
        }
    };

    // Helper for setData() that sets the view matrix and loads the render target height uniform
    void setRenderTargetState(const GrRenderTarget*, GrSurfaceOrigin);

    // GrManagedResources
    sk_sp<const GrVkPipeline> fPipeline;

    const GrVkDescriptorSetManager::Handle fSamplerDSHandle;

    SkSTArray<4, const GrVkSampler*> fImmutableSamplers;

    // Tracks the current render target uniforms stored in the vertex buffer.
    RenderTargetState fRenderTargetState;
    GrGLSLBuiltinUniformHandles fBuiltinUniformHandles;

    // Processors in the GrVkPipelineState
    std::unique_ptr<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    std::unique_ptr<GrGLSLXferProcessor> fXferProcessor;
    std::vector<std::unique_ptr<GrGLSLFragmentProcessor>> fFPImpls;

    GrVkPipelineStateDataManager fDataManager;

    int fNumSamplers;
};

#endif
