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

    void discard() override;
    void insertEventMarker(const char*) override;

    void inlineUpload(GrOpFlushState* state, GrDeferredTextureUploadFn& upload) override;

    void copy(GrSurface* src, GrSurfaceOrigin srcOrigin, const SkIRect& srcRect,
              const SkIPoint& dstPoint) override;

    void submit() override;

private:
    GrGpu* gpu() override;

    void setScissorState(const GrPipeline&);
    void beginDraw(const GrPipeline& pipeline,
                   const GrPrimitiveProcessor& primProc,
                   const GrPrimitiveType primitiveType,
                   bool hasPoints);

    void endDraw();
    void onDraw(const GrPipeline& pipeline,
                const GrPrimitiveProcessor& primProc,
                const GrMesh mesh[],
                const GrPipeline::DynamicState[],
                int meshCount,
                const SkRect& bounds) override;

    void sendMeshToGpu(const GrPrimitiveProcessor& primProc, GrPrimitiveType primType,
                       const GrBuffer* vertexBuffer, int vertexCount, int baseVertex) final {
        this->sendInstancedMeshToGpu(primProc, primType, vertexBuffer, vertexCount, baseVertex,
                                     nullptr, 1, 0);
    }

    void sendIndexedMeshToGpu(const GrPrimitiveProcessor& primProc, GrPrimitiveType primType,
                              const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                              uint16_t /*minIndexValue*/, uint16_t /*maxIndexValue*/,
                              const GrBuffer* vertexBuffer, int baseVertex) final {
        this->sendIndexedInstancedMeshToGpu(primProc, primType, indexBuffer, indexCount, baseIndex,
                                            vertexBuffer, baseVertex, nullptr, 1, 0);
    }

    void sendInstancedMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                const GrBuffer* vertexBuffer, int vertexCount, int baseVertex,
                                const GrBuffer* instanceBuffer, int instanceCount,
                                int baseInstance) final;

    void sendIndexedInstancedMeshToGpu(const GrPrimitiveProcessor&, GrPrimitiveType,
                                       const GrBuffer* indexBuffer, int indexCount, int baseIndex,
                                       const GrBuffer* vertexBuffer, int baseVertex,
                                       const GrBuffer* instanceBuffer, int instanceCount,
                                       int baseInstance) final;

    void onClear(const GrFixedClip&, GrColor color) override;

    void onClearStencilClip(const GrFixedClip&, bool insideStencilMask) override;

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
    nxt::RenderPass             fRenderPass;
    uint32_t                    fRenderPassHash;
    nxt::CommandBufferBuilder   fBuilder;

    typedef GrGpuRTCommandBuffer INHERITED;
};

#endif
