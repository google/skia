/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkPipelineState_DEFINED
#define GrVkPipelineState_DEFINED

#include "GrVkImage.h"
#include "GrVkProgramDesc.h"
#include "GrVkPipelineStateDataManager.h"
#include "glsl/GrGLSLProgramBuilder.h"

#include "vk/GrVkDefines.h"

class GrPipeline;
class GrVkCommandBuffer;
class GrVkDescriptorPool;
class GrVkGpu;
class GrVkImageView;
class GrVkPipeline;
class GrVkSampler;
class GrVkUniformBuffer;

/**
 * This class holds onto a GrVkPipeline object that we use for draws. Besides storing the acutal
 * GrVkPipeline object, this class is also responsible handling all uniforms, descriptors, samplers,
 * and other similar objects that are used along with the VkPipeline in the draw. This includes both
 * allocating and freeing these objects, as well as updating their values.
 */
class GrVkPipelineState : public SkRefCnt {
public:
    typedef GrGLSLProgramBuilder::BuiltinUniformHandles BuiltinUniformHandles;

    ~GrVkPipelineState();

    GrVkPipeline* vkPipeline() const { return fPipeline; }

    void setData(GrVkGpu*, const GrPrimitiveProcessor&, const GrPipeline&);

    void bind(const GrVkGpu* gpu, GrVkCommandBuffer* commandBuffer);

    void addUniformResources(GrVkCommandBuffer&);

    void freeGPUResources(const GrVkGpu* gpu);

    // This releases resources that only a given instance of a GrVkPipelineState needs to hold onto
    // and don't need to survive across new uses of the GrVkPipelineState.
    void freeTempResources(const GrVkGpu* gpu);

    void abandonGPUResources();

    // The key is composed of two parts:
    // 1. uint32_t for total key length
    // 2. Pipeline state data
    enum StateKeyOffsets {
        // Part 1.
        kLength_StateKeyOffset = 0,
        // Part 2.
        kData_StateKeyOffset = kLength_StateKeyOffset + sizeof(uint32_t),
    };
    static void BuildStateKey(const GrPipeline&, GrPrimitiveType primitiveType,
                               SkTArray<unsigned char, true>* key);

    /**
     * For Vulkan we want to cache the entire VkPipeline for reuse of draws. The Desc here holds all
     * the information needed to differentiate one pipeline from another.
     *
     * The GrVkProgramDesc contains all the information need to create the actual shaders for the
     * pipeline.
     *
     * The fStateKey is used to store all the inputs for the rest of the state stored on the
     * pipeline. This includes stencil settings, blending information, render pass format, draw face
     * information, and primitive type. Note that some state is set dynamically on the pipeline for
     * each draw  and thus is not included in this descriptor. This includes the viewport, scissor,
     * and blend constant.
     *
     * A checksum which includes the fProgramDesc and fStateKey is included at the top of the Desc
     * for caching purposes and faster equality checks.
     */
    struct Desc {
        uint32_t                fChecksum;
        GrVkProgramDesc         fProgramDesc;

        enum {
            kRenderPassKeyAlloc = 12, // This is typical color attachment with no stencil or msaa
            kStencilKeyAlloc = sizeof(GrStencilSettings),
            kDrawFaceKeyAlloc = 4,
            kBlendingKeyAlloc = 4,
            kPrimitiveTypeKeyAlloc = 4,
            kPreAllocSize = kData_StateKeyOffset + kRenderPassKeyAlloc + kStencilKeyAlloc +
                            kDrawFaceKeyAlloc + kBlendingKeyAlloc + kPrimitiveTypeKeyAlloc,
        };
        SkSTArray<kPreAllocSize, uint8_t, true> fStateKey;

        bool operator== (const Desc& that) const {
            if (fChecksum != that.fChecksum || fProgramDesc != that.fProgramDesc) {
                return false;
            }
            // We store the keyLength at the start of fVkKey. Thus we don't have to worry about
            // different length keys since we will fail on the comparison immediately. Therefore we
            // just use this PipelineDesc to get the length to iterate over.
            int keyLength = fStateKey.count();
            SkASSERT(SkIsAlign4(keyLength));
            int l = keyLength >> 2;
            const uint32_t* aKey = reinterpret_cast<const uint32_t*>(fStateKey.begin());
            const uint32_t* bKey = reinterpret_cast<const uint32_t*>(that.fStateKey.begin());
            for (int i = 0; i < l; ++i) {
                if (aKey[i] != bKey[i]) {
                    return false;
                }
            }
            return true;
        }

        static bool Less(const Desc& a, const Desc& b) {
            if (a.fChecksum != b.fChecksum) {
                return a.fChecksum < b.fChecksum ? true : false;
            }
            bool progDescLess = GrProgramDesc::Less(a.fProgramDesc, b.fProgramDesc);
            if (progDescLess || a.fProgramDesc != b.fProgramDesc) {
                return progDescLess;
            }

            int keyLength = a.fStateKey.count();
            SkASSERT(SkIsAlign4(keyLength));
            int l = keyLength >> 2;
            const uint32_t* aKey = reinterpret_cast<const uint32_t*>(a.fStateKey.begin());
            const uint32_t* bKey = reinterpret_cast<const uint32_t*>(b.fStateKey.begin());
            for (int i = 0; i < l; ++i) {
                if (aKey[i] != bKey[i]) {
                    return aKey[i] < bKey[i] ? true : false;
                }
            }
            return false;
        }
    };

    const Desc& getDesc() { return fDesc; }

private:
    typedef GrVkPipelineStateDataManager::UniformInfoArray UniformInfoArray;
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;

    GrVkPipelineState(GrVkGpu* gpu,
                      const GrVkPipelineState::Desc&,
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
            SkASSERT(descCount < (kMaxDescSetLimit >> 2));
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
        static const uint32_t kMaxDescSetLimit = 1 << 10;

        void getNewPool(GrVkGpu* gpu);
    };

    void writeUniformBuffers(const GrVkGpu* gpu);

    void writeSamplers(GrVkGpu* gpu, const SkTArray<const GrTextureAccess*>& textureBindings);

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
        * Gets a vec4 that adjusts the position from Skia device coords to Vulkans normalized device
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
    GrVkPipeline* fPipeline;

    // Used for binding DescriptorSets to the command buffer but does not need to survive during
    // command buffer execution. Thus this is not need to be a GrVkResource.
    VkPipelineLayout fPipelineLayout;

    // The DescriptorSets need to survive until the gpu has finished all draws that use them.
    // However, they will only be freed by the descriptor pool. Thus by simply keeping the
    // descriptor pool alive through the draw, the descritor sets will also stay alive. Thus we do
    // not need a GrVkResource versions of VkDescriptorSet. We hold on to these in the
    // GrVkPipelineState since we update the descriptor sets and bind them at separate times;
    VkDescriptorSet fDescriptorSets[2];

    // Meta data so we know which descriptor sets we are using and need to bind.
    int fStartDS;
    int fDSCount;

    SkAutoTDelete<GrVkUniformBuffer> fVertexUniformBuffer;
    SkAutoTDelete<GrVkUniformBuffer> fFragmentUniformBuffer;

    // GrVkResources used for sampling textures
    SkTDArray<GrVkSampler*> fSamplers;
    SkTDArray<const GrVkImageView*> fTextureViews;
    SkTDArray<const GrVkImage::Resource*> fTextures;

    // Tracks the current render target uniforms stored in the vertex buffer.
    RenderTargetState fRenderTargetState;
    BuiltinUniformHandles fBuiltinUniformHandles;

    // Processors in the GrVkPipelineState
    SkAutoTDelete<GrGLSLPrimitiveProcessor> fGeometryProcessor;
    SkAutoTDelete<GrGLSLXferProcessor> fXferProcessor;
    GrGLSLFragProcs fFragmentProcessors;

    Desc fDesc;

    GrVkPipelineStateDataManager fDataManager;

    DescriptorPoolManager fSamplerPoolManager;
    DescriptorPoolManager fUniformPoolManager;

    int fNumSamplers;

    friend class GrVkPipelineStateBuilder;
};

#endif
