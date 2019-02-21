/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkGpuCommandBuffer_DEFINED
#define GrVkGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"

#include "GrColor.h"
#include "GrMesh.h"
#include "GrTypes.h"
#include "GrVkPipelineState.h"
#include "vk/GrVkTypes.h"

class GrVkGpu;
class GrVkImage;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;

class GrVkGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrVkGpuTextureCommandBuffer(GrVkGpu* gpu) : fGpu(gpu) {}

    ~GrVkGpuTextureCommandBuffer() override;

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override;

    void insertEventMarker(const char*) override;

    void reset() {
        fCopies.reset();
        fTexture = nullptr;
    }

    void submit();

private:
    struct CopyInfo {
        CopyInfo(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                 const SkIPoint& dstPoint)
            : fSrc(sk_ref_sp(src)), fSrcOrigin(srcOrigin), fSrcRect(srcRect), fDstPoint(dstPoint) {}

        sk_sp<GrSurface> fSrc;
        GrSurfaceOrigin  fSrcOrigin;
        SkIRect          fSrcRect;
        SkIPoint         fDstPoint;
    };

    GrVkGpu*                    fGpu;
    SkTArray<CopyInfo>          fCopies;

    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrVkGpuRTCommandBuffer : public GrGpuRTCommandBuffer, private GrMesh::SendToGpuImpl {
public:
    GrVkGpuRTCommandBuffer(GrVkGpu*);

    ~GrVkGpuRTCommandBuffer() override;

    void begin() override { }
    void end() override;

    void discard() override;
    void insertEventMarker(const char*) override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override;

    void executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) override;

    void set(GrRenderTarget*, GrSurfaceOrigin,
             const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
             const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&);
    void reset();

    void submit();

private:
    void init();

    // Called instead of init when we are drawing to a render target that already wraps a secondary
    // command buffer.
    void initWrapped();

    bool wrapsSecondaryCommandBuffer() const;

    GrGpu* gpu() override;

    // Bind vertex and index buffers
    void bindGeometry(const GrGpuBuffer* indexBuffer,
                      const GrGpuBuffer* vertexBuffer,
                      const GrGpuBuffer* instanceBuffer);

    GrVkPipelineState* prepareDrawState(const GrPrimitiveProcessor&,
                                        const GrPipeline&,
                                        const GrPipeline::FixedDynamicState*,
                                        const GrPipeline::DynamicStateArrays*,
                                        GrPrimitiveType);

    void onDraw(const GrPrimitiveProcessor&,
                const GrPipeline&,
                const GrPipeline::FixedDynamicState*,
                const GrPipeline::DynamicStateArrays*,
                const GrMesh[],
                int meshCount,
                const SkRect& bounds) override;

    // GrMesh::SendToGpuImpl methods. These issue the actual Vulkan draw commands.
    // Marked final as a hint to the compiler to not use virtual dispatch.
    void sendMeshToGpu(GrPrimitiveType primType, const GrBuffer* vertexBuffer, int vertexCount,
                       int baseVertex) final {
        this->sendInstancedMeshToGpu(primType, vertexBuffer, vertexCount, baseVertex, nullptr, 1,
                                     0);
    }

    void sendIndexedMeshToGpu(GrPrimitiveType primType, const GrBuffer* indexBuffer, int indexCount,
                              int baseIndex, uint16_t /*minIndexValue*/, uint16_t /*maxIndexValue*/,
                              const GrBuffer* vertexBuffer, int baseVertex,
                              GrPrimitiveRestart restart) final {
        SkASSERT(restart == GrPrimitiveRestart::kNo);
        this->sendIndexedInstancedMeshToGpu(primType, indexBuffer, indexCount, baseIndex,
                                            vertexBuffer, baseVertex, nullptr, 1, 0,
                                            GrPrimitiveRestart::kNo);
    }

    void sendInstancedMeshToGpu(GrPrimitiveType, const GrBuffer* vertexBuffer, int vertexCount,
                                int baseVertex, const GrBuffer* instanceBuffer, int instanceCount,
                                int baseInstance) final;

    void sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrBuffer* indexBuffer, int indexCount,
                                       int baseIndex, const GrBuffer* vertexBuffer, int baseVertex,
                                       const GrBuffer* instanceBuffer, int instanceCount,
                                       int baseInstance, GrPrimitiveRestart) final;

    void onClear(const GrFixedClip&, const SkPMColor4f& color) override;

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override;

    void addAdditionalCommandBuffer();
    void addAdditionalRenderPass();

    struct InlineUploadInfo {
        InlineUploadInfo(GrOpFlushState* state, const GrDeferredTextureUploadFn& upload)
                : fFlushState(state), fUpload(upload) {}

        GrOpFlushState* fFlushState;
        GrDeferredTextureUploadFn fUpload;
    };

    struct CopyInfo {
        CopyInfo(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                 const SkIPoint& dstPoint, bool shouldDiscardDst)
            : fSrc(sk_ref_sp(src))
            , fSrcOrigin(srcOrigin)
            , fSrcRect(srcRect)
            , fDstPoint(dstPoint)
            , fShouldDiscardDst(shouldDiscardDst) {}

        sk_sp<GrSurface> fSrc;
        GrSurfaceOrigin  fSrcOrigin;
        SkIRect          fSrcRect;
        SkIPoint         fDstPoint;
        bool             fShouldDiscardDst;
    };

    enum class LoadStoreState {
        kUnknown,
        kStartsWithClear,
        kStartsWithDiscard,
        kLoadAndStore,
    };

    struct CommandBufferInfo {
        const GrVkRenderPass*                  fRenderPass;
        SkTArray<GrVkSecondaryCommandBuffer*>  fCommandBuffers;
        VkClearValue                           fColorClearValue;
        SkRect                                 fBounds;
        bool                                   fIsEmpty = true;
        LoadStoreState                         fLoadStoreState = LoadStoreState::kUnknown;
        // The PreDrawUploads and PreCopies are sent to the GPU before submitting the secondary
        // command buffer.
        SkTArray<InlineUploadInfo>             fPreDrawUploads;
        SkTArray<CopyInfo>                     fPreCopies;
        // Array of images that will be sampled and thus need to be transfered to sampled layout
        // before submitting the secondary command buffers. This must happen after we do any predraw
        // uploads or copies.
        SkTArray<sk_sp<GrVkTexture>>           fSampledTextures;

        GrVkSecondaryCommandBuffer* currentCmdBuf() {
            return fCommandBuffers.back();
        }
    };

    SkTArray<CommandBufferInfo> fCommandBufferInfos;
    int                         fCurrentCmdInfo;

    GrVkGpu*                    fGpu;
    VkAttachmentLoadOp          fVkColorLoadOp;
    VkAttachmentStoreOp         fVkColorStoreOp;
    VkAttachmentLoadOp          fVkStencilLoadOp;
    VkAttachmentStoreOp         fVkStencilStoreOp;
    SkPMColor4f                 fClearColor;
    GrVkPipelineState*          fLastPipelineState;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif
