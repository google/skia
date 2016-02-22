/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkProgram_DEFINED
#define GrVkProgram_DEFINED

#include "GrVkImage.h"
#include "GrVkProgramDesc.h"
#include "GrVkProgramDataManager.h"
#include "glsl/GrGLSLProgramBuilder.h"

#include "vulkan/vulkan.h"

class GrPipeline;
class GrVkCommandBuffer;
class GrVkDescriptorPool;
class GrVkGpu;
class GrVkImageView;
class GrVkPipeline;
class GrVkSampler;
class GrVkUniformBuffer;

class GrVkProgram : public SkRefCnt {
public:
    typedef GrGLSLProgramBuilder::BuiltinUniformHandles BuiltinUniformHandles;

    ~GrVkProgram();

    GrVkPipeline* vkPipeline() const { return fPipeline; }

    void setData(const GrVkGpu*, const GrPrimitiveProcessor&, const GrPipeline&);

    void bind(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer);

    void addUniformResources(GrVkCommandBuffer&);

    void freeGPUResources(const GrVkGpu* gpu);

    // This releases resources the only a given instance of a GrVkProgram needs to hold onto and do
    // don't need to survive across new uses of the program.
    void freeTempResources(const GrVkGpu* gpu);

    void abandonGPUResources();

private:
    typedef GrVkProgramDataManager::UniformInfoArray UniformInfoArray;
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    GrVkProgram(GrVkGpu* gpu,
                GrVkPipeline* pipeline,
                VkPipelineLayout layout,
                VkDescriptorSetLayout dsLayout[2],
                GrVkDescriptorPool* descriptorPool,
                VkDescriptorSet descriptorSets[2],
                const BuiltinUniformHandles& builtinUniformHandles,
                const UniformInfoArray& uniforms,
                uint32_t vertexUniformSize,
                uint32_t fragmentUniformSize,
                uint32_t numSamplers,
                GrGLSLPrimitiveProcessor* geometryProcessor,
                GrGLSLXferProcessor* xferProcessor,
                const GrGLSLFragProcs& fragmentProcessors);

    void writeUniformBuffers(const GrVkGpu* gpu);

    void writeSamplers(const GrVkGpu* gpu, const SkTArray<const GrTextureAccess*>& textureBindings);


    /**
    * We use the RT's size and origin to adjust from Skia device space to OpenGL normalized device
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
        * Gets a vec4 that adjusts the position from Skia device coords to GL's normalized device
        * coords. Assuming the transformed position, pos, is a homogeneous vec3, the vec, v, is
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
    void setRenderTargetState(const GrPipeline&);

//    GrVkGpu* fGpu;

    // GrVkResources
    GrVkDescriptorPool* fDescriptorPool;
    GrVkPipeline*       fPipeline;

    // Used for binding DescriptorSets to the command buffer but does not need to survive during
    // command buffer execution. Thus this is not need to be a GrVkResource.
    VkPipelineLayout fPipelineLayout;

    // The first set (index 0) will be used for samplers and the second set (index 1) will be
    // used for uniform buffers.
    // The DSLayouts only are needed for allocating the descriptor sets and must survive until after
    // descriptor sets have been updated. Thus the lifetime of the layouts will just be the life of
    //the GrVkProgram.
    VkDescriptorSetLayout fDSLayout[2];
    // The DescriptorSets need to survive until the gpu has finished all draws that use them.
    // However, they will only be freed by the descriptor pool. Thus by simply keeping the
    // descriptor pool alive through the draw, the descritor sets will also stay alive. Thus we do
    // not need a GrVkResource versions of VkDescriptorSet.
    VkDescriptorSet       fDescriptorSets[2];

    SkAutoTDelete<GrVkUniformBuffer>    fVertexUniformBuffer;
    SkAutoTDelete<GrVkUniformBuffer>    fFragmentUniformBuffer;

    // GrVkResources used for sampling textures
    SkTDArray<GrVkSampler*>                fSamplers;
    SkTDArray<const GrVkImageView*>        fTextureViews;
    SkTDArray<const GrVkImage::Resource*>  fTextures;

    // Tracks the current render target uniforms stored in the vertex buffer.
    RenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;

    // Processors in the program
    SkAutoTDelete<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    SkAutoTDelete<GrGLSLXferProcessor> fXferProcessor;
    GrGLSLFragProcs fFragmentProcessors;

    GrVkProgramDataManager fProgramDataManager;

#ifdef SK_DEBUG
    int fNumSamplers;
#endif

    friend class GrVkProgramBuilder;
};

#endif
