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

static sk_sp<GrGeometryProcessor> create_stroke_rect_gp(bool tweakAlphaForCoverage,
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
    return MakeForDeviceSpace(color, coverage, localCoords, viewMatrix);
}

class AAStrokeRectBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    // TODO support AA rotated stroke rects by copying around view matrices
    struct Geometry {
        SkRect fDevOutside;
        SkRect fDevOutsideAssist;
        SkRect fDevInside;
        GrColor fColor;
        bool fDegenerate;
    };

    static AAStrokeRectBatch* Create(const SkMatrix& viewMatrix, bool miterStroke) {
        return new AAStrokeRectBatch(viewMatrix, miterStroke);
    }

    const char* name() const override { return "AAStrokeRect"; }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

    SkSTArray<1, Geometry, true>* geoData() { return &fGeoData; }

    bool canAppend(const SkMatrix& viewMatrix, bool miterStroke) {
        return fViewMatrix.cheapEqualTo(viewMatrix) && fMiterStroke == miterStroke;
    }

    void append(GrColor color, const SkRect& devOutside, const SkRect& devOutsideAssist,
                const SkRect& devInside, bool degenerate) {
        Geometry& geometry = fGeoData.push_back();
        geometry.fColor = color;
        geometry.fDevOutside = devOutside;
        geometry.fDevOutsideAssist = devOutsideAssist;
        geometry.fDevInside = devInside;
        geometry.fDegenerate = degenerate;
    }

    void appendAndUpdateBounds(GrColor color, const SkRect& devOutside,
                               const SkRect& devOutsideAssist, const SkRect& devInside,
                               bool degenerate) {
        this->append(color, devOutside, devOutsideAssist, devInside, degenerate);

        SkRect bounds;
        this->updateBounds(&bounds, fGeoData.back());
        this->joinBounds(bounds);
    }

    void init() { this->updateBounds(&fBounds, fGeoData[0]); }

private:
    void updateBounds(SkRect* bounds, const Geometry& geo) {
        // If we have miterstroke then we inset devOutside and outset devOutsideAssist, so we need
        // the join for proper bounds
        *bounds = geo.fDevOutside;
        bounds->join(geo.fDevOutsideAssist);
    }

    void onPrepareDraws(Target*) const override;
    void initBatchTracker(const GrXPOverridesForBatch&) override;

    AAStrokeRectBatch(const SkMatrix& viewMatrix,bool miterStroke)
        : INHERITED(ClassID()) {
        fViewMatrix = viewMatrix;
        fMiterStroke = miterStroke;
    }

    static const int kMiterIndexCnt = 3 * 24;
    static const int kMiterVertexCnt = 16;
    static const int kNumMiterRectsInIndexBuffer = 256;

    static const int kBevelIndexCnt = 48 + 36 + 24;
    static const int kBevelVertexCnt = 24;
    static const int kNumBevelRectsInIndexBuffer = 256;

    static const GrBuffer* GetIndexBuffer(GrResourceProvider* resourceProvider, bool miterStroke);

    GrColor color() const { return fBatch.fColor; }
    bool usesLocalCoords() const { return fBatch.fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fBatch.fCanTweakAlphaForCoverage; }
    bool colorIgnored() const { return fBatch.fColorIgnored; }
    bool coverageIgnored() const { return fBatch.fCoverageIgnored; }
    const Geometry& geometry() const { return fGeoData[0]; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool miterStroke() const { return fMiterStroke; }

    bool onCombineIfPossible(GrBatch* t, const GrCaps&) override;

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
                                      bool degenerate,
                                      bool tweakAlphaForCoverage) const;

    struct BatchTracker {
        GrColor fColor;
        bool fUsesLocalCoords;
        bool fColorIgnored;
        bool fCoverageIgnored;
        bool fCanTweakAlphaForCoverage;
    };

    BatchTracker fBatch;
    SkSTArray<1, Geometry, true> fGeoData;
    SkMatrix fViewMatrix;
    bool fMiterStroke;

    typedef GrVertexBatch INHERITED;
};

void AAStrokeRectBatch::initBatchTracker(const GrXPOverridesForBatch& overrides) {
    // Handle any color overrides
    if (!overrides.readsColor()) {
        fGeoData[0].fColor = GrColor_ILLEGAL;
    }
    overrides.getOverrideColorIfSet(&fGeoData[0].fColor);

    // setup batch properties
    fBatch.fColorIgnored = !overrides.readsColor();
    fBatch.fColor = fGeoData[0].fColor;
    fBatch.fUsesLocalCoords = overrides.readsLocalCoords();
    fBatch.fCoverageIgnored = !overrides.readsCoverage();
    fBatch.fCanTweakAlphaForCoverage = overrides.canTweakAlphaForCoverage();
}

void AAStrokeRectBatch::onPrepareDraws(Target* target) const {
    bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

    sk_sp<GrGeometryProcessor> gp(create_stroke_rect_gp(canTweakAlphaForCoverage,
                                                        this->viewMatrix(),
                                                        this->usesLocalCoords(),
                                                        this->coverageIgnored()));
    if (!gp) {
        SkDebugf("Couldn't create GrGeometryProcessor\n");
        return;
    }

    size_t vertexStride = gp->getVertexStride();

    SkASSERT(canTweakAlphaForCoverage ?
             vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr) :
             vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));
    int innerVertexNum = 4;
    int outerVertexNum = this->miterStroke() ? 4 : 8;
    int verticesPerInstance = (outerVertexNum + innerVertexNum) * 2;
    int indicesPerInstance = this->miterStroke() ? kMiterIndexCnt : kBevelIndexCnt;
    int instanceCount = fGeoData.count();

    const SkAutoTUnref<const GrBuffer> indexBuffer(
        GetIndexBuffer(target->resourceProvider(), this->miterStroke()));
    InstancedHelper helper;
    void* vertices = helper.init(target, kTriangles_GrPrimitiveType, vertexStride,
                                 indexBuffer, verticesPerInstance, indicesPerInstance,
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
                                           fMiterStroke,
                                           args.fDegenerate,
                                           canTweakAlphaForCoverage);
    }
    helper.recordDraw(target, gp.get());
}

const GrBuffer* AAStrokeRectBatch::GetIndexBuffer(GrResourceProvider* resourceProvider,
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

bool AAStrokeRectBatch::onCombineIfPossible(GrBatch* t, const GrCaps& caps) {
    AAStrokeRectBatch* that = t->cast<AAStrokeRectBatch>();

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

static void setup_scale(int* scale, SkScalar inset) {
    if (inset < SK_ScalarHalf) {
        *scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(*scale >= 0 && *scale <= 255);
    } else {
        *scale = 0xff;
    }
}

void AAStrokeRectBatch::generateAAStrokeRectGeometry(void* vertices,
                                                     size_t offset,
                                                     size_t vertexStride,
                                                     int outerVertexNum,
                                                     int innerVertexNum,
                                                     GrColor color,
                                                     const SkRect& devOutside,
                                                     const SkRect& devOutsideAssist,
                                                     const SkRect& devInside,
                                                     bool miterStroke,
                                                     bool degenerate,
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
    SkScalar inset;
    if (!degenerate) {
        inset = SkMinScalar(SK_Scalar1, devOutside.fRight - devInside.fRight);
        inset = SkMinScalar(inset, devInside.fLeft - devOutside.fLeft);
        inset = SkMinScalar(inset, devInside.fTop - devOutside.fTop);
        if (miterStroke) {
            inset = SK_ScalarHalf * SkMinScalar(inset, devOutside.fBottom - devInside.fBottom);
        } else {
            inset = SK_ScalarHalf * SkMinScalar(inset, devOutsideAssist.fBottom -
                                                       devInside.fBottom);
        }
        SkASSERT(inset >= 0);
    } else {
        // TODO use real devRect here
        inset = SkMinScalar(devOutside.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, SkTMax(devOutside.height(),
                                                          devOutsideAssist.height()));
    }
#else
    SkScalar inset;
    if (!degenerate) {
        inset = SK_ScalarHalf;
    } else {
        // TODO use real devRect here
        inset = SkMinScalar(devOutside.width(), SK_Scalar1);
        inset = SK_ScalarHalf * SkMinScalar(inset, SkTMax(devOutside.height(),
                                                          devOutsideAssist.height()));
    }
#endif

    if (miterStroke) {
        // outermost
        set_inset_fan(fan0Pos, vertexStride, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
        // inner two
        set_inset_fan(fan1Pos, vertexStride, devOutside,  inset,  inset);
        if (!degenerate) {
            set_inset_fan(fan2Pos, vertexStride, devInside,  -inset, -inset);
            // innermost
            set_inset_fan(fan3Pos, vertexStride, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
        } else {
            // When the interior rect has become degenerate we smoosh to a single point
            SkASSERT(devInside.fLeft == devInside.fRight &&
                     devInside.fTop == devInside.fBottom);
            fan2Pos->setRectFan(devInside.fLeft, devInside.fTop,
                                devInside.fRight, devInside.fBottom, vertexStride);
            fan3Pos->setRectFan(devInside.fLeft, devInside.fTop,
                                devInside.fRight, devInside.fBottom, vertexStride);
        }
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
        if (!degenerate) {
            // inner one of the inner two
            set_inset_fan(fan2Pos, vertexStride, devInside,  -inset, -inset);
            // innermost
            set_inset_fan(fan3Pos, vertexStride, devInside,   SK_ScalarHalf,  SK_ScalarHalf);
        } else {
            // When the interior rect has become degenerate we smoosh to a single point
            SkASSERT(devInside.fLeft == devInside.fRight &&
                     devInside.fTop == devInside.fBottom);
            fan2Pos->setRectFan(devInside.fLeft, devInside.fTop,
                                devInside.fRight, devInside.fBottom, vertexStride);
            fan3Pos->setRectFan(devInside.fLeft, devInside.fTop,
                                devInside.fRight, devInside.fBottom, vertexStride);
        }
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
    setup_scale(&scale, inset);

    float innerCoverage = GrNormalizeByteToFloat(scale);
    GrColor scaledColor = (0xff == scale) ? color : SkAlphaMulQ(color, scale);

    verts += outerVertexNum * vertexStride;
    for (int i = 0; i < outerVertexNum + innerVertexNum; ++i) {
        if (tweakAlphaForCoverage) {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = innerCoverage;
        }
    }

    // The innermost rect has 0 coverage, unless we are degenerate, in which case we must apply the
    // scaled coverage
    verts += (outerVertexNum + innerVertexNum) * vertexStride;
    if (!degenerate) {
        innerCoverage = 0;
        scaledColor = 0;
    }

    for (int i = 0; i < innerVertexNum; ++i) {
        if (tweakAlphaForCoverage) {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = scaledColor;
        } else {
            *reinterpret_cast<GrColor*>(verts + i * vertexStride) = color;
            *reinterpret_cast<float*>(verts + i * vertexStride + sizeof(GrColor)) = innerCoverage;
        }
    }
}

inline static bool is_miter(const SkStrokeRec& stroke) {
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    // small miter limit means right angles show bevel...
    if ((stroke.getWidth() > 0) && (stroke.getJoin() != SkPaint::kMiter_Join ||
                                    stroke.getMiter() < SK_ScalarSqrt2)) {
        return false;
    }
    return true;
}

static void compute_rects(SkRect* devOutside, SkRect* devOutsideAssist, SkRect* devInside,
                          bool* isDegenerate, const SkMatrix& viewMatrix, const SkRect& rect,
                          SkScalar strokeWidth, bool miterStroke) {
    SkRect devRect;
    viewMatrix.mapRect(&devRect, rect);

    SkVector devStrokeSize;
    if (strokeWidth > 0) {
        devStrokeSize.set(strokeWidth, strokeWidth);
        viewMatrix.mapVectors(&devStrokeSize, 1);
        devStrokeSize.setAbs(devStrokeSize);
    } else {
        devStrokeSize.set(SK_Scalar1, SK_Scalar1);
    }

    const SkScalar dx = devStrokeSize.fX;
    const SkScalar dy = devStrokeSize.fY;
    const SkScalar rx = SkScalarMul(dx, SK_ScalarHalf);
    const SkScalar ry = SkScalarMul(dy, SK_ScalarHalf);

    *devOutside = devRect;
    *devOutsideAssist = devRect;
    *devInside = devRect;

    devOutside->outset(rx, ry);
    devInside->inset(rx, ry);

    // If we have a degenerate stroking rect(ie the stroke is larger than inner rect) then we
    // make a degenerate inside rect to avoid double hitting.  We will also jam all of the points
    // together when we render these rects.
    SkScalar spare;
    {
        SkScalar w = devRect.width() - dx;
        SkScalar h = devRect.height() - dy;
        spare = SkTMin(w, h);
    }

    *isDegenerate = spare <= 0;
    if (*isDegenerate) {
        devInside->fLeft = devInside->fRight = devRect.centerX();
        devInside->fTop = devInside->fBottom = devRect.centerY();
    }

    // For bevel-stroke, use 2 SkRect instances(devOutside and devOutsideAssist)
    // to draw the outside of the octagon. Because there are 8 vertices on the outer
    // edge, while vertex number of inner edge is 4, the same as miter-stroke.
    if (!miterStroke) {
        devOutside->inset(0, ry);
        devOutsideAssist->outset(0, ry);
    }
}

namespace GrAAStrokeRectBatch {

GrDrawBatch* CreateFillBetweenRects(GrColor color,
                                    const SkMatrix& viewMatrix,
                                    const SkRect& devOutside,
                                    const SkRect& devInside) {
    SkASSERT(!devOutside.isEmpty())
    SkASSERT(!devInside.isEmpty())
    AAStrokeRectBatch* batch = AAStrokeRectBatch::Create(viewMatrix, true);
    batch->append(color, devOutside, devOutside, devInside, false);
    batch->init();
    return batch;
}

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkStrokeRec& stroke) {
    bool isMiterStroke = is_miter(stroke);
    AAStrokeRectBatch* batch = AAStrokeRectBatch::Create(viewMatrix, isMiterStroke);

    SkRect devOutside, devOutsideAssist, devInside;
    bool isDegenerate;
    compute_rects(&devOutside, &devOutsideAssist, &devInside, &isDegenerate, viewMatrix,
                  rect, stroke.getWidth(), isMiterStroke);

    batch->append(color, devOutside, devOutsideAssist, devInside, isDegenerate);
    batch->init();
    return batch;
}

bool Append(GrBatch* origBatch,
            GrColor color,
            const SkMatrix& viewMatrix,
            const SkRect& rect,
            const SkStrokeRec& stroke) {
    AAStrokeRectBatch* batch = origBatch->cast<AAStrokeRectBatch>();

    // we can't batch across vm changes
    bool isMiterStroke = is_miter(stroke);
    if (!batch->canAppend(viewMatrix, isMiterStroke)) {
        return false;
    }

    SkRect devOutside, devOutsideAssist, devInside;
    bool isDegenerate;
    compute_rects(&devOutside, &devOutsideAssist, &devInside, &isDegenerate, viewMatrix,
                  rect, stroke.getWidth(), isMiterStroke);

    batch->appendAndUpdateBounds(color, devOutside, devOutsideAssist, devInside, isDegenerate);
    return true;
}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

#include "GrBatchTest.h"

DRAW_BATCH_TEST_DEFINE(AAStrokeRectBatch) {
    bool miterStroke = random->nextBool();

    // Create either a empty rect or a non-empty rect.
    SkRect rect = random->nextBool() ? SkRect::MakeXYWH(10, 10, 50, 40) :
                                       SkRect::MakeXYWH(6, 7, 0, 0);
    SkScalar minDim = SkMinScalar(rect.width(), rect.height());
    SkScalar strokeWidth = random->nextUScalar1() * minDim;

    GrColor color = GrRandomColor(random);

    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(strokeWidth);
    rec.setStrokeParams(SkPaint::kButt_Cap,
                        miterStroke ? SkPaint::kMiter_Join : SkPaint::kBevel_Join,
                        1.f);
    SkMatrix matrix = GrTest::TestMatrixRectStaysRect(random);
    return GrAAStrokeRectBatch::Create(color, matrix, rect, rec);
}

#endif
