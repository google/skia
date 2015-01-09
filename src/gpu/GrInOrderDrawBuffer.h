/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrInOrderDrawBuffer_DEFINED
#define GrInOrderDrawBuffer_DEFINED

#include "GrFlushToGpuDrawTarget.h"
#include "GrOptDrawState.h"
#include "GrPath.h"
#include "GrTRecorder.h"

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

private:
    enum {
        kDraw_Cmd           = 1,
        kStencilPath_Cmd    = 2,
        kSetState_Cmd       = 3,
        kClear_Cmd          = 4,
        kCopySurface_Cmd    = 5,
        kDrawPath_Cmd       = 6,
        kDrawPaths_Cmd      = 7,
    };

    struct Cmd : ::SkNoncopyable {
        Cmd(uint8_t type) : fType(type) {}
        virtual ~Cmd() {}

        virtual void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) = 0;

        uint8_t fType;
    };

    struct Draw : public Cmd {
        Draw(const DrawInfo& info) : Cmd(kDraw_Cmd), fInfo(info) {}

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

        DrawInfo     fInfo;
    };

    struct StencilPath : public Cmd {
        StencilPath(const GrPath* path, GrRenderTarget* rt)
            : Cmd(kStencilPath_Cmd)
            , fRenderTarget(rt)
            , fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

        SkMatrix                                                fViewMatrix;
        bool                                                    fUseHWAA;
        GrStencilSettings                                       fStencil;
        GrScissorState                                          fScissor;
    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType>    fRenderTarget;
        GrPendingIOResource<const GrPath, kRead_GrIOType>       fPath;
    };

    struct DrawPath : public Cmd {
        DrawPath(const GrPath* path) : Cmd(kDrawPath_Cmd), fPath(path) {}

        const GrPath* path() const { return fPath.get(); }

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

        GrStencilSettings       fStencilSettings;

    private:
        GrPendingIOResource<const GrPath, kRead_GrIOType> fPath;
    };

    struct DrawPaths : public Cmd {
        DrawPaths(const GrPathRange* pathRange) : Cmd(kDrawPaths_Cmd), fPathRange(pathRange) {}

        const GrPathRange* pathRange() const { return fPathRange.get();  }

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

        int                     fIndicesLocation;
        PathIndexType           fIndexType;
        int                     fTransformsLocation;
        PathTransformType       fTransformType;
        int                     fCount;
        GrStencilSettings       fStencilSettings;

    private:
        GrPendingIOResource<const GrPathRange, kRead_GrIOType> fPathRange;
    };

    // This is also used to record a discard by setting the color to GrColor_ILLEGAL
    struct Clear : public Cmd {
        Clear(GrRenderTarget* rt) : Cmd(kClear_Cmd), fRenderTarget(rt) {}

        GrRenderTarget* renderTarget() const { return fRenderTarget.get(); }

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

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

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

        SkIRect fRect;
        bool    fInsideClip;

    private:
        GrPendingIOResource<GrRenderTarget, kWrite_GrIOType> fRenderTarget;
    };

    struct CopySurface : public Cmd {
        CopySurface(GrSurface* dst, GrSurface* src) : Cmd(kCopySurface_Cmd), fDst(dst), fSrc(src) {}

        GrSurface* dst() const { return fDst.get(); }
        GrSurface* src() const { return fSrc.get(); }

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

        SkIPoint    fDstPoint;
        SkIRect     fSrcRect;

    private:
        GrPendingIOResource<GrSurface, kWrite_GrIOType> fDst;
        GrPendingIOResource<GrSurface, kRead_GrIOType> fSrc;
    };

    struct SetState : public Cmd {
        SetState(const GrDrawState& drawState, const GrGeometryProcessor* gp,
                 const GrPathProcessor* pp, const GrDrawTargetCaps& caps,
                 const GrScissorState& scissor, const GrDeviceCoordTexture* dstCopy,
                 GrGpu::DrawType drawType)
        : Cmd(kSetState_Cmd)
        , fState(drawState, gp, pp, caps, scissor, dstCopy, drawType) {}

        void execute(GrInOrderDrawBuffer*, const GrOptDrawState*) SK_OVERRIDE;

        GrOptDrawState          fState;
    };

    typedef void* TCmdAlign; // This wouldn't be enough align if a command used long double.
    typedef GrTRecorder<Cmd, TCmdAlign> CmdBuffer;

    void onReset() SK_OVERRIDE;
    void onFlush() SK_OVERRIDE;

    // overrides from GrDrawTarget
    void onDraw(const GrDrawState&,
                const GrGeometryProcessor*,
                const DrawInfo&,
                const GrScissorState&,
                const GrDeviceCoordTexture* dstCopy) SK_OVERRIDE;
    void onDrawRect(GrDrawState*,
                    GrColor,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix) SK_OVERRIDE;

    void onStencilPath(const GrDrawState&,
                       const GrPathProcessor*,
                       const GrPath*,
                       const GrScissorState&,
                       const GrStencilSettings&) SK_OVERRIDE;
    void onDrawPath(const GrDrawState&,
                    const GrPathProcessor*,
                    const GrPath*,
                    const GrScissorState&,
                    const GrStencilSettings&,
                    const GrDeviceCoordTexture* dstCopy) SK_OVERRIDE;
    void onDrawPaths(const GrDrawState&,
                     const GrPathProcessor*,
                     const GrPathRange*,
                     const void* indices,
                     PathIndexType,
                     const float transformValues[],
                     PathTransformType,
                     int count,
                     const GrScissorState&,
                     const GrStencilSettings&,
                     const GrDeviceCoordTexture*) SK_OVERRIDE;
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
    int concatInstancedDraw(const GrDrawState&, const DrawInfo&);

    // Determines whether the current draw operation requires a new GrOptDrawState and if so
    // records it. If the draw can be skipped false is returned and no new GrOptDrawState is
    // recorded.
    bool SK_WARN_UNUSED_RESULT recordStateAndShouldDraw(const GrDrawState&,
                                                        const GrGeometryProcessor*,
                                                        const GrPathProcessor*,
                                                        GrGpu::DrawType,
                                                        const GrScissorState&,
                                                        const GrDeviceCoordTexture*);
    // We lazily record clip changes in order to skip clips that have no effect.
    void recordClipIfNecessary();
    // Records any trace markers for a command after adding it to the buffer.
    void recordTraceMarkersIfNecessary();

    bool isIssued(uint32_t drawID) SK_OVERRIDE { return drawID != fDrawID; }

    // TODO: Use a single allocator for commands and records
    enum {
        kCmdBufferInitialSizeInBytes = 8 * 1024,
        kPathIdxBufferMinReserve     = 2 * 64,  // 64 uint16_t's
        kPathXformBufferMinReserve   = 2 * 64,  // 64 two-float transforms
    };

    CmdBuffer                           fCmdBuffer;
    GrOptDrawState*                     fPrevState;
    SkTArray<GrTraceMarkerSet, false>   fGpuCmdMarkers;
    SkTDArray<char>                     fPathIndexBuffer;
    SkTDArray<float>                    fPathTransformBuffer;
    uint32_t                            fDrawID;

    typedef GrFlushToGpuDrawTarget INHERITED;
};

#endif
