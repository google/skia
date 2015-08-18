/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAStrokeRectBatch.h"

#include "GrBatchFlushState.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gMiterIndexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gBevelIndexBufferKey);

static void set_inset_fan(SkPoint* pts, size_t stride,
                          const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy,
                    r.fRight - dx, r.fBottom - dy, stride);
}

static const GrGeometryProcessor* create_stroke_rect_gp(bool tweakAlphaForCoverage,
                                                        const SkMatrix& viewMatrix,
                                                        bool usesLocalCoords,
                                                        bool coverageIgnored) {
    using namespace GrDefaultGeoProcFactory;

    Color color(Color::kAttribute_Type);
    Coverage::Type coverageType;
    // TODO remove coverage if coverage is ignored
    /*if (coverageIgnored) {
        coverageType = Coverage::kNone_Type;
    } else*/ if (tweakAlphaForCoverage) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    Coverage coverage(coverageType);
    LocalCoords localCoords(usesLocalCoords ? LocalCoords::kUsePosition_Type :
                                              LocalCoords::kUnused_Type);
    return CreateForDeviceSpace(color, coverage, localCoords, viewMatrix);
}

void GrAAStrokeRectBatch::initBatchTracker(const GrPipelineOptimizations& opt) {
    // Handle any color overrides
    if (!opt.readsColor()) {
        fGeoData[0].fColor = GrColor_ILLEGAL;
    }
    opt.getOverrideColorIfSet(&fGeoData[0].fColor);

    // setup batch properties
    fBatch.fColorIgnored = !opt.readsColor();
    fBatch.fColor = fGeoData[0].fColor;
    fBatch.fUsesLocalCoords = opt.readsLocalCoords();
    fBatch.fCoverageIgnored = !opt.readsCoverage();
    fBatch.fMiterStroke = fGeoData[0].fMiterStroke;
    fBatch.fCanTweakAlphaForCoverage = opt.canTweakAlphaForCoverage();
}

void GrAAStrokeRectBatch::onPrepareDraws(Target* target) {
    bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

    SkAutoTUnref<const GrGeometryProcessor> gp(create_stroke_rect_gp(canTweakAlphaForCoverage,
                                                                     this->viewMatrix(),
                                                                     this->usesLocalCoords(),
                                                                     this->coverageIgnored()));
    if (!gp) {
        SkDebugf("Couldn't create GrGeometryProcessor\n");
        return;
    }

    target->initDraw(gp, this->pipeline());

    size_t vertexStride = gp->getVertexStride();

    SkASSERT(canTweakAlphaForCoverage ?
             vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
             vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));
    int innerVertexNum = 4;
    int outerVertexNum = this->miterStroke() ? 4 : 8;
    int verticesPerInstance = (outerVertexNum + innerVertexNum) * 2;
    int indicesPerInstance = this->miterStroke() ? kMiterIndexCnt : kBevelIndexCnt;
    int instanceCount = fGeoData.count();

    const SkAutoTUnref<const GrIndexBuffer> indexBuffer(
        GetIndexBuffer(target->resourceProvider(), this->miterStroke()));
    InstancedHelper helper;
    void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                 indexBuffer, verticesPerInstance,  indicesPerInstance,
                                 instanceCount);
    if (!vertices || !indexBuffer) {
         SkDebugf("Could not allocate vertices\n");
         return;
     }

    for (int i = 0; i < instanceCount; i++) {
        const Geometry& args = fGeoData[i];
        this->generateAAStrokeRectGeometry(vertices,
                                           i * verticesPerInstance * vertexStride,
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
    helper.recordDraw(target);
}

const GrIndexBuffer* GrAAStrokeRectBatch::GetIndexBuffer(GrResourceProvider* resourceProvider,
                                                         bool miterStroke) {

    if (miterStroke) {
        static const uint16_t gMiterIndices[] = {
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
        GR_STATIC_ASSERT(SK_ARRAY_COUNT(gMiterIndices) == kMiterIndexCnt);
        GR_DEFINE_STATIC_UNIQUE_KEY(gMiterIndexBufferKey);
        return resourceProvider->findOrCreateInstancedIndexBuffer(gMiterIndices,
            kMiterIndexCnt, kNumMiterRectsInIndexBuffer, kMiterVertexCnt,
            gMiterIndexBufferKey);
    } else {
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
        static const uint16_t gBevelIndices[] = {
            // Draw outer AA, from outer AA line to outer edge, shift is 0.
            0 + 0, 1 + 0,  9 + 0,  9 + 0,  8 + 0, 0 + 0,
            1 + 0, 5 + 0, 13 + 0, 13 + 0,  9 + 0, 1 + 0,
            5 + 0, 6 + 0, 14 + 0, 14 + 0, 13 + 0, 5 + 0,
            6 + 0, 2 + 0, 10 + 0, 10 + 0, 14 + 0, 6 + 0,
            2 + 0, 3 + 0, 11 + 0, 11 + 0, 10 + 0, 2 + 0,
            3 + 0, 7 + 0, 15 + 0, 15 + 0, 11 + 0, 3 + 0,
            7 + 0, 4 + 0, 12 + 0, 12 + 0, 15 + 0, 7 + 0,
            4 + 0, 0 + 0,  8 + 0,  8 + 0, 12 + 0, 4 + 0,

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
        GR_STATIC_ASSERT(SK_ARRAY_COUNT(gBevelIndices) == kBevelIndexCnt);

        GR_DEFINE_STATIC_UNIQUE_KEY(gBevelIndexBufferKey);
        return resourceProvider->findOrCreateInstancedIndexBuffer(gBevelIndices,
            kBevelIndexCnt, kNumBevelRectsInIndexBuffer, kBevelVertexCnt,
            gBevelIndexBufferKey);
    }
}

bool GrAAStrokeRectBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    GrAAStrokeRectBatch* that = t->cast<GrAAStrokeRectBatch>();

    if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                that->bounds(), caps)) {
        return false;
    }

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
    this->joinBounds(that->bounds());
    return true;
}

void GrAAStrokeRectBatch::generateAAStrokeRectGeometry(void* vertices,
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

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

DRAW_BATCH_TEST_DEFINE(AAStrokeRectBatch) {
    bool miterStroke = random->nextBool();

    // Create mock stroke rect
    SkRect outside = GrTest::TestRect(random);
    SkScalar minDim = SkMinScalar(outside.width(), outside.height());
    SkScalar strokeWidth = minDim * 0.1f;
    SkRect outsideAssist = outside;
    outsideAssist.outset(strokeWidth, strokeWidth);
    SkRect inside = outside;
    inside.inset(strokeWidth, strokeWidth);

    GrAAStrokeRectBatch::Geometry geo;
    geo.fColor = GrRandomColor(random);
    geo.fDevOutside = outside;
    geo.fDevOutsideAssist = outsideAssist;
    geo.fDevInside = inside;
    geo.fMiterStroke = miterStroke;

    return GrAAStrokeRectBatch::Create(geo, GrTest::TestMatrix(random));
}

#endif
