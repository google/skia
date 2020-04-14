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

class GrD3DOpsRenderPass : public GrOpsRenderPass {
public:
    GrD3DOpsRenderPass(GrD3DGpu*);

    ~GrD3DOpsRenderPass() override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override {}

    void onExecuteDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler>) override {}

    bool set(GrRenderTarget*, GrSurfaceOrigin, const SkIRect& bounds,
        const GrOpsRenderPass::LoadAndStoreInfo&,
        const GrOpsRenderPass::StencilLoadAndStoreInfo&,
        const SkTArray<GrSurfaceProxy*, true>& sampledProxies);

private:
    GrGpu* gpu() override;

    bool onBindPipeline(const GrProgramInfo&, const SkRect& drawBounds) override { return true; }
    void onSetScissorRect(const SkIRect&) override {}
    bool onBindTextures(const GrPrimitiveProcessor&, const GrSurfaceProxy* const primProcTextures[],
                        const GrPipeline&) override {
        return true;
    }
    void onBindBuffers(const GrBuffer* indexBuffer, const GrBuffer* instanceBuffer,
                       const GrBuffer* vertexBuffer, GrPrimitiveRestart) override {}
    void onDraw(int vertexCount, int baseVertex) override {}
    void onDrawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                       uint16_t maxIndexValue, int baseVertex) override {}
    void onDrawInstanced(int instanceCount, int baseInstance, int vertexCount,
                         int baseVertex) override {}
    void onDrawIndexedInstanced(int indexCount, int baseIndex, int instanceCount, int baseInstance,
                                int baseVertex) override {}

    void onClear(const GrFixedClip&, const SkPMColor4f& color) override {}

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override {}

    GrD3DGpu* fGpu;

    typedef GrOpsRenderPass INHERITED;
};

#endif
