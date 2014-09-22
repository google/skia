/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInOrderDrawBuffer.h"

#include "GrBufferAllocPool.h"
#include "GrDrawTargetCaps.h"
#include "GrTextStrike.h"
#include "GrGpu.h"
#include "GrTemplates.h"
#include "GrTexture.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrGpu* gpu,
                                         GrVertexBufferAllocPool* vertexPool,
                                         GrIndexBufferAllocPool* indexPool)
    : GrDrawTarget(gpu->getContext())
    , fDstGpu(gpu)
    , fClipSet(true)
    , fClipProxyState(kUnknown_ClipProxyState)
    , fVertexPool(*vertexPool)
    , fIndexPool(*indexPool)
    , fFlushing(false)
    , fDrawID(0) {

    fDstGpu->ref();
    fCaps.reset(SkRef(fDstGpu->caps()));

    SkASSERT(vertexPool);
    SkASSERT(indexPool);

    GeometryPoolState& poolState = fGeoPoolStateStack.push_back();
    poolState.fUsedPoolVertexBytes = 0;
    poolState.fUsedPoolIndexBytes = 0;
#ifdef SK_DEBUG
    poolState.fPoolVertexBuffer = (GrVertexBuffer*)~0;
    poolState.fPoolStartVertex = ~0;
    poolState.fPoolIndexBuffer = (GrIndexBuffer*)~0;
    poolState.fPoolStartIndex = ~0;
#endif
    this->reset();
}

GrInOrderDrawBuffer::~GrInOrderDrawBuffer() {
    this->reset();
    // This must be called by before the GrDrawTarget destructor
    this->releaseGeometry();
    fDstGpu->unref();
}

////////////////////////////////////////////////////////////////////////////////

namespace {
void get_vertex_bounds(const void* vertices,
                       size_t vertexSize,
                       int vertexCount,
                       SkRect* bounds) {
    SkASSERT(vertexSize >= sizeof(SkPoint));
    SkASSERT(vertexCount > 0);
    const SkPoint* point = static_cast<const SkPoint*>(vertices);
    bounds->fLeft = bounds->fRight = point->fX;
    bounds->fTop = bounds->fBottom = point->fY;
    for (int i = 1; i < vertexCount; ++i) {
        point = reinterpret_cast<SkPoint*>(reinterpret_cast<intptr_t>(point) + vertexSize);
        bounds->growToInclude(point->fX, point->fY);
    }
}
}


namespace {

extern const GrVertexAttrib kRectAttribs[] = {
    {kVec2f_GrVertexAttribType,  0,                               kPosition_GrVertexAttribBinding},
    {kVec4ub_GrVertexAttribType, sizeof(SkPoint),                 kColor_GrVertexAttribBinding},
    {kVec2f_GrVertexAttribType,  sizeof(SkPoint)+sizeof(GrColor), kLocalCoord_GrVertexAttribBinding},
};
}

/** We always use per-vertex colors so that rects can be batched across color changes. Sometimes we
    have explicit local coords and sometimes not. We *could* always provide explicit local coords
    and just duplicate the positions when the caller hasn't provided a local coord rect, but we
    haven't seen a use case which frequently switches between local rect and no local rect draws.

    The color param is used to determine whether the opaque hint can be set on the draw state.
    The caller must populate the vertex colors itself.

    The vertex attrib order is always pos, color, [local coords].
 */
static void set_vertex_attributes(GrDrawState* drawState, bool hasLocalCoords, GrColor color) {
    if (hasLocalCoords) {
        drawState->setVertexAttribs<kRectAttribs>(3, 2 * sizeof(SkPoint) + sizeof(SkColor));
    } else {
        drawState->setVertexAttribs<kRectAttribs>(2, sizeof(SkPoint) + sizeof(SkColor));
    }
    if (0xFF == GrColorUnpackA(color)) {
        drawState->setHint(GrDrawState::kVertexColorsAreOpaque_Hint, true);
    }
}

enum {
    kTraceCmdBit = 0x80,
    kCmdMask = 0x7f,
};

static inline uint8_t add_trace_bit(uint8_t cmd) { return cmd | kTraceCmdBit; }

static inline uint8_t strip_trace_bit(uint8_t cmd) { return cmd & kCmdMask; }

static inline bool cmd_has_trace_marker(uint8_t cmd) { return SkToBool(cmd & kTraceCmdBit); }

void GrInOrderDrawBuffer::onDrawRect(const SkRect& rect,
                                     const SkRect* localRect,
                                     const SkMatrix* localMatrix) {
    GrDrawState* drawState = this->drawState();

    GrColor color = drawState->getColor();

    set_vertex_attributes(drawState, SkToBool(localRect),  color);

    AutoReleaseGeometry geo(this, 4, 0);
    if (!geo.succeeded()) {
        GrPrintf("Failed to get space for vertices!\n");
        return;
    }

    // Go to device coords to allow batching across matrix changes
    SkMatrix matrix = drawState->getViewMatrix();

    // When the caller has provided an explicit source rect for a stage then we don't want to
    // modify that stage's matrix. Otherwise if the effect is generating its source rect from
    // the vertex positions then we have to account for the view matrix change.
    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(drawState)) {
        return;
    }

    size_t vstride = drawState->getVertexStride();

    geo.positions()->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vstride);
    matrix.mapPointsWithStride(geo.positions(), vstride, 4);

    SkRect devBounds;
    // since we already computed the dev verts, set the bounds hint. This will help us avoid
    // unnecessary clipping in our onDraw().
    get_vertex_bounds(geo.vertices(), vstride, 4, &devBounds);

    if (localRect) {
        static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
        SkPoint* coords = GrTCast<SkPoint*>(GrTCast<intptr_t>(geo.vertices()) + kLocalOffset);
        coords->setRectFan(localRect->fLeft, localRect->fTop,
                           localRect->fRight, localRect->fBottom,
                           vstride);
        if (localMatrix) {
            localMatrix->mapPointsWithStride(coords, vstride, 4);
        }
    }

    static const int kColorOffset = sizeof(SkPoint);
    GrColor* vertColor = GrTCast<GrColor*>(GrTCast<intptr_t>(geo.vertices()) + kColorOffset);
    for (int i = 0; i < 4; ++i) {
        *vertColor = color;
        vertColor = (GrColor*) ((intptr_t) vertColor + vstride);
    }

    this->setIndexSourceToBuffer(this->getContext()->getQuadIndexBuffer());
    this->drawIndexedInstances(kTriangles_GrPrimitiveType, 1, 4, 6, &devBounds);

    // to ensure that stashing the drawState ptr is valid
    SkASSERT(this->drawState() == drawState);
}

bool GrInOrderDrawBuffer::quickInsideClip(const SkRect& devBounds) {
    if (!this->getDrawState().isClipState()) {
        return true;
    }
    if (kUnknown_ClipProxyState == fClipProxyState) {
        SkIRect rect;
        bool iior;
        this->getClip()->getConservativeBounds(this->getDrawState().getRenderTarget(), &rect, &iior);
        if (iior) {
            // The clip is a rect. We will remember that in fProxyClip. It is common for an edge (or
            // all edges) of the clip to be at the edge of the RT. However, we get that clipping for
            // free via the viewport. We don't want to think that clipping must be enabled in this
            // case. So we extend the clip outward from the edge to avoid these false negatives.
            fClipProxyState = kValid_ClipProxyState;
            fClipProxy = SkRect::Make(rect);

            if (fClipProxy.fLeft <= 0) {
                fClipProxy.fLeft = SK_ScalarMin;
            }
            if (fClipProxy.fTop <= 0) {
                fClipProxy.fTop = SK_ScalarMin;
            }
            if (fClipProxy.fRight >= this->getDrawState().getRenderTarget()->width()) {
                fClipProxy.fRight = SK_ScalarMax;
            }
            if (fClipProxy.fBottom >= this->getDrawState().getRenderTarget()->height()) {
                fClipProxy.fBottom = SK_ScalarMax;
            }
        } else {
            fClipProxyState = kInvalid_ClipProxyState;
        }
    }
    if (kValid_ClipProxyState == fClipProxyState) {
        return fClipProxy.contains(devBounds);
    }
    SkPoint originOffset = {SkIntToScalar(this->getClip()->fOrigin.fX),
                            SkIntToScalar(this->getClip()->fOrigin.fY)};
    SkRect clipSpaceBounds = devBounds;
    clipSpaceBounds.offset(originOffset);
    return this->getClip()->fClipStack->quickContains(clipSpaceBounds);
}

int GrInOrderDrawBuffer::concatInstancedDraw(const DrawInfo& info) {
    SkASSERT(info.isInstanced());

    const GeometrySrcState& geomSrc = this->getGeomSrc();
    const GrDrawState& drawState = this->getDrawState();

    // we only attempt to concat the case when reserved verts are used with a client-specified index
    // buffer. To make this work with client-specified VBs we'd need to know if the VB was updated
    // between draws.
    if (kReserved_GeometrySrcType != geomSrc.fVertexSrc ||
        kBuffer_GeometrySrcType != geomSrc.fIndexSrc) {
        return 0;
    }
    // Check if there is a draw info that is compatible that uses the same VB from the pool and
    // the same IB
    if (kDraw_Cmd != strip_trace_bit(fCmds.back())) {
        return 0;
    }

    Draw* draw = &fDraws.back();
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GrVertexBuffer* vertexBuffer = poolState.fPoolVertexBuffer;

    if (!draw->isInstanced() ||
        draw->verticesPerInstance() != info.verticesPerInstance() ||
        draw->indicesPerInstance() != info.indicesPerInstance() ||
        draw->vertexBuffer() != vertexBuffer ||
        draw->indexBuffer() != geomSrc.fIndexBuffer) {
        return 0;
    }
    // info does not yet account for the offset from the start of the pool's VB while the previous
    // draw record does.
    int adjustedStartVertex = poolState.fPoolStartVertex + info.startVertex();
    if (draw->startVertex() + draw->vertexCount() != adjustedStartVertex) {
        return 0;
    }

    SkASSERT(poolState.fPoolStartVertex == draw->startVertex() + draw->vertexCount());

    // how many instances can be concat'ed onto draw given the size of the index buffer
    int instancesToConcat = this->indexCountInCurrentSource() / info.indicesPerInstance();
    instancesToConcat -= draw->instanceCount();
    instancesToConcat = SkTMin(instancesToConcat, info.instanceCount());

    // update the amount of reserved vertex data actually referenced in draws
    size_t vertexBytes = instancesToConcat * info.verticesPerInstance() *
                         drawState.getVertexStride();
    poolState.fUsedPoolVertexBytes = SkTMax(poolState.fUsedPoolVertexBytes, vertexBytes);

    draw->adjustInstanceCount(instancesToConcat);

    // update last fGpuCmdMarkers to include any additional trace markers that have been added
    if (this->getActiveTraceMarkers().count() > 0) {
        if (cmd_has_trace_marker(fCmds.back())) {
            fGpuCmdMarkers.back().addSet(this->getActiveTraceMarkers());
        } else {
            fGpuCmdMarkers.push_back(this->getActiveTraceMarkers());
            fCmds.back() = add_trace_bit(fCmds.back());
        }
    }

    return instancesToConcat;
}

class AutoClipReenable {
public:
    AutoClipReenable() : fDrawState(NULL) {}
    ~AutoClipReenable() {
        if (fDrawState) {
            fDrawState->enableState(GrDrawState::kClip_StateBit);
        }
    }
    void set(GrDrawState* drawState) {
        if (drawState->isClipState()) {
            fDrawState = drawState;
            drawState->disableState(GrDrawState::kClip_StateBit);
        }
    }
private:
    GrDrawState*    fDrawState;
};

void GrInOrderDrawBuffer::onDraw(const DrawInfo& info) {

    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GrDrawState& drawState = this->getDrawState();
    AutoClipReenable acr;

    if (drawState.isClipState() &&
        info.getDevBounds() &&
        this->quickInsideClip(*info.getDevBounds())) {
        acr.set(this->drawState());
    }

    if (this->needsNewClip()) {
       this->recordClip();
    }
    this->recordStateIfNecessary();

    const GrVertexBuffer* vb;
    if (kBuffer_GeometrySrcType == this->getGeomSrc().fVertexSrc) {
        vb = this->getGeomSrc().fVertexBuffer;
    } else {
        vb = poolState.fPoolVertexBuffer;
    }

    const GrIndexBuffer* ib = NULL;
    if (info.isIndexed()) {
        if (kBuffer_GeometrySrcType == this->getGeomSrc().fIndexSrc) {
            ib = this->getGeomSrc().fIndexBuffer;
        } else {
            ib = poolState.fPoolIndexBuffer;
        }
    }

    Draw* draw;
    if (info.isInstanced()) {
        int instancesConcated = this->concatInstancedDraw(info);
        if (info.instanceCount() > instancesConcated) {
            draw = this->recordDraw(info, vb, ib);
            draw->adjustInstanceCount(-instancesConcated);
        } else {
            return;
        }
    } else {
        draw = this->recordDraw(info, vb, ib);
    }

    // Adjust the starting vertex and index when we are using reserved or array sources to
    // compensate for the fact that the data was inserted into a larger vb/ib owned by the pool.
    if (kBuffer_GeometrySrcType != this->getGeomSrc().fVertexSrc) {
        size_t bytes = (info.vertexCount() + info.startVertex()) * drawState.getVertexStride();
        poolState.fUsedPoolVertexBytes = SkTMax(poolState.fUsedPoolVertexBytes, bytes);
        draw->adjustStartVertex(poolState.fPoolStartVertex);
    }
    
    if (info.isIndexed() && kBuffer_GeometrySrcType != this->getGeomSrc().fIndexSrc) {
        size_t bytes = (info.indexCount() + info.startIndex()) * sizeof(uint16_t);
        poolState.fUsedPoolIndexBytes = SkTMax(poolState.fUsedPoolIndexBytes, bytes);
        draw->adjustStartIndex(poolState.fPoolStartIndex);
    }
}

void GrInOrderDrawBuffer::onStencilPath(const GrPath* path, SkPath::FillType fill) {
    if (this->needsNewClip()) {
        this->recordClip();
    }
    // Only compare the subset of GrDrawState relevant to path stenciling?
    this->recordStateIfNecessary();
    StencilPath* sp = this->recordStencilPath(path);
    sp->fFill = fill;
}

void GrInOrderDrawBuffer::onDrawPath(const GrPath* path,
                                     SkPath::FillType fill, const GrDeviceCoordTexture* dstCopy) {
    if (this->needsNewClip()) {
        this->recordClip();
    }
    // TODO: Only compare the subset of GrDrawState relevant to path covering?
    this->recordStateIfNecessary();
    DrawPath* cp = this->recordDrawPath(path);
    cp->fFill = fill;
    if (dstCopy) {
        cp->fDstCopy = *dstCopy;
    }
}

void GrInOrderDrawBuffer::onDrawPaths(const GrPathRange* pathRange,
                                      const uint32_t indices[], int count,
                                      const float transforms[], PathTransformType transformsType,
                                      SkPath::FillType fill, const GrDeviceCoordTexture* dstCopy) {
    SkASSERT(pathRange);
    SkASSERT(indices);
    SkASSERT(transforms);

    if (this->needsNewClip()) {
        this->recordClip();
    }
    this->recordStateIfNecessary();
    DrawPaths* dp = this->recordDrawPaths(pathRange);
    dp->fIndices = SkNEW_ARRAY(uint32_t, count); // TODO: Accomplish this without a malloc
    memcpy(dp->fIndices, indices, sizeof(uint32_t) * count);
    dp->fCount = count;

    const int transformsLength = GrPathRendering::PathTransformSize(transformsType) * count;
    dp->fTransforms = SkNEW_ARRAY(float, transformsLength);
    memcpy(dp->fTransforms, transforms, sizeof(float) * transformsLength);
    dp->fTransformsType = transformsType;

    dp->fFill = fill;

    if (dstCopy) {
        dp->fDstCopy = *dstCopy;
    }
}

void GrInOrderDrawBuffer::clear(const SkIRect* rect, GrColor color,
                                bool canIgnoreRect, GrRenderTarget* renderTarget) {
    SkIRect r;
    if (NULL == renderTarget) {
        renderTarget = this->drawState()->getRenderTarget();
        SkASSERT(renderTarget);
    }
    if (NULL == rect) {
        // We could do something smart and remove previous draws and clears to
        // the current render target. If we get that smart we have to make sure
        // those draws aren't read before this clear (render-to-texture).
        r.setLTRB(0, 0, renderTarget->width(), renderTarget->height());
        rect = &r;
    }
    Clear* clr = this->recordClear(renderTarget);
    GrColorIsPMAssert(color);
    clr->fColor = color;
    clr->fRect = *rect;
    clr->fCanIgnoreRect = canIgnoreRect;
}

void GrInOrderDrawBuffer::discard(GrRenderTarget* renderTarget) {
    if (!this->caps()->discardRenderTargetSupport()) {
        return;
    }
    if (NULL == renderTarget) {
        renderTarget = this->drawState()->getRenderTarget();
        SkASSERT(renderTarget);
    }
    Clear* clr = this->recordClear(renderTarget);
    clr->fColor = GrColor_ILLEGAL;
}

void GrInOrderDrawBuffer::reset() {
    SkASSERT(1 == fGeoPoolStateStack.count());
    this->resetVertexSource();
    this->resetIndexSource();
        
    fCmds.reset();
    fDraws.reset();
    fStencilPaths.reset();
    fDrawPath.reset();
    fDrawPaths.reset();
    fStates.reset();
    fClears.reset();
    fVertexPool.reset();
    fIndexPool.reset();
    fClips.reset();
    fCopySurfaces.reset();
    fGpuCmdMarkers.reset();
    fClipSet = true;
}

void GrInOrderDrawBuffer::flush() {
    if (fFlushing) {
        return;
    }

    this->getContext()->getFontCache()->updateTextures();

    SkASSERT(kReserved_GeometrySrcType != this->getGeomSrc().fVertexSrc);
    SkASSERT(kReserved_GeometrySrcType != this->getGeomSrc().fIndexSrc);

    int numCmds = fCmds.count();
    if (0 == numCmds) {
        return;
    }

    GrAutoTRestore<bool> flushRestore(&fFlushing);
    fFlushing = true;

    fVertexPool.unmap();
    fIndexPool.unmap();

    GrDrawTarget::AutoClipRestore acr(fDstGpu);
    AutoGeometryAndStatePush agasp(fDstGpu, kPreserve_ASRInit);

    GrDrawState* prevDrawState = SkRef(fDstGpu->drawState());

    GrClipData clipData;

    StateAllocator::Iter stateIter(&fStates);
    ClipAllocator::Iter clipIter(&fClips);
    ClearAllocator::Iter clearIter(&fClears);
    DrawAllocator::Iter drawIter(&fDraws);
    StencilPathAllocator::Iter stencilPathIter(&fStencilPaths);
    DrawPathAllocator::Iter drawPathIter(&fDrawPath);
    DrawPathsAllocator::Iter drawPathsIter(&fDrawPaths);
    CopySurfaceAllocator::Iter copySurfaceIter(&fCopySurfaces);

    int currCmdMarker   = 0;

    fDstGpu->saveActiveTraceMarkers();
    for (int c = 0; c < numCmds; ++c) {
        GrGpuTraceMarker newMarker("", -1);
        SkString traceString;
        if (cmd_has_trace_marker(fCmds[c])) {
            traceString = fGpuCmdMarkers[currCmdMarker].toString();
            newMarker.fMarker = traceString.c_str();
            fDstGpu->addGpuTraceMarker(&newMarker);
            ++currCmdMarker;
        }
        switch (strip_trace_bit(fCmds[c])) {
            case kDraw_Cmd: {
                SkASSERT(fDstGpu->drawState() != prevDrawState);
                SkAssertResult(drawIter.next());
                fDstGpu->setVertexSourceToBuffer(drawIter->vertexBuffer());
                if (drawIter->isIndexed()) {
                    fDstGpu->setIndexSourceToBuffer(drawIter->indexBuffer());
                }
                fDstGpu->executeDraw(*drawIter);
                break;
            }
            case kStencilPath_Cmd: {
                SkASSERT(fDstGpu->drawState() != prevDrawState);
                SkAssertResult(stencilPathIter.next());
                fDstGpu->stencilPath(stencilPathIter->path(), stencilPathIter->fFill);
                break;
            }
            case kDrawPath_Cmd: {
                SkASSERT(fDstGpu->drawState() != prevDrawState);
                SkAssertResult(drawPathIter.next());
                fDstGpu->executeDrawPath(drawPathIter->path(), drawPathIter->fFill,
                                         drawPathIter->fDstCopy.texture() ?
                                            &drawPathIter->fDstCopy :
                                            NULL);
                break;
            }
            case kDrawPaths_Cmd: {
                SkASSERT(fDstGpu->drawState() != prevDrawState);
                SkAssertResult(drawPathsIter.next());
                const GrDeviceCoordTexture* dstCopy =
                    drawPathsIter->fDstCopy.texture() ? &drawPathsIter->fDstCopy : NULL;
                fDstGpu->executeDrawPaths(drawPathsIter->pathRange(),
                                          drawPathsIter->fIndices,
                                          drawPathsIter->fCount,
                                          drawPathsIter->fTransforms,
                                          drawPathsIter->fTransformsType,
                                          drawPathsIter->fFill,
                                          dstCopy);
                break;
            }
            case kSetState_Cmd:
                SkAssertResult(stateIter.next());
                fDstGpu->setDrawState(stateIter.get());
                break;
            case kSetClip_Cmd:
                SkAssertResult(clipIter.next());
                clipData.fClipStack = &clipIter->fStack;
                clipData.fOrigin = clipIter->fOrigin;
                fDstGpu->setClip(&clipData);
                break;
            case kClear_Cmd:
                SkAssertResult(clearIter.next());
                if (GrColor_ILLEGAL == clearIter->fColor) {
                    fDstGpu->discard(clearIter->renderTarget());
                } else {
                    fDstGpu->clear(&clearIter->fRect,
                                   clearIter->fColor,
                                   clearIter->fCanIgnoreRect,
                                   clearIter->renderTarget());
                }
                break;
            case kCopySurface_Cmd:
                SkAssertResult(copySurfaceIter.next());
                fDstGpu->copySurface(copySurfaceIter->dst(),
                                     copySurfaceIter->src(),
                                     copySurfaceIter->fSrcRect,
                                     copySurfaceIter->fDstPoint);
                break;
        }
        if (cmd_has_trace_marker(fCmds[c])) {
            fDstGpu->removeGpuTraceMarker(&newMarker);
        }
    }
    fDstGpu->restoreActiveTraceMarkers();
    // we should have consumed all the states, clips, etc.
    SkASSERT(!stateIter.next());
    SkASSERT(!clipIter.next());
    SkASSERT(!clearIter.next());
    SkASSERT(!drawIter.next());
    SkASSERT(!copySurfaceIter.next());
    SkASSERT(!stencilPathIter.next());
    SkASSERT(!drawPathIter.next());
    SkASSERT(!drawPathsIter.next());

    SkASSERT(fGpuCmdMarkers.count() == currCmdMarker);

    fDstGpu->setDrawState(prevDrawState);
    prevDrawState->unref();
    this->reset();
    ++fDrawID;
}

bool GrInOrderDrawBuffer::onCopySurface(GrSurface* dst,
                                        GrSurface* src,
                                        const SkIRect& srcRect,
                                        const SkIPoint& dstPoint) {
    if (fDstGpu->canCopySurface(dst, src, srcRect, dstPoint)) {
        CopySurface* cs = this->recordCopySurface(dst, src);
        cs->fSrcRect = srcRect;
        cs->fDstPoint = dstPoint;
        return true;
    } else {
        return false;
    }
}

bool GrInOrderDrawBuffer::onCanCopySurface(GrSurface* dst,
                                           GrSurface* src,
                                           const SkIRect& srcRect,
                                           const SkIPoint& dstPoint) {
    return fDstGpu->canCopySurface(dst, src, srcRect, dstPoint);
}

void GrInOrderDrawBuffer::initCopySurfaceDstDesc(const GrSurface* src, GrTextureDesc* desc) {
    fDstGpu->initCopySurfaceDstDesc(src, desc);
}

void GrInOrderDrawBuffer::willReserveVertexAndIndexSpace(int vertexCount,
                                                         int indexCount) {
    // We use geometryHints() to know whether to flush the draw buffer. We
    // can't flush if we are inside an unbalanced pushGeometrySource.
    // Moreover, flushing blows away vertex and index data that was
    // previously reserved. So if the vertex or index data is pulled from
    // reserved space and won't be released by this request then we can't
    // flush.
    bool insideGeoPush = fGeoPoolStateStack.count() > 1;

    bool unreleasedVertexSpace =
        !vertexCount &&
        kReserved_GeometrySrcType == this->getGeomSrc().fVertexSrc;

    bool unreleasedIndexSpace =
        !indexCount &&
        kReserved_GeometrySrcType == this->getGeomSrc().fIndexSrc;

    // we don't want to finalize any reserved geom on the target since
    // we don't know that the client has finished writing to it.
    bool targetHasReservedGeom = fDstGpu->hasReservedVerticesOrIndices();

    int vcount = vertexCount;
    int icount = indexCount;

    if (!insideGeoPush &&
        !unreleasedVertexSpace &&
        !unreleasedIndexSpace &&
        !targetHasReservedGeom &&
        this->geometryHints(&vcount, &icount)) {

        this->flush();
    }
}

bool GrInOrderDrawBuffer::geometryHints(int* vertexCount,
                                        int* indexCount) const {
    // we will recommend a flush if the data could fit in a single
    // preallocated buffer but none are left and it can't fit
    // in the current buffer (which may not be prealloced).
    bool flush = false;
    if (indexCount) {
        int32_t currIndices = fIndexPool.currentBufferIndices();
        if (*indexCount > currIndices &&
            (!fIndexPool.preallocatedBuffersRemaining() &&
             *indexCount <= fIndexPool.preallocatedBufferIndices())) {

            flush = true;
        }
        *indexCount = currIndices;
    }
    if (vertexCount) {
        size_t vertexStride = this->getDrawState().getVertexStride();
        int32_t currVertices = fVertexPool.currentBufferVertices(vertexStride);
        if (*vertexCount > currVertices &&
            (!fVertexPool.preallocatedBuffersRemaining() &&
             *vertexCount <= fVertexPool.preallocatedBufferVertices(vertexStride))) {

            flush = true;
        }
        *vertexCount = currVertices;
    }
    return flush;
}

bool GrInOrderDrawBuffer::onReserveVertexSpace(size_t vertexSize,
                                               int vertexCount,
                                               void** vertices) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    SkASSERT(vertexCount > 0);
    SkASSERT(vertices);
    SkASSERT(0 == poolState.fUsedPoolVertexBytes);

    *vertices = fVertexPool.makeSpace(vertexSize,
                                      vertexCount,
                                      &poolState.fPoolVertexBuffer,
                                      &poolState.fPoolStartVertex);
    return SkToBool(*vertices);
}

bool GrInOrderDrawBuffer::onReserveIndexSpace(int indexCount, void** indices) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    SkASSERT(indexCount > 0);
    SkASSERT(indices);
    SkASSERT(0 == poolState.fUsedPoolIndexBytes);

    *indices = fIndexPool.makeSpace(indexCount,
                                    &poolState.fPoolIndexBuffer,
                                    &poolState.fPoolStartIndex);
    return SkToBool(*indices);
}

void GrInOrderDrawBuffer::releaseReservedVertexSpace() {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GeometrySrcState& geoSrc = this->getGeomSrc();

    // If we get a release vertex space call then our current source should either be reserved
    // or array (which we copied into reserved space).
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fVertexSrc ||
             kArray_GeometrySrcType == geoSrc.fVertexSrc);

    // When the caller reserved vertex buffer space we gave it back a pointer
    // provided by the vertex buffer pool. At each draw we tracked the largest
    // offset into the pool's pointer that was referenced. Now we return to the
    // pool any portion at the tail of the allocation that no draw referenced.
    size_t reservedVertexBytes = geoSrc.fVertexSize * geoSrc.fVertexCount;
    fVertexPool.putBack(reservedVertexBytes -
                        poolState.fUsedPoolVertexBytes);
    poolState.fUsedPoolVertexBytes = 0;
    poolState.fPoolVertexBuffer = NULL;
    poolState.fPoolStartVertex = 0;
}

void GrInOrderDrawBuffer::releaseReservedIndexSpace() {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GeometrySrcState& geoSrc = this->getGeomSrc();

    // If we get a release index space call then our current source should either be reserved
    // or array (which we copied into reserved space).
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fIndexSrc ||
             kArray_GeometrySrcType == geoSrc.fIndexSrc);

    // Similar to releaseReservedVertexSpace we return any unused portion at
    // the tail
    size_t reservedIndexBytes = sizeof(uint16_t) * geoSrc.fIndexCount;
    fIndexPool.putBack(reservedIndexBytes - poolState.fUsedPoolIndexBytes);
    poolState.fUsedPoolIndexBytes = 0;
    poolState.fPoolIndexBuffer = NULL;
    poolState.fPoolStartIndex = 0;
}

void GrInOrderDrawBuffer::onSetVertexSourceToArray(const void* vertexArray, int vertexCount) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    SkASSERT(0 == poolState.fUsedPoolVertexBytes);
#ifdef SK_DEBUG
    bool success =
#endif
    fVertexPool.appendVertices(this->getVertexSize(),
                               vertexCount,
                               vertexArray,
                               &poolState.fPoolVertexBuffer,
                               &poolState.fPoolStartVertex);
    GR_DEBUGASSERT(success);
}

void GrInOrderDrawBuffer::onSetIndexSourceToArray(const void* indexArray,
                                                  int indexCount) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    SkASSERT(0 == poolState.fUsedPoolIndexBytes);
#ifdef SK_DEBUG
    bool success =
#endif
    fIndexPool.appendIndices(indexCount,
                             indexArray,
                             &poolState.fPoolIndexBuffer,
                             &poolState.fPoolStartIndex);
    GR_DEBUGASSERT(success);
}

void GrInOrderDrawBuffer::releaseVertexArray() {
    // When the client provides an array as the vertex source we handled it
    // by copying their array into reserved space.
    this->GrInOrderDrawBuffer::releaseReservedVertexSpace();
}

void GrInOrderDrawBuffer::releaseIndexArray() {
    // When the client provides an array as the index source we handled it
    // by copying their array into reserved space.
    this->GrInOrderDrawBuffer::releaseReservedIndexSpace();
}

void GrInOrderDrawBuffer::geometrySourceWillPush() {
    GeometryPoolState& poolState = fGeoPoolStateStack.push_back();
    poolState.fUsedPoolVertexBytes = 0;
    poolState.fUsedPoolIndexBytes = 0;
#ifdef SK_DEBUG
    poolState.fPoolVertexBuffer = (GrVertexBuffer*)~0;
    poolState.fPoolStartVertex = ~0;
    poolState.fPoolIndexBuffer = (GrIndexBuffer*)~0;
    poolState.fPoolStartIndex = ~0;
#endif
}

void GrInOrderDrawBuffer::geometrySourceWillPop(const GeometrySrcState& restoredState) {
    SkASSERT(fGeoPoolStateStack.count() > 1);
    fGeoPoolStateStack.pop_back();
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    // we have to assume that any slack we had in our vertex/index data
    // is now unreleasable because data may have been appended later in the
    // pool.
    if (kReserved_GeometrySrcType == restoredState.fVertexSrc ||
        kArray_GeometrySrcType == restoredState.fVertexSrc) {
        poolState.fUsedPoolVertexBytes = restoredState.fVertexSize * restoredState.fVertexCount;
    }
    if (kReserved_GeometrySrcType == restoredState.fIndexSrc ||
        kArray_GeometrySrcType == restoredState.fIndexSrc) {
        poolState.fUsedPoolIndexBytes = sizeof(uint16_t) *
                                         restoredState.fIndexCount;
    }
}

void GrInOrderDrawBuffer::recordStateIfNecessary() {
    if (fStates.empty()) {
        this->convertDrawStateToPendingExec(&fStates.push_back(this->getDrawState()));
        this->addToCmdBuffer(kSetState_Cmd);
        return;
    }
    const GrDrawState& curr = this->getDrawState();
    GrDrawState& prev = fStates.back();
    switch (GrDrawState::CombineIfPossible(prev, curr, *this->caps())) {
        case GrDrawState::kIncompatible_CombinedState:
            this->convertDrawStateToPendingExec(&fStates.push_back(curr));
            this->addToCmdBuffer(kSetState_Cmd);
            break;
        case GrDrawState::kA_CombinedState:
        case GrDrawState::kAOrB_CombinedState: // Treat the same as kA.
            break;
        case GrDrawState::kB_CombinedState:
            // prev has already been converted to pending execution. That is a one-way ticket.
            // So here we just delete prev and push back a new copy of curr. Note that this
            // goes away when we move GrIODB over to taking optimized snapshots of draw states.
            fStates.pop_back();
            this->convertDrawStateToPendingExec(&fStates.push_back(curr));
            break;
    }
}

bool GrInOrderDrawBuffer::needsNewClip() const {
    if (this->getDrawState().isClipState()) {
       if (fClipSet &&
           (fClips.empty() ||
            fClips.back().fStack != *this->getClip()->fClipStack ||
            fClips.back().fOrigin != this->getClip()->fOrigin)) {
           return true;
       }
    }
    return false;
}

void GrInOrderDrawBuffer::addToCmdBuffer(uint8_t cmd) {
    SkASSERT(!cmd_has_trace_marker(cmd));
    const GrTraceMarkerSet& activeTraceMarkers = this->getActiveTraceMarkers();
    if (activeTraceMarkers.count() > 0) {
        fCmds.push_back(add_trace_bit(cmd));
        fGpuCmdMarkers.push_back(activeTraceMarkers);
    } else {
        fCmds.push_back(cmd);
    }
}

void GrInOrderDrawBuffer::recordClip() {
    fClips.push_back().fStack = *this->getClip()->fClipStack;
    fClips.back().fOrigin = this->getClip()->fOrigin;
    fClipSet = false;
    this->addToCmdBuffer(kSetClip_Cmd);
}

GrInOrderDrawBuffer::Draw* GrInOrderDrawBuffer::recordDraw(const DrawInfo& info,
                                                           const GrVertexBuffer* vb,
                                                           const GrIndexBuffer* ib) {
    this->addToCmdBuffer(kDraw_Cmd);
    return GrNEW_APPEND_TO_ALLOCATOR(&fDraws, Draw, (info, vb, ib));
}

GrInOrderDrawBuffer::StencilPath* GrInOrderDrawBuffer::recordStencilPath(const GrPath* path) {
    this->addToCmdBuffer(kStencilPath_Cmd);
    return GrNEW_APPEND_TO_ALLOCATOR(&fStencilPaths, StencilPath, (path));
}

GrInOrderDrawBuffer::DrawPath* GrInOrderDrawBuffer::recordDrawPath(const GrPath* path) {
    this->addToCmdBuffer(kDrawPath_Cmd);
    return GrNEW_APPEND_TO_ALLOCATOR(&fDrawPath, DrawPath, (path));
}

GrInOrderDrawBuffer::DrawPaths* GrInOrderDrawBuffer::recordDrawPaths(const GrPathRange* pathRange) {
    this->addToCmdBuffer(kDrawPaths_Cmd);
    return GrNEW_APPEND_TO_ALLOCATOR(&fDrawPaths, DrawPaths, (pathRange));
}

GrInOrderDrawBuffer::Clear* GrInOrderDrawBuffer::recordClear(GrRenderTarget* rt) {
    this->addToCmdBuffer(kClear_Cmd);
    return GrNEW_APPEND_TO_ALLOCATOR(&fClears, Clear, (rt));
}

GrInOrderDrawBuffer::CopySurface* GrInOrderDrawBuffer::recordCopySurface(GrSurface* dst,
                                                                         GrSurface* src) {
    this->addToCmdBuffer(kCopySurface_Cmd);
    return GrNEW_APPEND_TO_ALLOCATOR(&fCopySurfaces, CopySurface, (dst, src));
}

void GrInOrderDrawBuffer::clipWillBeSet(const GrClipData* newClipData) {
    INHERITED::clipWillBeSet(newClipData);
    fClipSet = true;
    fClipProxyState = kUnknown_ClipProxyState;
}
