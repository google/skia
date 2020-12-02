/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DOpsRenderPass_DEFINED
#define GrD3DOpsRenderPass_DEFINED

#include "src/gpu/GrOpsRenderPass.h"

#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"

class GrD3DGpu;
class GrD3DPipelineState;

class GrD3DOpsRenderPass : public GrOpsRenderPass {
public:
    GrD3DOpsRenderPass(GrD3DGpu*);

    ~GrD3DOpsRenderPass() override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void onExecuteDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) override {}

    bool set(GrRenderTarget*, GrSurfaceOrigin, const SkIRect& bounds,
        const GrOpsRenderPass::LoadAndStoreInfo&,
        const GrOpsRenderPass::StencilLoadAndStoreInfo&,
        const SkTArray<GrSurfaceProxy*, true>& sampledProxies);

private:
    GrGpu* gpu() override;

    void onBegin() override;

    bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) override;
    void onSetScissorRect(const SkIRect&) override;
    bool onBindTextures(const GrPrimitiveProcessor&, const GrSurfaceProxy* const primProcTextures[],
                        const GrPipeline&) override;
    void onBindBuffers(sk_sp<const GrBuffer> indexBuffer, sk_sp<const GrBuffer> instanceBuffer,
                       sk_sp<const GrBuffer> vertexBuffer, GrPrimitiveRestart) override;
    void onDraw(int vertexCount, int baseVertex) override {
        this->onDrawInstanced(1, 0, vertexCount, baseVertex);
    }
    void onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                       uint16_t maxIndexValue, int baseVertex) override {
        this->onDrawIndexedInstanced(indexCount, baseIndex, 1, 0, baseVertex);
    }
    void onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                         int baseVertex) override;
    void onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                                int baseVertex) override;
    void onDrawIndirect(const GrBuffer*, size_t offset, int drawCount) override;
    void onDrawIndexedIndirect(const GrBuffer*, size_t offset, int drawCount) override;

    void onClear(const GrScissorState& scissor, std::array<float, 4> color) override;

    void onClearStencilClip(const GrScissorState& scissor, bool insideStencilMask) override;

    GrD3DGpu* fGpu;

    sk_sp<GrD3DPipelineState> fCurrentPipelineState;

    SkIRect fBounds;
    SkIRect fCurrentPipelineBounds;

    GrLoadOp fColorLoadOp;
    std::array<float, 4> fClearColor;
    GrLoadOp fStencilLoadOp;

    using INHERITED = GrOpsRenderPass;
};

#endif
