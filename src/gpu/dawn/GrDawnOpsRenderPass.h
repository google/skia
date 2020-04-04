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
    void insertEventMarker(const char*) override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void submit();

private:
    GrGpu* gpu() override;

    void setScissorState(const GrProgramInfo&);
    void applyState(GrDawnProgram*, const GrProgramInfo& programInfo);

    void onDraw(const GrProgramInfo& programInfo,
                const GrMesh mesh[],
                int meshCount,
                const SkRect& bounds) override;

    void sendMeshToGpu(GrPrimitiveType primType, const GrBuffer* vertexBuffer, int vertexCount,
                       int baseVertex) final {
        this->sendInstancedMeshToGpu(primType, vertexBuffer, vertexCount, baseVertex,
                                     nullptr, 1, 0);
    }

    void sendIndexedMeshToGpu(GrPrimitiveType primType,
                              const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                              uint16_t /*minIndexValue*/, uint16_t /*maxIndexValue*/,
                              const GrBuffer* vertexBuffer, int baseVertex,
                              GrPrimitiveRestart restart) final {
        this->sendIndexedInstancedMeshToGpu(primType, indexBuffer, indexCount, baseIndex,
                                            vertexBuffer, baseVertex, nullptr, 1, 0, restart);
    }

    void sendInstancedMeshToGpu(GrPrimitiveType,
                                const GrBuffer* vertexBuffer, int vertexCount, int baseVertex,
                                const GrBuffer* instanceBuffer, int instanceCount,
                                int baseInstance) final;

    void sendIndexedInstancedMeshToGpu(GrPrimitiveType,
                                       const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                                       const GrBuffer* vertexBuffer, int baseVertex,
                                       const GrBuffer* instanceBuffer, int instanceCount,
                                       int baseInstance, GrPrimitiveRestart) final;

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
