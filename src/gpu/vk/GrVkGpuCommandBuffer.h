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

class GrVkGpu;
class GrVkImage;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;

class GrVkGpuCommandBuffer : public GrGpuCommandBuffer, private GrMesh::SendToGpuImpl {
public:
    GrVkGpuCommandBuffer(GrVkGpu*, GrRenderTarget*, GrSurfaceOrigin,
                         const LoadAndStoreInfo&,
                         const StencilLoadAndStoreInfo&);

    ~GrVkGpuCommandBuffer() override;

    void begin() override { }
    void end() override;

    void discard() override;
    void insertEventMarker(const char*) override;

    void inlineUpload(GrOpFlushState* state, GrDrawOp::DeferredUploadFn& upload) override;

private:
    void init();

    GrGpu* gpu() override;

    void onSubmit() override;

    // Bind vertex and index buffers
    void bindGeometry(const GrPrimitiveProcessor&,
                      const GrBuffer* indexBuffer,
                      const GrBuffer* vertexBuffer,
                      const GrBuffer* instanceBuffer);

    sk_sp<GrVkPipelineState> prepareDrawState(const GrPipeline&,
                                              const GrPrimitiveProcessor&,
                                              GrPrimitiveType,
                                              bool hasDynamicState);

    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh mesh[],
                const GrPipeline::DynamicState[],
                int meshCount,
                const SkRect& bounds) override;

    // GrMesh::SendToGpuImpl methods. These issue the actual Vulkan draw commands.
    // Marked final as a hint to the compiler to not use virtual dispatch.
    void sendMeshToGpu(const GrPrimitiveProcessor& primProc, GrPrimitiveType primType,
                       const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) final {
        this->sendInstancedMeshToGpu(primProc, primType, vertexBuffer, vertexCount, baseVertex,
                                     nullptr, 1, 0);
    }

    void sendIndexedMeshToGpu(const GrPrimitiveProcessor& primProc, GrPrimitiveType primType,
                              const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                              uint16_t /*minIndexValue*/, uint16_t /*maxIndexValue*/,
                              const GrBuffer* vertexBuffer, int baseVertex) final {
        this->sendIndexedInstancedMeshToGpu(primProc, primType, indexBuffer, indexCount, baseIndex,
                                            vertexBuffer, baseVertex, nullptr, 1, 0);
    }

    void sendInstancedMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                const GrBuffer* vertexBuffer, int vertexCount, int baseVertex,
                                const GrBuffer* instanceBuffer, int instanceCount,
                                int baseInstance) final;

    void sendIndexedInstancedMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                       const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                                       const GrBuffer* vertexBuffer, int baseVertex,
                                       const GrBuffer* instanceBuffer, int instanceCount,
                                       int baseInstance) final;

    void onClear(const GrFixedClip&, GrColor color) override;

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override;

    void addAdditionalCommandBuffer();
    void addAdditionalRenderPass();

    struct InlineUploadInfo {
        InlineUploadInfo(GrOpFlushState* state, const GrDrawOp::DeferredUploadFn& upload)
            : fFlushState(state), fUpload(upload) {}

        GrOpFlushState* fFlushState;
        GrDrawOp::DeferredUploadFn fUpload;
    };

    struct CommandBufferInfo {
        const GrVkRenderPass*                  fRenderPass;
        SkTArray<GrVkSecondaryCommandBuffer*>  fCommandBuffers;
        VkClearValue                           fColorClearValue;
        VkClearValue                           fStencilClearValue;
        SkRect                                 fBounds;
        bool                                   fIsEmpty;
        bool                                   fStartsWithClear;
        SkTArray<InlineUploadInfo>             fPreDrawUploads;

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
    GrColor4f                   fClearColor;
    GrVkPipelineState*          fLastPipelineState;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif
