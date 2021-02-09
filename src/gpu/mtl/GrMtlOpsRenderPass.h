/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlOpsRenderPass_DEFINED
#define GrMtlOpsRenderPass_DEFINED

#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/mtl/GrMtlGpu.h"

#import <Metal/Metal.h>

typedef uint32_t GrColor;
class GrMtlBuffer;
class GrMtlPipelineState;
class GrMtlRenderTarget;

class GrMtlOpsRenderPass : public GrOpsRenderPass {
public:
    GrMtlOpsRenderPass(GrMtlGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                       const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                       const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo);

    ~GrMtlOpsRenderPass() override;

    void initRenderState(id<MTLRenderCommandEncoder>);

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;
    void submit();

private:
    GrGpu* gpu() override { return fGpu; }

    bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) override;
    void onSetScissorRect(const SkIRect&) override;
    bool onBindTextures(const GrPrimitiveProcessor&, const GrSurfaceProxy* const primProcTextures[],
                        const GrPipeline&) override;
    void onBindBuffers(sk_sp<const GrBuffer> indexBuffer, sk_sp<const GrBuffer> instanceBuffer,
                       sk_sp<const GrBuffer> vertexBuffer, GrPrimitiveRestart) override;
    void onDraw(int vertexCount, int baseVertex) override;
    void onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                       uint16_t maxIndexValue, int baseVertex) override;
    void onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                         int baseVertex) override;
    void onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                                int baseVertex) override;
    void onDrawIndirect(const GrBuffer* drawIndirectBuffer, size_t bufferOffset,
                        int drawCount) override;
    void onDrawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t bufferOffset,
                               int drawCount) override;

    void onClear(const GrScissorState& scissor, std::array<float, 4> color) override;

    void onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) override;

    void setupRenderPass(const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                         const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo);

    void setVertexBuffer(id<MTLRenderCommandEncoder>, const GrBuffer*, size_t offset,
                         size_t inputBufferIndex);
    void resetBufferBindings();
    void precreateCmdEncoder();

    GrMtlGpu*                   fGpu;

    id<MTLRenderCommandEncoder> fActiveRenderCmdEncoder;
    GrMtlPipelineState*         fActivePipelineState = nullptr;
    MTLPrimitiveType            fActivePrimitiveType;
    MTLRenderPassDescriptor*    fRenderPassDesc;
    SkRect                      fBounds;
    size_t                      fCurrentVertexStride;

    static constexpr size_t kNumBindings = GrMtlUniformHandler::kLastUniformBinding + 3;
    struct {
        id<MTLBuffer> fBuffer;
        size_t fOffset;
    } fBufferBindings[kNumBindings];

    using INHERITED = GrOpsRenderPass;
};

#endif
