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
#include "src/gpu/GrMesh.h"

class GrD3DGpu;

class GrD3DOpsRenderPass : public GrOpsRenderPass {
public:
    GrD3DOpsRenderPass(GrD3DGpu*);

    ~GrD3DOpsRenderPass() override;

    void begin() override {}
    void end() override {}

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
    bool onBindTextures(const GrPrimitiveProcessor&, const GrPipeline&,
        const GrSurfaceProxy* const primProcTextures[]) override {
        return true;
    }
    void onDraw(const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) override {}
    void onDrawIndexed(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
        GrPrimitiveRestart primitiveRestart, uint16_t minIndexValue,
        uint16_t maxIndexValue, const GrBuffer* vertexBuffer,
        int baseVertex) override {}
    void onDrawInstanced(const GrBuffer* instanceBuffer, int instanceCount, int baseInstance,
        const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) override {}
    void onDrawIndexedInstanced(const GrBuffer* indexBuffer, int indexCount, int baseIndex,
        GrPrimitiveRestart, const GrBuffer* instanceBuffer,
        int instanceCount, int baseInstance, const GrBuffer* vertexBuffer,
        int baseVertex) override {}

    void onClear(const GrFixedClip&, const SkPMColor4f& color) override {}

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override {}

    GrD3DGpu* fGpu;

    typedef GrOpsRenderPass INHERITED;
};

#endif
