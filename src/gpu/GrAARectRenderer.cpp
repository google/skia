/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAARectRenderer.h"
#include "GrBatch.h"
#include "GrBatchTarget.h"
#include "GrBufferAllocPool.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrGeometryProcessor.h"
#include "GrGpu.h"
#include "GrInvariantOutput.h"
#include "SkColorPriv.h"
#include "gl/GrGLProcessor.h"
#include "gl/GrGLGeometryProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

///////////////////////////////////////////////////////////////////////////////

static void set_inset_fan(SkPoint* pts, size_t stride,
                          const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

static const uint16_t gFillAARectIdx[] = {
    0, 1, 5, 5, 4, 0,
    1, 2, 6, 6, 5, 1,
    2, 3, 7, 7, 6, 2,
    3, 0, 4, 4, 7, 3,
    4, 5, 6, 6, 7, 4,
};

static const int kIndicesPerAAFillRect = SK_ARRAY_COUNT(gFillAARectIdx);
static const int kVertsPerAAFillRect = 8;
static const int kNumAAFillRectsInIndexBuffer = 256;

static const GrGeometryProcessor* create_fill_rect_gp(bool tweakAlphaForCoverage,
                                                      const SkMatrix& localMatrix) {
    uint32_t flags = GrDefaultGeoProcFactory::kColor_GPType;
    const GrGeometryProcessor* gp;
    if (tweakAlphaForCoverage) {
        gp = GrDefaultGeoProcFactory::Create(flags, GrColor_WHITE, SkMatrix::I(), localMatrix,
                                             false, 0xff);
    } else {
        flags |= GrDefaultGeoProcFactory::kCoverage_GPType;
        gp = GrDefaultGeoProcFactory::Create(flags, GrColor_WHITE, SkMatrix::I(), localMatrix,
                                             false, 0xff);
    }
    return gp;
}

class AAFillRectBatch : public GrBatch {
public:
    struct Geometry {
        GrColor fColor;
        SkMatrix fViewMatrix;
        SkRect fRect;
        SkRect fDevRect;
    };

    static GrBatch* Create(const Geometry& geometry, const GrIndexBuffer* indexBuffer) {
        return SkNEW_ARGS(AAFillRectBatch, (geometry, indexBuffer));
    }

    const char* name() const SK_OVERRIDE { return "AAFillRectBatch"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
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
        fBatch.fCanTweakAlphaForCoverage = init.fCanTweakAlphaForCoverage;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

        SkMatrix localMatrix;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
            SkDebugf("Cannot invert\n");
            return;
        }

        SkAutoTUnref<const GrGeometryProcessor> gp(create_fill_rect_gp(canTweakAlphaForCoverage,
                                                                       localMatrix));

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

        SkASSERT(canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

        int instanceCount = fGeoData.count();
        int vertexCount = kVertsPerAAFillRect * instanceCount;

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];
            this->generateAAFillRectGeometry(vertices,
                                             i * kVertsPerAAFillRect * vertexStride,
                                             vertexStride,
                                             args.fColor,
                                             args.fViewMatrix,
                                             args.fRect,
                                             args.fDevRect,
                                             canTweakAlphaForCoverage);
        }

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(kVertsPerAAFillRect);
        drawInfo.setIndicesPerInstance(kIndicesPerAAFillRect);
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(fIndexBuffer);

        int maxInstancesPerDraw = kNumAAFillRectsInIndexBuffer;

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
    AAFillRectBatch(const Geometry& geometry, const GrIndexBuffer* indexBuffer)
        : fIndexBuffer(indexBuffer) {
        this->initClassID<AAFillRectBatch>();
        fGeoData.push_back(geometry);
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fGeoData[0].fViewMatrix; }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        AAFillRectBatch* that = t->cast<AAFillRectBatch>();

        SkASSERT(this->usesLocalCoords() == that->usesLocalCoords());
        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        if (this->color() != that->color()) {
            fBatch.fColor = GrColor_ILLEGAL;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (this->canTweakAlphaForCoverage() != that->canTweakAlphaForCoverage()) {
            fBatch.fCanTweakAlphaForCoverage = false;
        }

        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    void generateAAFillRectGeometry(void* vertices,
                                    size_t offset,
                                    size_t vertexStride,
                                    GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect& devRect,
                                    bool tweakAlphaForCoverage) const {
        intptr_t verts = reinterpret_cast<intptr_t>(vertices) + offset;

        SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
        SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);

        SkScalar inset = SkMinScalar(devRect.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, devRect.height());

        if (viewMatrix.rectStaysRect()) {
            set_inset_fan(fan0Pos, vertexStride, devRect, -SK_ScalarHalf, -SK_ScalarHalf);
            set_inset_fan(fan1Pos, vertexStride, devRect, inset,  inset);
        } else {
            // compute transformed (1, 0) and (0, 1) vectors
            SkVector vec[2] = {
              { viewMatrix[SkMatrix::kMScaleX], viewMatrix[SkMatrix::kMSkewY] },
              { viewMatrix[SkMatrix::kMSkewX],  viewMatrix[SkMatrix::kMScaleY] }
            };

            vec[0].normalize();
            vec[0].scale(SK_ScalarHalf);
            vec[1].normalize();
            vec[1].scale(SK_ScalarHalf);

            // create the rotated rect
            fan0Pos->setRectFan(rect.fLeft, rect.fTop,
                                rect.fRight, rect.fBottom, vertexStride);
            viewMatrix.mapPointsWithStride(fan0Pos, vertexStride, 4);

            // Now create the inset points and then outset the original
            // rotated points

            // TL
            *((SkPoint*)((intptr_t)fan1Pos + 0 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) + vec[0] + vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 0 * vertexStride)) -= vec[0] + vec[1];
            // BL
            *((SkPoint*)((intptr_t)fan1Pos + 1 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) + vec[0] - vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 1 * vertexStride)) -= vec[0] - vec[1];
            // BR
            *((SkPoint*)((intptr_t)fan1Pos + 2 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) - vec[0] - vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 2 * vertexStride)) += vec[0] + vec[1];
            // TR
            *((SkPoint*)((intptr_t)fan1Pos + 3 * vertexStride)) =
                *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) - vec[0] + vec[1];
            *((SkPoint*)((intptr_t)fan0Pos + 3 * vertexStride)) += vec[0] - vec[1];
        }

        // Make verts point to vertex color and then set all the color and coverage vertex attrs
        // values.
        verts += sizeof(SkPoint);
        for (int i = 0; i < 4; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = 0;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
                *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = 0;
            }
        }

        int scale;
        if (inset < SK_ScalarHalf) {
            scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
            SkASSERT(scale >= 0 && scale <= 255);
        } else {
            scale = 0xff;
        }

        verts += 4 * vertexStride;

        float innerCoverage = GrNormalizeByteToFloat(scale);
        GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);

        for (int i = 0; i < 4; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
                *reinterpret_cast<float*>(verts + i * vertexStride +
                                          sizeof(GrColor)) = innerCoverage;
            }
        }
    }

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    const GrIndexBuffer* fIndexBuffer;
    SkSTArray<1, Geometry, true> fGeoData;
};

namespace {
// Should the coverage be multiplied into the color attrib or use a separate attrib.
enum CoverageAttribType {
    kUseColor_CoverageAttribType,
    kUseCoverage_CoverageAttribType,
};
}

void GrAARectRenderer::reset() {
    SkSafeSetNull(fAAFillRectIndexBuffer);
    SkSafeSetNull(fAAMiterStrokeRectIndexBuffer);
    SkSafeSetNull(fAABevelStrokeRectIndexBuffer);
}

static const uint16_t gMiterStrokeAARectIdx[] = {
    0 + 0, 1 + 0, 5 + 0, 5 + 0, 4 + 0, 0 + 0,
    1 + 0, 2 + 0, 6 + 0, 6 + 0, 5 + 0, 1 + 0,
    2 + 0, 3 + 0, 7 + 0, 7 + 0, 6 + 0, 2 + 0,
    3 + 0, 0 + 0, 4 + 0, 4 + 0, 7 + 0, 3 + 0,

    0 + 4, 1 + 4, 5 + 4, 5 + 4, 4 + 4, 0 + 4,
    1 + 4, 2 + 4, 6 + 4, 6 + 4, 5 + 4, 1 + 4,
    2 + 4, 3 + 4, 7 + 4, 7 + 4, 6 + 4, 2 + 4,
    3 + 4, 0 + 4, 4 + 4, 4 + 4, 7 + 4, 3 + 4,

    0 + 8, 1 + 8, 5 + 8, 5 + 8, 4 + 8, 0 + 8,
    1 + 8, 2 + 8, 6 + 8, 6 + 8, 5 + 8, 1 + 8,
    2 + 8, 3 + 8, 7 + 8, 7 + 8, 6 + 8, 2 + 8,
    3 + 8, 0 + 8, 4 + 8, 4 + 8, 7 + 8, 3 + 8,
};

static const int kIndicesPerMiterStrokeRect = SK_ARRAY_COUNT(gMiterStrokeAARectIdx);
static const int kVertsPerMiterStrokeRect = 16;
static const int kNumMiterStrokeRectsInIndexBuffer = 256;

/**
 * As in miter-stroke, index = a + b, and a is the current index, b is the shift
 * from the first index. The index layout:
 * outer AA line: 0~3, 4~7
 * outer edge:    8~11, 12~15
 * inner edge:    16~19
 * inner AA line: 20~23
 * Following comes a bevel-stroke rect and its indices:
 *
 *           4                                 7
 *            *********************************
 *          *   ______________________________  *
 *         *  / 12                          15 \  *
 *        *  /                                  \  *
 *     0 *  |8     16_____________________19  11 |  * 3
 *       *  |       |                    |       |  *
 *       *  |       |  ****************  |       |  *
 *       *  |       |  * 20        23 *  |       |  *
 *       *  |       |  *              *  |       |  *
 *       *  |       |  * 21        22 *  |       |  *
 *       *  |       |  ****************  |       |  *
 *       *  |       |____________________|       |  *
 *     1 *  |9    17                      18   10|  * 2
 *        *  \                                  /  *
 *         *  \13 __________________________14/  *
 *          *                                   *
 *           **********************************
 *          5                                  6
 */
static const uint16_t gBevelStrokeAARectIdx[] = {
    // Draw outer AA, from outer AA line to outer edge, shift is 0.
    0 + 0, 1 + 0, 9 + 0, 9 + 0, 8 + 0, 0 + 0,
    1 + 0, 5 + 0, 13 + 0, 13 + 0, 9 + 0, 1 + 0,
    5 + 0, 6 + 0, 14 + 0, 14 + 0, 13 + 0, 5 + 0,
    6 + 0, 2 + 0, 10 + 0, 10 + 0, 14 + 0, 6 + 0,
    2 + 0, 3 + 0, 11 + 0, 11 + 0, 10 + 0, 2 + 0,
    3 + 0, 7 + 0, 15 + 0, 15 + 0, 11 + 0, 3 + 0,
    7 + 0, 4 + 0, 12 + 0, 12 + 0, 15 + 0, 7 + 0,
    4 + 0, 0 + 0, 8 + 0, 8 + 0, 12 + 0, 4 + 0,

    // Draw the stroke, from outer edge to inner edge, shift is 8.
    0 + 8, 1 + 8, 9 + 8, 9 + 8, 8 + 8, 0 + 8,
    1 + 8, 5 + 8, 9 + 8,
    5 + 8, 6 + 8, 10 + 8, 10 + 8, 9 + 8, 5 + 8,
    6 + 8, 2 + 8, 10 + 8,
    2 + 8, 3 + 8, 11 + 8, 11 + 8, 10 + 8, 2 + 8,
    3 + 8, 7 + 8, 11 + 8,
    7 + 8, 4 + 8, 8 + 8, 8 + 8, 11 + 8, 7 + 8,
    4 + 8, 0 + 8, 8 + 8,

    // Draw the inner AA, from inner edge to inner AA line, shift is 16.
    0 + 16, 1 + 16, 5 + 16, 5 + 16, 4 + 16, 0 + 16,
    1 + 16, 2 + 16, 6 + 16, 6 + 16, 5 + 16, 1 + 16,
    2 + 16, 3 + 16, 7 + 16, 7 + 16, 6 + 16, 2 + 16,
    3 + 16, 0 + 16, 4 + 16, 4 + 16, 7 + 16, 3 + 16,
};

static const int kIndicesPerBevelStrokeRect = SK_ARRAY_COUNT(gBevelStrokeAARectIdx);
static const int kVertsPerBevelStrokeRect = 24;
static const int kNumBevelStrokeRectsInIndexBuffer = 256;

static int aa_stroke_rect_index_count(bool miterStroke) {
    return miterStroke ? SK_ARRAY_COUNT(gMiterStrokeAARectIdx) :
                         SK_ARRAY_COUNT(gBevelStrokeAARectIdx);
}

GrIndexBuffer* GrAARectRenderer::aaStrokeRectIndexBuffer(bool miterStroke) {
    if (miterStroke) {
        if (NULL == fAAMiterStrokeRectIndexBuffer) {
            fAAMiterStrokeRectIndexBuffer =
                    fGpu->createInstancedIndexBuffer(gMiterStrokeAARectIdx,
                                                     kIndicesPerMiterStrokeRect,
                                                     kNumMiterStrokeRectsInIndexBuffer,
                                                     kVertsPerMiterStrokeRect);
        }
        return fAAMiterStrokeRectIndexBuffer;
    } else {
        if (NULL == fAABevelStrokeRectIndexBuffer) {
            fAABevelStrokeRectIndexBuffer =
                    fGpu->createInstancedIndexBuffer(gBevelStrokeAARectIdx,
                                                     kIndicesPerBevelStrokeRect,
                                                     kNumBevelStrokeRectsInIndexBuffer,
                                                     kVertsPerBevelStrokeRect);
        }
        return fAABevelStrokeRectIndexBuffer;
    }
}

void GrAARectRenderer::geometryFillAARect(GrDrawTarget* target,
                                          GrPipelineBuilder* pipelineBuilder,
                                          GrColor color,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& rect,
                                          const SkRect& devRect) {
    if (!fAAFillRectIndexBuffer) {
        fAAFillRectIndexBuffer = fGpu->createInstancedIndexBuffer(gFillAARectIdx,
                                                                  kIndicesPerAAFillRect,
                                                                  kNumAAFillRectsInIndexBuffer,
                                                                  kVertsPerAAFillRect);
    }

    if (!fAAFillRectIndexBuffer) {
        SkDebugf("Unable to create index buffer\n");
        return;
    }

    AAFillRectBatch::Geometry geometry;
    geometry.fRect = rect;
    geometry.fViewMatrix = viewMatrix;
    geometry.fDevRect = devRect;
    geometry.fColor = color;

    SkAutoTUnref<GrBatch> batch(AAFillRectBatch::Create(geometry, fAAFillRectIndexBuffer));
    target->drawBatch(pipelineBuilder, batch, &devRect);
}

void GrAARectRenderer::strokeAARect(GrDrawTarget* target,
                                    GrPipelineBuilder* pipelineBuilder,
                                    GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& rect,
                                    const SkRect& devRect,
                                    const SkStrokeRec& stroke) {
    SkVector devStrokeSize;
    SkScalar width = stroke.getWidth();
    if (width > 0) {
        devStrokeSize.set(width, width);
        viewMatrix.mapVectors(&devStrokeSize, 1);
        devStrokeSize.setAbs(devStrokeSize);
    } else {
        devStrokeSize.set(SK_Scalar1, SK_Scalar1);
    }

    const SkScalar dx = devStrokeSize.fX;
    const SkScalar dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarMul(dx, SK_ScalarHalf);
    const SkScalar ry = SkScalarMul(dy, SK_ScalarHalf);

    // Temporarily #if'ed out. We don't want to pass in the devRect but
    // right now it is computed in GrContext::apply_aa_to_rect and we don't
    // want to throw away the work
#if 0
    SkRect devRect;
    combinedMatrix.mapRect(&devRect, rect);
#endif

    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = SkTMin(w, h);
    }

    SkRect devOutside(devRect);
    devOutside.outset(rx, ry);

    bool miterStroke = true;
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    // small miter limit means right angles show bevel...
    if ((width > 0) && (stroke.getJoin() != SkPaint::kMiter_Join ||
                        stroke.getMiter() < SK_ScalarSqrt2)) {
        miterStroke = false;
    }

    if (spare <= 0 && miterStroke) {
        this->fillAARect(target, pipelineBuilder, color, viewMatrix, devOutside, devOutside);
        return;
    }

    SkRect devInside(devRect);
    devInside.inset(rx, ry);

    SkRect devOutsideAssist(devRect);

    // For bevel-stroke, use 2 SkRect instances(devOutside and devOutsideAssist)
    // to draw the outer of the rect. Because there are 8 vertices on the outer
    // edge, while vertex number of inner edge is 4, the same as miter-stroke.
    if (!miterStroke) {
        devOutside.inset(0, ry);
        devOutsideAssist.outset(0, ry);
    }

    this->geometryStrokeAARect(target, pipelineBuilder, color, viewMatrix, devOutside,
                               devOutsideAssist, devInside, miterStroke);
}

class AAStrokeRectBatch : public GrBatch {
public:
    // TODO support AA rotated stroke rects by copying around view matrices
    struct Geometry {
        GrColor fColor;
        SkRect fDevOutside;
        SkRect fDevOutsideAssist;
        SkRect fDevInside;
        bool fMiterStroke;
    };

    static GrBatch* Create(const Geometry& geometry, const SkMatrix& viewMatrix,
                           const GrIndexBuffer* indexBuffer) {
        return SkNEW_ARGS(AAStrokeRectBatch, (geometry, viewMatrix, indexBuffer));
    }

    const char* name() const SK_OVERRIDE { return "AAStrokeRect"; }

    void getInvariantOutputColor(GrInitInvariantOutput* out) const SK_OVERRIDE {
        // When this is called on a batch, there is only one geometry bundle
        out->setKnownFourComponents(fGeoData[0].fColor);
    }

    void getInvariantOutputCoverage(GrInitInvariantOutput* out) const SK_OVERRIDE {
        out->setUnknownSingleComponent();
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
        fBatch.fMiterStroke = fGeoData[0].fMiterStroke;
        fBatch.fCanTweakAlphaForCoverage = init.fCanTweakAlphaForCoverage;
    }

    void generateGeometry(GrBatchTarget* batchTarget, const GrPipeline* pipeline) SK_OVERRIDE {
        bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

        // Local matrix is ignored if we don't have local coords.  If we have localcoords we only
        // batch with identical view matrices
        SkMatrix localMatrix;
        if (this->usesLocalCoords() && !this->viewMatrix().invert(&localMatrix)) {
            SkDebugf("Cannot invert\n");
            return;
        }

        SkAutoTUnref<const GrGeometryProcessor> gp(create_fill_rect_gp(canTweakAlphaForCoverage,
                                                                       localMatrix));

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

        SkASSERT(canTweakAlphaForCoverage ?
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
                 vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));

        int innerVertexNum = 4;
        int outerVertexNum = this->miterStroke() ? 4 : 8;
        int totalVertexNum = (outerVertexNum + innerVertexNum) * 2;

        int instanceCount = fGeoData.count();
        int vertexCount = totalVertexNum * instanceCount;

        const GrVertexBuffer* vertexBuffer;
        int firstVertex;

        void* vertices = batchTarget->vertexPool()->makeSpace(vertexStride,
                                                              vertexCount,
                                                              &vertexBuffer,
                                                              &firstVertex);

        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];
            this->generateAAStrokeRectGeometry(vertices,
                                               i * totalVertexNum * vertexStride,
                                               vertexStride,
                                               outerVertexNum,
                                               innerVertexNum,
                                               args.fColor,
                                               args.fDevOutside,
                                               args.fDevOutsideAssist,
                                               args.fDevInside,
                                               args.fMiterStroke,
                                               canTweakAlphaForCoverage);
        }

        GrDrawTarget::DrawInfo drawInfo;
        drawInfo.setPrimitiveType(kTriangles_GrPrimitiveType);
        drawInfo.setStartVertex(0);
        drawInfo.setStartIndex(0);
        drawInfo.setVerticesPerInstance(totalVertexNum);
        drawInfo.setIndicesPerInstance(aa_stroke_rect_index_count(this->miterStroke()));
        drawInfo.adjustStartVertex(firstVertex);
        drawInfo.setVertexBuffer(vertexBuffer);
        drawInfo.setIndexBuffer(fIndexBuffer);

        int maxInstancesPerDraw = kNumBevelStrokeRectsInIndexBuffer;

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
    AAStrokeRectBatch(const Geometry& geometry, const SkMatrix& viewMatrix,
                      const GrIndexBuffer* indexBuffer)
        : fIndexBuffer(indexBuffer) {
        this->initClassID<AAStrokeRectBatch>();
        fBatch.fViewMatrix = viewMatrix;
        fGeoData.push_back(geometry);
    }

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    const SkMatrix& viewMatrix() const { return fBatch.fViewMatrix; }
    bool miterStroke() const { return fBatch.fMiterStroke; }

    bool onCombineIfPossible(GrBatch* t) SK_OVERRIDE {
        AAStrokeRectBatch* that = t->cast<AAStrokeRectBatch>();

        // TODO batch across miterstroke changes
        if (this->miterStroke() != that->miterStroke()) {
            return false;
        }

        // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
        // local coords then we won't be able to batch.  We could actually upload the viewmatrix
        // using vertex attributes in these cases, but haven't investigated that
        if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
            return false;
        }

        // In the event of two batches, one who can tweak, one who cannot, we just fall back to
        // not tweaking
        if (this->canTweakAlphaForCoverage() != that->canTweakAlphaForCoverage()) {
            fBatch.fCanTweakAlphaForCoverage = false;
        }

        if (this->color() != that->color()) {
            fBatch.fColor = GrColor_ILLEGAL;
        }
        fGeoData.push_back_n(that->geoData()->count(), that->geoData()->begin());
        return true;
    }

    void generateAAStrokeRectGeometry(void* vertices,
                                      size_t offset,
                                      size_t vertexStride,
                                      int outerVertexNum,
                                      int innerVertexNum,
                                      GrColor color,
                                      const SkRect& devOutside,
                                      const SkRect& devOutsideAssist,
                                      const SkRect& devInside,
                                      bool miterStroke,
                                      bool tweakAlphaForCoverage) const {
        intptr_t verts = reinterpret_cast<intptr_t>(vertices) + offset;

        // We create vertices for four nested rectangles. There are two ramps from 0 to full
        // coverage, one on the exterior of the stroke and the other on the interior.
        // The following pointers refer to the four rects, from outermost to innermost.
        SkPoint* fan0Pos = reinterpret_cast<SkPoint*>(verts);
        SkPoint* fan1Pos = reinterpret_cast<SkPoint*>(verts + outerVertexNum * vertexStride);
        SkPoint* fan2Pos = reinterpret_cast<SkPoint*>(verts + 2 * outerVertexNum * vertexStride);
        SkPoint* fan3Pos = reinterpret_cast<SkPoint*>(verts +
                                                      (2 * outerVertexNum + innerVertexNum) *
                                                      vertexStride);

    #ifndef SK_IGNORE_THIN_STROKED_RECT_FIX
        // TODO: this only really works if the X & Y margins are the same all around
        // the rect (or if they are all >= 1.0).
        SkScalar inset = SkMinScalar(SK_Scalar1, devOutside.fRight - devInside.fRight);
        inset = SkMinScalar(inset, devInside.fLeft - devOutside.fLeft);
        inset = SkMinScalar(inset, devInside.fTop - devOutside.fTop);
        if (miterStroke) {
            inset = SK_ScalarHalf * SkMinScalar(inset, devOutside.fBottom - devInside.fBottom);
        } else {
            inset = SK_ScalarHalf * SkMinScalar(inset, devOutsideAssist.fBottom -
                                                       devInside.fBottom);
        }
        SkASSERT(inset >= 0);
    #else
        SkScalar inset = SK_ScalarHalf;
    #endif

        if (miterStroke) {
            // outermost
            set_inset_fan(fan0Pos, vertexStride, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
            // inner two
            set_inset_fan(fan1Pos, vertexStride, devOutside,  inset,  inset);
            set_inset_fan(fan2Pos, vertexStride, devInside,  -inset, -inset);
            // innermost
            set_inset_fan(fan3Pos, vertexStride, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
        } else {
            SkPoint* fan0AssistPos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);
            SkPoint* fan1AssistPos = reinterpret_cast<SkPoint*>(verts +
                                                                (outerVertexNum + 4) *
                                                                vertexStride);
            // outermost
            set_inset_fan(fan0Pos, vertexStride, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
            set_inset_fan(fan0AssistPos, vertexStride, devOutsideAssist, -SK_ScalarHalf,
                          -SK_ScalarHalf);
            // outer one of the inner two
            set_inset_fan(fan1Pos, vertexStride, devOutside,  inset,  inset);
            set_inset_fan(fan1AssistPos, vertexStride, devOutsideAssist,  inset,  inset);
            // inner one of the inner two
            set_inset_fan(fan2Pos, vertexStride, devInside,  -inset, -inset);
            // innermost
            set_inset_fan(fan3Pos, vertexStride, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
        }

        // Make verts point to vertex color and then set all the color and coverage vertex attrs
        // values. The outermost rect has 0 coverage
        verts += sizeof(SkPoint);
        for (int i = 0; i < outerVertexNum; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = 0;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
                *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = 0;
            }
        }

        // scale is the coverage for the the inner two rects.
        int scale;
        if (inset < SK_ScalarHalf) {
            scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
            SkASSERT(scale >= 0 && scale <= 255);
        } else {
            scale = 0xff;
        }

        float innerCoverage = GrNormalizeByteToFloat(scale);
        GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);

        verts += outerVertexNum * vertexStride;
        for (int i = 0; i < outerVertexNum + innerVertexNum; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
                *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) =
                        innerCoverage;
            }
        }

        // The innermost rect has 0 coverage
        verts += (outerVertexNum + innerVertexNum) * vertexStride;
        for (int i = 0; i < innerVertexNum; ++i) {
            if (tweakAlphaForCoverage) {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = 0;
            } else {
                *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
                *reinterpret_cast<GrColor*>(verts + i * vertexStride + sizeof(GrColor)) = 0;
            }
        }
    }

    struct BatchTracker {
        SkMatrix fViewMatrix;
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fMiterStroke;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    const GrIndexBuffer* fIndexBuffer;
    SkSTArray<1, Geometry, true> fGeoData;
};


void GrAARectRenderer::geometryStrokeAARect(GrDrawTarget* target,
                                            GrPipelineBuilder* pipelineBuilder,
                                            GrColor color,
                                            const SkMatrix& viewMatrix,
                                            const SkRect& devOutside,
                                            const SkRect& devOutsideAssist,
                                            const SkRect& devInside,
                                            bool miterStroke) {
    GrIndexBuffer* indexBuffer = this->aaStrokeRectIndexBuffer(miterStroke);
    if (!indexBuffer) {
        SkDebugf("Failed to create index buffer!\n");
        return;
    }

    AAStrokeRectBatch::Geometry geometry;
    geometry.fColor = color;
    geometry.fDevOutside = devOutside;
    geometry.fDevOutsideAssist = devOutsideAssist;
    geometry.fDevInside = devInside;
    geometry.fMiterStroke = miterStroke;

    SkAutoTUnref<GrBatch> batch(AAStrokeRectBatch::Create(geometry, viewMatrix, indexBuffer));
    target->drawBatch(pipelineBuilder, batch);
}

void GrAARectRenderer::fillAANestedRects(GrDrawTarget* target,
                                         GrPipelineBuilder* pipelineBuilder,
                                         GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[1].isEmpty());

    SkRect devOutside, devOutsideAssist, devInside;
    viewMatrix.mapRect(&devOutside, rects[0]);
    // can't call mapRect for devInside since it calls sort
    viewMatrix.mapPoints((SkPoint*)&devInside, (const SkPoint*)&rects[1], 2);

    if (devInside.isEmpty()) {
        this->fillAARect(target, pipelineBuilder, color, viewMatrix, devOutside, devOutside);
        return;
    }

    this->geometryStrokeAARect(target, pipelineBuilder, color, viewMatrix, devOutside,
                               devOutsideAssist, devInside, true);
}
