/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInOrderDrawBuffer.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrTemplates.h"
#include "GrFontCache.h"
#include "GrTexture.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrGpu* gpu,
                                         GrVertexBufferAllocPool* vertexPool,
                                         GrIndexBufferAllocPool* indexPool)
    : INHERITED(gpu, vertexPool, indexPool)
    , fCmdBuffer(kCmdBufferInitialSizeInBytes)
    , fPrevState(NULL)
    , fDrawID(0) {

    SkASSERT(vertexPool);
    SkASSERT(indexPool);

    fPathIndexBuffer.setReserve(kPathIdxBufferMinReserve);
    fPathTransformBuffer.setReserve(kPathXformBufferMinReserve);
}

GrInOrderDrawBuffer::~GrInOrderDrawBuffer() {
    this->reset();
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
static const GrGeometryProcessor* create_rect_gp(bool hasExplicitLocalCoords,
                                                 GrColor color,
                                                 const SkMatrix* localMatrix) {
    uint32_t flags = GrDefaultGeoProcFactory::kPosition_GPType |
                     GrDefaultGeoProcFactory::kColor_GPType;
    flags |= hasExplicitLocalCoords ? GrDefaultGeoProcFactory::kLocalCoord_GPType : 0;
    if (localMatrix) {
        return GrDefaultGeoProcFactory::Create(flags, color, SkMatrix::I(), *localMatrix,
                                               GrColorIsOpaque(color));
    } else {
        return GrDefaultGeoProcFactory::Create(flags, color, SkMatrix::I(), SkMatrix::I(),
                                               GrColorIsOpaque(color));
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
                                     GrColor color,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& rect,
                                     const SkRect* localRect,
                                     const SkMatrix* localMatrix) {
    GrDrawState::AutoRestoreEffects are(ds);

    // Go to device coords to allow batching across matrix changes
    SkMatrix invert = SkMatrix::I();

    // if we have a local rect, then we apply the localMatrix directly to the localRect to generate
    // vertex local coords
    bool hasExplicitLocalCoords = SkToBool(localRect);
    if (!hasExplicitLocalCoords) {
        if (!viewMatrix.isIdentity() && !viewMatrix.invert(&invert)) {
            SkDebugf("Could not invert\n");
            return;
        }

        if (localMatrix) {
            invert.preConcat(*localMatrix);
        }
    }

    SkAutoTUnref<const GrGeometryProcessor> gp(create_rect_gp(hasExplicitLocalCoords,
                                                              color,
                                                              &invert));

    size_t vstride = gp->getVertexStride();
    SkASSERT(vstride == sizeof(SkPoint) + sizeof(GrColor) + (SkToBool(localRect) ? sizeof(SkPoint) :
                                                                                   0));
    AutoReleaseGeometry geo(this, 4, vstride, 0);
    if (!geo.succeeded()) {
        SkDebugf("Failed to get space for vertices!\n");
        return;
    }

    geo.positions()->setRectFan(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, vstride);
    viewMatrix.mapPointsWithStride(geo.positions(), vstride, 4);

    // When the caller has provided an explicit source rect for a stage then we don't want to
    // modify that stage's matrix. Otherwise if the effect is generating its source rect from
    // the vertex positions then we have to account for the view matrix
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
    this->drawIndexedInstances(ds, gp, kTriangles_GrPrimitiveType, 1, 4, 6, &devBounds);
}

int GrInOrderDrawBuffer::concatInstancedDraw(const GrDrawState& ds, const DrawInfo& info) {
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

    if (!draw->fInfo.isInstanced() ||
        draw->fInfo.verticesPerInstance() != info.verticesPerInstance() ||
        draw->fInfo.indicesPerInstance() != info.indicesPerInstance() ||
        draw->fInfo.vertexBuffer() != info.vertexBuffer() ||
        draw->fInfo.indexBuffer() != geomSrc.fIndexBuffer) {
        return 0;
    }
    if (draw->fInfo.startVertex() + draw->fInfo.vertexCount() != info.startVertex()) {
        return 0;
    }

    // how many instances can be concat'ed onto draw given the size of the index buffer
    int instancesToConcat = this->indexCountInCurrentSource() / info.indicesPerInstance();
    instancesToConcat -= draw->fInfo.instanceCount();
    instancesToConcat = SkTMin(instancesToConcat, info.instanceCount());

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
                                 const GrGeometryProcessor* gp,
                                 const DrawInfo& info,
                                 const GrScissorState& scissorState,
                                 const GrDeviceCoordTexture* dstCopy) {
    SkASSERT(info.vertexBuffer() && (!info.isIndexed() || info.indexBuffer()));

    if (!this->recordStateAndShouldDraw(ds, gp,
                                        GrGpu::PrimTypeToDrawType(info.primitiveType()),
                                        scissorState, dstCopy)) {
        return;
    }

    Draw* draw;
    if (info.isInstanced()) {
        int instancesConcated = this->concatInstancedDraw(ds, info);
        if (info.instanceCount() > instancesConcated) {
            draw = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Draw, (info));
            draw->fInfo.adjustInstanceCount(-instancesConcated);
        } else {
            return;
        }
    } else {
        draw = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, Draw, (info));
    }
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onStencilPath(const GrDrawState& ds,
                                        const GrPathProcessor* pathProc,
                                        const GrPath* path,
                                        const GrScissorState& scissorState,
                                        const GrStencilSettings& stencilSettings) {
    StencilPath* sp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, StencilPath,
                                               (path, ds.getRenderTarget()));
    sp->fScissor = scissorState;
    sp->fUseHWAA = ds.isHWAntialias();
    sp->fViewMatrix = pathProc->viewMatrix();
    sp->fStencil = stencilSettings;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onDrawPath(const GrDrawState& ds,
                                     const GrPathProcessor* pathProc,
                                     const GrPath* path,
                                     const GrScissorState& scissorState,
                                     const GrStencilSettings& stencilSettings,
                                     const GrDeviceCoordTexture* dstCopy) {
    // TODO: Only compare the subset of GrDrawState relevant to path covering?
    if (!this->recordStateAndShouldDraw(ds, pathProc, GrGpu::kDrawPath_DrawType,
                                        scissorState, dstCopy)) {
        return;
    }
    DrawPath* dp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawPath, (path));
    dp->fStencilSettings = stencilSettings;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onDrawPaths(const GrDrawState& ds,
                                      const GrPathProcessor* pathProc,
                                      const GrPathRange* pathRange,
                                      const void* indices,
                                      PathIndexType indexType,
                                      const float transformValues[],
                                      PathTransformType transformType,
                                      int count,
                                      const GrScissorState& scissorState,
                                      const GrStencilSettings& stencilSettings,
                                      const GrDeviceCoordTexture* dstCopy) {
    SkASSERT(pathRange);
    SkASSERT(indices);
    SkASSERT(transformValues);

    if (!this->recordStateAndShouldDraw(ds, pathProc, GrGpu::kDrawPath_DrawType, scissorState,
                                        dstCopy)) {
        return;
    }

    int indexBytes = GrPathRange::PathIndexSizeInBytes(indexType);
    if (int misalign = fPathIndexBuffer.count() % indexBytes) {
        // Add padding to the index buffer so the indices are aligned properly.
        fPathIndexBuffer.append(indexBytes - misalign);
    }

    char* savedIndices = fPathIndexBuffer.append(count * indexBytes,
                                                 reinterpret_cast<const char*>(indices));
    float* savedTransforms = fPathTransformBuffer.append(
                                 count * GrPathRendering::PathTransformSize(transformType),
                                 transformValues);

    if (kDrawPaths_Cmd == strip_trace_bit(fCmdBuffer.back().fType)) {
        // The previous command was also DrawPaths. Try to collapse this call into the one
        // before. Note that stenciling all the paths at once, then covering, may not be
        // equivalent to two separate draw calls if there is overlap. Blending won't work,
        // and the combined calls may also cancel each other's winding numbers in some
        // places. For now the winding numbers are only an issue if the fill is even/odd,
        // because DrawPaths is currently only used for glyphs, and glyphs in the same
        // font tend to all wind in the same direction.
        DrawPaths* previous = static_cast<DrawPaths*>(&fCmdBuffer.back());
        if (pathRange == previous->pathRange() &&
            indexType == previous->fIndexType &&
            transformType == previous->fTransformType &&
            stencilSettings == previous->fStencilSettings &&
            path_fill_type_is_winding(stencilSettings) &&
            !ds.willBlendWithDst(pathProc)) {
            // Fold this DrawPaths call into the one previous.
            previous->fCount += count;
            return;
        }
    }

    DrawPaths* dp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawPaths, (pathRange));
    dp->fIndicesLocation = SkToU32(savedIndices - fPathIndexBuffer.begin());
    dp->fIndexType = indexType;
    dp->fTransformsLocation = SkToU32(savedTransforms - fPathTransformBuffer.begin());
    dp->fTransformType = transformType;
    dp->fCount = count;
    dp->fStencilSettings = stencilSettings;

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

void GrInOrderDrawBuffer::onReset() {
    fCmdBuffer.reset();
    fPrevState = NULL;
    reset_data_buffer(&fPathIndexBuffer, kPathIdxBufferMinReserve);
    reset_data_buffer(&fPathTransformBuffer, kPathXformBufferMinReserve);
    fGpuCmdMarkers.reset();
}

void GrInOrderDrawBuffer::onFlush() {
    if (fCmdBuffer.empty()) {
        return;
    }


    CmdBuffer::Iter iter(fCmdBuffer);

    int currCmdMarker = 0;

    // Updated every time we find a set state cmd to reflect the current state in the playback
    // stream.
    SetState* currentState = NULL;

    while (iter.next()) {
        GrGpuTraceMarker newMarker("", -1);
        SkString traceString;
        if (cmd_has_trace_marker(iter->fType)) {
            traceString = fGpuCmdMarkers[currCmdMarker].toString();
            newMarker.fMarker = traceString.c_str();
            this->getGpu()->addGpuTraceMarker(&newMarker);
            ++currCmdMarker;
        }

        if (kSetState_Cmd == strip_trace_bit(iter->fType)) {
            SetState* ss = reinterpret_cast<SetState*>(iter.get());

            this->getGpu()->buildProgramDesc(&ss->fDesc, *ss->fPrimitiveProcessor, ss->fState,
                                             ss->fState.descInfo(), ss->fState.drawType(),
                                             ss->fBatchTracker);
            currentState = ss;

        } else {
            iter->execute(this, currentState);
        }

        if (cmd_has_trace_marker(iter->fType)) {
            this->getGpu()->removeGpuTraceMarker(&newMarker);
        }
    }

    SkASSERT(fGpuCmdMarkers.count() == currCmdMarker);
    ++fDrawID;
}

void GrInOrderDrawBuffer::Draw::execute(GrInOrderDrawBuffer* buf, const SetState* state) {
    SkASSERT(state);
    DrawArgs args(state->fPrimitiveProcessor.get(), &state->fState, &state->fDesc,
                  &state->fBatchTracker);
    buf->getGpu()->draw(args, fInfo);
}

void GrInOrderDrawBuffer::StencilPath::execute(GrInOrderDrawBuffer* buf, const SetState*) {
    GrGpu::StencilPathState state;
    state.fRenderTarget = fRenderTarget.get();
    state.fScissor = &fScissor;
    state.fStencil = &fStencil;
    state.fUseHWAA = fUseHWAA;
    state.fViewMatrix = &fViewMatrix;

    buf->getGpu()->stencilPath(this->path(), state);
}

void GrInOrderDrawBuffer::DrawPath::execute(GrInOrderDrawBuffer* buf, const SetState* state) {
    SkASSERT(state);
    DrawArgs args(state->fPrimitiveProcessor.get(), &state->fState, &state->fDesc,
                  &state->fBatchTracker);
    buf->getGpu()->drawPath(args, this->path(), fStencilSettings);
}

void GrInOrderDrawBuffer::DrawPaths::execute(GrInOrderDrawBuffer* buf, const SetState* state) {
    SkASSERT(state);
    DrawArgs args(state->fPrimitiveProcessor.get(), &state->fState, &state->fDesc,
                  &state->fBatchTracker);
    buf->getGpu()->drawPaths(args, this->pathRange(),
                            &buf->fPathIndexBuffer[fIndicesLocation], fIndexType,
                            &buf->fPathTransformBuffer[fTransformsLocation], fTransformType,
                            fCount, fStencilSettings);
}

void GrInOrderDrawBuffer::SetState::execute(GrInOrderDrawBuffer*, const SetState*) {}

void GrInOrderDrawBuffer::Clear::execute(GrInOrderDrawBuffer* buf, const SetState*) {
    if (GrColor_ILLEGAL == fColor) {
        buf->getGpu()->discard(this->renderTarget());
    } else {
        buf->getGpu()->clear(&fRect, fColor, fCanIgnoreRect, this->renderTarget());
    }
}

void GrInOrderDrawBuffer::ClearStencilClip::execute(GrInOrderDrawBuffer* buf, const SetState*) {
    buf->getGpu()->clearStencilClip(fRect, fInsideClip, this->renderTarget());
}

void GrInOrderDrawBuffer::CopySurface::execute(GrInOrderDrawBuffer* buf, const SetState*) {
    buf->getGpu()->copySurface(this->dst(), this->src(), fSrcRect, fDstPoint);
}

bool GrInOrderDrawBuffer::onCopySurface(GrSurface* dst,
                                        GrSurface* src,
                                        const SkIRect& srcRect,
                                        const SkIPoint& dstPoint) {
    if (getGpu()->canCopySurface(dst, src, srcRect, dstPoint)) {
        CopySurface* cs = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, CopySurface, (dst, src));
        cs->fSrcRect = srcRect;
        cs->fDstPoint = dstPoint;
        this->recordTraceMarkersIfNecessary();
        return true;
    }
    return false;
}

bool GrInOrderDrawBuffer::recordStateAndShouldDraw(const GrDrawState& ds,
                                                   const GrPrimitiveProcessor* primProc,
                                                   GrGpu::DrawType drawType,
                                                   const GrScissorState& scissor,
                                                   const GrDeviceCoordTexture* dstCopy) {
    SetState* ss = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, SetState,
                                            (ds, primProc, *this->getGpu()->caps(), scissor,
                                             dstCopy, drawType));
    if (ss->fState.mustSkip()) {
        fCmdBuffer.pop_back();
        return false;
    }

    ss->fPrimitiveProcessor->initBatchTracker(&ss->fBatchTracker,
                                              ss->fState.getInitBatchTracker());

    if (fPrevState &&
        fPrevState->fPrimitiveProcessor->canMakeEqual(fPrevState->fBatchTracker,
                                                      *ss->fPrimitiveProcessor,
                                                      ss->fBatchTracker) &&
        fPrevState->fState.isEqual(ss->fState)) {
        fCmdBuffer.pop_back();
    } else {
        fPrevState = ss;
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
