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
#include "GrFontCache.h"
#include "GrTexture.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrGpu* gpu,
                                         GrVertexBufferAllocPool* vertexPool,
                                         GrIndexBufferAllocPool* indexPool)
    : INHERITED(gpu, vertexPool, indexPool)
    , fCmdBuffer(kCmdBufferInitialSizeInBytes)
    , fPrevState(NULL)
    , fDrawID(0)
    , fBatchTarget(gpu, vertexPool, indexPool)
    , fDrawBatch(NULL) {

    SkASSERT(vertexPool);
    SkASSERT(indexPool);

    fPathIndexBuffer.setReserve(kPathIdxBufferMinReserve);
    fPathTransformBuffer.setReserve(kPathXformBufferMinReserve);
}

GrInOrderDrawBuffer::~GrInOrderDrawBuffer() {
    this->reset();
}

////////////////////////////////////////////////////////////////////////////////

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

class RectBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        bool fHasLocalRect;
        bool fHasLocalMatrix;
        SkRect fLocalRect;
        SkMatrix fLocalMatrix;
    };

    static GrBatch* Create(const Geometry& geometry) {
        return SkNEW_ARGS(RectBatch, (geometry));
    }

    const char* name() const SK_OVERRIDE { return "RectBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setKnownSingleComponent(0xff);
    }

    void initBatchTracker(const GrPipelineInfo& init) SK_OVERRIDE {
        // Handle any color overrides
        if (init.fColorIgnored) {
            fGeoData[0].fColor = GrColor_ILLEGAL;
        } else if (GrColor_ILLEGAL != init.fOverrideColor) {
            fGeoData[0].fColor = init.fOverrideColor;
        }

        // setup batch properties
        fBatch.fColorIgnored = init.fColorIgnored;
        fBatch.fColor = fGeoData[0].fColor;
        fBatch.fUsesLocalCoords = init.fUsesLocalCoords;
        fBatch.fCoverageIgnored = init.fCoverageIgnored;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        // Go to device coords to allow batching across matrix changes
        SkMatrix invert = SkMatrix::I();

        // if we have a local rect, then we apply the localMatrix directly to the localRect to
        // generate vertex local coords
        bool hasExplicitLocalCoords = this->hasLocalRect();
        if (!hasExplicitLocalCoords) {
            if (!this->viewMatrix().isIdentity() && !this->viewMatrix().invert(&invert)) {
                SkDebugf("Could not invert\n");
                return;
            }

            if (this->hasLocalMatrix()) {
                invert.preConcat(this->localMatrix());
            }
        }

        SkAutoTUnref<const GrGeometryProcessor> gp(create_rect_gp(hasExplicitLocalCoords,
                                                                  this->color(),
                                                                  &invert));

        batchTarget->initDraw(gp, pipeline);

        // TODO this is hacky, but the only way we have to initialize the GP is to use the
        // GrPipelineInfo struct so we can generate the correct shader.  Once we have GrBatch
        // everywhere we can remove this nastiness
        GrPipelineInfo init;
        init.fColorIgnored = fBatch.fColorIgnored;
        init.fOverrideColor = GrColor_ILLEGAL;
        init.fCoverageIgnored = fBatch.fCoverageIgnored;
        init.fUsesLocalCoords = this->usesLocalCoords();
        gp->initBatchTracker(batchTarget->currentBatchTracker(), init);

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(hasExplicitLocalCoords ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorLocalCoordAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr));

        int instanceCount = fGeoData.count();
        int vertexCount = kVertsPerRect * instanceCount;

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            intptr_t offset = GrTCast<intptr_t>(vertices) + kVertsPerRect * i * vertexStride;
            SkPoint* positions = GrTCast<SkPoint*>(offset);

            positions->setRectFan(args.fRect.fLeft, args.fRect.fTop,
                                  args.fRect.fRight, args.fRect.fBottom, vertexStride);
            args.fViewMatrix.mapPointsWithStride(positions, vertexStride, kVertsPerRect);

            if (args.fHasLocalRect) {
                static const int kLocalOffset = sizeof(SkPoint) + sizeof(GrColor);
                SkPoint* coords = GrTCast<SkPoint*>(offset + kLocalOffset);
                coords->setRectFan(args.fLocalRect.fLeft, args.fLocalRect.fTop,
                                   args.fLocalRect.fRight, args.fLocalRect.fBottom,
                                   vertexStride);
                if (args.fHasLocalMatrix) {
                    args.fLocalMatrix.mapPointsWithStride(coords, vertexStride, kVertsPerRect);
                }
            }

            static const int kColorOffset = sizeof(SkPoint);
            GrColor* vertColor = GrTCast<GrColor*>(offset + kColorOffset);
            for (int j = 0; j < 4; ++j) {
                *vertColor = args.fColor;
                vertColor = (GrColor*) ((intptr_t) vertColor + vertexStride);
            }
        }

        const GrIndexBuffer* quadIndexBuffer = batchTarget->quadIndexBuffer();

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVertsPerRect);
        drawInfo.setIndicesPerInstance(kIndicesPerRect);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(quadIndexBuffer);

        int maxInstancesPerDraw = quadIndexBuffer->maxQuads();
        while (instanceCount) {
            drawInfo.setInstanceCount(SkTMin(instanceCount, maxInstancesPerDraw));
            drawInfo.setVertexCount(drawInfo.instanceCount() * drawInfo.verticesPerInstance());
            drawInfo.setIndexCount(drawInfo.instanceCount() * drawInfo.indicesPerInstance());

            batchTarget->draw(drawInfo);

            drawInfo.setStartVertex(drawInfo.startVertex() + drawInfo.vertexCount());
            instanceCount -= drawInfo.instanceCount();
       }
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    RectBatch(const Geometry& geometry) {
        this->initClassID<RectBatch>();
        fGeoData.push_back(geometry);
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    const SkMatrix& localMatrix() const { return fGeoData[0].fLocalMatrix; }
    bool hasLocalRect() const { return fGeoData[0].fHasLocalRect; }
    bool hasLocalMatrix() const { return fGeoData[0].fHasLocalMatrix; }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        RectBatch* that = t->cast<RectBatch>();

        if (this->hasLocalRect() != that->hasLocalRect()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        if (!this->hasLocalRect() && this->usesLocalCoords()) {
            if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
                return false;
            }

            if (this->hasLocalMatrix() != that->hasLocalMatrix()) {
                return false;
            }

            if (this->hasLocalMatrix() && !this->localMatrix().cheapEqualTo(that->localMatrix())) {
                return false;
            }
        }

        if (this->color() != that->color()) {
            fBatch.fColor = GrColor_ILLEGAL;
        }
        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
    };

    const static int kVertsPerRect = 4;
    const static int kIndicesPerRect = 6;

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

void GrInOrderDrawBuffer::onDrawRect(GrPipelineBuilder* pipelineBuilder,
                                     GrColor color,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& rect,
                                     const SkRect* localRect,
                                     const SkMatrix* localMatrix) {
    GrPipelineBuilder::AutoRestoreEffects are(pipelineBuilder);
    RectBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fViewMatrix = viewMatrix;
    geometry.fRect = rect;

    if (localRect) {
        geometry.fHasLocalRect = true;
        geometry.fLocalRect = *localRect;
    } else {
        geometry.fHasLocalRect = false;
    }

    if (localMatrix) {
        geometry.fHasLocalMatrix = true;
        geometry.fLocalMatrix = *localMatrix;
    } else {
        geometry.fHasLocalMatrix = false;
    }

    SkAutoTUnref<GrBatch> batch(RectBatch::Create(geometry));

    SkRect bounds = rect;
    viewMatrix.mapRect(&bounds);
    this->drawBatch(pipelineBuilder, batch, &bounds);
}

int GrInOrderDrawBuffer::concatInstancedDraw(const DrawInfo& info) {
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
    if (Cmd::kDraw_Cmd != fCmdBuffer.back().type()) {
        return 0;
    }

    Draw* draw = static_cast<Draw*>(&fCmdBuffer.back());

    if (!draw->fInfo.isInstanced() ||
        draw->fInfo.primitiveType() != info.primitiveType() ||
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
        if (draw->isTraced()) {
            fGpuCmdMarkers.back().addSet(this->getActiveTraceMarkers());
        } else {
            fGpuCmdMarkers.push_back(this->getActiveTraceMarkers());
            draw->makeTraced();
        }
    }

    return instancesToConcat;
}

void GrInOrderDrawBuffer::onDraw(const GrGeometryProcessor* gp,
                                 const DrawInfo& info,
                                 const PipelineInfo& pipelineInfo) {
    SkASSERT(info.vertexBuffer() && (!info.isIndexed() || info.indexBuffer()));
    this->closeBatch();

    if (!this->setupPipelineAndShouldDraw(gp, pipelineInfo)) {
        return;
    }

    Draw* draw;
    if (info.isInstanced()) {
        int instancesConcated = this->concatInstancedDraw(info);
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

void GrInOrderDrawBuffer::onDrawBatch(GrBatch* batch,
                                      const PipelineInfo& pipelineInfo) {
    if (!this->setupPipelineAndShouldDraw(batch, pipelineInfo)) {
        return;
    }

    // Check if there is a Batch Draw we can batch with
    if (Cmd::kDrawBatch_Cmd != fCmdBuffer.back().type()) {
        fDrawBatch = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawBatch, (batch));
        return;
    }

    DrawBatch* draw = static_cast<DrawBatch*>(&fCmdBuffer.back());
    if (draw->fBatch->combineIfPossible(batch)) {
        return;
    } else {
        this->closeBatch();
        fDrawBatch = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawBatch, (batch));
    }
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onStencilPath(const GrPipelineBuilder& pipelineBuilder,
                                        const GrPathProcessor* pathProc,
                                        const GrPath* path,
                                        const GrScissorState& scissorState,
                                        const GrStencilSettings& stencilSettings) {
    this->closeBatch();

    StencilPath* sp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, StencilPath,
                                               (path, pipelineBuilder.getRenderTarget()));

    sp->fScissor = scissorState;
    sp->fUseHWAA = pipelineBuilder.isHWAntialias();
    sp->fViewMatrix = pathProc->viewMatrix();
    sp->fStencil = stencilSettings;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onDrawPath(const GrPathProcessor* pathProc,
                                     const GrPath* path,
                                     const GrStencilSettings& stencilSettings,
                                     const PipelineInfo& pipelineInfo) {
    this->closeBatch();

    // TODO: Only compare the subset of GrPipelineBuilder relevant to path covering?
    if (!this->setupPipelineAndShouldDraw(pathProc, pipelineInfo)) {
        return;
    }
    DrawPath* dp = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, DrawPath, (path));
    dp->fStencilSettings = stencilSettings;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::onDrawPaths(const GrPathProcessor* pathProc,
                                      const GrPathRange* pathRange,
                                      const void* indices,
                                      PathIndexType indexType,
                                      const float transformValues[],
                                      PathTransformType transformType,
                                      int count,
                                      const GrStencilSettings& stencilSettings,
                                      const PipelineInfo& pipelineInfo) {
    SkASSERT(pathRange);
    SkASSERT(indices);
    SkASSERT(transformValues);
    this->closeBatch();

    if (!this->setupPipelineAndShouldDraw(pathProc, pipelineInfo)) {
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

    if (Cmd::kDrawPaths_Cmd == fCmdBuffer.back().type()) {
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
            !pipelineInfo.willBlendWithDst(pathProc)) {
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
    this->closeBatch();

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
    this->closeBatch();

    ClearStencilClip* clr = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, ClearStencilClip, (renderTarget));
    clr->fRect = rect;
    clr->fInsideClip = insideClip;
    this->recordTraceMarkersIfNecessary();
}

void GrInOrderDrawBuffer::discard(GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    this->closeBatch();

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
    fDrawBatch = NULL;
}

void GrInOrderDrawBuffer::onFlush() {
    if (fCmdBuffer.empty()) {
        return;
    }

    // Updated every time we find a set state cmd to reflect the current state in the playback
    // stream.
    SetState* currentState = NULL;

    // TODO this is temporary while batch is being rolled out
    this->closeBatch();
    this->getVertexAllocPool()->unmap();
    this->getIndexAllocPool()->unmap();
    fBatchTarget.preFlush();

    currentState = NULL;
    CmdBuffer::Iter iter(fCmdBuffer);

    int currCmdMarker = 0;

    int i = 0;
    while (iter.next()) {
        i++;
        GrGpuTraceMarker newMarker("", -1);
        SkString traceString;
        if (iter->isTraced()) {
            traceString = fGpuCmdMarkers[currCmdMarker].toString();
            newMarker.fMarker = traceString.c_str();
            this->getGpu()->addGpuTraceMarker(&newMarker);
            ++currCmdMarker;
        }

        // TODO temporary hack
        if (Cmd::kDrawBatch_Cmd == iter->type()) {
            DrawBatch* db = reinterpret_cast<DrawBatch*>(iter.get());
            fBatchTarget.flushNext(db->fBatch->numberOfDraws());
            continue;
        }

        if (Cmd::kSetState_Cmd == iter->type()) {
            SetState* ss = reinterpret_cast<SetState*>(iter.get());

            // TODO sometimes we have a prim proc, othertimes we have a GrBatch.  Eventually we will
            // only have GrBatch and we can delete this
            if (ss->fPrimitiveProcessor) {
                this->getGpu()->buildProgramDesc(&ss->fDesc, *ss->fPrimitiveProcessor,
                                                 *ss->getPipeline(),
                                                 ss->fBatchTracker);
            }
            currentState = ss;
        } else {
            iter->execute(this, currentState);
        }

        if (iter->isTraced()) {
            this->getGpu()->removeGpuTraceMarker(&newMarker);
        }
    }

    // TODO see copious notes about hack
    fBatchTarget.postFlush();

    SkASSERT(fGpuCmdMarkers.count() == currCmdMarker);
    ++fDrawID;
}

void GrInOrderDrawBuffer::Draw::execute(GrInOrderDrawBuffer* buf, const SetState* state) {
    SkASSERT(state);
    DrawArgs args(state->fPrimitiveProcessor.get(), state->getPipeline(), &state->fDesc,
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
    DrawArgs args(state->fPrimitiveProcessor.get(), state->getPipeline(), &state->fDesc,
                  &state->fBatchTracker);
    buf->getGpu()->drawPath(args, this->path(), fStencilSettings);
}

void GrInOrderDrawBuffer::DrawPaths::execute(GrInOrderDrawBuffer* buf, const SetState* state) {
    SkASSERT(state);
    DrawArgs args(state->fPrimitiveProcessor.get(), state->getPipeline(), &state->fDesc,
                  &state->fBatchTracker);
    buf->getGpu()->drawPaths(args, this->pathRange(),
                            &buf->fPathIndexBuffer[fIndicesLocation], fIndexType,
                            &buf->fPathTransformBuffer[fTransformsLocation], fTransformType,
                            fCount, fStencilSettings);
}

void GrInOrderDrawBuffer::DrawBatch::execute(GrInOrderDrawBuffer* buf, const SetState* state) {
    SkASSERT(state);
    fBatch->generateGeometry(buf->getBatchTarget(), state->getPipeline());
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
        this->closeBatch();
        CopySurface* cs = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, CopySurface, (dst, src));
        cs->fSrcRect = srcRect;
        cs->fDstPoint = dstPoint;
        this->recordTraceMarkersIfNecessary();
        return true;
    }
    return false;
}

bool GrInOrderDrawBuffer::setupPipelineAndShouldDraw(const GrPrimitiveProcessor* primProc,
                                                     const PipelineInfo& pipelineInfo) {
    SetState* ss = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, SetState, (primProc));
    this->setupPipeline(pipelineInfo, ss->pipelineLocation()); 

    if (ss->getPipeline()->mustSkip()) {
        fCmdBuffer.pop_back();
        return false;
    }

    ss->fPrimitiveProcessor->initBatchTracker(&ss->fBatchTracker,
                                              ss->getPipeline()->getInitBatchTracker());

    if (fPrevState && fPrevState->fPrimitiveProcessor.get() &&
        fPrevState->fPrimitiveProcessor->canMakeEqual(fPrevState->fBatchTracker,
                                                      *ss->fPrimitiveProcessor,
                                                      ss->fBatchTracker) &&
        fPrevState->getPipeline()->isEqual(*ss->getPipeline())) {
        fCmdBuffer.pop_back();
    } else {
        fPrevState = ss;
        this->recordTraceMarkersIfNecessary();
    }
    return true;
}

bool GrInOrderDrawBuffer::setupPipelineAndShouldDraw(GrBatch* batch,
                                                     const PipelineInfo& pipelineInfo) {
    SetState* ss = GrNEW_APPEND_TO_RECORDER(fCmdBuffer, SetState, ());
    this->setupPipeline(pipelineInfo, ss->pipelineLocation()); 

    if (ss->getPipeline()->mustSkip()) {
        fCmdBuffer.pop_back();
        return false;
    }

    batch->initBatchTracker(ss->getPipeline()->getInitBatchTracker());

    if (fPrevState && !fPrevState->fPrimitiveProcessor.get() &&
        fPrevState->getPipeline()->isEqual(*ss->getPipeline())) {
        fCmdBuffer.pop_back();
    } else {
        this->closeBatch();
        fPrevState = ss;
        this->recordTraceMarkersIfNecessary();
    }
    return true;
}

void GrInOrderDrawBuffer::recordTraceMarkersIfNecessary() {
    SkASSERT(!fCmdBuffer.empty());
    SkASSERT(!fCmdBuffer.back().isTraced());
    const GrTraceMarkerSet& activeTraceMarkers = this->getActiveTraceMarkers();
    if (activeTraceMarkers.count() > 0) {
        fCmdBuffer.back().makeTraced();
        fGpuCmdMarkers.push_back(activeTraceMarkers);
    }
}

void GrInOrderDrawBuffer::willReserveVertexAndIndexSpace(int vertexCount,
                                                         size_t vertexStride,
                                                         int indexCount) {
    this->closeBatch();

    this->INHERITED::willReserveVertexAndIndexSpace(vertexCount, vertexStride, indexCount);
}
