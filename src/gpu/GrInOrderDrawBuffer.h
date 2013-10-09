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
#include "GrPath.h"

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
                       GrRenderTarget* renderTarget = NULL) SK_OVERRIDE;

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
    };

    class DrawRecord : public DrawInfo {
    public:
        DrawRecord(const DrawInfo& info) : DrawInfo(info) {}
        const GrVertexBuffer*   fVertexBuffer;
        const GrIndexBuffer*    fIndexBuffer;
    };

    struct StencilPath : public ::SkNoncopyable {
        StencilPath();

        SkAutoTUnref<const GrPath>  fPath;
        SkPath::FillType            fFill;
    };

    struct DrawPath : public ::SkNoncopyable {
        DrawPath();

        SkAutoTUnref<const GrPath>  fPath;
        SkPath::FillType            fFill;
        GrDeviceCoordTexture        fDstCopy;
    };

    struct Clear : public ::SkNoncopyable {
        Clear() : fRenderTarget(NULL) {}
        ~Clear() { SkSafeUnref(fRenderTarget); }

        SkIRect         fRect;
        GrColor         fColor;
        GrRenderTarget* fRenderTarget;
    };

    struct CopySurface : public ::SkNoncopyable {
        SkAutoTUnref<GrSurface> fDst;
        SkAutoTUnref<GrSurface> fSrc;
        SkIRect                 fSrcRect;
        SkIPoint                fDstPoint;
    };

    // overrides from GrDrawTarget
    virtual void onDraw(const DrawInfo&) SK_OVERRIDE;
    virtual void onDrawRect(const SkRect& rect,
                            const SkMatrix* matrix,
                            const SkRect* localRect,
                            const SkMatrix* localMatrix) SK_OVERRIDE;

    virtual void onStencilPath(const GrPath*, SkPath::FillType) SK_OVERRIDE;
    virtual void onDrawPath(const GrPath*, SkPath::FillType,
                            const GrDeviceCoordTexture* dstCopy) SK_OVERRIDE;

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

    // Attempts to concat instances from info onto the previous draw. info must represent an
    // instanced draw. The caller must have already recorded a new draw state and clip if necessary.
    int concatInstancedDraw(const DrawInfo& info);

    // we lazily record state and clip changes in order to skip clips and states that have no
    // effect.
    bool needsNewState() const;
    bool needsNewClip() const;

    // these functions record a command
    void            recordState();
    void            recordClip();
    DrawRecord*     recordDraw(const DrawInfo&);
    StencilPath*    recordStencilPath();
    DrawPath*       recordDrawPath();
    Clear*          recordClear();
    CopySurface*    recordCopySurface();

    // TODO: Use a single allocator for commands and records
    enum {
        kCmdPreallocCnt          = 32,
        kDrawPreallocCnt         = 8,
        kStencilPathPreallocCnt  = 8,
        kDrawPathPreallocCnt     = 8,
        kStatePreallocCnt        = 8,
        kClipPreallocCnt         = 8,
        kClearPreallocCnt        = 4,
        kGeoPoolStatePreAllocCnt = 4,
        kCopySurfacePreallocCnt  = 4,
    };

    SkSTArray<kCmdPreallocCnt, uint8_t, true>                          fCmds;
    GrSTAllocator<kDrawPreallocCnt, DrawRecord>                        fDraws;
    GrSTAllocator<kStatePreallocCnt, StencilPath>                      fStencilPaths;
    GrSTAllocator<kStatePreallocCnt, DrawPath>                         fDrawPaths;
    GrSTAllocator<kStatePreallocCnt, GrDrawState::DeferredState>       fStates;
    GrSTAllocator<kClearPreallocCnt, Clear>                            fClears;
    GrSTAllocator<kCopySurfacePreallocCnt, CopySurface>                fCopySurfaces;
    GrSTAllocator<kClipPreallocCnt, SkClipStack>                       fClips;
    GrSTAllocator<kClipPreallocCnt, SkIPoint>                          fClipOrigins;

    GrDrawTarget*                   fDstGpu;

    bool                            fClipSet;

    enum ClipProxyState {
        kUnknown_ClipProxyState,
        kValid_ClipProxyState,
        kInvalid_ClipProxyState
    };
    ClipProxyState                  fClipProxyState;
    SkRect                          fClipProxy;

    GrVertexBufferAllocPool&        fVertexPool;

    GrIndexBufferAllocPool&         fIndexPool;

    struct GeometryPoolState {
        const GrVertexBuffer*           fPoolVertexBuffer;
        int                             fPoolStartVertex;
        const GrIndexBuffer*            fPoolIndexBuffer;
        int                             fPoolStartIndex;
        // caller may conservatively over reserve vertices / indices.
        // we release unused space back to allocator if possible
        // can only do this if there isn't an intervening pushGeometrySource()
        size_t                          fUsedPoolVertexBytes;
        size_t                          fUsedPoolIndexBytes;
    };
    SkSTArray<kGeoPoolStatePreAllocCnt, GeometryPoolState> fGeoPoolStateStack;

    virtual bool       isIssued(uint32_t drawID) { return drawID != fDrawID; }

    bool                            fFlushing;
    uint32_t                        fDrawID;

    typedef GrDrawTarget INHERITED;
};

#endif
