/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTGpuCommandBuffer_DEFINED
#define GrNXTGpuCommandBuffer_DEFINED

#include "GrGpuCommandBuffer.h"

#include "GrColor.h"
#include "GrMesh.h"
#include "GrTypes.h"
#include "nxt/nxtcpp.h"

class GrNXTGpu;
class GrNXTRenderTarget;

class GrNXTGpuTextureCommandBuffer : public GrGpuTextureCommandBuffer {
public:
    GrNXTGpuTextureCommandBuffer(GrNXTGpu* gpu, GrTexture* texture, GrSurfaceOrigin origin)
        : INHERITED(texture, origin)
        , fGpu(gpu) {
    }

    ~GrNXTGpuTextureCommandBuffer() override;

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override;

    void insertEventMarker(const char*) override;

private:
    void submit() override;

    struct CopyInfo {
        CopyInfo(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                 const SkIPoint& dstPoint)
            : fSrc(src), fSrcOrigin(srcOrigin), fSrcRect(srcRect), fDstPoint(dstPoint) {}

        GrSurface*      fSrc;
        GrSurfaceOrigin fSrcOrigin;
        SkIRect         fSrcRect;
        SkIPoint        fDstPoint;
    };

    GrNXTGpu*                   fGpu;
    SkTArray<CopyInfo>          fCopies;

    typedef GrGpuTextureCommandBuffer INHERITED;
};

class GrNXTGpuRTCommandBuffer : public GrGpuRTCommandBuffer, private GrMesh::SendToGpuImpl {
public:
    GrNXTGpuRTCommandBuffer(GrNXTGpu*, GrRenderTarget*, GrSurfaceOrigin,
                           const LoadAndStoreInfo&,
                           const StencilLoadAndStoreInfo&);

    ~GrNXTGpuRTCommandBuffer() override;

    void begin() override { }
    void end() override;

    nxt::RenderPassDescriptor createRenderPassDescriptor(nxt::LoadOp colorOp,
                                                         nxt::LoadOp stencilOp) const;

    void discard() override;
    void insertEventMarker(const char*) override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override;

    void submit() override;

private:
    GrGpu* gpu() override;

    void setScissorState(const GrPipeline&,
                         const GrPipeline::FixedDynamicState* fixedDynamicState,
                         const GrPipeline::DynamicStateArrays* dynamicStateArrays);
    void applyState(const GrPipeline& pipeline,
                    const GrPrimitiveProcessor& primProc,
                    const GrPipeline::FixedDynamicState* fixedDynamicState,
                    const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                    const GrPrimitiveType primitiveType,
                    bool hasPoints);

    void onDraw(const GrPrimitiveProcessor& primProc,
                const GrPipeline& pipeline,
                const GrPipeline::FixedDynamicState* fixedDynamicState,
                const GrPipeline::DynamicStateArrays* dynamicStateArrays,
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

    void onClear(const GrFixedClip&, GrColor color) override;

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override;

    void tick();

    struct InlineUploadInfo {
        InlineUploadInfo(GrOpFlushState* state, const GrDeferredTextureUploadFn& upload)
                : fFlushState(state), fUpload(upload) {}

        GrOpFlushState* fFlushState;
        GrDeferredTextureUploadFn fUpload;
    };

    struct CopyInfo {
        CopyInfo(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
                 const SkIPoint& dstPoint)
            : fSrc(src), fSrcOrigin(srcOrigin), fSrcRect(srcRect), fDstPoint(dstPoint) {}

        GrSurface*      fSrc;
        GrSurfaceOrigin fSrcOrigin;
        SkIRect         fSrcRect;
        SkIPoint        fDstPoint;
    };

    GrNXTGpu*                   fGpu;
    nxt::RenderPassDescriptor   fRenderPassDescriptor;
    nxt::CommandBufferBuilder   fBuilder;
    LoadAndStoreInfo            fColorInfo;
    StencilLoadAndStoreInfo     fStencilInfo;
    size_t                      fVertexStride;      // Used during GrMesh callbacks; temporary.

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif
