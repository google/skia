/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInOrderDrawBuffer_DEFINED
#define GrInOrderDrawBuffer_DEFINED

#include "GrFlushToGpuDrawTarget.h"
#include "GrTargetCommands.h"
#include "SkChunkAlloc.h"

/**
 * GrInOrderDrawBuffer is an implementation of GrDrawTarget that queues up draws for eventual
 * playback into a GrGpu. In theory one draw buffer could playback into another. When index or
 * vertex buffers are used as geometry sources it is the callers the draw buffer only holds
 * references to the buffers. It is the callers responsibility to ensure that the data is still
 * valid when the draw buffer is played back into a GrGpu. Similarly, it is the caller's
 * responsibility to ensure that all referenced textures, buffers, and render-targets are associated
 * in the GrGpu object that the buffer is played back into. The buffer requires VB and IB pools to
 * store geometry.
 */
class GrInOrderDrawBuffer : public GrFlushToGpuDrawTarget {
public:

    /**
     * Creates a GrInOrderDrawBuffer
     *
     * @param gpu        the gpu object that this draw buffer flushes to.
     * @param vertexPool pool where vertices for queued draws will be saved when
     *                   the vertex source is either reserved or array.
     * @param indexPool  pool where indices for queued draws will be saved when
     *                   the index source is either reserved or array.
     */
    GrInOrderDrawBuffer(GrGpu* gpu,
                        GrVertexBufferAllocPool* vertexPool,
                        GrIndexBufferAllocPool* indexPool);

    ~GrInOrderDrawBuffer() SK_OVERRIDE;

    // tracking for draws
    DrawToken getCurrentDrawToken() SK_OVERRIDE { return DrawToken(this, fDrawID); }

    void clearStencilClip(const SkIRect& rect,
                          bool insideClip,
                          GrRenderTarget* renderTarget) SK_OVERRIDE;

    void discard(GrRenderTarget*) SK_OVERRIDE;

protected:
    void willReserveVertexAndIndexSpace(int vertexCount,
                                        size_t vertexStride,
                                        int indexCount) SK_OVERRIDE;

    void appendIndicesAndTransforms(const void* indexValues, PathIndexType indexType, 
                                    const float* transformValues, PathTransformType transformType,
                                    int count, char** indicesLocation, float** xformsLocation) {
        int indexBytes = GrPathRange::PathIndexSizeInBytes(indexType);
        *indicesLocation = (char*) fPathIndexBuffer.alloc(count * indexBytes,
                                                          SkChunkAlloc::kThrow_AllocFailType);
        SkASSERT(SkIsAlign4((uintptr_t)*indicesLocation));
        memcpy(*indicesLocation, reinterpret_cast<const char*>(indexValues), count * indexBytes);

        const int xformBytes = GrPathRendering::PathTransformSize(transformType) * sizeof(float);
        *xformsLocation = NULL;

        if (0 != xformBytes) {
            *xformsLocation = (float*) fPathTransformBuffer.alloc(count * xformBytes,
                                                               SkChunkAlloc::kThrow_AllocFailType);
            SkASSERT(SkIsAlign4((uintptr_t)*xformsLocation));
            memcpy(*xformsLocation, transformValues, count * xformBytes);
        }
    }

    bool canConcatToIndexBuffer(const GrIndexBuffer** ib) {
        const GrDrawTarget::GeometrySrcState& geomSrc = this->getGeomSrc();

        // we only attempt to concat when reserved verts are used with a client-specified
        // index buffer. To make this work with client-specified VBs we'd need to know if the VB
        // was updated between draws.
        if (kReserved_GeometrySrcType != geomSrc.fVertexSrc ||
            kBuffer_GeometrySrcType != geomSrc.fIndexSrc) {
            return false;
        }

        *ib = geomSrc.fIndexBuffer;
        return true;
    }

private:
    friend class GrTargetCommands;

    void onReset() SK_OVERRIDE;
    void onFlush() SK_OVERRIDE;

    // overrides from GrDrawTarget
    void onDraw(const GrGeometryProcessor*, const DrawInfo&, const PipelineInfo&) SK_OVERRIDE;
    void onDrawBatch(GrBatch*, const PipelineInfo&) SK_OVERRIDE;
    void onDrawRect(GrPipelineBuilder*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix) SK_OVERRIDE;

    void onStencilPath(const GrPipelineBuilder&,
                       const GrPathProcessor*,
                       const GrPath*,
                       const GrScissorState&,
                       const GrStencilSettings&) SK_OVERRIDE;
    void onDrawPath(const GrPathProcessor*,
                    const GrPath*,
                    const GrStencilSettings&,
                    const PipelineInfo&) SK_OVERRIDE;
    void onDrawPaths(const GrPathProcessor*,
                     const GrPathRange*,
                     const void* indices,
                     PathIndexType,
                     const float transformValues[],
                     PathTransformType,
                     int count,
                     const GrStencilSettings&,
                     const PipelineInfo&) SK_OVERRIDE;
    void onClear(const SkIRect* rect,
                 GrColor color,
                 bool canIgnoreRect,
                 GrRenderTarget* renderTarget) SK_OVERRIDE;
    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) SK_OVERRIDE;

    // Attempts to concat instances from info onto the previous draw. info must represent an
    // instanced draw. The caller must have already recorded a new draw state and clip if necessary.
    int concatInstancedDraw(const DrawInfo&);

    // We lazily record clip changes in order to skip clips that have no effect.
    void recordClipIfNecessary();
    // Records any trace markers for a command
    void recordTraceMarkersIfNecessary(GrTargetCommands::Cmd*);
    SkString getCmdString(int index) const {
        SkASSERT(index < fGpuCmdMarkers.count());
        return fGpuCmdMarkers[index].toString();
    }
    bool isIssued(uint32_t drawID) SK_OVERRIDE { return drawID != fDrawID; }

    // TODO: Use a single allocator for commands and records
    enum {
        kPathIdxBufferMinReserve     = 2 * 64,  // 64 uint16_t's
        kPathXformBufferMinReserve   = 2 * 64,  // 64 two-float transforms
    };

    GrTargetCommands                    fCommands;
    SkTArray<GrTraceMarkerSet, false>   fGpuCmdMarkers;
    SkChunkAlloc                        fPathIndexBuffer;
    SkChunkAlloc                        fPathTransformBuffer;
    uint32_t                            fDrawID;

    typedef GrFlushToGpuDrawTarget INHERITED;
};

#endif
