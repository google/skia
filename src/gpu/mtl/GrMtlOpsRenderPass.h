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

class GrMtlOpsRenderPass : public GrOpsRenderPass, private GrMesh::SendToGpuImpl {
public:
    GrMtlOpsRenderPass(GrMtlGpu* gpu, GrRenderTarget* rt, GrSurfaceOrigin origin,
                       const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                       const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo);

    ~GrMtlOpsRenderPass() override;

    void begin() override {}
    void end() override {}

    void insertEventMarker(const char* msg) override {}

    void initRenderState(id<MTLRenderCommandEncoder>);

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override {
        // TODO: this could be more efficient
        state->doUpload(upload);
    }
    void submit();

private:
    GrGpu* gpu() override { return fGpu; }

    GrMtlPipelineState* prepareDrawState(const GrProgramInfo&);

    void onDraw(const GrProgramInfo& programInfo,
                const GrMesh mesh[],
                int meshCount,
                const SkRect& bounds) override;

    void onClear(const GrFixedClip& clip, const SkPMColor4f& color) override;

    void onClearStencilClip(const GrFixedClip& clip, bool insideStencilMask) override;

    void setupRenderPass(const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
                         const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo);

    void bindGeometry(const GrBuffer* vertexBuffer, size_t vertexOffset,
                      const GrBuffer* instanceBuffer);

    // GrMesh::SendToGpuImpl methods. These issue the actual Metal draw commands.
    // Marked final as a hint to the compiler to not use virtual dispatch.
    void sendMeshToGpu(GrPrimitiveType primType, const GrBuffer* vertexBuffer, int vertexCount,
                       int baseVertex) final;

    void sendIndexedMeshToGpu(GrPrimitiveType primType, const GrBuffer* indexBuffer, int indexCount,
                              int baseIndex, uint16_t /*minIndexValue*/, uint16_t /*maxIndexValue*/,
                              const GrBuffer* vertexBuffer, int baseVertex,
                              GrPrimitiveRestart restart) final;

    void sendInstancedMeshToGpu(GrPrimitiveType, const GrBuffer* vertexBuffer, int vertexCount,
                                int baseVertex, const GrBuffer* instanceBuffer, int instanceCount,
                                int baseInstance) final;

    void sendIndexedInstancedMeshToGpu(GrPrimitiveType, const GrBuffer* indexBuffer, int indexCount,
                                       int baseIndex, const GrBuffer* vertexBuffer, int baseVertex,
                                       const GrBuffer* instanceBuffer, int instanceCount,
                                       int baseInstance, GrPrimitiveRestart) final;

    void setVertexBuffer(id<MTLRenderCommandEncoder>, const GrMtlBuffer*, size_t offset,
                         size_t index);
    void resetBufferBindings();
    void precreateCmdEncoder();

    GrMtlGpu*                   fGpu;

    id<MTLRenderCommandEncoder> fActiveRenderCmdEncoder;
    MTLRenderPassDescriptor*    fRenderPassDesc;
    SkRect                      fBounds;
    size_t                      fCurrentVertexStride;

    static constexpr size_t kNumBindings = GrMtlUniformHandler::kLastUniformBinding + 3;
    struct {
        id<MTLBuffer> fBuffer;
        size_t fOffset;
    } fBufferBindings[kNumBindings];

    typedef GrOpsRenderPass INHERITED;
};

#endif

