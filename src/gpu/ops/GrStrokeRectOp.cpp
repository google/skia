/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStrokeRectOp.h"

#include "GrCaps.h"
#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "GrVertexWriter.h"
#include "ops/GrFillRectOp.h"
#include "SkRandom.h"
#include "SkStrokeRec.h"

namespace {

// We support all hairlines, bevels, and miters, but not round joins. Also, check whether the miter
// limit makes a miter join effectively beveled. If the miter is effectively beveled, it is only
// supported when using an AA stroke.
inline static bool allowed_stroke(const SkStrokeRec& stroke, GrAA aa, bool* isMiter) {
    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style ||
             stroke.getStyle() == SkStrokeRec::kHairline_Style);
    // For hairlines, make bevel and round joins appear the same as mitered ones.
    if (!stroke.getWidth()) {
        *isMiter = true;
        return true;
    }
    if (stroke.getJoin() == SkPaint::kBevel_Join) {
        *isMiter = false;
        return aa == GrAA::kYes; // bevel only supported with AA
    }
    if (stroke.getJoin() == SkPaint::kMiter_Join) {
        *isMiter = stroke.getMiter() >= SK_ScalarSqrt2;
        // Supported under non-AA only if it remains mitered
        return aa == GrAA::kYes || *isMiter;
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Non-AA Stroking
///////////////////////////////////////////////////////////////////////////////////////////////////

/*  create a triangle strip that strokes the specified rect. There are 8
    unique vertices, but we repeat the last 2 to close up. Alternatively we
    could use an indices array, and then only send 8 verts, but not sure that
    would be faster.
    */
static void init_nonaa_stroke_rect_strip(SkPoint verts[10], const SkRect& rect, SkScalar width) {
    const SkScalar rad = SkScalarHalf(width);

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

    // TODO: we should be catching this higher up the call stack and just draw a single
    // non-AA rect
    if (2*rad >= rect.width()) {
        verts[0].fX = verts[2].fX = verts[4].fX = verts[6].fX = verts[8].fX = rect.centerX();
    }
    if (2*rad >= rect.height()) {
        verts[0].fY = verts[2].fY = verts[4].fY = verts[6].fY = verts[8].fY = rect.centerY();
    }
}

class NonAAStrokeRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "NonAAStrokeRectOp"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        string.appendf(
                "Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                "StrokeWidth: %.2f\n",
                fColor.toBytes_RGBA(), fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom,
                fStrokeWidth);
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }
#endif

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& rect,
                                          const SkStrokeRec& stroke,
                                          GrAAType aaType) {
        bool isMiter;
        if (!allowed_stroke(stroke, GrAA::kNo, &isMiter)) {
            return nullptr;
        }
        Helper::InputFlags inputFlags = Helper::InputFlags::kNone;
        // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
        // hairline rects. We jam all the vertices to pixel centers to avoid this, but not
        // when MSAA is enabled because it can cause ugly artifacts.
        if (stroke.getStyle() == SkStrokeRec::kHairline_Style && aaType != GrAAType::kMSAA) {
            inputFlags |= Helper::InputFlags::kSnapVerticesToPixelCenters;
        }
        return Helper::FactoryHelper<NonAAStrokeRectOp>(context, std::move(paint), inputFlags,
                                                        viewMatrix, rect,
                                                        stroke, aaType);
    }

    NonAAStrokeRectOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                      Helper::InputFlags inputFlags, const SkMatrix& viewMatrix, const SkRect& rect,
                      const SkStrokeRec& stroke, GrAAType aaType)
            : INHERITED(ClassID()), fHelper(helperArgs, aaType, inputFlags) {
        fColor = color;
        fViewMatrix = viewMatrix;
        fRect = rect;
        // Sort the rect for hairlines
        fRect.sort();
        fStrokeWidth = stroke.getWidth();

        SkScalar rad = SkScalarHalf(fStrokeWidth);
        SkRect bounds = rect;
        bounds.outset(rad, rad);

        // If our caller snaps to pixel centers then we have to round out the bounds
        if (inputFlags & Helper::InputFlags::kSnapVerticesToPixelCenters) {
            viewMatrix.mapRect(&bounds);
            // We want to be consistent with how we snap non-aa lines. To match what we do in
            // GrGLSLVertexShaderBuilder, we first floor all the vertex values and then add half a
            // pixel to force us to pixel centers.
            bounds.set(SkScalarFloorToScalar(bounds.fLeft),
                       SkScalarFloorToScalar(bounds.fTop),
                       SkScalarFloorToScalar(bounds.fRight),
                       SkScalarFloorToScalar(bounds.fBottom));
            bounds.offset(0.5f, 0.5f);
            this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
        } else {
            this->setTransformedBounds(bounds, fViewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
        }
    }

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        // This Op uses uniform (not vertex) color, so doesn't need to track wide color.
        return fHelper.finalizeProcessors(caps, clip, fsaaType, clampType,
                                          GrProcessorAnalysisCoverage::kNone, &fColor, nullptr);
    }

private:
    void onPrepareDraws(Target* target) override {
        sk_sp<GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(fColor);
            LocalCoords::Type localCoordsType = fHelper.usesLocalCoords()
                                                        ? LocalCoords::kUsePosition_Type
                                                        : LocalCoords::kUnused_Type;
            gp = GrDefaultGeoProcFactory::Make(target->caps().shaderCaps(), color,
                                               Coverage::kSolid_Type, localCoordsType,
                                               fViewMatrix);
        }

        size_t kVertexStride = gp->vertexStride();
        int vertexCount = kVertsPerHairlineRect;
        if (fStrokeWidth > 0) {
            vertexCount = kVertsPerStrokeRect;
        }

        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;

        void* verts =
                target->makeVertexSpace(kVertexStride, vertexCount, &vertexBuffer, &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        SkPoint* vertex = reinterpret_cast<SkPoint*>(verts);

        GrPrimitiveType primType;
        if (fStrokeWidth > 0) {
            primType = GrPrimitiveType::kTriangleStrip;
            init_nonaa_stroke_rect_strip(vertex, fRect, fStrokeWidth);
        } else {
            // hairline
            primType = GrPrimitiveType::kLineStrip;
            vertex[0].set(fRect.fLeft, fRect.fTop);
            vertex[1].set(fRect.fRight, fRect.fTop);
            vertex[2].set(fRect.fRight, fRect.fBottom);
            vertex[3].set(fRect.fLeft, fRect.fBottom);
            vertex[4].set(fRect.fLeft, fRect.fTop);
        }

        GrMesh* mesh = target->allocMesh(primType);
        mesh->setNonIndexedNonInstanced(vertexCount);
        mesh->setVertexData(std::move(vertexBuffer), firstVertex);
        target->recordDraw(std::move(gp), mesh);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
    }

    // TODO: override onCombineIfPossible

    Helper fHelper;
    SkPMColor4f fColor;
    SkMatrix fViewMatrix;
    SkRect fRect;
    SkScalar fStrokeWidth;

    const static int kVertsPerHairlineRect = 5;
    const static int kVertsPerStrokeRect = 10;

    typedef GrMeshDrawOp INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// AA Stroking
///////////////////////////////////////////////////////////////////////////////////////////////////

GR_DECLARE_STATIC_UNIQUE_KEY(gMiterIndexBufferKey);
GR_DECLARE_STATIC_UNIQUE_KEY(gBevelIndexBufferKey);

static void compute_aa_rects(SkRect* devOutside, SkRect* devOutsideAssist, SkRect* devInside,
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

static sk_sp<GrGeometryProcessor> create_aa_stroke_rect_gp(const GrShaderCaps* shaderCaps,
                                                           bool tweakAlphaForCoverage,
                                                           const SkMatrix& viewMatrix,
                                                           bool usesLocalCoords,
                                                           bool wideColor) {
    using namespace GrDefaultGeoProcFactory;

    Coverage::Type coverageType =
        tweakAlphaForCoverage ? Coverage::kSolid_Type : Coverage::kAttribute_Type;
    LocalCoords::Type localCoordsType =
        usesLocalCoords ? LocalCoords::kUsePosition_Type : LocalCoords::kUnused_Type;
    Color::Type colorType =
        wideColor ? Color::kPremulWideColorAttribute_Type: Color::kPremulGrColorAttribute_Type;

    return MakeForDeviceSpace(shaderCaps, colorType, coverageType, localCoordsType, viewMatrix);
}

class AAStrokeRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& devOutside,
                                          const SkRect& devInside) {
        return Helper::FactoryHelper<AAStrokeRectOp>(context, std::move(paint), viewMatrix,
                                                     devOutside, devInside);
    }

    AAStrokeRectOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, const SkRect& devOutside, const SkRect& devInside)
            : INHERITED(ClassID())
            , fHelper(helperArgs, GrAAType::kCoverage)
            , fViewMatrix(viewMatrix) {
        SkASSERT(!devOutside.isEmpty());
        SkASSERT(!devInside.isEmpty());

        fRects.emplace_back(RectInfo{color, devOutside, devOutside, devInside, false});
        this->setBounds(devOutside, HasAABloat::kYes, IsZeroArea::kNo);
        fMiterStroke = true;
    }

    static std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                                          GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& rect,
                                          const SkStrokeRec& stroke) {
        bool isMiter;
        if (!allowed_stroke(stroke, GrAA::kYes, &isMiter)) {
            return nullptr;
        }
        return Helper::FactoryHelper<AAStrokeRectOp>(context, std::move(paint), viewMatrix, rect,
                                                     stroke, isMiter);
    }

    AAStrokeRectOp(const Helper::MakeArgs& helperArgs, const SkPMColor4f& color,
                   const SkMatrix& viewMatrix, const SkRect& rect, const SkStrokeRec& stroke,
                   bool isMiter)
            : INHERITED(ClassID())
            , fHelper(helperArgs, GrAAType::kCoverage)
            , fViewMatrix(viewMatrix) {
        fMiterStroke = isMiter;
        RectInfo& info = fRects.push_back();
        compute_aa_rects(&info.fDevOutside, &info.fDevOutsideAssist, &info.fDevInside,
                         &info.fDegenerate, viewMatrix, rect, stroke.getWidth(), isMiter);
        info.fColor = color;
        if (isMiter) {
            this->setBounds(info.fDevOutside, HasAABloat::kYes, IsZeroArea::kNo);
        } else {
            // The outer polygon of the bevel stroke is an octagon specified by the points of a
            // pair of overlapping rectangles where one is wide and the other is narrow.
            SkRect bounds = info.fDevOutside;
            bounds.joinPossiblyEmptyRect(info.fDevOutsideAssist);
            this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kNo);
        }
    }

    const char* name() const override { return "AAStrokeRect"; }

    void visitProxies(const VisitProxyFunc& func, VisitorType) const override {
        fHelper.visitProxies(func);
    }

#ifdef SK_DEBUG
    SkString dumpInfo() const override {
        SkString string;
        for (const auto& info : fRects) {
            string.appendf(
                    "Color: 0x%08x, ORect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "AssistORect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "IRect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], Degen: %d",
                    info.fColor.toBytes_RGBA(), info.fDevOutside.fLeft, info.fDevOutside.fTop,
                    info.fDevOutside.fRight, info.fDevOutside.fBottom, info.fDevOutsideAssist.fLeft,
                    info.fDevOutsideAssist.fTop, info.fDevOutsideAssist.fRight,
                    info.fDevOutsideAssist.fBottom, info.fDevInside.fLeft, info.fDevInside.fTop,
                    info.fDevInside.fRight, info.fDevInside.fBottom, info.fDegenerate);
        }
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }
#endif

    FixedFunctionFlags fixedFunctionFlags() const override { return fHelper.fixedFunctionFlags(); }

    GrProcessorSet::Analysis finalize(const GrCaps& caps, const GrAppliedClip* clip,
                                      GrFSAAType fsaaType, GrClampType clampType) override {
        return fHelper.finalizeProcessors(
                caps, clip, fsaaType, clampType, GrProcessorAnalysisCoverage::kSingleChannel,
                &fRects.back().fColor, &fWideColor);
    }

private:
    void onPrepareDraws(Target*) override;
    void onExecute(GrOpFlushState*, const SkRect& chainBounds) override;

    static const int kMiterIndexCnt = 3 * 24;
    static const int kMiterVertexCnt = 16;
    static const int kNumMiterRectsInIndexBuffer = 256;

    static const int kBevelIndexCnt = 48 + 36 + 24;
    static const int kBevelVertexCnt = 24;
    static const int kNumBevelRectsInIndexBuffer = 256;

    static sk_sp<const GrGpuBuffer> GetIndexBuffer(GrResourceProvider*, bool miterStroke);

    const SkMatrix& viewMatrix() const { return fViewMatrix; }
    bool miterStroke() const { return fMiterStroke; }

    CombineResult onCombineIfPossible(GrOp* t, const GrCaps&) override;

    void generateAAStrokeRectGeometry(GrVertexWriter& vertices,
                                      const SkPMColor4f& color,
                                      bool wideColor,
                                      const SkRect& devOutside,
                                      const SkRect& devOutsideAssist,
                                      const SkRect& devInside,
                                      bool miterStroke,
                                      bool degenerate,
                                      bool tweakAlphaForCoverage) const;

    // TODO support AA rotated stroke rects by copying around view matrices
    struct RectInfo {
        SkPMColor4f fColor;
        SkRect fDevOutside;
        SkRect fDevOutsideAssist;
        SkRect fDevInside;
        bool fDegenerate;
    };

    Helper fHelper;
    SkSTArray<1, RectInfo, true> fRects;
    SkMatrix fViewMatrix;
    bool fMiterStroke;
    bool fWideColor;

    typedef GrMeshDrawOp INHERITED;
};

void AAStrokeRectOp::onPrepareDraws(Target* target) {
    sk_sp<GrGeometryProcessor> gp(create_aa_stroke_rect_gp(target->caps().shaderCaps(),
                                                           fHelper.compatibleWithCoverageAsAlpha(),
                                                           this->viewMatrix(),
                                                           fHelper.usesLocalCoords(),
                                                           fWideColor));
    if (!gp) {
        SkDebugf("Couldn't create GrGeometryProcessor\n");
        return;
    }

    int innerVertexNum = 4;
    int outerVertexNum = this->miterStroke() ? 4 : 8;
    int verticesPerInstance = (outerVertexNum + innerVertexNum) * 2;
    int indicesPerInstance = this->miterStroke() ? kMiterIndexCnt : kBevelIndexCnt;
    int instanceCount = fRects.count();

    sk_sp<const GrGpuBuffer> indexBuffer =
            GetIndexBuffer(target->resourceProvider(), this->miterStroke());
    if (!indexBuffer) {
        SkDebugf("Could not allocate indices\n");
        return;
    }
    PatternHelper helper(target, GrPrimitiveType::kTriangles, gp->vertexStride(),
                         std::move(indexBuffer), verticesPerInstance, indicesPerInstance,
                         instanceCount);
    GrVertexWriter vertices{ helper.vertices() };
    if (!vertices.fPtr) {
        SkDebugf("Could not allocate vertices\n");
        return;
    }

    for (int i = 0; i < instanceCount; i++) {
        const RectInfo& info = fRects[i];
        this->generateAAStrokeRectGeometry(vertices,
                                           info.fColor,
                                           fWideColor,
                                           info.fDevOutside,
                                           info.fDevOutsideAssist,
                                           info.fDevInside,
                                           fMiterStroke,
                                           info.fDegenerate,
                                           fHelper.compatibleWithCoverageAsAlpha());
    }
    helper.recordDraw(target, std::move(gp));
}

void AAStrokeRectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    fHelper.executeDrawsAndUploads(this, flushState, chainBounds);
}

sk_sp<const GrGpuBuffer> AAStrokeRectOp::GetIndexBuffer(GrResourceProvider* resourceProvider,
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
        return resourceProvider->findOrCreatePatternedIndexBuffer(
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
        return resourceProvider->findOrCreatePatternedIndexBuffer(
                gBevelIndices, kBevelIndexCnt, kNumBevelRectsInIndexBuffer, kBevelVertexCnt,
                gBevelIndexBufferKey);
    }
}

GrOp::CombineResult AAStrokeRectOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    AAStrokeRectOp* that = t->cast<AAStrokeRectOp>();

    if (!fHelper.isCompatible(that->fHelper, caps, this->bounds(), that->bounds())) {
        return CombineResult::kCannotCombine;
    }

    // TODO combine across miterstroke changes
    if (this->miterStroke() != that->miterStroke()) {
        return CombineResult::kCannotCombine;
    }

    // We apply the viewmatrix to the rect points on the cpu.  However, if the pipeline uses
    // local coords then we won't be able to combine. TODO: Upload local coords as an attribute.
    if (fHelper.usesLocalCoords() && !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return CombineResult::kCannotCombine;
    }

    fRects.push_back_n(that->fRects.count(), that->fRects.begin());
    fWideColor |= that->fWideColor;
    return CombineResult::kMerged;
}

static void setup_scale(int* scale, SkScalar inset) {
    if (inset < SK_ScalarHalf) {
        *scale = SkScalarFloorToInt(512.0f * inset / (inset + SK_ScalarHalf));
        SkASSERT(*scale >= 0 && *scale <= 255);
    } else {
        *scale = 0xff;
    }
}

void AAStrokeRectOp::generateAAStrokeRectGeometry(GrVertexWriter& vertices,
                                                  const SkPMColor4f& color,
                                                  bool wideColor,
                                                  const SkRect& devOutside,
                                                  const SkRect& devOutsideAssist,
                                                  const SkRect& devInside,
                                                  bool miterStroke,
                                                  bool degenerate,
                                                  bool tweakAlphaForCoverage) const {
    // We create vertices for four nested rectangles. There are two ramps from 0 to full
    // coverage, one on the exterior of the stroke and the other on the interior.

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

    auto inset_fan = [](const SkRect& r, SkScalar dx, SkScalar dy) {
        return GrVertexWriter::TriFanFromRect(r.makeInset(dx, dy));
    };

    auto maybe_coverage = [tweakAlphaForCoverage](float coverage) {
        return GrVertexWriter::If(!tweakAlphaForCoverage, coverage);
    };

    GrVertexColor outerColor(tweakAlphaForCoverage ? SK_PMColor4fTRANSPARENT : color, wideColor);

    // Outermost rect
    vertices.writeQuad(inset_fan(devOutside, -SK_ScalarHalf, -SK_ScalarHalf),
                       outerColor,
                       maybe_coverage(0.0f));

    if (!miterStroke) {
        // Second outermost
        vertices.writeQuad(inset_fan(devOutsideAssist, -SK_ScalarHalf, -SK_ScalarHalf),
                           outerColor,
                           maybe_coverage(0.0f));
    }

    // scale is the coverage for the the inner two rects.
    int scale;
    setup_scale(&scale, inset);

    float innerCoverage = GrNormalizeByteToFloat(scale);
    SkPMColor4f scaledColor = color * innerCoverage;
    GrVertexColor innerColor(tweakAlphaForCoverage ? scaledColor : color, wideColor);

    // Inner rect
    vertices.writeQuad(inset_fan(devOutside, inset, inset),
                       innerColor,
                       maybe_coverage(innerCoverage));

    if (!miterStroke) {
        // Second inner
        vertices.writeQuad(inset_fan(devOutsideAssist, inset, inset),
                           innerColor,
                           maybe_coverage(innerCoverage));
    }

    if (!degenerate) {
        vertices.writeQuad(inset_fan(devInside, -inset, -inset),
                           innerColor,
                           maybe_coverage(innerCoverage));

        // The innermost rect has 0 coverage...
        vertices.writeQuad(inset_fan(devInside, SK_ScalarHalf, SK_ScalarHalf),
                           GrVertexColor(SK_PMColor4fTRANSPARENT, wideColor),
                           maybe_coverage(0.0f));
    } else {
        // When the interior rect has become degenerate we smoosh to a single point
        SkASSERT(devInside.fLeft == devInside.fRight && devInside.fTop == devInside.fBottom);

        vertices.writeQuad(GrVertexWriter::TriFanFromRect(devInside),
                           innerColor,
                           maybe_coverage(innerCoverage));

        // ... unless we are degenerate, in which case we must apply the scaled coverage
        vertices.writeQuad(GrVertexWriter::TriFanFromRect(devInside),
                           innerColor,
                           maybe_coverage(innerCoverage));
    }
}

}  // anonymous namespace

namespace GrStrokeRectOp {

std::unique_ptr<GrDrawOp> Make(GrRecordingContext* context,
                               GrPaint&& paint,
                               GrAAType aaType,
                               const SkMatrix& viewMatrix,
                               const SkRect& rect,
                               const SkStrokeRec& stroke) {
    if (aaType == GrAAType::kCoverage) {
        // The AA op only supports axis-aligned rectangles
        if (!viewMatrix.rectStaysRect()) {
            return nullptr;
        }
        return AAStrokeRectOp::Make(context, std::move(paint), viewMatrix, rect, stroke);
    } else {
        return NonAAStrokeRectOp::Make(context, std::move(paint), viewMatrix, rect, stroke, aaType);
    }
}

std::unique_ptr<GrDrawOp> MakeNested(GrRecordingContext* context,
                                     GrPaint&& paint,
                                     const SkMatrix& viewMatrix,
                                     const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[0].isEmpty() && !rects[1].isEmpty());

    SkRect devOutside, devInside;
    viewMatrix.mapRect(&devOutside, rects[0]);
    viewMatrix.mapRect(&devInside, rects[1]);
    if (devInside.isEmpty()) {
        if (devOutside.isEmpty()) {
            return nullptr;
        }
        return GrFillRectOp::Make(context, std::move(paint), GrAAType::kCoverage, viewMatrix,
                                  rects[0]);
    }

    return AAStrokeRectOp::Make(context, std::move(paint), viewMatrix, devOutside, devInside);
}

}  // namespace GrStrokeRectOp

#if GR_TEST_UTILS

#include "GrDrawOpTest.h"

GR_DRAW_OP_TEST_DEFINE(NonAAStrokeRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    SkRect rect = GrTest::TestRect(random);
    SkScalar strokeWidth = random->nextBool() ? 0.0f : 2.0f;
    SkPaint strokePaint;
    strokePaint.setStrokeWidth(strokeWidth);
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeJoin(SkPaint::kMiter_Join);
    SkStrokeRec strokeRec(strokePaint);
    GrAAType aaType = GrAAType::kNone;
    if (fsaaType == GrFSAAType::kUnifiedMSAA) {
        aaType = random->nextBool() ? GrAAType::kMSAA : GrAAType::kNone;
    }
    return NonAAStrokeRectOp::Make(context, std::move(paint), viewMatrix, rect, strokeRec, aaType);
}

GR_DRAW_OP_TEST_DEFINE(AAStrokeRectOp) {
    bool miterStroke = random->nextBool();

    // Create either a empty rect or a non-empty rect.
    SkRect rect =
            random->nextBool() ? SkRect::MakeXYWH(10, 10, 50, 40) : SkRect::MakeXYWH(6, 7, 0, 0);
    SkScalar minDim = SkMinScalar(rect.width(), rect.height());
    SkScalar strokeWidth = random->nextUScalar1() * minDim;

    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(strokeWidth);
    rec.setStrokeParams(SkPaint::kButt_Cap,
                        miterStroke ? SkPaint::kMiter_Join : SkPaint::kBevel_Join, 1.f);
    SkMatrix matrix = GrTest::TestMatrixRectStaysRect(random);
    return AAStrokeRectOp::Make(context, std::move(paint), matrix, rect, rec);
}

#endif
