/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNonAAStrokeRectOp.h"

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "SkStrokeRec.h"
#include "SkRandom.h"

/*  create a triangle strip that strokes the specified rect. There are 8
    unique vertices, but we repeat the last 2 to close up. Alternatively we
    could use an indices array, and then only send 8 verts, but not sure that
    would be faster.
    */
static void init_stroke_rect_strip(SkPoint verts[10], const SkRect& rect, SkScalar width) {
    const SkScalar rad = SkScalarHalf(width);
    // TODO we should be able to enable this assert, but we'd have to filter these draws
    // this is a bug
    // SkASSERT(rad < rect.width() / 2 && rad < rect.height() / 2);

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

// Allow all hairlines and all miters, so long as the miter limit doesn't produce beveled corners.
inline static bool allowed_stroke(const SkStrokeRec& stroke) {
    SkASSERT(stroke.getStyle() == SkStrokeRec::kStroke_Style ||
             stroke.getStyle() == SkStrokeRec::kHairline_Style);
    return !stroke.getWidth() ||
           (stroke.getJoin() == SkPaint::kMiter_Join && stroke.getMiter() > SK_ScalarSqrt2);
}

class NonAAStrokeRectOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "NonAAStrokeRectOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf(
                "Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                "StrokeWidth: %.2f\n",
                fColor, fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom, fStrokeWidth);
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, const SkMatrix& viewMatrix,
                                                    const SkRect& rect, const SkStrokeRec& stroke,
                                                    bool snapToPixelCenters) {
        if (!allowed_stroke(stroke)) {
            return nullptr;
        }
        NonAAStrokeRectOp* op = new NonAAStrokeRectOp();
        op->fColor = color;
        op->fViewMatrix = viewMatrix;
        op->fRect = rect;
        // Sort the rect for hairlines
        op->fRect.sort();
        op->fStrokeWidth = stroke.getWidth();

        SkScalar rad = SkScalarHalf(op->fStrokeWidth);
        SkRect bounds = rect;
        bounds.outset(rad, rad);

        // If our caller snaps to pixel centers then we have to round out the bounds
        if (snapToPixelCenters) {
            viewMatrix.mapRect(&bounds);
            // We want to be consistent with how we snap non-aa lines. To match what we do in
            // GrGLSLVertexShaderBuilder, we first floor all the vertex values and then add half a
            // pixel to force us to pixel centers.
            bounds.set(SkScalarFloorToScalar(bounds.fLeft),
                       SkScalarFloorToScalar(bounds.fTop),
                       SkScalarFloorToScalar(bounds.fRight),
                       SkScalarFloorToScalar(bounds.fBottom));
            bounds.offset(0.5f, 0.5f);
            op->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
        } else {
            op->setTransformedBounds(bounds, op->fViewMatrix, HasAABloat::kNo, IsZeroArea::kNo);
        }
        return std::unique_ptr<GrLegacyMeshDrawOp>(op);
    }

private:
    NonAAStrokeRectOp() : INHERITED(ClassID()) {}

    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fColor);
        *coverage = GrProcessorAnalysisCoverage::kNone;
    }

    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(fColor);
            LocalCoords::Type localCoordsType = fNeedsLocalCoords
                                                        ? LocalCoords::kUsePosition_Type
                                                        : LocalCoords::kUnused_Type;
            gp = GrDefaultGeoProcFactory::Make(color, Coverage::kSolid_Type, localCoordsType,
                                               fViewMatrix);
        }

        size_t vertexStride = gp->getVertexStride();

        SkASSERT(vertexStride == sizeof(GrDefaultGeoProcFactory::PositionAttr));

        int vertexCount = kVertsPerHairlineRect;
        if (fStrokeWidth > 0) {
            vertexCount = kVertsPerStrokeRect;
        }

        const GrBuffer* vertexBuffer;
        int firstVertex;

        void* verts =
                target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer, &firstVertex);

        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        SkPoint* vertex = reinterpret_cast<SkPoint*>(verts);

        GrPrimitiveType primType;
        if (fStrokeWidth > 0) {
            primType = kTriangleStrip_GrPrimitiveType;
            init_stroke_rect_strip(vertex, fRect, fStrokeWidth);
        } else {
            // hairline
            primType = kLineStrip_GrPrimitiveType;
            vertex[0].set(fRect.fLeft, fRect.fTop);
            vertex[1].set(fRect.fRight, fRect.fTop);
            vertex[2].set(fRect.fRight, fRect.fBottom);
            vertex[3].set(fRect.fLeft, fRect.fBottom);
            vertex[4].set(fRect.fLeft, fRect.fTop);
        }

        GrMesh mesh;
        mesh.init(primType, vertexBuffer, firstVertex, vertexCount);
        target->draw(gp.get(), this->pipeline(), mesh);
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fColor);
        fNeedsLocalCoords = optimizations.readsLocalCoords();
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps&) override {
        // NonAA stroke rects cannot combine right now
        // TODO make these combinable.
        return false;
    }

    GrColor fColor;
    SkMatrix fViewMatrix;
    SkRect fRect;
    SkScalar fStrokeWidth;
    bool fNeedsLocalCoords;

    const static int kVertsPerHairlineRect = 5;
    const static int kVertsPerStrokeRect = 10;

    typedef GrLegacyMeshDrawOp INHERITED;
};

namespace GrNonAAStrokeRectOp {

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkStrokeRec& stroke,
                                         bool snapToPixelCenters) {
    return NonAAStrokeRectOp::Make(color, viewMatrix, rect, stroke, snapToPixelCenters);
}
}

#if GR_TEST_UTILS

DRAW_OP_TEST_DEFINE(NonAAStrokeRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrix(random);
    GrColor color = GrRandomColor(random);
    SkRect rect = GrTest::TestRect(random);
    SkScalar strokeWidth = random->nextBool() ? 0.0f : 2.0f;
    SkPaint paint;
    paint.setStrokeWidth(strokeWidth);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeJoin(SkPaint::kMiter_Join);
    SkStrokeRec strokeRec(paint);
    return GrNonAAStrokeRectOp::Make(color, viewMatrix, rect, strokeRec, random->nextBool());
}

#endif
