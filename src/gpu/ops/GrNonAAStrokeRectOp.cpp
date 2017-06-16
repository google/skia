/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColor.h"
#include "GrDefaultGeoProcFactory.h"
#include "GrDrawOpTest.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrRectOpFactory.h"
#include "GrSimpleMeshDrawOpHelper.h"
#include "SkRandom.h"
#include "SkStrokeRec.h"

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

namespace {

class NonAAStrokeRectOp final : public GrMeshDrawOp {
private:
    using Helper = GrSimpleMeshDrawOpHelper;

public:
    DEFINE_OP_CLASS_ID

    const char* name() const override { return "NonAAStrokeRectOp"; }

    SkString dumpInfo() const override {
        SkString string;
        string.appendf(
                "Color: 0x%08x, Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                "StrokeWidth: %.2f\n",
                fColor, fRect.fLeft, fRect.fTop, fRect.fRight, fRect.fBottom, fStrokeWidth);
        string += fHelper.dumpInfo();
        string += INHERITED::dumpInfo();
        return string;
    }

    static std::unique_ptr<GrDrawOp> Make(GrPaint&& paint, const SkMatrix& viewMatrix,
                                          const SkRect& rect, const SkStrokeRec& stroke,
                                          GrAAType aaType) {
        if (!allowed_stroke(stroke)) {
            return nullptr;
        }
        Helper::Flags flags = Helper::Flags::kNone;
        // Depending on sub-pixel coordinates and the particular GPU, we may lose a corner of
        // hairline rects. We jam all the vertices to pixel centers to avoid this, but not
        // when MSAA is enabled because it can cause ugly artifacts.
        if (stroke.getStyle() == SkStrokeRec::kHairline_Style && aaType != GrAAType::kMSAA) {
            flags |= Helper::Flags::kSnapVerticesToPixelCenters;
        }
        return Helper::FactoryHelper<NonAAStrokeRectOp>(std::move(paint), flags, viewMatrix, rect,
                                                        stroke, aaType);
    }

    NonAAStrokeRectOp(const Helper::MakeArgs& helperArgs, GrColor color, Helper::Flags flags,
                      const SkMatrix& viewMatrix, const SkRect& rect, const SkStrokeRec& stroke,
                      GrAAType aaType)
            : INHERITED(ClassID()), fHelper(helperArgs, aaType, flags) {
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
        if (flags & Helper::Flags::kSnapVerticesToPixelCenters) {
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

    RequiresDstTexture finalize(const GrCaps& caps, const GrAppliedClip* clip) override {
        return fHelper.xpRequiresDstTexture(caps, clip, GrProcessorAnalysisCoverage::kNone,
                                            &fColor);
    }

private:
    void onPrepareDraws(Target* target) const override {
        sk_sp<GrGeometryProcessor> gp;
        {
            using namespace GrDefaultGeoProcFactory;
            Color color(fColor);
            LocalCoords::Type localCoordsType = fHelper.usesLocalCoords()
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
            primType = GrPrimitiveType::kTriangleStrip;
            init_stroke_rect_strip(vertex, fRect, fStrokeWidth);
        } else {
            // hairline
            primType = GrPrimitiveType::kLineStrip;
            vertex[0].set(fRect.fLeft, fRect.fTop);
            vertex[1].set(fRect.fRight, fRect.fTop);
            vertex[2].set(fRect.fRight, fRect.fBottom);
            vertex[3].set(fRect.fLeft, fRect.fBottom);
            vertex[4].set(fRect.fLeft, fRect.fTop);
        }

        GrMesh mesh(primType);
        mesh.setNonIndexedNonInstanced(vertexCount);
        mesh.setVertexData(vertexBuffer, firstVertex);
        target->draw(gp.get(), fHelper.makePipeline(target), mesh);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps&) override {
        // NonAA stroke rects cannot combine right now
        // TODO make these combinable.
        return false;
    }

    Helper fHelper;
    GrColor fColor;
    SkMatrix fViewMatrix;
    SkRect fRect;
    SkScalar fStrokeWidth;

    const static int kVertsPerHairlineRect = 5;
    const static int kVertsPerStrokeRect = 10;

    typedef GrMeshDrawOp INHERITED;
};

}  // anonymous namespace

namespace GrRectOpFactory {
std::unique_ptr<GrDrawOp> MakeNonAAStroke(GrPaint&& paint,
                                          const SkMatrix& viewMatrix,
                                          const SkRect& rect,
                                          const SkStrokeRec& stroke,
                                          GrAAType aaType) {
    return NonAAStrokeRectOp::Make(std::move(paint), viewMatrix, rect, stroke, aaType);
}
}  // namespace GrRectOpFactory

#if GR_TEST_UTILS

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
    return NonAAStrokeRectOp::Make(std::move(paint), viewMatrix, rect, strokeRec, aaType);
}

#endif
