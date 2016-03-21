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

    void setData(GrVkGpu*, const GrPrimitiveProcessor&, const GrPipeline&);

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
                const BuiltinUniformHandles& builtinUniformHandles,
                const UniformInfoArray& uniforms,
                uint32_t vertexUniformSize,
                uint32_t fragmentUniformSize,
                uint32_t numSamplers,
                GrGLSLPrimitiveProcessor* geometryProcessor,
                GrGLSLXferProcessor* xferProcessor,
                const GrGLSLFragProcs& fragmentProcessors);

    // Each pool will manage one type of descriptor. Thus each descriptor set we use will all be of
    // one VkDescriptorType.
    struct DescriptorPoolManager {
        DescriptorPoolManager(VkDescriptorSetLayout layout, VkDescriptorType type,
                              uint32_t descCount, GrVkGpu* gpu)
            : fDescLayout(layout)
            , fDescType(type)
            , fCurrentDescriptorSet(0)
            , fPool(nullptr) {
            SkASSERT(descCount < (SK_MaxU32 >> 2));
            fMaxDescriptorSets = descCount << 2;
            this->getNewPool(gpu);
        }

        ~DescriptorPoolManager() {
            SkASSERT(!fDescLayout);
            SkASSERT(!fPool);
        }

        void getNewDescriptorSet(GrVkGpu* gpu, VkDescriptorSet* ds);

        void freeGPUResources(const GrVkGpu* gpu);
        void abandonGPUResources();

        VkDescriptorSetLayout  fDescLayout;
        VkDescriptorType       fDescType;
        uint32_t               fMaxDescriptorSets;
        uint32_t               fCurrentDescriptorSet;
        GrVkDescriptorPool*    fPool;

    private:
        void getNewPool(GrVkGpu* gpu);
    };

    void writeUniformBuffers(const GrVkGpu* gpu);

    void writeSamplers(GrVkGpu* gpu, const SkTArray<const GrTextureAccess*>& textureBindings);


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

    // GrVkResources
    GrVkPipeline*       fPipeline;

    // Used for binding DescriptorSets to the command buffer but does not need to survive during
    // command buffer execution. Thus this is not need to be a GrVkResource.
    VkPipelineLayout fPipelineLayout;

    // The DescriptorSets need to survive until the gpu has finished all draws that use them.
    // However, they will only be freed by the descriptor pool. Thus by simply keeping the
    // descriptor pool alive through the draw, the descritor sets will also stay alive. Thus we do
    // not need a GrVkResource versions of VkDescriptorSet. We hold on to these in the program since
    // we update the descriptor sets and bind them at separate times;
    VkDescriptorSet       fDescriptorSets[2];

    // Meta data so we know which descriptor sets we are using and need to bind.
    int                   fStartDS;
    int                   fDSCount;

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

    DescriptorPoolManager  fSamplerPoolManager;
    DescriptorPoolManager  fUniformPoolManager;

    int fNumSamplers;

    friend class GrVkProgramBuilder;
};

#endif
