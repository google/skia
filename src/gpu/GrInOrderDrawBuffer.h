/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInOrderDrawBuffer_DEFINED
#define GrInOrderDrawBuffer_DEFINED

#include "GrDrawTarget.h"
#include "GrAllocPool.h"
#include "GrAllocator.h"
#include "GrIndexBuffer.h"
#include "GrRenderTarget.h"
#include "GrPath.h"
#include "GrPathRange.h"
#include "GrSurface.h"
#include "GrVertexBuffer.h"

#include "SkClipStack.h"
#include "SkTemplates.h"
#include "SkTypes.h"

class GrGpu;
class GrIndexBufferAllocPool;
class GrVertexBufferAllocPool;

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
class GrInOrderDrawBuffer : public GrDrawTarget {
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

    virtual ~GrInOrderDrawBuffer();

    /**
     * Empties the draw buffer of any queued up draws. This must not be called while inside an
     * unbalanced pushGeometrySource(). The current draw state and clip are preserved.
     */
    void reset();

    /**
     * This plays the queued up draws to its GrGpu target. It also resets this object (i.e. flushing
     * is destructive). This buffer must not have an active reserved vertex or index source. Any
     * reserved geometry on the target will be finalized because it's geometry source will be pushed
     * before flushing and popped afterwards.
     */
    void flush();

    // tracking for draws
    virtual DrawToken getCurrentDrawToken() { return DrawToken(this, fDrawID); }

    // overrides from GrDrawTarget
    virtual bool geometryHints(int* vertexCount,
                               int* indexCount) const SK_OVERRIDE;
    virtual void clear(const SkIRect* rect,
                       GrColor color,
                       bool canIgnoreRect,
                       GrRenderTarget* renderTarget) SK_OVERRIDE;

    virtual void discard(GrRenderTarget*) SK_OVERRIDE;

    virtual void initCopySurfaceDstDesc(const GrSurface* src, GrTextureDesc* desc) SK_OVERRIDE;

protected:
    virtual void clipWillBeSet(const GrClipData* newClip) SK_OVERRIDE;

private:
    enum Cmd {
        kDraw_Cmd           = 1,
        kStencilPath_Cmd    = 2,
        kSetState_Cmd       = 3,
        kSetClip_Cmd        = 4,
        kClear_Cmd          = 5,
        kCopySurface_Cmd    = 6,
        kDrawPath_Cmd       = 7,
        kDrawPaths_Cmd      = 8,
    };

    class Draw : public DrawInfo {
    public:
        Draw(const DrawInfo& info, const GrVertexBuffer* vb, const GrIndexBuffer* ib)
            : DrawInfo(info)
            , fVertexBuffer(vb)
            , fIndexBuffer(ib) {}

        const GrVertexBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
        const GrIndexBuffer* indexBuffer() const { return fIndexBuffer.get(); }

    private:
        GrPendingIOResource<const GrVertexBuffer, GrIORef::kRead_IOType>    fVertexBuffer;
        GrPendingIOResource<const GrIndexBuffer, GrIORef::kRead_IOType>     fIndexBuffer;
    };

    struct StencilPath : public ::SkNoncopyable {
        StencilPath(const GrPath* path) : fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        SkPath::FillType fFill;

    private:
        GrPendingIOResource<const GrPath, GrIORef::kRead_IOType>   fPath;
    };

    struct DrawPath : public ::SkNoncopyable {
        DrawPath(const GrPath* path) : fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        SkPath::FillType        fFill;
        GrDeviceCoordTexture    fDstCopy;

    private:
        GrPendingIOResource<const GrPath, GrIORef::kRead_IOType> fPath;
    };

    struct DrawPaths : public ::SkNoncopyable {
        DrawPaths(const GrPathRange* pathRange)
            : fPathRange(pathRange) {}

        ~DrawPaths() {
            if (fTransforms) {
                SkDELETE_ARRAY(fTransforms);
            }
            if (fIndices) {
                SkDELETE_ARRAY(fIndices);
            }
        }

        const GrPathRange* pathRange() const { return fPathRange.get();  }

        uint32_t*               fIndices;
        size_t                  fCount;
        float*                  fTransforms;
        PathTransformType       fTransformsType;
        SkPath::FillType        fFill;
        GrDeviceCoordTexture    fDstCopy;

    private:
        GrPendingIOResource<const GrPathRange, GrIORef::kRead_IOType> fPathRange;
    };

    // This is also used to record a discard by setting the color to GrColor_ILLEGAL
    struct Clear : public ::SkNoncopyable {
        Clear(GrRenderTarget* rt) : fRenderTarget(rt) {}
        ~Clear() { }
        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        SkIRect fRect;
        GrColor fColor;
        bool    fCanIgnoreRect;

    private:
        GrPendingIOResource<GrRenderTarget, GrIORef::kWrite_IOType> fRenderTarget;
    };

    struct CopySurface : public ::SkNoncopyable {
        CopySurface(GrSurface* dst, GrSurface* src) : fDst(dst), fSrc(src) {}

        GrSurface* dst() const { return fDst.get(); }
        GrSurface* src() const { return fSrc.get(); }

        SkIPoint    fDstPoint;
        SkIRect     fSrcRect;

    private:
        GrPendingIOResource<GrSurface, GrIORef::kWrite_IOType> fDst;
        GrPendingIOResource<GrSurface, GrIORef::kRead_IOType> fSrc;
    };

    struct Clip : public ::SkNoncopyable {
        SkClipStack fStack;
        SkIPoint    fOrigin;
    };

    // overrides from GrDrawTarget
    virtual void onDraw(const DrawInfo&) SK_OVERRIDE;
    virtual void onDrawRect(const SkRect& rect,
                            const SkRect* localRect,
                            const SkMatrix* localMatrix) SK_OVERRIDE;

    virtual void onStencilPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;
    virtual void onDrawPath(const GrPath*, SkPath::FillType,
                            const GrDeviceCoordTexture* dstCopy) SK_OVERRIDE;
    virtual void onDrawPaths(const GrPathRange*,
                             const uint32_t indices[], int count,
                             const float transforms[], PathTransformType,
                             SkPath::FillType, const GrDeviceCoordTexture*) SK_OVERRIDE;

    virtual bool onReserveVertexSpace(size_t vertexSize,
                                      int vertexCount,
                                      void** vertices) SK_OVERRIDE;
    virtual bool onReserveIndexSpace(int indexCount,
                                     void** indices) SK_OVERRIDE;
    virtual void releaseReservedVertexSpace() SK_OVERRIDE;
    virtual void releaseReservedIndexSpace() SK_OVERRIDE;
    virtual void onSetVertexSourceToArray(const void* vertexArray,
                                          int vertexCount) SK_OVERRIDE;
    virtual void onSetIndexSourceToArray(const void* indexArray,
                                         int indexCount) SK_OVERRIDE;
    virtual void releaseVertexArray() SK_OVERRIDE;
    virtual void releaseIndexArray() SK_OVERRIDE;
    virtual void geometrySourceWillPush() SK_OVERRIDE;
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState) SK_OVERRIDE;
    virtual void willReserveVertexAndIndexSpace(int vertexCount,
                                                int indexCount) SK_OVERRIDE;
    virtual bool onCopySurface(GrSurface* dst,
                               GrSurface* src,
                               const SkIRect& srcRect,
                               const SkIPoint& dstPoint)  SK_OVERRIDE;
    virtual bool onCanCopySurface(GrSurface* dst,
                                  GrSurface* src,
                                  const SkIRect& srcRect,
                                  const SkIPoint& dstPoint) SK_OVERRIDE;

    bool quickInsideClip(const SkRect& devBounds);

    virtual void didAddGpuTraceMarker() SK_OVERRIDE {}
    virtual void didRemoveGpuTraceMarker() SK_OVERRIDE {}

    // Attempts to concat instances from info onto the previous draw. info must represent an
    // instanced draw. The caller must have already recorded a new draw state and clip if necessary.
    int concatInstancedDraw(const DrawInfo& info);

    // Determines whether the current draw operation requieres a new drawstate and if so records it.
    void recordStateIfNecessary();
    // We lazily record clip changes in order to skip clips that have no effect.
    bool needsNewClip() const;

    // these functions record a command
    void            recordState();
    void            recordClip();
    Draw*           recordDraw(const DrawInfo&, const GrVertexBuffer*, const GrIndexBuffer*);
    StencilPath*    recordStencilPath(const GrPath*);
    DrawPath*       recordDrawPath(const GrPath*);
    DrawPaths*      recordDrawPaths(const GrPathRange*);
    Clear*          recordClear(GrRenderTarget*);
    CopySurface*    recordCopySurface(GrSurface* dst, GrSurface* src);

    virtual bool isIssued(uint32_t drawID) { return drawID != fDrawID; }
    void addToCmdBuffer(uint8_t cmd);

    // TODO: Use a single allocator for commands and records
    enum {
        kCmdPreallocCnt          = 32,
        kDrawPreallocCnt         = 16,
        kStencilPathPreallocCnt  = 8,
        kDrawPathPreallocCnt     = 8,
        kDrawPathsPreallocCnt    = 8,
        kStatePreallocCnt        = 8,
        kClipPreallocCnt         = 8,
        kClearPreallocCnt        = 8,
        kGeoPoolStatePreAllocCnt = 4,
        kCopySurfacePreallocCnt  = 4,
    };

    typedef GrTAllocator<Draw>          DrawAllocator;
    typedef GrTAllocator<StencilPath>   StencilPathAllocator;
    typedef GrTAllocator<DrawPath>      DrawPathAllocator;
    typedef GrTAllocator<DrawPaths>     DrawPathsAllocator;
    typedef GrTAllocator<GrDrawState>   StateAllocator;
    typedef GrTAllocator<Clear>         ClearAllocator;
    typedef GrTAllocator<CopySurface>   CopySurfaceAllocator;
    typedef GrTAllocator<Clip>          ClipAllocator;

    GrSTAllocator<kDrawPreallocCnt, Draw>               fDraws;
    GrSTAllocator<kStencilPathPreallocCnt, StencilPath> fStencilPaths;
    GrSTAllocator<kDrawPathPreallocCnt, DrawPath>       fDrawPath;
    GrSTAllocator<kDrawPathsPreallocCnt, DrawPaths>     fDrawPaths;
    GrSTAllocator<kStatePreallocCnt, GrDrawState>       fStates;
    GrSTAllocator<kClearPreallocCnt, Clear>             fClears;
    GrSTAllocator<kCopySurfacePreallocCnt, CopySurface> fCopySurfaces;
    GrSTAllocator<kClipPreallocCnt, Clip>               fClips;

    SkTArray<GrTraceMarkerSet, false>                   fGpuCmdMarkers;
    SkSTArray<kCmdPreallocCnt, uint8_t, true>           fCmds;
    GrDrawTarget*                                       fDstGpu;
    bool                                                fClipSet;

    enum ClipProxyState {
        kUnknown_ClipProxyState,
        kValid_ClipProxyState,
        kInvalid_ClipProxyState
    };

    ClipProxyState                                      fClipProxyState;
    SkRect                                              fClipProxy;
    GrVertexBufferAllocPool&                            fVertexPool;
    GrIndexBufferAllocPool&                             fIndexPool;

    struct GeometryPoolState {
        const GrVertexBuffer*   fPoolVertexBuffer;
        int                     fPoolStartVertex;
        const GrIndexBuffer*    fPoolIndexBuffer;
        int                     fPoolStartIndex;
        // caller may conservatively over reserve vertices / indices.
        // we release unused space back to allocator if possible
        // can only do this if there isn't an intervening pushGeometrySource()
        size_t                  fUsedPoolVertexBytes;
        size_t                  fUsedPoolIndexBytes;
    };

    typedef SkSTArray<kGeoPoolStatePreAllocCnt, GeometryPoolState> GeoPoolStateStack;

    GeoPoolStateStack                                   fGeoPoolStateStack;
    bool                                                fFlushing;
    uint32_t                                            fDrawID;

    typedef GrDrawTarget INHERITED;
};

#endif
