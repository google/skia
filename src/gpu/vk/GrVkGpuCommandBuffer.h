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
#include "GrTypes.h"
#include "GrVkPipelineState.h"

class GrNonInstancedMesh;
class GrVkGpu;
class GrVkImage;
class GrVkRenderPass;
class GrVkRenderTarget;
class GrVkSecondaryCommandBuffer;

class GrVkGpuCommandBuffer : public GrGpuCommandBuffer {
public:
    GrVkGpuCommandBuffer(GrVkGpu* gpu,
                         const LoadAndStoreInfo& colorInfo,
                         const LoadAndStoreInfo& stencilInfo);

    ~GrVkGpuCommandBuffer() override;

    void end() override;

    void discard(GrRenderTarget*) override;

    void inlineUpload(GrOpFlushState* state, GrDrawOp::DeferredUploadFn& upload,
                      GrRenderTarget*) override;

private:
    // Performs lazy initialization on the first operation seen by the command buffer.
    void init(GrVkRenderTarget* rt);

    GrGpu* gpu() override;
    GrRenderTarget* renderTarget() override;

    void onSubmit() override;

    // Bind vertex and index buffers
    void bindGeometry(const GrPrimitiveProcessor&, const GrNonInstancedMesh&);

    sk_sp<GrVkPipelineState> prepareDrawState(const GrPipeline&,
                                              const GrPrimitiveProcessor&,
                                              GrPrimitiveType);

    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh* mesh,
                int meshCount,
                const SkRect& bounds) override;

    void onClear(GrRenderTarget*, const GrFixedClip&, GrColor color) override;

    void onClearStencilClip(GrRenderTarget*, const GrFixedClip&, bool insideStencilMask) override;

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
    GrVkRenderTarget*           fRenderTarget;
    VkAttachmentLoadOp          fVkColorLoadOp;
    VkAttachmentStoreOp         fVkColorStoreOp;
    VkAttachmentLoadOp          fVkStencilLoadOp;
    VkAttachmentStoreOp         fVkStencilStoreOp;
    GrColor4f                   fClearColor;
    GrVkPipelineState*          fLastPipelineState;

    typedef GrGpuCommandBuffer INHERITED;
};

#endif
