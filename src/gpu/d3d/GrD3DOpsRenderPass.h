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

class GrD3DOpsRenderPass : public GrOpsRenderPass, private GrMesh::SendToGpuImpl {
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
    void onDrawMesh(GrPrimitiveType, const GrMesh&) override {}

    void sendArrayMeshToGpu(GrPrimitiveType primitiveType, const GrMesh& mesh, int vertexCount,
        int baseVertex) final {}
    void sendIndexedMeshToGpu(GrPrimitiveType primitiveType, const GrMesh& mesh, int indexCount,
        int baseIndex, uint16_t minIndexValue, uint16_t maxIndexValue,
        int baseVertex) final {}
    void sendInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int vertexCount, int baseVertex,
        int instanceCount, int baseInstance) final {}
    void sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int indexCount,
        int baseIndex, int baseVertex, int instanceCount,
        int baseInstance) final {}

    void onClear(const GrFixedClip&, const SkPMColor4f& color) override {}

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override {}

    GrD3DGpu* fGpu;

    typedef GrOpsRenderPass INHERITED;
};

#endif
