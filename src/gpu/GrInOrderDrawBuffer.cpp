/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInOrderDrawBuffer.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrTemplates.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrGpu* gpu,
                                         GrVertexBufferAllocPool* vertexPool,
                                         GrIndexBufferAllocPool* indexPool)
    : INHERITED(gpu, vertexPool, indexPool)
    , fCommands(gpu, vertexPool, indexPool)
    , fPathIndexBuffer(kPathIdxBufferMinReserve * sizeof(char)/4)
    , fPathTransformBuffer(kPathXformBufferMinReserve * sizeof(float)/4)
    , fDrawID(0) {

    SkASSERT(vertexPool);
    SkASSERT(indexPool);
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

        if (!vertices || !batchTarget->quadIndexBuffer()) {
            SkDebugf("Could not allocate buffers\n");
            return;
        }

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

void GrInOrderDrawBuffer::onDraw(const GrGeometryProcessor* gp,
                                 const DrawInfo& info,
                                 const PipelineInfo& pipelineInfo) {
    GrTargetCommands::Cmd* cmd = fCommands.recordDraw(this, gp, info, pipelineInfo);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onDrawBatch(GrBatch* batch,
                                      const PipelineInfo& pipelineInfo) {
    GrTargetCommands::Cmd* cmd = fCommands.recordDrawBatch(this, batch, pipelineInfo);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onStencilPath(const GrPipelineBuilder& pipelineBuilder,
                                        const GrPathProcessor* pathProc,
                                        const GrPath* path,
                                        const GrScissorState& scissorState,
                                        const GrStencilSettings& stencilSettings) {
    GrTargetCommands::Cmd* cmd = fCommands.recordStencilPath(this, pipelineBuilder, 
                                                             pathProc, path, scissorState,
                                                             stencilSettings);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onDrawPath(const GrPathProcessor* pathProc,
                                     const GrPath* path,
                                     const GrStencilSettings& stencilSettings,
                                     const PipelineInfo& pipelineInfo) {
    GrTargetCommands::Cmd* cmd = fCommands.recordDrawPath(this, pathProc,
                                                          path, stencilSettings,
                                                          pipelineInfo);
    this->recordTraceMarkersIfNecessary(cmd);
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
    GrTargetCommands::Cmd* cmd = fCommands.recordDrawPaths(this, pathProc, pathRange,
                                                           indices, indexType, transformValues,
                                                           transformType, count,
                                                           stencilSettings, pipelineInfo);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onClear(const SkIRect* rect, GrColor color,
                                  bool canIgnoreRect, GrRenderTarget* renderTarget) {
    GrTargetCommands::Cmd* cmd = fCommands.recordClear(this, rect, color,
                                                       canIgnoreRect, renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::clearStencilClip(const SkIRect& rect,
                                           bool insideClip,
                                           GrRenderTarget* renderTarget) {
    GrTargetCommands::Cmd* cmd = fCommands.recordClearStencilClip(this, rect,
                                                                  insideClip, renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::discard(GrRenderTarget* renderTarget) {
    if (!this->caps()->discardRenderTargetSupport()) {
        return;
    }

    GrTargetCommands::Cmd* cmd = fCommands.recordDiscard(this, renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onReset() {
    fCommands.reset();
    fPathIndexBuffer.rewind();
    fPathTransformBuffer.rewind();
    fGpuCmdMarkers.reset();
}

void GrInOrderDrawBuffer::onFlush() {
    fCommands.flush(this);
    ++fDrawID;
}

bool GrInOrderDrawBuffer::onCopySurface(GrSurface* dst,
                                        GrSurface* src,
                                        const SkIRect& srcRect,
                                        const SkIPoint& dstPoint) {
    GrTargetCommands::Cmd* cmd = fCommands.recordCopySurface(this, dst, src,
                                                             srcRect, dstPoint);
    this->recordTraceMarkersIfNecessary(cmd);
    return SkToBool(cmd);
}

void GrInOrderDrawBuffer::recordTraceMarkersIfNecessary(GrTargetCommands::Cmd* cmd) {
    if (!cmd) {
        return;
    }
    const GrTraceMarkerSet& activeTraceMarkers = this->getActiveTraceMarkers();
    if (activeTraceMarkers.count() > 0) {
        if (cmd->isTraced()) {
            fGpuCmdMarkers[cmd->markerID()].addSet(activeTraceMarkers);
        } else {
            cmd->setMarkerID(fGpuCmdMarkers.count());
            fGpuCmdMarkers.push_back(activeTraceMarkers);
        }
    }
}

void GrInOrderDrawBuffer::willReserveVertexAndIndexSpace(int vertexCount,
                                                         size_t vertexStride,
                                                         int indexCount) {
    fCommands.closeBatch();

    this->INHERITED::willReserveVertexAndIndexSpace(vertexCount, vertexStride, indexCount);
}
