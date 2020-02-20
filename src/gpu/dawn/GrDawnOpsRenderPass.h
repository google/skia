/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnOpsRenderPass_DEFINED
#define GrDawnOpsRenderPass_DEFINED

#include "src/gpu/GrOpsRenderPass.h"

#include "include/gpu/GrTypes.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrMesh.h"
#include "dawn/webgpu_cpp.h"

class GrDawnGpu;
class GrDawnRenderTarget;
struct GrDawnProgram;

class GrDawnOpsRenderPass : public GrOpsRenderPass, private GrMesh::SendToGpuImpl {
public:
    GrDawnOpsRenderPass(GrDawnGpu*, GrRenderTarget*, GrSurfaceOrigin,
                        const LoadAndStoreInfo&, const StencilLoadAndStoreInfo&);

    ~GrDawnOpsRenderPass() override;

    void begin() override { }
    void end() override;

    wgpu::RenderPassEncoder beginRenderPass(wgpu::LoadOp colorOp, wgpu::LoadOp stencilOp);

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void submit();

private:
    GrGpu* gpu() override;

    void setScissorState(const GrProgramInfo&);
    void applyState(GrDawnProgram*, const GrProgramInfo& programInfo);

    bool onBindPipeline(const GrProgramInfo& programInfo, const SkRect& drawBounds) override;
    void onDrawMeshes(const GrProgramInfo& programInfo,
                      const GrMesh mesh[],
                      int meshCount) override;

    void sendArrayMeshToGpu(GrPrimitiveType type, const GrMesh& mesh, int vertexCount,
                            int baseVertex) final {
        SkASSERT(!mesh.instanceBuffer());
        this->sendInstancedMeshToGpu(type, mesh, vertexCount, baseVertex, 1, 0);
    }
    void sendIndexedMeshToGpu(GrPrimitiveType type, const GrMesh& mesh, int indexCount,
                              int baseIndex, uint16_t minIndexValue, uint16_t maxIndexValue,
                              int baseVertex) final {
        SkASSERT(!mesh.instanceBuffer());
        this->sendIndexedInstancedMeshToGpu(type, mesh, indexCount, baseIndex, baseVertex, 1, 0);
    }
    void sendInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int vertexCount, int baseVertex,
                                int instanceCount, int baseInstance) final;
    void sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrMesh&, int indexCount,
                                       int baseIndex, int baseVertex, int instanceCount,
                                       int baseInstance) final;

    void onClear(const GrFixedClip&, const SkPMColor4f& color) override;

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override;

    struct InlineUploadInfo {
        InlineUploadInfo(GrOpFlushState* state, const GrDeferredTextureUploadFn& upload)
                : fFlushState(state), fUpload(upload) {}

        GrOpFlushState* fFlushState;
        GrDeferredTextureUploadFn fUpload;
    };

    GrDawnGpu*                  fGpu;
    wgpu::CommandEncoder        fEncoder;
    wgpu::RenderPassEncoder     fPassEncoder;
    LoadAndStoreInfo            fColorInfo;

    typedef GrOpsRenderPass     INHERITED;
};

#endif
