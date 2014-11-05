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
#include "GrTRecorder.h"
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
class GrInOrderDrawBuffer : public GrClipTarget {
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

    virtual bool copySurface(GrSurface* dst,
                             GrSurface* src,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint)  SK_OVERRIDE;

    virtual bool canCopySurface(GrSurface* dst,
                                GrSurface* src,
                                const SkIRect& srcRect,
                                const SkIPoint& dstPoint) SK_OVERRIDE;

    virtual void clearStencilClip(const SkIRect& rect,
                                  bool insideClip,
                                  GrRenderTarget* renderTarget) SK_OVERRIDE;

    virtual void discard(GrRenderTarget*) SK_OVERRIDE;

    virtual void initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) SK_OVERRIDE;

protected:
    virtual void clipWillBeSet(const GrClipData* newClip) SK_OVERRIDE;

private:
    enum {
        kDraw_Cmd           = 1,
        kStencilPath_Cmd    = 2,
        kSetState_Cmd       = 3,
        kSetClip_Cmd        = 4,
        kClear_Cmd          = 5,
        kCopySurface_Cmd    = 6,
        kDrawPath_Cmd       = 7,
        kDrawPaths_Cmd      = 8,
    };

    struct Cmd : ::SkNoncopyable {
        Cmd(uint8_t type) : fType(type) {}
        virtual ~Cmd() {}

        virtual void execute(GrClipTarget*) = 0;

        uint8_t fType;
    };

    struct Draw : public Cmd {
        Draw(const DrawInfo& info, const GrVertexBuffer* vb, const GrIndexBuffer* ib)
            : Cmd(kDraw_Cmd)
            , fInfo(info)
            , fVertexBuffer(vb)
            , fIndexBuffer(ib) {}

        const GrVertexBuffer* vertexBuffer() const { return fVertexBuffer.get(); }
        const GrIndexBuffer* indexBuffer() const { return fIndexBuffer.get(); }

        virtual void execute(GrClipTarget*);

        DrawInfo fInfo;

    private:
        GrPendingIOResource<const GrVertexBuffer, kRead_GrIOType>    fVertexBuffer;
        GrPendingIOResource<const GrIndexBuffer, kRead_GrIOType>     fIndexBuffer;
    };

    struct StencilPath : public Cmd {
        StencilPath(const GrPath* path) : Cmd(kStencilPath_Cmd), fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        virtual void execute(GrClipTarget*);

        GrPathRendering::FillType fFill;

    private:
        GrPendingIOResource<const GrPath, kRead_GrIOType>   fPath;
    };

    struct DrawPath : public Cmd {
        DrawPath(const GrPath* path) : Cmd(kDrawPath_Cmd), fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        virtual void execute(GrClipTarget*);

        GrPathRendering::FillType fFill;
        GrDeviceCoordTexture      fDstCopy;

    private:
        GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;
    };

    struct DrawPaths : public Cmd {
        DrawPaths(const GrPathRange* pathRange) : Cmd(kDrawPaths_Cmd), fPathRange(pathRange) {}

        const GrPathRange* pathRange() const { return fPathRange.get();  }
        uint32_t* indices() { return reinterpret_cast<uint32_t*>(CmdBuffer::GetDataForItem(this)); }
        float* transforms() { return reinterpret_cast<float*>(&this->indices()[fCount]); }

        virtual void execute(GrClipTarget*);

        size_t                    fCount;
        PathTransformType         fTransformsType;
        GrPathRendering::FillType fFill;
        GrDeviceCoordTexture      fDstCopy;

    private:
        GrPendingIOResource<const GrPathRange, kRead_GrIOType> fPathRange;
    };

    // This is also used to record a discard by setting the color to GrColor_ILLEGAL
    struct Clear : public Cmd {
        Clear(GrRenderTarget* rt) : Cmd(kClear_Cmd), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        virtual void execute(GrClipTarget*);

        SkIRect fRect;
        GrColor fColor;
        bool    fCanIgnoreRect;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    // This command is ONLY used by the clip mask manager to clear the stencil clip bits
    struct ClearStencilClip : public Cmd {
        ClearStencilClip(GrRenderTarget* rt) : Cmd(kClear_Cmd), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        virtual void execute(GrClipTarget*);

        SkIRect fRect;
        bool    fInsideClip;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    struct CopySurface : public Cmd {
        CopySurface(GrSurface* dst, GrSurface* src) : Cmd(kCopySurface_Cmd), fDst(dst), fSrc(src) {}

        GrSurface* dst() const { return fDst.get(); }
        GrSurface* src() const { return fSrc.get(); }

        virtual void execute(GrClipTarget*);

        SkIPoint    fDstPoint;
        SkIRect     fSrcRect;

    private:
        GrPendingIOResource<GrSurface, kWrite_GrIOType> fDst;
        GrPendingIOResource<GrSurface, kRead_GrIOType> fSrc;
    };

    struct SetState : public Cmd {
        SetState(const GrDrawState& state) : Cmd(kSetState_Cmd), fState(state) {}

        virtual void execute(GrClipTarget*);

        GrDrawState fState;
    };

    struct SetClip : public Cmd {
        SetClip(const GrClipData* clipData)
            : Cmd(kSetClip_Cmd),
              fStackStorage(*clipData->fClipStack) {
            fClipData.fClipStack = &fStackStorage;
            fClipData.fOrigin = clipData->fOrigin;
        }

        virtual void execute(GrClipTarget*);

        GrClipData fClipData;

    private:
        SkClipStack fStackStorage;
    };

    typedef void* TCmdAlign; // This wouldn't be enough align if a command used long double.
    typedef GrTRecorder<Cmd, TCmdAlign> CmdBuffer;

    // overrides from GrDrawTarget
    virtual void onDraw(const DrawInfo&) SK_OVERRIDE;
    virtual void onDrawRect(const SkRect& rect,
                            const SkRect* localRect,
                            const SkMatrix* localMatrix) SK_OVERRIDE;

    virtual void onStencilPath(const GrPath*, GrPathRendering::FillType) SK_OVERRIDE;
    virtual void onDrawPath(const GrPath*, GrPathRendering::FillType,
                            const GrDeviceCoordTexture* dstCopy) SK_OVERRIDE;
    virtual void onDrawPaths(const GrPathRange*,
                             const uint32_t indices[], int count,
                             const float transforms[], PathTransformType,
                             GrPathRendering::FillType, const GrDeviceCoordTexture*) SK_OVERRIDE;
    virtual void onClear(const SkIRect* rect,
                         GrColor color,
                         bool canIgnoreRect,
                         GrRenderTarget* renderTarget) SK_OVERRIDE;

    virtual bool onReserveVertexSpace(size_t vertexSize,
                                      int vertexCount,
                                      void** vertices) SK_OVERRIDE;
    virtual bool onReserveIndexSpace(int indexCount,
                                     void** indices) SK_OVERRIDE;
    virtual void releaseReservedVertexSpace() SK_OVERRIDE;
    virtual void releaseReservedIndexSpace() SK_OVERRIDE;
    virtual void geometrySourceWillPush() SK_OVERRIDE;
    virtual void geometrySourceWillPop(const GeometrySrcState& restoredState) SK_OVERRIDE;
    virtual void willReserveVertexAndIndexSpace(int vertexCount,
                                                int indexCount) SK_OVERRIDE;

    bool quickInsideClip(const SkRect& devBounds);

    virtual void didAddGpuTraceMarker() SK_OVERRIDE {}
    virtual void didRemoveGpuTraceMarker() SK_OVERRIDE {}

    // Attempts to concat instances from info onto the previous draw. info must represent an
    // instanced draw. The caller must have already recorded a new draw state and clip if necessary.
    int concatInstancedDraw(const DrawInfo& info);

    // Determines whether the current draw operation requieres a new drawstate and if so records it.
    void recordStateIfNecessary();
    // We lazily record clip changes in order to skip clips that have no effect.
    void recordClipIfNecessary();
    // Records any trace markers for a command after adding it to the buffer.
    void recordTraceMarkersIfNecessary();

    virtual bool isIssued(uint32_t drawID) { return drawID != fDrawID; }

    // TODO: Use a single allocator for commands and records
    enum {
        kCmdBufferInitialSizeInBytes = 64 * 1024,
        kGeoPoolStatePreAllocCnt     = 4,
    };

    CmdBuffer                         fCmdBuffer;
    GrDrawState*                      fLastState;
    GrClipData*                       fLastClip;

    SkTArray<GrTraceMarkerSet, false> fGpuCmdMarkers;
    GrClipTarget*                     fDstGpu;
    bool                              fClipSet;

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

    typedef GrClipTarget INHERITED;
};

#endif
