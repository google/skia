/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAAStrokeRectOp.h"

#include "GrDefaultGeoProcFactory.h"
#include "GrOpFlushState.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "SkStrokeRec.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gMiterIndexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gBevelIndexBufferKey);

static void set_inset_fan(SkPoint* pts, size_t stride, const SkRect& r, SkScalar dx, SkScalar dy) {
    pts->setRectFan(r.fLeft + dx, r.fTop + dy, r.fRight - dx, r.fBottom - dy, stride);
}

// We support all hairlines, bevels, and miters, but not round joins. Also, check whether the miter
// limit makes a miter join effectively beveled.
inline static bool allowed_stroke(const SkStrokeRec& stroke, bool* isMiter) {
    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style ||
             stroke.getStyle() == SkStrokeRec::kHairline_Style);
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    if (!stroke.getWidth()) {
        *isMiter = true;
        return true;
    }
    if (stroke.getJoin() == SkPaint::kBevel_Join) {
        *isMiter = false;
        return true;
    }
    if (stroke.getJoin() == SkPaint::kMiter_Join) {
        *isMiter = stroke.getMiter() >= SK_ScalarSqrt2;
        return true;
    }
    return false;
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
    const SkScalar rx = SkScalarHalf(dx);
    const SkScalar ry = SkScalarHalf(dy);

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

static sk_sp<GrGeometryProcessor> create_stroke_rect_gp(bool tweakAlphaForCoverage,
                                                        const SkMatrix& viewMatrix,
                                                        bool usesLocalCoords) {
    using namespace GrDefaultGeoProcFactory;

    Coverage::Type coverageType;
    if (tweakAlphaForCoverage) {
        coverageType = Coverage::kSolid_Type;
    } else {
        coverageType = Coverage::kAttribute_Type;
    }
    LocalCoords::Type localCoordsType =
            usesLocalCoords ? LocalCoords::kUsePosition_Type : LocalCoords::kUnused_Type;
    return MakeForDeviceSpace(Color::kPremulGrColorAttribute_Type, coverageType, localCoordsType,
                              viewMatrix);
}

class AAStrokeRectOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    AAStrokeRectOp(GrColor color, const SkMatrix& viewMatrix, const SkRect& devOutside,
                   const SkRect& devInside)
            : INHERITED(ClassID()), fViewMatrix(viewMatrix) {
        SkASSERT(!devOutside.isEmpty());
        SkASSERT(!devInside.isEmpty());

        fRects.emplace_back(RectInfo{color, devOutside, devOutside, devInside, false});
        this->setBounds(devOutside, HasAABloat::kYes, IsZeroArea::kNo);
        fMiterStroke = true;
    }

    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, const SkMatrix& viewMatrix,
                                                    const SkRect& rect, const SkStrokeRec& stroke) {
        bool isMiter;
        if (!allowed_stroke(stroke, &isMiter)) {
            return nullptr;
        }

        AAStrokeRectOp* op = new AAStrokeRectOp();
        op->fMiterStroke = isMiter;
        RectInfo& info = op->fRects.push_back();
        compute_rects(&info.fDevOutside, &info.fDevOutsideAssist, &info.fDevInside,
                      &info.fDegenerate, viewMatrix, rect, stroke.getWidth(), isMiter);
        info.fColor = color;
        if (isMiter) {
            op->setBounds(info.fDevOutside, HasAABloat::kYes, IsZeroArea::kNo);
        } else {
            // The outer polygon of the bevel stroke is an octagon specified by the points of a
            // pair of overlapping rectangles where one is wide and the other is narrow.
            SkRect bounds = info.fDevOutside;
            bounds.joinPossiblyEmptyRect(info.fDevOutsideAssist);
            op->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kNo);
        }
        op->fViewMatrix = viewMatrix;
        return std::unique_ptr<GrLegacyMeshDrawOp>(op);
    }

    const char* name() const override { return "AAStrokeRect"; }

    SkString dumpInfo() const override {
        SkString string;
        for (const auto& info : fRects) {
            string.appendf(
                    "Color: 0x%08x, ORect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "AssistORect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "IRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], Degen: %d",
                    info.fColor, info.fDevOutside.fLeft, info.fDevOutside.fTop,
                    info.fDevOutside.fRight, info.fDevOutside.fBottom, info.fDevOutsideAssist.fLeft,
                    info.fDevOutsideAssist.fTop, info.fDevOutsideAssist.fRight,
                    info.fDevOutsideAssist.fBottom, info.fDevInside.fLeft, info.fDevInside.fTop,
                    info.fDevInside.fRight, info.fDevInside.fBottom, info.fDegenerate);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    AAStrokeRectOp() : INHERITED(ClassID()) {}

    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fRects[0].fColor);
        *coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    }
    void applyPipelineOptimizations(const PipelineOptimizations&) override;
    void onPrepareDraws(Target*) const override;

    static const int kMiterIndexCnt = 3 * 24;
    static const int kMiterVertexCnt = 16;
    static const int kNumMiterRectsInIndexBuffer = 256;

    static const int kBevelIndexCnt = 48 + 36 + 24;
    static const int kBevelVertexCnt = 24;
    static const int kNumBevelRectsInIndexBuffer = 256;

    static const GrBuffer* GetIndexBuffer(GrResourceProvider* resourceProvider, bool miterStroke);

    bool usesLocalCoords() const { return fUsesLocalCoords; }
    bool canTweakAlphaForCoverage() const { return fCanTweakAlphaForCoverage; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool miterStroke() const { return fMiterStroke; }

    bool onCombineIfPossible(GrOp* t, const GrCaps&) override;

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

    // TODO support AA rotated stroke rects by copying around view matrices
    struct RectInfo {
        GrColor fColor;
        SkRect fDevOutside;
        SkRect fDevOutsideAssist;
        SkRect fDevInside;
        bool fDegenerate;
    };

    SkSTArray<1, RectInfo, true> fRects;
    bool fUsesLocalCoords;
    bool fCanTweakAlphaForCoverage;
    SkMatrix fViewMatrix;
    bool fMiterStroke;

    typedef GrLegacyMeshDrawOp INHERITED;
};

void AAStrokeRectOp::applyPipelineOptimizations(const PipelineOptimizations& optimizations) {
    optimizations.getOverrideColorIfSet(&fRects[0].fColor);

    fUsesLocalCoords = optimizations.readsLocalCoords();
    fCanTweakAlphaForCoverage = optimizations.canTweakAlphaForCoverage();
    fCanTweakAlphaForCoverage = optimizations.canTweakAlphaForCoverage();
}

void AAStrokeRectOp::onPrepareDraws(Target* target) const {
    bool canTweakAlphaForCoverage = this->canTweakAlphaForCoverage();

    sk_sp<GrGeometryProcessor> gp(create_stroke_rect_gp(canTweakAlphaForCoverage,
                                                        this->viewMatrix(),
                                                        this->usesLocalCoords()));
    if (!gp) {
        SkDebugf("Couldn't create GrGeometryProcessor\n");
        return;
    }

    size_t vertexStride = gp->getVertexStride();

    SkASSERT(canTweakAlphaForCoverage
                     ? vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorAttr)
                     : vertexStride == sizeof(GrDefaultGeoProcFactory::PositionColorCoverageAttr));
    int innerVertexNum = 4;
    int outerVertexNum = this->miterStroke() ? 4 : 8;
    int verticesPerInstance = (outerVertexNum + innerVertexNum) * 2;
    int indicesPerInstance = this->miterStroke() ? kMiterIndexCnt : kBevelIndexCnt;
    int instanceCount = fRects.count();

    const sk_sp<const GrBuffer> indexBuffer(
            GetIndexBuffer(target->resourceProvider(), this->miterStroke()));
    InstancedHelper helper;
    void* vertices =
            helper.init(target, kTriangles_GrPrimitiveType, vertexStride, indexBuffer.get(),
                        verticesPerInstance, indicesPerInstance, instanceCount);
    if (!vertices || !indexBuffer) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    for (int i = 0; i < instanceCount; i++) {
        const RectInfo& info = fRects[i];
        this->generateAAStrokeRectGeometry(vertices,
                                           i * verticesPerInstance * vertexStride,
                                           vertexStride,
                                           outerVertexNum,
                                           innerVertexNum,
                                           info.fColor,
                                           info.fDevOutside,
                                           info.fDevOutsideAssist,
                                           info.fDevInside,
                                           fMiterStroke,
                                           info.fDegenerate,
                                           canTweakAlphaForCoverage);
    }
    helper.recordDraw(target, gp.get(), this->pipeline());
}

const GrBuffer* AAStrokeRectOp::GetIndexBuffer(GrResourceProvider* resourceProvider,
                                               bool miterStroke) {
    if (miterStroke) {
        // clang-format off
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
        // clang-format on
        GR_STATIC_ASSERT(SK_ARRAY_COUNT(gMiterIndices) == kMiterIndexCnt);
        GR_DEFINE_STATIC_UNIQUE_KEY(gMiterIndexBufferKey);
        return resourceProvider->findOrCreateInstancedIndexBuffer(
                gMiterIndices, kMiterIndexCnt, kNumMiterRectsInIndexBuffer, kMiterVertexCnt,
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
        // clang-format off
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
        // clang-format on
        GR_STATIC_ASSERT(SK_ARRAY_COUNT(gBevelIndices) == kBevelIndexCnt);

        GR_DEFINE_STATIC_UNIQUE_KEY(gBevelIndexBufferKey);
        return resourceProvider->findOrCreateInstancedIndexBuffer(
                gBevelIndices, kBevelIndexCnt, kNumBevelRectsInIndexBuffer, kBevelVertexCnt,
                gBevelIndexBufferKey);
    }
}

bool AAStrokeRectOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    AAStrokeRectOp* that = t->cast<AAStrokeRectOp>();

    if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                that->bounds(), caps)) {
        return false;
    }

    // TODO combine across miterstroke changes
    if (this->miterStroke() != that->miterStroke()) {
        return false;
    }

    // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
    // local coords then we won't be able to combine.  We could actually upload the viewmatrix
    // using vertex attributes in these cases, but haven't investigated that
    if (this->usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return false;
    }

    // In the event of two ops, one who can tweak, one who cannot, we just fall back to not
    // tweaking.
    if (this->canTweakAlphaForCoverage() != that->canTweakAlphaForCoverage()) {
        fCanTweakAlphaForCoverage = false;
    }

    fRects.push_back_n(that->fRects.count(), that->fRects.begin());
    this->joinBounds(*that);
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

void AAStrokeRectOp::generateAAStrokeRectGeometry(void* vertices,
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
    SkPoint* fan3Pos = reinterpret_cast<SkPoint*>(
            verts + (2 * outerVertexNum + innerVertexNum) * vertexStride);

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
            inset = SK_ScalarHalf *
                    SkMinScalar(inset, devOutsideAssist.fBottom - devInside.fBottom);
        }
        SkASSERT(inset >= 0);
    } else {
        // TODO use real devRect here
        inset = SkMinScalar(devOutside.width(), SK_Scalar1);
        inset = SK_ScalarHalf *
                SkMinScalar(inset, SkTMax(devOutside.height(), devOutsideAssist.height()));
    }
#else
    SkScalar inset;
    if (!degenerate) {
        inset = SK_ScalarHalf;
    } else {
        // TODO use real devRect here
        inset = SkMinScalar(devOutside.width(), SK_Scalar1);
        inset = SK_ScalarHalf *
                SkMinScalar(inset, SkTMax(devOutside.height(), devOutsideAssist.height()));
    }
#endif

    if (miterStroke) {
        // outermost
        set_inset_fan(fan0Pos, vertexStride, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
        // inner two
        set_inset_fan(fan1Pos, vertexStride, devOutside, inset, inset);
        if (!degenerate) {
            set_inset_fan(fan2Pos, vertexStride, devInside, -inset, -inset);
            // innermost
            set_inset_fan(fan3Pos, vertexStride, devInside, SK_ScalarHalf, SK_ScalarHalf);
        } else {
            // When the interior rect has become degenerate we smoosh to a single point
            SkASSERT(devInside.fLeft == devInside.fRight && devInside.fTop == devInside.fBottom);
            fan2Pos->setRectFan(devInside.fLeft, devInside.fTop, devInside.fRight,
                                devInside.fBottom, vertexStride);
            fan3Pos->setRectFan(devInside.fLeft, devInside.fTop, devInside.fRight,
                                devInside.fBottom, vertexStride);
        }
    } else {
        SkPoint* fan0AssistPos = reinterpret_cast<SkPoint*>(verts + 4 * vertexStride);
        SkPoint* fan1AssistPos =
                reinterpret_cast<SkPoint*>(verts + (outerVertexNum + 4) * vertexStride);
        // outermost
        set_inset_fan(fan0Pos, vertexStride, devOutside, -SK_ScalarHalf, -SK_ScalarHalf);
        set_inset_fan(fan0AssistPos, vertexStride, devOutsideAssist, -SK_ScalarHalf,
                      -SK_ScalarHalf);
        // outer one of the inner two
        set_inset_fan(fan1Pos, vertexStride, devOutside, inset, inset);
        set_inset_fan(fan1AssistPos, vertexStride, devOutsideAssist, inset, inset);
        if (!degenerate) {
            // inner one of the inner two
            set_inset_fan(fan2Pos, vertexStride, devInside, -inset, -inset);
            // innermost
            set_inset_fan(fan3Pos, vertexStride, devInside, SK_ScalarHalf, SK_ScalarHalf);
        } else {
            // When the interior rect has become degenerate we smoosh to a single point
            SkASSERT(devInside.fLeft == devInside.fRight && devInside.fTop == devInside.fBottom);
            fan2Pos->setRectFan(devInside.fLeft, devInside.fTop, devInside.fRight,
                                devInside.fBottom, vertexStride);
            fan3Pos->setRectFan(devInside.fLeft, devInside.fTop, devInside.fRight,
                                devInside.fBottom, vertexStride);
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

namespace GrAAStrokeRectOp {

std::unique_ptr<GrLegacyMeshDrawOp> MakeFillBetweenRects(GrColor color,
                                                         const SkMatrix& viewMatrix,
                                                         const SkRect& devOutside,
                                                         const SkRect& devInside) {
    return std::unique_ptr<GrLegacyMeshDrawOp>(
            new AAStrokeRectOp(color, viewMatrix, devOutside, devInside));
}

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkStrokeRec& stroke) {
    return AAStrokeRectOp::Make(color, viewMatrix, rect, stroke);
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

#include "GrDrawOpTest.h"

DRAW_OP_TEST_DEFINE(AAStrokeRectOp) {
    bool miterStroke = random->nextBool();

    // Create either a empty rect or a non-empty rect.
    SkRect rect =
            random->nextBool() ? SkRect::MakeXYWH(10, 10, 50, 40) : SkRect::MakeXYWH(6, 7, 0, 0);
    SkScalar minDim = SkMinScalar(rect.width(), rect.height());
    SkScalar strokeWidth = random->nextUScalar1() * minDim;

    GrColor color = GrRandomColor(random);

    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(strokeWidth);
    rec.setStrokeParams(SkPaint::kButt_Cap,
                        miterStroke ? SkPaint::kMiter_Join : SkPaint::kBevel_Join, 1.f);
    SkMatrix matrix = GrTest::TestMatrixRectStaysRect(random);
    return GrAAStrokeRectOp::Make(color, matrix, rect, rec);
}

#endif
