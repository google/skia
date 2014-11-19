/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInOrderDrawBuffer.h"

#include "GrBufferAllocPool.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrTemplates.h"
#include "GrTextStrike.h"
#include "GrTexture.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrGpu* gpu,
                                         GrVertexBufferAllocPool* vertexPool,
                                         GrIndexBufferAllocPool* indexPool)
    : INHERITED(gpu->getContext())
    , fCmdBuffer(kCmdBufferInitialSizeInBytes)
    , fLastState(NULL)
    , fDstGpu(gpu)
    , fVertexPool(*vertexPool)
    , fIndexPool(*indexPool)
    , fFlushing(false)
    , fDrawID(0) {

    fDstGpu->ref();
    fCaps.reset(SkRef(fDstGpu->caps()));

    SkASSERT(vertexPool);
    SkASSERT(indexPool);

    fPathIndexBuffer.setReserve(kPathIdxBufferMinReserve);
    fPathTransformBuffer.setReserve(kPathXformBufferMinReserve);

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

/** We always use per-vertex colors so that rects can be batched across color changes. Sometimes we
    have explicit local coords and sometimes not. We *could* always provide explicit local coords
    and just duplicate the positions when the caller hasn't provided a local coord rect, but we
    haven't seen a use case which frequently switches between local rect and no local rect draws.

    The color param is used to determine whether the opaque hint can be set on the draw state.
    The caller must populate the vertex colors itself.

    The vertex attrib order is always pos, color, [local coords].
 */
static void set_vertex_attributes(GrDrawState* drawState, bool hasLocalCoords, GrColor color) {
    uint32_t flags = GrDefaultGeoProcFactory::kPosition_GPType |
                     GrDefaultGeoProcFactory::kColor_GPType;
    flags |= hasLocalCoords ? GrDefaultGeoProcFactory::kLocalCoord_GPType : 0;
    drawState->setGeometryProcessor(GrDefaultGeoProcFactory::CreateAndSetAttribs(drawState,
                                                                                 flags))->unref();
    if (0xFF == GrColorUnpackA(color)) {
        drawState->setHint(GrDrawState::kVertexColorsAreOpaque_Hint, true);
    }
}

static bool path_fill_type_is_winding(const GrStencilSettings& pathStencilSettings) {
    static const GrStencilSettings::Face pathFace = GrStencilSettings::kFront_Face;
    bool isWinding = kInvert_StencilOp != pathStencilSettings.passOp(pathFace);
    if (isWinding) {
        // Double check that it is in fact winding.
        SkASSERT(kIncClamp_StencilOp == pathStencilSettings.passOp(pathFace));
        SkASSERT(kIncClamp_StencilOp == pathStencilSettings.failOp(pathFace));
        SkASSERT(0x1 != pathStencilSettings.writeMask(pathFace));
        SkASSERT(!pathStencilSettings.isTwoSided());
    }
    return isWinding;
}

template<typename T> static void reset_data_buffer(SkTDArray<T>* buffer, int minReserve) {
    // Assume the next time this buffer fills up it will use approximately the same amount
    // of space as last time. Only resize if we're using less than a third of the
    // allocated space, and leave enough for 50% growth over last time.
    if (3 * buffer->count() < buffer->reserved() && buffer->reserved() > minReserve) {
        int reserve = SkTMax(minReserve, buffer->count() * 3 / 2);
        buffer->reset();
        buffer->setReserve(reserve);
    } else {
        buffer->rewind();
    }
}

enum {
    kTraceCmdBit = 0x80,
    kCmdMask = 0x7f,
};

static inline uint8_t add_trace_bit(uint8_t cmd) { return cmd | kTraceCmdBit; }

static inline uint8_t strip_trace_bit(uint8_t cmd) { return cmd & kCmdMask; }

static inline bool cmd_has_trace_marker(uint8_t cmd) { return SkToBool(cmd & kTraceCmdBit); }

void GrInOrderDrawBuffer::onDrawRect(GrDrawState* ds,
                                     const SkRect& rect,
                                     const SkRect* localRect,
                                     const SkMatrix* localMatrix) {
    GrDrawState::AutoRestoreEffects are(ds);

    GrColor color = ds->getColor();
    set_vertex_attributes(ds, SkToBool(localRect),  color);

    AutoReleaseGeometry geo(this, 4, ds->getVertexStride(), 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return;
    }

    // Go to device coords to allow batching across matrix changes
    SkMatrix matrix = ds->getViewMatrix();

    // When the caller has provided an explicit source rect for a stage then we don't want to
    // modify that stage's matrix. Otherwise if the effect is generating its source rect from
    // the vertex positions then we have to account for the view matrix change.
    GrDrawState::AutoViewMatrixRestore avmr;
    if (!avmr.setIdentity(ds)) {
        return;
    }

    size_t vstride = ds->getVertexStride();

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
    this->drawIndexedInstances(ds, kTriangles_GrPrimitiveType, 1, 4, 6, &devBounds);
}

int GrInOrderDrawBuffer::concatInstancedDraw(const GrDrawState& ds,
                                             const DrawInfo& info,
                                             const GrClipMaskManager::ScissorState& scissorState) {
    SkASSERT(!fCmdBuffer.empty());
    SkASSERT(info.isInstanced());

    const GeometrySrcState& geomSrc = this->getGeomSrc();

    // we only attempt to concat the case when reserved verts are used with a client-specified index
    // buffer. To make this work with client-specified VBs we'd need to know if the VB was updated
    // between draws.
    if (kReserved_GeometrySrcType != geomSrc.fVertexSrc ||
        kBuffer_GeometrySrcType != geomSrc.fIndexSrc) {
        return 0;
    }
    // Check if there is a draw info that is compatible that uses the same VB from the pool and
    // the same IB
    if (kDraw_Cmd != strip_trace_bit(fCmdBuffer.back().fType)) {
        return 0;
    }

    Draw* draw = static_cast<Draw*>(&fCmdBuffer.back());
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GrVertexBuffer* vertexBuffer = poolState.fPoolVertexBuffer;

    if (!draw->fInfo.isInstanced() ||
        draw->fInfo.verticesPerInstance() != info.verticesPerInstance() ||
        draw->fInfo.indicesPerInstance() != info.indicesPerInstance() ||
        draw->fInfo.vertexBuffer() != vertexBuffer ||
        draw->fInfo.indexBuffer() != geomSrc.fIndexBuffer ||
        draw->fScissorState != scissorState) {
        return 0;
    }
    // info does not yet account for the offset from the start of the pool's VB while the previous
    // draw record does.
    int adjustedStartVertex = poolState.fPoolStartVertex + info.startVertex();
    if (draw->fInfo.startVertex() + draw->fInfo.vertexCount() != adjustedStartVertex) {
        return 0;
    }

    SkASSERT(poolState.fPoolStartVertex == draw->fInfo.startVertex() + draw->fInfo.vertexCount());

    // how many instances can be concat'ed onto draw given the size of the index buffer
    int instancesToConcat = this->indexCountInCurrentSource() / info.indicesPerInstance();
    instancesToConcat -= draw->fInfo.instanceCount();
    instancesToConcat = SkTMin(instancesToConcat, info.instanceCount());

    // update the amount of reserved vertex data actually referenced in draws
    size_t vertexBytes = instancesToConcat * info.verticesPerInstance() * ds.getVertexStride();
    poolState.fUsedPoolVertexBytes = SkTMax(poolState.fUsedPoolVertexBytes, vertexBytes);

    draw->fInfo.adjustInstanceCount(instancesToConcat);

    // update last fGpuCmdMarkers to include any additional trace markers that have been added
    if (this->getActiveTraceMarkers().count() > 0) {
        if (cmd_has_trace_marker(draw->fType)) {
            fGpuCmdMarkers.back().addSet(this->getActiveTraceMarkers());
        } else {
            fGpuCmdMarkers.push_back(this->getActiveTraceMarkers());
            draw->fType = add_trace_bit(draw->fType);
        }
    }

    return instancesToConcat;
}

void GrInOrderDrawBuffer::onDraw(const GrDrawState& ds,
                                 const DrawInfo& info,
                                 const GrClipMaskManager::ScissorState& scissorState) {
    SkASSERT(info.vertexBuffer() && (!info.isIndexed() || info.indexBuffer()));

    GeometryPoolState& poolState = fGeoPoolStateStack.back();

    if (!this->recordStateAndShouldDraw(ds, GrGpu::PrimTypeToDrawType(info.primitiveType()),
                                        info.getDstCopy())) {
        return;
    }

    Draw* draw;
    if (info.isInstanced()) {
        int instancesConcated = this->concatInstancedDraw(ds, info, scissorState);
        if (info.instanceCount() > instancesConcated) {
            draw = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Draw, (info, scissorState));
            draw->fInfo.adjustInstanceCount(-instancesConcated);
        } else {
            return;
        }
    } else {
        draw = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Draw, (info, scissorState));
    }
    this->recordTraceMarkersIfNecessary();

    // Adjust the starting vertex and index when we are using reserved or array sources to
    // compensate for the fact that the data was inserted into a larger vb/ib owned by the pool.
    if (kBuffer_GeometrySrcType != this->getGeomSrc().fVertexSrc) {
        size_t bytes = (info.vertexCount() + info.startVertex()) * ds.getVertexStride();
        poolState.fUsedPoolVertexBytes = SkTMax(poolState.fUsedPoolVertexBytes, bytes);
        draw->fInfo.adjustStartVertex(poolState.fPoolStartVertex);
    }
    
    if (info.isIndexed() && kBuffer_GeometrySrcType != this->getGeomSrc().fIndexSrc) {
        size_t bytes = (info.indexCount() + info.startIndex()) * sizeof(uint16_t);
        poolState.fUsedPoolIndexBytes = SkTMax(poolState.fUsedPoolIndexBytes, bytes);
        draw->fInfo.adjustStartIndex(poolState.fPoolStartIndex);
    }
}

void GrInOrderDrawBuffer::onStencilPath(const GrDrawState& ds,
                                        const GrPath* path,
                                        const GrClipMaskManager::ScissorState& scissorState,
                                        const GrStencilSettings& stencilSettings) {
    // Only compare the subset of GrDrawState relevant to path stenciling?
    if (!this->recordStateAndShouldDraw(ds, GrGpu::kStencilPath_DrawType, NULL)) {
        return;
    }
    StencilPath* sp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, StencilPath, (path));
    sp->fScissorState = scissorState;
    sp->fStencilSettings = stencilSettings;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onDrawPath(const GrDrawState& ds,
                                     const GrPath* path,
                                     const GrClipMaskManager::ScissorState& scissorState,
                                     const GrStencilSettings& stencilSettings,
                                     const GrDeviceCoordTexture* dstCopy) {
    // TODO: Only compare the subset of GrDrawState relevant to path covering?
    if (!this->recordStateAndShouldDraw(ds, GrGpu::kDrawPath_DrawType, dstCopy)) {
        return;
    }
    DrawPath* dp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawPath, (path));
    if (dstCopy) {
        dp->fDstCopy = *dstCopy;
    }
    dp->fScissorState = scissorState;
    dp->fStencilSettings = stencilSettings;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onDrawPaths(const GrDrawState& ds,
                                      const GrPathRange* pathRange,
                                      const uint32_t indices[],
                                      int count,
                                      const float transforms[],
                                      PathTransformType transformsType,
                                      const GrClipMaskManager::ScissorState& scissorState,
                                      const GrStencilSettings& stencilSettings,
                                      const GrDeviceCoordTexture* dstCopy) {
    SkASSERT(pathRange);
    SkASSERT(indices);
    SkASSERT(transforms);

    if (!this->recordStateAndShouldDraw(ds, GrGpu::kDrawPath_DrawType, dstCopy)) {
        return;
    }

    uint32_t* savedIndices = fPathIndexBuffer.append(count, indices);
    float* savedTransforms = fPathTransformBuffer.append(count *
                                 GrPathRendering::PathTransformSize(transformsType), transforms);

    if (kDrawPaths_Cmd == strip_trace_bit(fCmdBuffer.back().fType)) {
        // The previous command was also DrawPaths. Try to collapse this call into the one
        // before. Note that stencilling all the paths at once, then covering, may not be
        // equivalent to two separate draw calls if there is overlap. Blending won't work,
        // and the combined calls may also cancel each other's winding numbers in some
        // places. For now the winding numbers are only an issue if the fill is even/odd,
        // because DrawPaths is currently only used for glyphs, and glyphs in the same
        // font tend to all wind in the same direction.
        DrawPaths* previous = static_cast<DrawPaths*>(&fCmdBuffer.back());
        if (pathRange == previous->pathRange() &&
            transformsType == previous->fTransformsType &&
            scissorState == previous->fScissorState &&
            stencilSettings == previous->fStencilSettings &&
            path_fill_type_is_winding(stencilSettings) &&
            !ds.willBlendWithDst()) {
            // Fold this DrawPaths call into the one previous.
            previous->fCount += count;
            return;
        }
    }

    DrawPaths* dp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawPaths, (pathRange));
    dp->fIndicesLocation = savedIndices - fPathIndexBuffer.begin();
    dp->fCount = count;
    dp->fTransformsLocation = savedTransforms - fPathTransformBuffer.begin();
    dp->fTransformsType = transformsType;
    dp->fScissorState = scissorState;
    dp->fStencilSettings = stencilSettings;
    if (dstCopy) {
        dp->fDstCopy = *dstCopy;
    }

    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onClear(const SkIRect* rect, GrColor color,
                                  bool canIgnoreRect, GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    SkIRect r;
    if (NULL == rect) {
        // We could do something smart and remove previous draws and clears to
        // the current render target. If we get that smart we have to make sure
        // those draws aren't read before this clear (render-to-texture).
        r.setLTRB(0, 0, renderTarget->width(), renderTarget->height());
        rect = &r;
    }
    Clear* clr = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Clear, (renderTarget));
    GrColorIsPMAssert(color);
    clr->fColor = color;
    clr->fRect = *rect;
    clr->fCanIgnoreRect = canIgnoreRect;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::clearStencilClip(const SkIRect& rect,
                                           bool insideClip,
                                           GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    ClearStencilClip* clr = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, ClearStencilClip, (renderTarget));
    clr->fRect = rect;
    clr->fInsideClip = insideClip;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::discard(GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    if (!this->caps()->discardRenderTargetSupport()) {
        return;
    }
    Clear* clr = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Clear, (renderTarget));
    clr->fColor = GrColor_ILLEGAL;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::setDrawBuffers(DrawInfo* info) {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    if (kBuffer_GeometrySrcType == this->getGeomSrc().fVertexSrc) {
        info->setVertexBuffer(this->getGeomSrc().fVertexBuffer);
    } else {
        info->setVertexBuffer(poolState.fPoolVertexBuffer);
    }

    if (info->isIndexed()) {
        if (kBuffer_GeometrySrcType == this->getGeomSrc().fIndexSrc) {
            info->setIndexBuffer(this->getGeomSrc().fIndexBuffer);
        } else {
            info->setIndexBuffer(poolState.fPoolIndexBuffer);
        }
    }
}

void GrInOrderDrawBuffer::reset() {
    SkASSERT(1 == fGeoPoolStateStack.count());
    this->resetVertexSource();
    this->resetIndexSource();

    fCmdBuffer.reset();
    fLastState.reset(NULL);
    fVertexPool.reset();
    fIndexPool.reset();
    reset_data_buffer(&fPathIndexBuffer, kPathIdxBufferMinReserve);
    reset_data_buffer(&fPathTransformBuffer, kPathXformBufferMinReserve);
    fGpuCmdMarkers.reset();
}

void GrInOrderDrawBuffer::flush() {
    if (fFlushing) {
        return;
    }

    this->getContext()->getFontCache()->updateTextures();

    SkASSERT(kReserved_GeometrySrcType != this->getGeomSrc().fVertexSrc);
    SkASSERT(kReserved_GeometrySrcType != this->getGeomSrc().fIndexSrc);

    if (fCmdBuffer.empty()) {
        return;
    }

    GrAutoTRestore<bool> flushRestore(&fFlushing);
    fFlushing = true;

    fVertexPool.unmap();
    fIndexPool.unmap();

    CmdBuffer::Iter iter(fCmdBuffer);

    int currCmdMarker = 0;
    fDstGpu->saveActiveTraceMarkers();

    // Updated every time we find a set state cmd to reflect the current state in the playback
    // stream.
    SkAutoTUnref<const GrOptDrawState> currentOptState;

    while (iter.next()) {
        GrGpuTraceMarker newMarker("", -1);
        SkString traceString;
        if (cmd_has_trace_marker(iter->fType)) {
            traceString = fGpuCmdMarkers[currCmdMarker].toString();
            newMarker.fMarker = traceString.c_str();
            fDstGpu->addGpuTraceMarker(&newMarker);
            ++currCmdMarker;
        }

        if (kSetState_Cmd == strip_trace_bit(iter->fType)) {
            SetState* ss = reinterpret_cast<SetState*>(iter.get());
            currentOptState.reset(SkRef(ss->fState.get()));
        } else {
            iter->execute(this, currentOptState.get());
        }

        if (cmd_has_trace_marker(iter->fType)) {
            fDstGpu->removeGpuTraceMarker(&newMarker);
        }
    }

    fDstGpu->restoreActiveTraceMarkers();
    SkASSERT(fGpuCmdMarkers.count() == currCmdMarker);

    this->reset();
    ++fDrawID;
}

void GrInOrderDrawBuffer::Draw::execute(GrInOrderDrawBuffer* buf, const GrOptDrawState* optState) {
    buf->fDstGpu->draw(*optState, fInfo, fScissorState);
}

void GrInOrderDrawBuffer::StencilPath::execute(GrInOrderDrawBuffer* buf,
                                               const GrOptDrawState* optState) {
    buf->fDstGpu->stencilPath(*optState, this->path(), fScissorState, fStencilSettings);
}

void GrInOrderDrawBuffer::DrawPath::execute(GrInOrderDrawBuffer* buf,
                                            const GrOptDrawState* optState) {
    buf->fDstGpu->drawPath(*optState, this->path(), fScissorState, fStencilSettings,
                           fDstCopy.texture() ? &fDstCopy : NULL);
}

void GrInOrderDrawBuffer::DrawPaths::execute(GrInOrderDrawBuffer* buf,
                                             const GrOptDrawState* optState) {
    buf->fDstGpu->drawPaths(*optState, this->pathRange(),
                            &buf->fPathIndexBuffer[fIndicesLocation], fCount,
                            &buf->fPathTransformBuffer[fTransformsLocation], fTransformsType,
                            fScissorState, fStencilSettings, fDstCopy.texture() ? &fDstCopy : NULL);
}

void GrInOrderDrawBuffer::SetState::execute(GrInOrderDrawBuffer*, const GrOptDrawState*) {
}

void GrInOrderDrawBuffer::Clear::execute(GrInOrderDrawBuffer* buf, const GrOptDrawState*) {
    if (GrColor_ILLEGAL == fColor) {
        buf->fDstGpu->discard(this->renderTarget());
    } else {
        buf->fDstGpu->clear(&fRect, fColor, fCanIgnoreRect, this->renderTarget());
    }
}

void GrInOrderDrawBuffer::ClearStencilClip::execute(GrInOrderDrawBuffer* buf,
                                                    const GrOptDrawState*) {
    buf->fDstGpu->clearStencilClip(fRect, fInsideClip, this->renderTarget());
}

void GrInOrderDrawBuffer::CopySurface::execute(GrInOrderDrawBuffer* buf, const GrOptDrawState*) {
    buf->fDstGpu->copySurface(this->dst(), this->src(), fSrcRect, fDstPoint);
}

bool GrInOrderDrawBuffer::copySurface(GrSurface* dst,
                                      GrSurface* src,
                                      const SkIRect& srcRect,
                                      const SkIPoint& dstPoint) {
    if (fDstGpu->canCopySurface(dst, src, srcRect, dstPoint)) {
        CopySurface* cs = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, CopySurface, (dst, src));
        cs->fSrcRect = srcRect;
        cs->fDstPoint = dstPoint;
        this->recordTraceMarkersIfNecessary();
        return true;
    } else if (GrDrawTarget::canCopySurface(dst, src, srcRect, dstPoint)) {
        GrDrawTarget::copySurface(dst, src, srcRect, dstPoint);
        return true;
    } else {
        return false;
    }
}

bool GrInOrderDrawBuffer::canCopySurface(const GrSurface* dst,
                                         const GrSurface* src,
                                         const SkIRect& srcRect,
                                         const SkIPoint& dstPoint) {
    return fDstGpu->canCopySurface(dst, src, srcRect, dstPoint) ||
           GrDrawTarget::canCopySurface(dst, src, srcRect, dstPoint);
}

void GrInOrderDrawBuffer::initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) {
    fDstGpu->initCopySurfaceDstDesc(src, desc);
}

void GrInOrderDrawBuffer::willReserveVertexAndIndexSpace(int vertexCount,
                                                         size_t vertexStride,
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

    int vcount = vertexCount;
    int icount = indexCount;

    if (!insideGeoPush &&
        !unreleasedVertexSpace &&
        !unreleasedIndexSpace &&
        this->geometryHints(vertexStride, &vcount, &icount)) {
        this->flush();
    }
}

bool GrInOrderDrawBuffer::geometryHints(size_t vertexStride,
                                        int* vertexCount,
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
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fVertexSrc);

    // When the caller reserved vertex buffer space we gave it back a pointer
    // provided by the vertex buffer pool. At each draw we tracked the largest
    // offset into the pool's pointer that was referenced. Now we return to the
    // pool any portion at the tail of the allocation that no draw referenced.
    size_t reservedVertexBytes = geoSrc.fVertexSize * geoSrc.fVertexCount;
    fVertexPool.putBack(reservedVertexBytes - poolState.fUsedPoolVertexBytes);
    poolState.fUsedPoolVertexBytes = 0;
    poolState.fPoolVertexBuffer = NULL;
    poolState.fPoolStartVertex = 0;
}

void GrInOrderDrawBuffer::releaseReservedIndexSpace() {
    GeometryPoolState& poolState = fGeoPoolStateStack.back();
    const GeometrySrcState& geoSrc = this->getGeomSrc();

    // If we get a release index space call then our current source should either be reserved
    // or array (which we copied into reserved space).
    SkASSERT(kReserved_GeometrySrcType == geoSrc.fIndexSrc);

    // Similar to releaseReservedVertexSpace we return any unused portion at
    // the tail
    size_t reservedIndexBytes = sizeof(uint16_t) * geoSrc.fIndexCount;
    fIndexPool.putBack(reservedIndexBytes - poolState.fUsedPoolIndexBytes);
    poolState.fUsedPoolIndexBytes = 0;
    poolState.fPoolIndexBuffer = NULL;
    poolState.fPoolStartIndex = 0;
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
    if (kReserved_GeometrySrcType == restoredState.fVertexSrc) {
        poolState.fUsedPoolVertexBytes = restoredState.fVertexSize * restoredState.fVertexCount;
    }
    if (kReserved_GeometrySrcType == restoredState.fIndexSrc) {
        poolState.fUsedPoolIndexBytes = sizeof(uint16_t) * restoredState.fIndexCount;
    }
}

bool GrInOrderDrawBuffer::recordStateAndShouldDraw(const GrDrawState& ds,
                                                   GrGpu::DrawType drawType,
                                                   const GrDeviceCoordTexture* dstCopy) {
    SkAutoTUnref<GrOptDrawState> optState(GrOptDrawState::Create(ds, fDstGpu, dstCopy, drawType));
    if (!optState) {
        return false;
    }
    if (!fLastState || *optState != *fLastState) {
        SetState* ss = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, SetState, (optState));
        fLastState.reset(SkRef(optState.get()));
        if (dstCopy) {
            ss->fDstCopy = *dstCopy;
        }
        ss->fDrawType = drawType;
        this->recordTraceMarkersIfNecessary();
    }
    return true;
}

void GrInOrderDrawBuffer::recordTraceMarkersIfNecessary() {
    SkASSERT(!fCmdBuffer.empty());
    SkASSERT(!cmd_has_trace_marker(fCmdBuffer.back().fType));
    const GrTraceMarkerSet& activeTraceMarkers = this->getActiveTraceMarkers();
    if (activeTraceMarkers.count() > 0) {
        fCmdBuffer.back().fType = add_trace_bit(fCmdBuffer.back().fType);
        fGpuCmdMarkers.push_back(activeTraceMarkers);
    }
}
