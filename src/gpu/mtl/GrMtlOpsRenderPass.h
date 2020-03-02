/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlOpsRenderPass_DEFINED
#define GrMtlOpsRenderPass_DEFINED

#include "src/gpu/GrMesh.h"
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

    void begin() override {}
    void end() override {}

    void initRenderState(id<MTLRenderCommandEncoder>);

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;
    void submit();

private:
    GrGpu* gpu() override { return fGpu; }

    bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) override;
    void onSetScissorRect(const SkIRect&) override;
    bool onBindTextures(const GrPrimitiveProcessor&, const GrPipeline&,
                        const GrSurfaceProxy* const primProcTextures[]) override;
    void onBindBuffers(const GrBuffer* indexBuffer, const GrBuffer* instanceBuffer,
                       const GrBuffer* vertexBuffer, GrPrimitiveRestart) override;
    void onDraw(int vertexCount, int baseVertex) override;
    void onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                       uint16_t maxIndexValue, int baseVertex) override;
    void onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                         int baseVertex) override;
    void onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                                int baseVertex) override;

    void onClear(const GrFixedClip& clip, const SkPMColor4f& color) override;

    void onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) override;

    void setupRenderPass(const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                         const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo);

    void setVertexBuffer(id<MTLRenderCommandEncoder>, const GrMtlBuffer*, size_t offset,
                         size_t inputBufferIndex);
    void resetBufferBindings();
    void precreateCmdEncoder();

    GrMtlGpu*                   fGpu;

    id<MTLRenderCommandEncoder> fActiveRenderCmdEncoder;
    GrMtlPipelineState*         fActivePipelineState = nullptr;
    MTLPrimitiveType            fActivePrimitiveType;
    MTLRenderPassDescriptor*    fRenderPassDesc;
    SkRect                      fBounds;

    // The index buffer in metal is an argument to the draw call, rather than a stateful binding.
    sk_sp<const GrMtlBuffer>    fIndexBuffer;

    // We defer binding of the vertex buffer because Metal doesn't have baseVertex for drawIndexed.
    sk_sp<const GrMtlBuffer>    fDeferredVertexBuffer;
    size_t                      fCurrentVertexStride;

    static constexpr size_t kNumBindings = GrMtlUniformHandler::kLastUniformBinding + 3;
    struct {
        id<MTLBuffer> fBuffer;
        size_t fOffset;
    } fBufferBindings[kNumBindings];

    typedef GrOpsRenderPass INHERITED;
};

#endif

