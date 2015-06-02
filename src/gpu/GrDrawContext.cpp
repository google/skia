
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARectRenderer.h"
#include "GrBatch.h"
#include "GrBatchTest.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawContext.h"
#include "GrOvalRenderer.h"
#include "GrPathRenderer.h"

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == fContext)
#define RETURN_IF_ABANDONED        if (!fDrawTarget) { return; }
#define RETURN_FALSE_IF_ABANDONED  if (!fDrawTarget) { return false; }
#define RETURN_NULL_IF_ABANDONED   if (!fDrawTarget) { return NULL; }

class AutoCheckFlush {
public:
    AutoCheckFlush(GrContext* context) : fContext(context) { SkASSERT(context); }
    ~AutoCheckFlush() { fContext->flushIfNecessary(); }

private:
    GrContext* fContext;
};

GrDrawContext::GrDrawContext(GrContext* context, GrDrawTarget* drawTarget)
    : fContext(context)
    , fDrawTarget(SkRef(drawTarget)) {
}

GrDrawContext::~GrDrawContext() {
    SkSafeUnref(fDrawTarget);
}

void GrDrawContext::copySurface(GrRenderTarget* dst, GrSurface* src,
                                const SkIRect& srcRect, const SkIPoint& dstPoint) {
    if (!this->prepareToDraw(dst)) {
        return;
    }

    fDrawTarget->copySurface(dst, src, srcRect, dstPoint);
}

void GrDrawContext::drawText(GrPipelineBuilder* pipelineBuilder, GrBatch* batch) {
    fDrawTarget->drawBatch(pipelineBuilder, batch);
}

void GrDrawContext::drawPaths(GrPipelineBuilder* pipelineBuilder,
                              const GrPathProcessor* pathProc,
                              const GrPathRange* pathRange,
                              const void* indices,
                              int /*GrDrawTarget::PathIndexType*/ indexType,
                              const float transformValues[],
                              int /*GrDrawTarget::PathTransformType*/ transformType,
                              int count,
                              int /*GrPathRendering::FillType*/ fill) {
    fDrawTarget->drawPaths(pipelineBuilder, pathProc, pathRange,
                           indices, (GrDrawTarget::PathIndexType) indexType,
                           transformValues,
                           (GrDrawTarget::PathTransformType) transformType,
                           count, (GrPathRendering::FillType) fill);
}

void GrDrawContext::discard(GrRenderTarget* renderTarget) {
    RETURN_IF_ABANDONED
    SkASSERT(renderTarget);
    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(renderTarget)) {
        return;
    }
    fDrawTarget->discard(renderTarget);
}

void GrDrawContext::clear(GrRenderTarget* renderTarget,
                          const SkIRect* rect,
                          const GrColor color,
                          bool canIgnoreRect) {
    RETURN_IF_ABANDONED
    SkASSERT(renderTarget);

    AutoCheckFlush acf(fContext);
    if (!this->prepareToDraw(renderTarget)) {
        return;
    }
    fDrawTarget->clear(rect, color, canIgnoreRect, renderTarget);
}


void GrDrawContext::drawPaint(GrRenderTarget* rt,
                              const GrClip& clip,
                              const GrPaint& origPaint,
                              const SkMatrix& viewMatrix) {
    RETURN_IF_ABANDONED
    // set rect to be big enough to fill the space, but not super-huge, so we
    // don't overflow fixed-point implementations
    SkRect r;
    r.setLTRB(0, 0,
              SkIntToScalar(rt->width()),
              SkIntToScalar(rt->height()));
    SkTCopyOnFirstWrite<GrPaint> paint(origPaint);

    // by definition this fills the entire clip, no need for AA
    if (paint->isAntiAlias()) {
        paint.writable()->setAntiAlias(false);
    }

    bool isPerspective = viewMatrix.hasPerspective();

    // We attempt to map r by the inverse matrix and draw that. mapRect will
    // map the four corners and bound them with a new rect. This will not
    // produce a correct result for some perspective matrices.
    if (!isPerspective) {
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }
        inverse.mapRect(&r);
        this->drawRect(rt, clip, *paint, viewMatrix, r);
    } else {
        SkMatrix localMatrix;
        if (!viewMatrix.invert(&localMatrix)) {
            SkDebugf("Could not invert matrix\n");
            return;
        }

        AutoCheckFlush acf(fContext);
        GrPipelineBuilder pipelineBuilder;
        if (!this->prepareToDraw(&pipelineBuilder, rt, clip, paint)) {
            return;
        }

        fDrawTarget->drawBWRect(&pipelineBuilder,
                                paint->getColor(),
                                SkMatrix::I(),
                                r,
                                NULL,
                                &localMatrix);
    }
}

static inline bool is_irect(const SkRect& r) {
  return SkScalarIsInt(r.fLeft)  && SkScalarIsInt(r.fTop) &&
         SkScalarIsInt(r.fRight) && SkScalarIsInt(r.fBottom);
}

static bool apply_aa_to_rect(GrDrawTarget* target,
                             GrPipelineBuilder* pipelineBuilder,
                             SkRect* devBoundRect,
                             const SkRect& rect,
                             SkScalar strokeWidth,
                             const SkMatrix& combinedMatrix,
                             GrColor color) {
    if (pipelineBuilder->getRenderTarget()->isMultisampled()) {
        return false;
    }

#if defined(SHADER_AA_FILL_RECT) || !defined(IGNORE_ROT_AA_RECT_OPT)
    if (strokeWidth >= 0) {
#endif
        if (!combinedMatrix.preservesAxisAlignment()) {
            return false;
        }

#if defined(SHADER_AA_FILL_RECT) || !defined(IGNORE_ROT_AA_RECT_OPT)
    } else {
        if (!combinedMatrix.preservesRightAngles()) {
            return false;
        }
    }
#endif

    combinedMatrix.mapRect(devBoundRect, rect);
    if (!combinedMatrix.rectStaysRect()) {
        return true;
    }

    if (strokeWidth < 0) {
        return !is_irect(*devBoundRect);
    }

    return true;
}

static inline bool rect_contains_inclusive(const SkRect& rect, const SkPoint& point) {
    return point.fX >= rect.fLeft && point.fX <= rect.fRight &&
           point.fY >= rect.fTop && point.fY <= rect.fBottom;
}

class StrokeRectBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkScalar fStrokeWidth;
    };

    static GrBatch* Create(const Geometry& geometry, bool snapToPixelCenters) {
        return SkNEW_ARGS(StrokeRectBatch, (geometry, snapToPixelCenters));
    }

    const char* name() const override { return "StrokeRectBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setKnownSingleComponent(0xff);
    }

    void initBatchTracker(const GrPipelineInfo& init) override {
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

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) override {
        SkAutoTUnref<const GrGeometryProcessor> gp(
                GrDefaultGeoProcFactory::Create(GrDefaultGeoProcFactory::kPosition_GPType,
                                                this->color(),
                                                this->usesLocalCoords(),
                                                this->coverageIgnored(),
                                                this->viewMatrix(),
                                                SkMatrix::I()));

        batchTarget->initDraw(gp, pipeline);

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(vertexStride == sizeof(GrDefaultGeoProcFactory::PositionAttr));

        Geometry& args = fGeoData[0];

        int vertexCount = kVertsPerHairlineRect;
        if (args.fStrokeWidth > 0) {
            vertexCount = kVertsPerStrokeRect;
        }

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* verts = batchTarget->makeVertSpace(vertexStride, vertexCount,
                                                 &vertexBuffer, &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        SkPoint* vertex = reinterpret_cast<SkPoint*>(verts);

        GrPrimitiveType primType;

        if (args.fStrokeWidth > 0) {;
            primType = kTriangleStrip_GrPrimitiveType;
            args.fRect.sort();
            this->setStrokeRectStrip(vertex, args.fRect, args.fStrokeWidth);
        } else {
            // hairline
            primType = kLineStrip_GrPrimitiveType;
            vertex[0].set(args.fRect.fLeft, args.fRect.fTop);
            vertex[1].set(args.fRect.fRight, args.fRect.fTop);
            vertex[2].set(args.fRect.fRight, args.fRect.fBottom);
            vertex[3].set(args.fRect.fLeft, args.fRect.fBottom);
            vertex[4].set(args.fRect.fLeft, args.fRect.fTop);
        }

        GrVertices vertices;
        vertices.init(primType, vertexBuffer, firstVertex, vertexCount);
        batchTarget->draw(vertices);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    StrokeRectBatch(const Geometry& geometry, bool snapToPixelCenters) {
        this->initClassID<StrokeRectBatch>();

        fBatch.fHairline = geometry.fStrokeWidth == 0;

        fGeoData.push_back(geometry);

        // setup bounds
        fBounds = geometry.fRect;
        SkScalar rad = SkScalarHalf(geometry.fStrokeWidth);
        fBounds.outset(rad, rad);
        geometry.fViewMatrix.mapRect(&fBounds);

        // If our caller snaps to pixel centers then we have to round out the bounds
        if (snapToPixelCenters) {
            fBounds.roundOut();
        }
    }

    /*  create a triangle strip that strokes the specified rect. There are 8
     unique vertices, but we repeat the last 2 to close up. Alternatively we
     could use an indices array, and then only send 8 verts, but not sure that
     would be faster.
     */
    void setStrokeRectStrip(SkPoint verts[10], const SkRect& rect, SkScalar width) {
        const SkScalar rad = SkScalarHalf(width);
        // TODO we should be able to enable this assert, but we'd have to filter these draws
        // this is a bug
        //SkASSERT(rad < rect.width() / 2 && rad < rect.height() / 2);

        verts[0].set(rect.fLeft + rad, rect.fTop + rad);
        verts[1].set(rect.fLeft - rad, rect.fTop - rad);
        verts[2].set(rect.fRight - rad, rect.fTop + rad);
        verts[3].set(rect.fRight + rad, rect.fTop - rad);
        verts[4].set(rect.fRight - rad, rect.fBottom - rad);
        verts[5].set(rect.fRight + rad, rect.fBottom + rad);
        verts[6].set(rect.fLeft + rad, rect.fBottom - rad);
        verts[7].set(rect.fLeft - rad, rect.fBottom + rad);
        verts[8] = verts[0];
        verts[9] = verts[1];
    }


    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }
    bool hairline() const { return fBatch.fHairline; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t) override {
        // StrokeRectBatch* that = t->cast<StrokeRectBatch>();

        // NonAA stroke rects cannot batch right now
        // TODO make these batchable
        return false;
    }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fHairline;
    };

    const static int kVertsPerHairlineRect = 5;
    const static int kVertsPerStrokeRect = 10;

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

void GrDrawContext::drawRect(GrRenderTarget* rt,
                             const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& rect,
                             const GrStrokeInfo* strokeInfo) {
    RETURN_IF_ABANDONED
    if (strokeInfo && strokeInfo->isDashed()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRect(rect);
        this->drawPath(rt, clip, paint, viewMatrix, path, *strokeInfo);
        return;
    }

    AutoCheckFlush acf(fContext);
    GrPipelineBuilder pipelineBuilder;
    if (!this->prepareToDraw(&pipelineBuilder, rt, clip, &paint)) {
        return;
    }

    SkScalar width = NULL == strokeInfo ? -1 : strokeInfo->getWidth();

    // Check if this is a full RT draw and can be replaced with a clear. We don't bother checking
    // cases where the RT is fully inside a stroke.
    if (width < 0) {
        SkRect rtRect;
        pipelineBuilder.getRenderTarget()->getBoundsRect(&rtRect);
        SkRect clipSpaceRTRect = rtRect;
        bool checkClip = GrClip::kWideOpen_ClipType != clip.clipType();
        if (checkClip) {
            clipSpaceRTRect.offset(SkIntToScalar(clip.origin().fX),
                                   SkIntToScalar(clip.origin().fY));
        }
        // Does the clip contain the entire RT?
        if (!checkClip || clip.quickContains(clipSpaceRTRect)) {
            SkMatrix invM;
            if (!viewMatrix.invert(&invM)) {
                return;
            }
            // Does the rect bound the RT?
            SkPoint srcSpaceRTQuad[4];
            invM.mapRectToQuad(srcSpaceRTQuad, rtRect);
            if (rect_contains_inclusive(rect, srcSpaceRTQuad[0]) &&
                rect_contains_inclusive(rect, srcSpaceRTQuad[1]) &&
                rect_contains_inclusive(rect, srcSpaceRTQuad[2]) &&
                rect_contains_inclusive(rect, srcSpaceRTQuad[3])) {
                // Will it blend?
                GrColor clearColor;
                if (paint.isConstantBlendedColor(&clearColor)) {
                    fDrawTarget->clear(NULL, clearColor, true, rt);
                    return;
                }
            }
        }
    }

    GrColor color = paint.getColor();
    SkRect devBoundRect;
    bool needAA = paint.isAntiAlias() && !pipelineBuilder.getRenderTarget()->isMultisampled();
    bool doAA = needAA && apply_aa_to_rect(fDrawTarget, &pipelineBuilder, &devBoundRect, rect,
                                           width, viewMatrix, color);

    if (doAA) {
        if (width >= 0) {
            GrAARectRenderer::StrokeAARect(fDrawTarget,
                                           &pipelineBuilder,
                                           color,
                                           viewMatrix,
                                           rect,
                                           devBoundRect,
                                           *strokeInfo);
        } else {
            // filled AA rect
            GrAARectRenderer::FillAARect(fDrawTarget,
                                         &pipelineBuilder,
                                         color,
                                         viewMatrix,
                                         rect,
                                         devBoundRect);
        }
        return;
    }

    if (width >= 0) {
        StrokeRectBatch::Geometry geometry;
        geometry.fViewMatrix = viewMatrix;
        geometry.fColor = color;
        geometry.fRect = rect;
        geometry.fStrokeWidth = width;

        // Non-AA hairlines are snapped to pixel centers to make which pixels are hit deterministic
        bool snapToPixelCenters = (0 == width && !rt->isMultisampled());
        SkAutoTUnref<GrBatch> batch(StrokeRectBatch::Create(geometry, snapToPixelCenters));

        // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
        // hairline rects. We jam all the vertices to pixel centers to avoid this, but not when MSAA
        // is enabled because it can cause ugly artifacts.
        pipelineBuilder.setState(GrPipelineBuilder::kSnapVerticesToPixelCenters_Flag,
                                 snapToPixelCenters);
        fDrawTarget->drawBatch(&pipelineBuilder, batch);
    } else {
        // filled BW rect
        fDrawTarget->drawSimpleRect(&pipelineBuilder, color, viewMatrix, rect);
    }
}

void GrDrawContext::drawNonAARectToRect(GrRenderTarget* rt,
                                        const GrClip& clip,
                                        const GrPaint& paint,
                                        const SkMatrix& viewMatrix,
                                        const SkRect& rectToDraw,
                                        const SkRect& localRect,
                                        const SkMatrix* localMatrix) {
    RETURN_IF_ABANDONED
    AutoCheckFlush acf(fContext);
    GrPipelineBuilder pipelineBuilder;
    if (!this->prepareToDraw(&pipelineBuilder, rt, clip, &paint)) {
        return;
    }

    fDrawTarget->drawBWRect(&pipelineBuilder,
                            paint.getColor(),
                            viewMatrix,
                            rectToDraw,
                            &localRect,
                            localMatrix);
}

static const GrGeometryProcessor* set_vertex_attributes(bool hasLocalCoords,
                                                        bool hasColors,
                                                        int* colorOffset,
                                                        int* texOffset,
                                                        GrColor color,
                                                        const SkMatrix& viewMatrix,
                                                        bool coverageIgnored) {
    *texOffset = -1;
    *colorOffset = -1;
    uint32_t flags = GrDefaultGeoProcFactory::kPosition_GPType;
    if (hasLocalCoords && hasColors) {
        *colorOffset = sizeof(SkPoint);
        *texOffset = sizeof(SkPoint) + sizeof(GrColor);
        flags |= GrDefaultGeoProcFactory::kColor_GPType |
                 GrDefaultGeoProcFactory::kLocalCoord_GPType;
    } else if (hasLocalCoords) {
        *texOffset = sizeof(SkPoint);
        flags |= GrDefaultGeoProcFactory::kLocalCoord_GPType;
    } else if (hasColors) {
        *colorOffset = sizeof(SkPoint);
        flags |= GrDefaultGeoProcFactory::kColor_GPType;
    }
    return GrDefaultGeoProcFactory::Create(flags, color, hasLocalCoords, coverageIgnored,
                                           viewMatrix, SkMatrix::I());
}

class DrawVerticesBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkTDArray<SkPoint> fPositions;
        SkTDArray<uint16_t> fIndices;
        SkTDArray<GrColor> fColors;
        SkTDArray<SkPoint> fLocalCoords;
    };

    static GrBatch* Create(const Geometry& geometry, GrPrimitiveType primitiveType,
                           const SkMatrix& viewMatrix,
                           const SkPoint* positions, int vertexCount,
                           const uint16_t* indices, int indexCount,
                           const GrColor* colors, const SkPoint* localCoords,
                           const SkRect& bounds) {
        return SkNEW_ARGS(DrawVerticesBatch, (geometry, primitiveType, viewMatrix, positions,
                                              vertexCount, indices, indexCount, colors,
                                              localCoords, bounds));
    }

    const char* name() const override { return "DrawVerticesBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const override {
        // When this is called on a batch, there is only one geometry bundle
        if (this->hasColors()) {
            out->setUnknownFourComponents();
        } else {
            out->setKnownFourComponents(fGeoData[0].fColor);
        }
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const override {
        out->setKnownSingleComponent(0xff);
    }

    void initBatchTracker(const GrPipelineInfo& init) override {
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

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) override {
        int colorOffset = -1, texOffset = -1;
        SkAutoTUnref<const GrGeometryProcessor> gp(
                set_vertex_attributes(this->hasLocalCoords(), this->hasColors(), &colorOffset,
                                      &texOffset, this->color(), this->viewMatrix(),
                                      this->coverageIgnored()));

        batchTarget->initDraw(gp, pipeline);

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(vertexStride == sizeof(SkPoint) + (this->hasLocalCoords() ? sizeof(SkPoint) : 0)
                                                 + (this->hasColors() ? sizeof(GrColor) : 0));

        int instanceCount = fGeoData.count();

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* verts = batchTarget->makeVertSpace(vertexStride, this->vertexCount(),
                                                 &vertexBuffer, &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const GrIndexBuffer* indexBuffer = NULL;
        int firstIndex = 0;

        uint16_t* indices = NULL;
        if (this->hasIndices()) {
            indices = batchTarget->makeIndexSpace(this->indexCount(), &indexBuffer, &firstIndex);

            if (!indices) {
                SkDebugf("Could not allocate indices\n");
                return;
            }
        }

        int indexOffset = 0;
        int vertexOffset = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            // TODO we can actually cache this interleaved and then just memcopy
            if (this->hasIndices()) {
                for (int j = 0; j < args.fIndices.count(); ++j, ++indexOffset) {
                    *(indices + indexOffset) = args.fIndices[j] + vertexOffset;
                }
            }

            for (int j = 0; j < args.fPositions.count(); ++j) {
                *((SkPoint*)verts) = args.fPositions[j];
                if (this->hasColors()) {
                    *(GrColor*)((intptr_t)verts + colorOffset) = args.fColors[j];
                }
                if (this->hasLocalCoords()) {
                    *(SkPoint*)((intptr_t)verts + texOffset) = args.fLocalCoords[j];
                }
                verts = (void*)((intptr_t)verts + vertexStride);
                vertexOffset++;
            }
        }

        GrVertices vertices;
        if (this->hasIndices()) {
            vertices.initIndexed(this->primitiveType(), vertexBuffer, indexBuffer, firstVertex,
                                 firstIndex, this->vertexCount(), this->indexCount());

        } else {
            vertices.init(this->primitiveType(), vertexBuffer, firstVertex, this->vertexCount());
        }
        batchTarget->draw(vertices);
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

private:
    DrawVerticesBatch(const Geometry& geometry, GrPrimitiveType primitiveType,
                      const SkMatrix& viewMatrix,
                      const SkPoint* positions, int vertexCount,
                      const uint16_t* indices, int indexCount,
                      const GrColor* colors, const SkPoint* localCoords, const SkRect& bounds) {
        this->initClassID<DrawVerticesBatch>();
        SkASSERT(positions);

        fBatch.fViewMatrix = viewMatrix;
        Geometry& installedGeo = fGeoData.push_back(geometry);

        installedGeo.fPositions.append(vertexCount, positions);
        if (indices) {
            installedGeo.fIndices.append(indexCount, indices);
            fBatch.fHasIndices = true;
        } else {
            fBatch.fHasIndices = false;
        }

        if (colors) {
            installedGeo.fColors.append(vertexCount, colors);
            fBatch.fHasColors = true;
        } else {
            fBatch.fHasColors = false;
        }

        if (localCoords) {
            installedGeo.fLocalCoords.append(vertexCount, localCoords);
            fBatch.fHasLocalCoords = true;
        } else {
            fBatch.fHasLocalCoords = false;
        }
        fBatch.fVertexCount = vertexCount;
        fBatch.fIndexCount = indexCount;
        fBatch.fPrimitiveType = primitiveType;

        this->setBounds(bounds);
    }

    GrPrimitiveType primitiveType() const { return fBatch.fPrimitiveType; }
    bool batchablePrimitiveType() const {
        return kTriangles_GrPrimitiveType == fBatch.fPrimitiveType ||
               kLines_GrPrimitiveType == fBatch.fPrimitiveType ||
               kPoints_GrPrimitiveType == fBatch.fPrimitiveType;
    }
    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool hasColors() const { return fBatch.fHasColors; }
    bool hasIndices() const { return fBatch.fHasIndices; }
    bool hasLocalCoords() const { return fBatch.fHasLocalCoords; }
    int vertexCount() const { return fBatch.fVertexCount; }
    int indexCount() const { return fBatch.fIndexCount; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }

    bool onCombineIfPossible(GrBatch* t) override {
        DrawVerticesBatch* that = t->cast<DrawVerticesBatch>();

        if (!this->batchablePrimitiveType() || this->primitiveType() != that->primitiveType()) {
            return false;
        }

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());

        // We currently use a uniform viewmatrix for this batch
        if (!this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (this->hasColors() != that->hasColors()) {
            return false;
        }

        if (this->hasIndices() != that->hasIndices()) {
            return false;
        }

        if (this->hasLocalCoords() != that->hasLocalCoords()) {
            return false;
        }

        if (!this->hasColors() && this->color() != that->color()) {
            return false;
        }

        if (this->color() != that->color()) {
            fBatch.fColor = GrColor_ILLEGAL;
        }
        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        fBatch.fVertexCount += that->vertexCount();
        fBatch.fIndexCount += that->indexCount();

        this->joinBounds(that->bounds());
        return true;
    }

    struct BatchTracker {
        GrPrimitiveType fPrimitiveType;
        SkMatrix fViewMatrix;
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fHasColors;
        bool fHasIndices;
        bool fHasLocalCoords;
        int fVertexCount;
        int fIndexCount;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
};

void GrDrawContext::drawVertices(GrRenderTarget* rt,
                                 const GrClip& clip,
                                 const GrPaint& paint,
                                 const SkMatrix& viewMatrix,
                                 GrPrimitiveType primitiveType,
                                 int vertexCount,
                                 const SkPoint positions[],
                                 const SkPoint texCoords[],
                                 const GrColor colors[],
                                 const uint16_t indices[],
                                 int indexCount) {
    RETURN_IF_ABANDONED
    AutoCheckFlush acf(fContext);
    GrPipelineBuilder pipelineBuilder;

    if (!this->prepareToDraw(&pipelineBuilder, rt, clip, &paint)) {
        return;
    }

    // TODO clients should give us bounds
    SkRect bounds;
    if (!bounds.setBoundsCheck(positions, vertexCount)) {
        SkDebugf("drawVertices call empty bounds\n");
        return;
    }

    viewMatrix.mapRect(&bounds);

    // If we don't have AA then we outset for a half pixel in each direction to account for
    // snapping
    if (!paint.isAntiAlias()) {
        bounds.outset(0.5f, 0.5f);
    }

    DrawVerticesBatch::Geometry geometry;
    geometry.fColor = paint.getColor();
    SkAutoTUnref<GrBatch> batch(DrawVerticesBatch::Create(geometry, primitiveType, viewMatrix,
                                                          positions, vertexCount, indices,
                                                          indexCount, colors, texCoords,
                                                          bounds));

    fDrawTarget->drawBatch(&pipelineBuilder, batch);
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawRRect(GrRenderTarget*rt,
                              const GrClip& clip,
                              const GrPaint& paint,
                              const SkMatrix& viewMatrix,
                              const SkRRect& rrect,
                              const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    if (rrect.isEmpty()) {
       return;
    }

    if (strokeInfo.isDashed()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rrect);
        this->drawPath(rt, clip, paint, viewMatrix, path, strokeInfo);
        return;
    }

    AutoCheckFlush acf(fContext);
    GrPipelineBuilder pipelineBuilder;
    if (!this->prepareToDraw(&pipelineBuilder, rt, clip, &paint)) {
        return;
    }

    GrColor color = paint.getColor();
    if (!GrOvalRenderer::DrawRRect(fDrawTarget,
                                   &pipelineBuilder,
                                   color,
                                   viewMatrix,
                                   paint.isAntiAlias(),
                                   rrect,
                                   strokeInfo)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(rrect);
        this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, strokeInfo);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawDRRect(GrRenderTarget* rt,
                               const GrClip& clip,
                               const GrPaint& paint,
                               const SkMatrix& viewMatrix,
                               const SkRRect& outer,
                               const SkRRect& inner) {
    RETURN_IF_ABANDONED
    if (outer.isEmpty()) {
       return;
    }

    AutoCheckFlush acf(fContext);
    GrPipelineBuilder pipelineBuilder;
    if (!this->prepareToDraw(&pipelineBuilder, rt, clip, &paint)) {
        return;
    }

    GrColor color = paint.getColor();
    if (!GrOvalRenderer::DrawDRRect(fDrawTarget,
                                    &pipelineBuilder,
                                    color,
                                    viewMatrix,
                                    paint.isAntiAlias(),
                                    outer,
                                    inner)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addRRect(inner);
        path.addRRect(outer);
        path.setFillType(SkPath::kEvenOdd_FillType);

        GrStrokeInfo fillRec(SkStrokeRec::kFill_InitStyle);
        this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, fillRec);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrDrawContext::drawOval(GrRenderTarget* rt,
                             const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkRect& oval,
                             const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    if (oval.isEmpty()) {
       return;
    }

    if (strokeInfo.isDashed()) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->drawPath(rt, clip, paint, viewMatrix, path, strokeInfo);
        return;
    }

    AutoCheckFlush acf(fContext);
    GrPipelineBuilder pipelineBuilder;
    if (!this->prepareToDraw(&pipelineBuilder, rt, clip, &paint)) {
        return;
    }

    GrColor color = paint.getColor();
    if (!GrOvalRenderer::DrawOval(fDrawTarget,
                                  &pipelineBuilder,
                                  color,
                                  viewMatrix,
                                  paint.isAntiAlias(),
                                  oval,
                                  strokeInfo)) {
        SkPath path;
        path.setIsVolatile(true);
        path.addOval(oval);
        this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color,
                               paint.isAntiAlias(), path, strokeInfo);
    }
}

// Can 'path' be drawn as a pair of filled nested rectangles?
static bool is_nested_rects(GrDrawTarget* target,
                            GrPipelineBuilder* pipelineBuilder,
                            GrColor color,
                            const SkMatrix& viewMatrix,
                            const SkPath& path,
                            const SkStrokeRec& stroke,
                            SkRect rects[2]) {
    SkASSERT(stroke.isFillStyle());

    if (path.isInverseFillType()) {
        return false;
    }

    // TODO: this restriction could be lifted if we were willing to apply
    // the matrix to all the points individually rather than just to the rect
    if (!viewMatrix.preservesAxisAlignment()) {
        return false;
    }

    SkPath::Direction dirs[2];
    if (!path.isNestedFillRects(rects, dirs)) {
        return false;
    }

    if (SkPath::kWinding_FillType == path.getFillType() && dirs[0] == dirs[1]) {
        // The two rects need to be wound opposite to each other
        return false;
    }

    // Right now, nested rects where the margin is not the same width
    // all around do not render correctly
    const SkScalar* outer = rects[0].asScalars();
    const SkScalar* inner = rects[1].asScalars();

    bool allEq = true;

    SkScalar margin = SkScalarAbs(outer[0] - inner[0]);
    bool allGoE1 = margin >= SK_Scalar1;

    for (int i = 1; i < 4; ++i) {
        SkScalar temp = SkScalarAbs(outer[i] - inner[i]);
        if (temp < SK_Scalar1) {
            allGoE1 = false;
        }
        if (!SkScalarNearlyEqual(margin, temp)) {
            allEq = false;
        }
    }

    return allEq || allGoE1;
}

void GrDrawContext::drawPath(GrRenderTarget* rt,
                             const GrClip& clip,
                             const GrPaint& paint,
                             const SkMatrix& viewMatrix,
                             const SkPath& path,
                             const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    if (path.isEmpty()) {
       if (path.isInverseFillType()) {
           this->drawPaint(rt, clip, paint, viewMatrix);
       }
       return;
    }

    GrColor color = paint.getColor();

    // Note that internalDrawPath may sw-rasterize the path into a scratch texture.
    // Scratch textures can be recycled after they are returned to the texture
    // cache. This presents a potential hazard for buffered drawing. However,
    // the writePixels that uploads to the scratch will perform a flush so we're
    // OK.
    AutoCheckFlush acf(fContext);
    GrPipelineBuilder pipelineBuilder;
    if (!this->prepareToDraw(&pipelineBuilder, rt, clip, &paint)) {
        return;
    }

    if (!strokeInfo.isDashed()) {
        bool useCoverageAA = paint.isAntiAlias() &&
                !pipelineBuilder.getRenderTarget()->isMultisampled();

        if (useCoverageAA && strokeInfo.getWidth() < 0 && !path.isConvex()) {
            // Concave AA paths are expensive - try to avoid them for special cases
            SkRect rects[2];

            if (is_nested_rects(fDrawTarget, &pipelineBuilder, color, viewMatrix, path, strokeInfo,
                                rects)) {
                GrAARectRenderer::FillAANestedRects(fDrawTarget, &pipelineBuilder, color,
                                                    viewMatrix, rects);
                return;
            }
        }
        SkRect ovalRect;
        bool isOval = path.isOval(&ovalRect);

        if (isOval && !path.isInverseFillType()) {
            if (GrOvalRenderer::DrawOval(fDrawTarget,
                                         &pipelineBuilder,
                                         color,
                                         viewMatrix,
                                         paint.isAntiAlias(),
                                         ovalRect,
                                         strokeInfo)) {
                return;
            }
        }
    }
    this->internalDrawPath(fDrawTarget, &pipelineBuilder, viewMatrix, color, paint.isAntiAlias(),
                           path, strokeInfo);
}

void GrDrawContext::internalDrawPath(GrDrawTarget* target,
                                     GrPipelineBuilder* pipelineBuilder,
                                     const SkMatrix& viewMatrix,
                                     GrColor color,
                                     bool useAA,
                                     const SkPath& path,
                                     const GrStrokeInfo& strokeInfo) {
    RETURN_IF_ABANDONED
    SkASSERT(!path.isEmpty());


    // An Assumption here is that path renderer would use some form of tweaking
    // the src color (either the input alpha or in the frag shader) to implement
    // aa. If we have some future driver-mojo path AA that can do the right
    // thing WRT to the blend then we'll need some query on the PR.
    bool useCoverageAA = useAA &&
        !pipelineBuilder->getRenderTarget()->isMultisampled();


    GrPathRendererChain::DrawType type =
        useCoverageAA ? GrPathRendererChain::kColorAntiAlias_DrawType :
                        GrPathRendererChain::kColor_DrawType;

    const SkPath* pathPtr = &path;
    SkTLazy<SkPath> tmpPath;
    const GrStrokeInfo* strokeInfoPtr = &strokeInfo;

    // Try a 1st time without stroking the path and without allowing the SW renderer
    GrPathRenderer* pr = fContext->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr,
                                                    *strokeInfoPtr, false, type);

    GrStrokeInfo dashlessStrokeInfo(strokeInfo, false);
    if (NULL == pr && strokeInfo.isDashed()) {
        // It didn't work above, so try again with dashed stroke converted to a dashless stroke.
        if (!strokeInfo.applyDashToPath(tmpPath.init(), &dashlessStrokeInfo, *pathPtr)) {
            return;
        }
        pathPtr = tmpPath.get();
        if (pathPtr->isEmpty()) {
            return;
        }
        strokeInfoPtr = &dashlessStrokeInfo;
        pr = fContext->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr, *strokeInfoPtr,
                                       false, type);
    }

    if (NULL == pr) {
        if (!GrPathRenderer::IsStrokeHairlineOrEquivalent(*strokeInfoPtr, viewMatrix, NULL) &&
            !strokeInfoPtr->isFillStyle()) {
            // It didn't work above, so try again with stroke converted to a fill.
            if (!tmpPath.isValid()) {
                tmpPath.init();
            }
            dashlessStrokeInfo.setResScale(SkScalarAbs(viewMatrix.getMaxScale()));
            if (!dashlessStrokeInfo.applyToPath(tmpPath.get(), *pathPtr)) {
                return;
            }
            pathPtr = tmpPath.get();
            if (pathPtr->isEmpty()) {
                return;
            }
            dashlessStrokeInfo.setFillStyle();
            strokeInfoPtr = &dashlessStrokeInfo;
        }

        // This time, allow SW renderer
        pr = fContext->getPathRenderer(target, pipelineBuilder, viewMatrix, *pathPtr, *strokeInfoPtr,
                                       true, type);
    }

    if (NULL == pr) {
#ifdef SK_DEBUG
        SkDebugf("Unable to find path renderer compatible with path.\n");
#endif
        return;
    }

    pr->drawPath(target, pipelineBuilder, color, viewMatrix, *pathPtr, *strokeInfoPtr, useCoverageAA);
}

bool GrDrawContext::prepareToDraw(GrPipelineBuilder* pipelineBuilder,
                                  GrRenderTarget* rt,
                                  const GrClip& clip,
                                  const GrPaint* paint) {
    RETURN_FALSE_IF_ABANDONED

    ASSERT_OWNED_RESOURCE(rt);
    SkASSERT(rt && paint);
    pipelineBuilder->setFromPaint(*paint, rt, clip);
    return true;
}

bool GrDrawContext::prepareToDraw(GrRenderTarget* rt) {
    RETURN_FALSE_IF_ABANDONED

    ASSERT_OWNED_RESOURCE(rt);
    SkASSERT(rt);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

BATCH_TEST_DEFINE(StrokeRectBatch) {
    StrokeRectBatch::Geometry geometry;
    geometry.fViewMatrix = GrTest::TestMatrix(random);
    geometry.fColor = GrRandomColor(random);
    geometry.fRect = GrTest::TestRect(random);
    geometry.fStrokeWidth = random->nextBool() ? 0.0f : 1.0f;

    return StrokeRectBatch::Create(geometry, random->nextBool());
}

static uint32_t seed_vertices(GrPrimitiveType type) {
    switch (type) {
        case kTriangles_GrPrimitiveType:
        case kTriangleStrip_GrPrimitiveType:
        case kTriangleFan_GrPrimitiveType:
            return 3;
        case kPoints_GrPrimitiveType:
            return 1;
        case kLines_GrPrimitiveType:
        case kLineStrip_GrPrimitiveType:
            return 2;
    }
    SkFAIL("Incomplete switch\n");
    return 0;
}

static uint32_t primitive_vertices(GrPrimitiveType type) {
    switch (type) {
        case kTriangles_GrPrimitiveType:
            return 3;
        case kLines_GrPrimitiveType:
            return 2;
        case kTriangleStrip_GrPrimitiveType:
        case kTriangleFan_GrPrimitiveType:
        case kPoints_GrPrimitiveType:
        case kLineStrip_GrPrimitiveType:
            return 1;
    }
    SkFAIL("Incomplete switch\n");
    return 0;
}

static SkPoint random_point(SkRandom* random, SkScalar min, SkScalar max) {
    SkPoint p;
    p.fX = random->nextRangeScalar(min, max);
    p.fY = random->nextRangeScalar(min, max);
    return p;
}

static void randomize_params(size_t count, size_t maxVertex, SkScalar min, SkScalar max,
                             SkRandom* random,
                             SkTArray<SkPoint>* positions,
                             SkTArray<SkPoint>* texCoords, bool hasTexCoords,
                             SkTArray<GrColor>* colors, bool hasColors,
                             SkTArray<uint16_t>* indices, bool hasIndices) {
    for (uint32_t v = 0; v < count; v++) {
        positions->push_back(random_point(random, min, max));
        if (hasTexCoords) {
            texCoords->push_back(random_point(random, min, max));
        }
        if (hasColors) {
            colors->push_back(GrRandomColor(random));
        }
        if (hasIndices) {
            SkASSERT(maxVertex <= SK_MaxU16);
            indices->push_back(random->nextULessThan((uint16_t)maxVertex));
        }
    }
}

BATCH_TEST_DEFINE(VerticesBatch) {
    GrPrimitiveType type = GrPrimitiveType(random->nextULessThan(kLast_GrPrimitiveType + 1));
    uint32_t primitiveCount = random->nextRangeU(1, 100);

    // TODO make 'sensible' indexbuffers
    SkTArray<SkPoint> positions;
    SkTArray<SkPoint> texCoords;
    SkTArray<GrColor> colors;
    SkTArray<uint16_t> indices;

    bool hasTexCoords = random->nextBool();
    bool hasIndices = random->nextBool();
    bool hasColors = random->nextBool();

    uint32_t vertexCount = seed_vertices(type) + (primitiveCount - 1) * primitive_vertices(type);

    static const SkScalar kMinVertExtent = -100.f;
    static const SkScalar kMaxVertExtent = 100.f;
    randomize_params(seed_vertices(type), vertexCount, kMinVertExtent, kMaxVertExtent,
                     random,
                     &positions,
                     &texCoords, hasTexCoords,
                     &colors, hasColors,
                     &indices, hasIndices);

    for (uint32_t i = 1; i < primitiveCount; i++) {
        randomize_params(primitive_vertices(type), vertexCount, kMinVertExtent, kMaxVertExtent,
                         random,
                         &positions,
                         &texCoords, hasTexCoords,
                         &colors, hasColors,
                         &indices, hasIndices);
    }

    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect bounds;
    SkDEBUGCODE(bool result = ) bounds.setBoundsCheck(positions.begin(), vertexCount);
    SkASSERT(result);

    viewMatrix.mapRect(&bounds);

    DrawVerticesBatch::Geometry geometry;
    geometry.fColor = GrRandomColor(random);
    return DrawVerticesBatch::Create(geometry, type, viewMatrix,
                                     positions.begin(), vertexCount,
                                     indices.begin(), hasIndices ? vertexCount : 0,
                                     colors.begin(),
                                     texCoords.begin(),
                                     bounds);
}

#endif

