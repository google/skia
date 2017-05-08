/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShadowRRectOp.h"

#include "GrDrawOpTest.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"
#include "GrStyle.h"

#include "effects/GrShadowGeoProc.h"

///////////////////////////////////////////////////////////////////////////////

// We have two possible cases for geometry for a circle:

// In the case of a normal fill, we draw geometry for the circle as an octagon.
static const uint16_t gFillCircleIndices[] = {
        // enter the octagon
        // clang-format off
        0, 1, 8, 1, 2, 8,
        2, 3, 8, 3, 4, 8,
        4, 5, 8, 5, 6, 8,
        6, 7, 8, 7, 0, 8,
        // clang-format on
};

// For stroked circles, we use two nested octagons.
static const uint16_t gStrokeCircleIndices[] = {
        // enter the octagon
        // clang-format off
        0, 1,  9, 0,  9,  8,
        1, 2, 10, 1, 10,  9,
        2, 3, 11, 2, 11, 10,
        3, 4, 12, 3, 12, 11,
        4, 5, 13, 4, 13, 12,
        5, 6, 14, 5, 14, 13,
        6, 7, 15, 6, 15, 14,
        7, 0,  8, 7,  8, 15,
        // clang-format on
};

static const int kIndicesPerFillCircle = SK_ARRAY_COUNT(gFillCircleIndices);
static const int kIndicesPerStrokeCircle = SK_ARRAY_COUNT(gStrokeCircleIndices);
static const int kVertsPerStrokeCircle = 16;
static const int kVertsPerFillCircle = 9;

static int circle_type_to_vert_count(bool stroked) {
    return stroked ? kVertsPerStrokeCircle : kVertsPerFillCircle;
}

static int circle_type_to_index_count(bool stroked) {
    return stroked ? kIndicesPerStrokeCircle : kIndicesPerFillCircle;
}

static const uint16_t* circle_type_to_indices(bool stroked) {
    return stroked ? gStrokeCircleIndices : gFillCircleIndices;
}

///////////////////////////////////////////////////////////////////////////////

class ShadowCircleOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    static std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color, const SkMatrix& viewMatrix,
                                                    SkPoint center, SkScalar radius,
                                                    SkScalar blurRadius, const GrStyle& style) {
        SkASSERT(viewMatrix.isSimilarity());
        const SkStrokeRec& stroke = style.strokeRec();
        if (style.hasPathEffect()) {
            return nullptr;
        }
        SkStrokeRec::Style recStyle = stroke.getStyle();

        viewMatrix.mapPoints(&center, 1);
        radius = viewMatrix.mapRadius(radius);
        SkScalar strokeWidth = viewMatrix.mapRadius(stroke.getWidth());

        bool isStrokeOnly =
                SkStrokeRec::kStroke_Style == recStyle || SkStrokeRec::kHairline_Style == recStyle;
        bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == recStyle;

        SkScalar innerRadius = -SK_ScalarHalf;
        SkScalar outerRadius = radius;
        SkScalar halfWidth = 0;
        if (hasStroke) {
            if (SkScalarNearlyZero(strokeWidth)) {
                halfWidth = SK_ScalarHalf;
            } else {
                halfWidth = SkScalarHalf(strokeWidth);
            }

            outerRadius += halfWidth;
            if (isStrokeOnly) {
                innerRadius = radius - halfWidth;
            }
        }

        bool stroked = isStrokeOnly && innerRadius > 0.0f;
        std::unique_ptr<ShadowCircleOp> op(new ShadowCircleOp());
        op->fViewMatrixIfUsingLocalCoords = viewMatrix;

        SkRect devBounds = SkRect::MakeLTRB(center.fX - outerRadius, center.fY - outerRadius,
                                            center.fX + outerRadius, center.fY + outerRadius);

        op->fCircles.emplace_back(
                Circle{color, outerRadius, innerRadius, blurRadius, devBounds, stroked});

        // Use the original radius and stroke radius for the bounds so that it does not include the
        // AA bloat.
        radius += halfWidth;
        op->setBounds(
                {center.fX - radius, center.fY - radius, center.fX + radius, center.fY + radius},
                HasAABloat::kNo, IsZeroArea::kNo);
        op->fVertCount = circle_type_to_vert_count(stroked);
        op->fIndexCount = circle_type_to_index_count(stroked);
        return std::move(op);
    }

    const char* name() const override { return "ShadowCircleOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fCircles.count(); ++i) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                    "OuterRad: %.2f, InnerRad: %.2f, BlurRad: %.2f\n",
                    fCircles[i].fColor, fCircles[i].fDevBounds.fLeft, fCircles[i].fDevBounds.fTop,
                    fCircles[i].fDevBounds.fRight, fCircles[i].fDevBounds.fBottom,
                    fCircles[i].fOuterRadius, fCircles[i].fInnerRadius, fCircles[i].fBlurRadius);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    ShadowCircleOp() : INHERITED(ClassID()) {}

    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fCircles[0].fColor);
        *coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fCircles[0].fColor);
        if (!optimizations.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    void onPrepareDraws(Target* target) const override {
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(GrRRectShadowGeoProc::Make(localMatrix));

        struct CircleVertex {
            SkPoint fPos;
            GrColor fColor;
            SkPoint fOffset;
            SkScalar fOuterRadius;
            SkScalar fBlurRadius;
        };

        int instanceCount = fCircles.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(CircleVertex));

        const GrBuffer* vertexBuffer;
        int firstVertex;
        char* vertices = (char*)target->makeVertexSpace(vertexStride, fVertCount, &vertexBuffer,
                                                        &firstVertex);
        if (!vertices) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const GrBuffer* indexBuffer = nullptr;
        int firstIndex = 0;
        uint16_t* indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }

        int currStartVertex = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Circle& circle = fCircles[i];

            GrColor color = circle.fColor;
            SkScalar outerRadius = circle.fOuterRadius;
            SkScalar innerRadius = circle.fInnerRadius;
            SkScalar blurRadius = circle.fBlurRadius;

            const SkRect& bounds = circle.fDevBounds;
            CircleVertex* ov0 = reinterpret_cast<CircleVertex*>(vertices + 0 * vertexStride);
            CircleVertex* ov1 = reinterpret_cast<CircleVertex*>(vertices + 1 * vertexStride);
            CircleVertex* ov2 = reinterpret_cast<CircleVertex*>(vertices + 2 * vertexStride);
            CircleVertex* ov3 = reinterpret_cast<CircleVertex*>(vertices + 3 * vertexStride);
            CircleVertex* ov4 = reinterpret_cast<CircleVertex*>(vertices + 4 * vertexStride);
            CircleVertex* ov5 = reinterpret_cast<CircleVertex*>(vertices + 5 * vertexStride);
            CircleVertex* ov6 = reinterpret_cast<CircleVertex*>(vertices + 6 * vertexStride);
            CircleVertex* ov7 = reinterpret_cast<CircleVertex*>(vertices + 7 * vertexStride);

            // The inner radius in the vertex data must be specified in normalized space.
            innerRadius = innerRadius / outerRadius;

            SkPoint center = SkPoint::Make(bounds.centerX(), bounds.centerY());
            SkScalar halfWidth = 0.5f * bounds.width();
            SkScalar octOffset = 0.41421356237f;  // sqrt(2) - 1

            ov0->fPos = center + SkPoint::Make(-octOffset * halfWidth, -halfWidth);
            ov0->fColor = color;
            ov0->fOffset = SkPoint::Make(-octOffset, -1);
            ov0->fOuterRadius = outerRadius;
            ov0->fBlurRadius = blurRadius;

            ov1->fPos = center + SkPoint::Make(octOffset * halfWidth, -halfWidth);
            ov1->fColor = color;
            ov1->fOffset = SkPoint::Make(octOffset, -1);
            ov1->fOuterRadius = outerRadius;
            ov1->fBlurRadius = blurRadius;

            ov2->fPos = center + SkPoint::Make(halfWidth, -octOffset * halfWidth);
            ov2->fColor = color;
            ov2->fOffset = SkPoint::Make(1, -octOffset);
            ov2->fOuterRadius = outerRadius;
            ov2->fBlurRadius = blurRadius;

            ov3->fPos = center + SkPoint::Make(halfWidth, octOffset * halfWidth);
            ov3->fColor = color;
            ov3->fOffset = SkPoint::Make(1, octOffset);
            ov3->fOuterRadius = outerRadius;
            ov3->fBlurRadius = blurRadius;

            ov4->fPos = center + SkPoint::Make(octOffset * halfWidth, halfWidth);
            ov4->fColor = color;
            ov4->fOffset = SkPoint::Make(octOffset, 1);
            ov4->fOuterRadius = outerRadius;
            ov4->fBlurRadius = blurRadius;

            ov5->fPos = center + SkPoint::Make(-octOffset * halfWidth, halfWidth);
            ov5->fColor = color;
            ov5->fOffset = SkPoint::Make(-octOffset, 1);
            ov5->fOuterRadius = outerRadius;
            ov5->fBlurRadius = blurRadius;

            ov6->fPos = center + SkPoint::Make(-halfWidth, octOffset * halfWidth);
            ov6->fColor = color;
            ov6->fOffset = SkPoint::Make(-1, octOffset);
            ov6->fOuterRadius = outerRadius;
            ov6->fBlurRadius = blurRadius;

            ov7->fPos = center + SkPoint::Make(-halfWidth, -octOffset * halfWidth);
            ov7->fColor = color;
            ov7->fOffset = SkPoint::Make(-1, -octOffset);
            ov7->fOuterRadius = outerRadius;
            ov7->fBlurRadius = blurRadius;

            if (circle.fStroked) {
                // compute the inner ring
                CircleVertex* iv0 = reinterpret_cast<CircleVertex*>(vertices + 8 * vertexStride);
                CircleVertex* iv1 = reinterpret_cast<CircleVertex*>(vertices + 9 * vertexStride);
                CircleVertex* iv2 = reinterpret_cast<CircleVertex*>(vertices + 10 * vertexStride);
                CircleVertex* iv3 = reinterpret_cast<CircleVertex*>(vertices + 11 * vertexStride);
                CircleVertex* iv4 = reinterpret_cast<CircleVertex*>(vertices + 12 * vertexStride);
                CircleVertex* iv5 = reinterpret_cast<CircleVertex*>(vertices + 13 * vertexStride);
                CircleVertex* iv6 = reinterpret_cast<CircleVertex*>(vertices + 14 * vertexStride);
                CircleVertex* iv7 = reinterpret_cast<CircleVertex*>(vertices + 15 * vertexStride);

                // cosine and sine of pi/8
                SkScalar c = 0.923579533f;
                SkScalar s = 0.382683432f;
                SkScalar r = circle.fInnerRadius;

                iv0->fPos = center + SkPoint::Make(-s * r, -c * r);
                iv0->fColor = color;
                iv0->fOffset = SkPoint::Make(-s * innerRadius, -c * innerRadius);
                iv0->fOuterRadius = outerRadius;
                iv0->fBlurRadius = blurRadius;

                iv1->fPos = center + SkPoint::Make(s * r, -c * r);
                iv1->fColor = color;
                iv1->fOffset = SkPoint::Make(s * innerRadius, -c * innerRadius);
                iv1->fOuterRadius = outerRadius;
                iv1->fBlurRadius = blurRadius;

                iv2->fPos = center + SkPoint::Make(c * r, -s * r);
                iv2->fColor = color;
                iv2->fOffset = SkPoint::Make(c * innerRadius, -s * innerRadius);
                iv2->fOuterRadius = outerRadius;
                iv2->fBlurRadius = blurRadius;

                iv3->fPos = center + SkPoint::Make(c * r, s * r);
                iv3->fColor = color;
                iv3->fOffset = SkPoint::Make(c * innerRadius, s * innerRadius);
                iv3->fOuterRadius = outerRadius;
                iv3->fBlurRadius = blurRadius;

                iv4->fPos = center + SkPoint::Make(s * r, c * r);
                iv4->fColor = color;
                iv4->fOffset = SkPoint::Make(s * innerRadius, c * innerRadius);
                iv4->fOuterRadius = outerRadius;
                iv4->fBlurRadius = blurRadius;

                iv5->fPos = center + SkPoint::Make(-s * r, c * r);
                iv5->fColor = color;
                iv5->fOffset = SkPoint::Make(-s * innerRadius, c * innerRadius);
                iv5->fOuterRadius = outerRadius;
                iv5->fBlurRadius = blurRadius;

                iv6->fPos = center + SkPoint::Make(-c * r, s * r);
                iv6->fColor = color;
                iv6->fOffset = SkPoint::Make(-c * innerRadius, s * innerRadius);
                iv6->fOuterRadius = outerRadius;
                iv6->fBlurRadius = blurRadius;

                iv7->fPos = center + SkPoint::Make(-c * r, -s * r);
                iv7->fColor = color;
                iv7->fOffset = SkPoint::Make(-c * innerRadius, -s * innerRadius);
                iv7->fOuterRadius = outerRadius;
                iv7->fBlurRadius = blurRadius;
            } else {
                // filled
                CircleVertex* iv = reinterpret_cast<CircleVertex*>(vertices + 8 * vertexStride);
                iv->fPos = center;
                iv->fColor = color;
                iv->fOffset = SkPoint::Make(0, 0);
                iv->fOuterRadius = outerRadius;
                iv->fBlurRadius = blurRadius;
            }

            const uint16_t* primIndices = circle_type_to_indices(circle.fStroked);
            const int primIndexCount = circle_type_to_index_count(circle.fStroked);
            for (int i = 0; i < primIndexCount; ++i) {
                *indices++ = primIndices[i] + currStartVertex;
            }

            currStartVertex += circle_type_to_vert_count(circle.fStroked);
            vertices += circle_type_to_vert_count(circle.fStroked) * vertexStride;
        }

        GrMesh mesh;
        mesh.initIndexed(kTriangles_GrPrimitiveType, vertexBuffer, indexBuffer, firstVertex,
                         firstIndex, fVertCount, fIndexCount);
        target->draw(gp.get(), this->pipeline(), mesh);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        ShadowCircleOp* that = t->cast<ShadowCircleOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (!fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fCircles.push_back_n(that->fCircles.count(), that->fCircles.begin());
        this->joinBounds(*that);
        fVertCount += that->fVertCount;
        fIndexCount += that->fIndexCount;
        return true;
    }

    struct Circle {
        GrColor fColor;
        SkScalar fOuterRadius;
        SkScalar fInnerRadius;
        SkScalar fBlurRadius;
        SkRect fDevBounds;
        bool fStroked;
    };

    SkSTArray<1, Circle, true> fCircles;
    SkMatrix fViewMatrixIfUsingLocalCoords;
    int fVertCount;
    int fIndexCount;

    typedef GrLegacyMeshDrawOp INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// We have two possible cases for geometry for a shadow roundrect.
//
// In the case of a normal stroke, we draw the roundrect as a 9-patch without the center quad.
//    ____________
//   |_|________|_|
//   | |        | |
//   | |        | |
//   | |        | |
//   |_|________|_|
//   |_|________|_|
//
// In the case where the stroke width is greater than twice the corner radius (overstroke),
// we add additional geometry to mark out the rectangle in the center. The shared vertices
// are duplicated so we can set a different outer radius for the fill calculation.
//    ____________
//   |_|________|_|
//   | |\ ____ /| |
//   | | |    | | |
//   | | |____| | |
//   |_|/______\|_|
//   |_|________|_|
//
// For filled rrects we reuse the overstroke geometry but make the inner rect degenerate
// (either a point or a horizontal or vertical line).

static const uint16_t gOverstrokeRRectIndices[] = {
        // clang-format off
        // corners
        0, 1, 5, 0, 5, 4,
        2, 3, 7, 2, 7, 6,
        8, 9, 13, 8, 13, 12,
        10, 11, 15, 10, 15, 14,

        // edges
        1, 2, 6, 1, 6, 5,
        4, 5, 9, 4, 9, 8,
        6, 7, 11, 6, 11, 10,
        9, 10, 14, 9, 14, 13,

        // overstroke quads
        // we place this at the end so that we can skip these indices when rendering as stroked
        16, 17, 19, 16, 19, 18,
        19, 17, 23, 19, 23, 21,
        21, 23, 22, 21, 22, 20,
        22, 16, 18, 22, 18, 20,
        // clang-format on
};
// standard stroke indices start at the same place, but will skip the overstroke "ring"
static const uint16_t* gStrokeRRectIndices = gOverstrokeRRectIndices;

// overstroke count
static const int kIndicesPerOverstrokeRRect = SK_ARRAY_COUNT(gOverstrokeRRectIndices);
// simple stroke count skips overstroke indices
static const int kIndicesPerStrokeRRect = kIndicesPerOverstrokeRRect - 6 * 4 + 6;
static const int kVertsPerStrokeRRect = 16;
static const int kVertsPerOverstrokeRRect = 24;

enum RRectType {
    kFill_RRectType,
    kStroke_RRectType,
    kOverstroke_RRectType,
};

static int rrect_type_to_vert_count(RRectType type) {
    switch (type) {
        case kFill_RRectType:
            return kVertsPerOverstrokeRRect;
        case kStroke_RRectType:
            return kVertsPerStrokeRRect;
        case kOverstroke_RRectType:
            return kVertsPerOverstrokeRRect;
    }
    SkFAIL("Invalid type");
    return 0;
}

static int rrect_type_to_index_count(RRectType type) {
    switch (type) {
        case kFill_RRectType:
            return kIndicesPerOverstrokeRRect;
        case kStroke_RRectType:
            return kIndicesPerStrokeRRect;
        case kOverstroke_RRectType:
            return kIndicesPerOverstrokeRRect;
    }
    SkFAIL("Invalid type");
    return 0;
}

static const uint16_t* rrect_type_to_indices(RRectType type) {
    switch (type) {
        case kFill_RRectType:
            return gOverstrokeRRectIndices;
        case kStroke_RRectType:
            return gStrokeRRectIndices;
        case kOverstroke_RRectType:
            return gOverstrokeRRectIndices;
    }
    SkFAIL("Invalid type");
    return nullptr;
}

// For distance computations in the interior of filled rrects we:
//
//   add a interior degenerate (point or line) rect
//   each vertex of that rect gets -outerRad as its radius
//      this makes the computation of the distance to the outer edge be negative
//      negative values are caught and then handled differently in the GP's onEmitCode
//   each vertex is also given the normalized x & y distance from the interior rect's edge
//      the GP takes the min of those depths +1 to get the normalized distance to the outer edge

class ShadowCircularRRectOp final : public GrLegacyMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    // A devStrokeWidth <= 0 indicates a fill only. If devStrokeWidth > 0 then strokeOnly indicates
    // whether the rrect is only stroked or stroked and filled.
    ShadowCircularRRectOp(GrColor color, const SkMatrix& viewMatrix, const SkRect& devRect,
                          float devRadius, float blurRadius, float devStrokeWidth, bool strokeOnly)
            : INHERITED(ClassID()), fViewMatrixIfUsingLocalCoords(viewMatrix) {
        SkRect bounds = devRect;
        SkASSERT(!(devStrokeWidth <= 0 && strokeOnly));
        SkScalar innerRadius = 0.0f;
        SkScalar outerRadius = devRadius;
        SkScalar halfWidth = 0;
        RRectType type = kFill_RRectType;
        if (devStrokeWidth > 0) {
            if (SkScalarNearlyZero(devStrokeWidth)) {
                halfWidth = SK_ScalarHalf;
            } else {
                halfWidth = SkScalarHalf(devStrokeWidth);
            }

            if (strokeOnly) {
                // If stroke is greater than width or height, this is still a fill
                // Otherwise we compute stroke params
                if (devStrokeWidth <= devRect.width() && devStrokeWidth <= devRect.height()) {
                    innerRadius = devRadius - halfWidth;
                    type = (innerRadius >= 0) ? kStroke_RRectType : kOverstroke_RRectType;
                }
            }
            outerRadius += halfWidth;
            bounds.outset(halfWidth, halfWidth);
        }

        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);

        fGeoData.emplace_back(Geometry{color, outerRadius, innerRadius, blurRadius, bounds, type});
        fVertCount = rrect_type_to_vert_count(type);
        fIndexCount = rrect_type_to_index_count(type);
    }

    const char* name() const override { return "ShadowCircularRRectOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fGeoData.count(); ++i) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f],"
                    "OuterRad: %.2f, InnerRad: %.2f, BlurRad: %.2f\n",
                    fGeoData[i].fColor, fGeoData[i].fDevBounds.fLeft, fGeoData[i].fDevBounds.fTop,
                    fGeoData[i].fDevBounds.fRight, fGeoData[i].fDevBounds.fBottom,
                    fGeoData[i].fOuterRadius, fGeoData[i].fInnerRadius, fGeoData[i].fBlurRadius);
        }
        string.append(DumpPipelineInfo(*this->pipeline()));
        string.append(INHERITED::dumpInfo());
        return string;
    }

private:
    void getProcessorAnalysisInputs(GrProcessorAnalysisColor* color,
                                    GrProcessorAnalysisCoverage* coverage) const override {
        color->setToConstant(fGeoData[0].fColor);
        *coverage = GrProcessorAnalysisCoverage::kSingleChannel;
    }

    void applyPipelineOptimizations(const PipelineOptimizations& optimizations) override {
        optimizations.getOverrideColorIfSet(&fGeoData[0].fColor);
        if (!optimizations.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    struct CircleVertex {
        SkPoint fPos;
        GrColor fColor;
        SkPoint fOffset;
        SkScalar fOuterRadius;
        SkScalar fBlurRadius;
    };

    static void FillInOverstrokeVerts(CircleVertex** verts, const SkRect& bounds, SkScalar smInset,
                                      SkScalar bigInset, SkScalar xOffset, SkScalar outerRadius,
                                      GrColor color, SkScalar blurRadius) {
        SkASSERT(smInset < bigInset);

        // TL
        (*verts)->fPos = SkPoint::Make(bounds.fLeft + smInset, bounds.fTop + smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;

        // TR
        (*verts)->fPos = SkPoint::Make(bounds.fRight - smInset, bounds.fTop + smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fLeft + bigInset, bounds.fTop + bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fRight - bigInset, bounds.fTop + bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fLeft + bigInset, bounds.fBottom - bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;

        (*verts)->fPos = SkPoint::Make(bounds.fRight - bigInset, bounds.fBottom - bigInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;

        // BL
        (*verts)->fPos = SkPoint::Make(bounds.fLeft + smInset, bounds.fBottom - smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;

        // BR
        (*verts)->fPos = SkPoint::Make(bounds.fRight - smInset, bounds.fBottom - smInset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(xOffset, 0);
        (*verts)->fOuterRadius = outerRadius;
        (*verts)->fBlurRadius = blurRadius;
        (*verts)++;
    }

    void onPrepareDraws(Target* target) const override {
        // Invert the view matrix as a local matrix (if any other processors require coords).
        SkMatrix localMatrix;
        if (!fViewMatrixIfUsingLocalCoords.invert(&localMatrix)) {
            return;
        }

        // Setup geometry processor
        sk_sp<GrGeometryProcessor> gp(GrRRectShadowGeoProc::Make(localMatrix));

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(sizeof(CircleVertex) == vertexStride);

        const GrBuffer* vertexBuffer;
        int firstVertex;

        CircleVertex* verts = (CircleVertex*)target->makeVertexSpace(vertexStride, fVertCount,
                                                                     &vertexBuffer, &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        const GrBuffer* indexBuffer = nullptr;
        int firstIndex = 0;
        uint16_t* indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }

        int currStartVertex = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            GrColor color = args.fColor;
            SkScalar outerRadius = args.fOuterRadius;

            const SkRect& bounds = args.fDevBounds;

            SkScalar yCoords[4] = {bounds.fTop, bounds.fTop + outerRadius,
                                   bounds.fBottom - outerRadius, bounds.fBottom};

            SkScalar yOuterRadii[4] = {-1, 0, 0, 1};
            // The inner radius in the vertex data must be specified in normalized space.
            // For fills, specifying -1/outerRadius guarantees an alpha of 1.0 at the inner radius.
            SkScalar blurRadius = args.fBlurRadius;
            for (int i = 0; i < 4; ++i) {
                verts->fPos = SkPoint::Make(bounds.fLeft, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(-1, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fBlurRadius = blurRadius;
                verts++;

                verts->fPos = SkPoint::Make(bounds.fLeft + outerRadius, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(0, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fBlurRadius = blurRadius;
                verts++;

                verts->fPos = SkPoint::Make(bounds.fRight - outerRadius, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(0, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fBlurRadius = blurRadius;
                verts++;

                verts->fPos = SkPoint::Make(bounds.fRight, yCoords[i]);
                verts->fColor = color;
                verts->fOffset = SkPoint::Make(1, yOuterRadii[i]);
                verts->fOuterRadius = outerRadius;
                verts->fBlurRadius = blurRadius;
                verts++;
            }
            // Add the additional vertices for overstroked rrects.
            // Effectively this is an additional stroked rrect, with its
            // outer radius = outerRadius - innerRadius, and inner radius = 0.
            // This will give us correct AA in the center and the correct
            // distance to the outer edge.
            //
            // Also, the outer offset is a constant vector pointing to the right, which
            // guarantees that the distance value along the outer rectangle is constant.
            if (kOverstroke_RRectType == args.fType) {
                SkASSERT(args.fInnerRadius <= 0.0f);

                SkScalar overstrokeOuterRadius = outerRadius - args.fInnerRadius;
                // this is the normalized distance from the outer rectangle of this
                // geometry to the outer edge
                SkScalar maxOffset = -args.fInnerRadius / overstrokeOuterRadius;

                FillInOverstrokeVerts(&verts, bounds, outerRadius, overstrokeOuterRadius, maxOffset,
                                      overstrokeOuterRadius, color, blurRadius);
            }

            if (kFill_RRectType == args.fType) {
                SkScalar halfMinDim = 0.5f * SkTMin(bounds.width(), bounds.height());

                SkScalar xOffset = 1.0f - outerRadius / halfMinDim;

                FillInOverstrokeVerts(&verts, bounds, outerRadius, halfMinDim, xOffset, halfMinDim,
                                      color, blurRadius);
            }

            const uint16_t* primIndices = rrect_type_to_indices(args.fType);
            const int primIndexCount = rrect_type_to_index_count(args.fType);
            for (int i = 0; i < primIndexCount; ++i) {
                *indices++ = primIndices[i] + currStartVertex;
            }

            currStartVertex += rrect_type_to_vert_count(args.fType);
        }

        GrMesh mesh;
        mesh.initIndexed(kTriangles_GrPrimitiveType, vertexBuffer, indexBuffer, firstVertex,
                         firstIndex, fVertCount, fIndexCount);
        target->draw(gp.get(), this->pipeline(), mesh);
    }

    bool onCombineIfPossible(GrOp* t, const GrCaps& caps) override {
        ShadowCircularRRectOp* that = t->cast<ShadowCircularRRectOp>();
        if (!GrPipeline::CanCombine(*this->pipeline(), this->bounds(), *that->pipeline(),
                                    that->bounds(), caps)) {
            return false;
        }

        if (!fViewMatrixIfUsingLocalCoords.cheapEqualTo(that->fViewMatrixIfUsingLocalCoords)) {
            return false;
        }

        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        this->joinBounds(*that);
        fVertCount += that->fVertCount;
        fIndexCount += that->fIndexCount;
        return true;
    }

    struct Geometry {
        GrColor fColor;
        SkScalar fOuterRadius;
        SkScalar fInnerRadius;
        SkScalar fBlurRadius;
        SkRect fDevBounds;
        RRectType fType;
    };

    SkSTArray<1, Geometry, true> fGeoData;
    SkMatrix fViewMatrixIfUsingLocalCoords;
    int fVertCount;
    int fIndexCount;

    typedef GrLegacyMeshDrawOp INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrLegacyMeshDrawOp> make_shadow_circle_op(GrColor color,
                                                          const SkMatrix& viewMatrix,
                                                          const SkRect& oval,
                                                          SkScalar blurRadius,
                                                          const SkStrokeRec& stroke,
                                                          const GrShaderCaps* shaderCaps) {
    // we can only draw circles
    SkScalar width = oval.width();
    SkASSERT(SkScalarNearlyEqual(width, oval.height()) && viewMatrix.isSimilarity());
    SkPoint center = {oval.centerX(), oval.centerY()};
    return ShadowCircleOp::Make(color, viewMatrix, center, width / 2.f, blurRadius,
                                GrStyle(stroke, nullptr));
}

static std::unique_ptr<GrLegacyMeshDrawOp> make_shadow_rrect_op(GrColor color,
                                                                const SkMatrix& viewMatrix,
                                                                const SkRRect& rrect,
                                                                SkScalar blurRadius,
                                                                const SkStrokeRec& stroke) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(rrect.isSimple());
    SkASSERT(!rrect.isOval());

    // Shadow rrect ops only handle simple circular rrects.
    // Do any matrix crunching before we reset the draw state for device coords.
    const SkRect& rrectBounds = rrect.getBounds();
    SkRect bounds;
    viewMatrix.mapRect(&bounds, rrectBounds);

    SkVector radii = rrect.getSimpleRadii();
    SkScalar xRadius = SkScalarAbs(viewMatrix[SkMatrix::kMScaleX] * radii.fX +
                                   viewMatrix[SkMatrix::kMSkewY] * radii.fY);
    SkScalar yRadius = SkScalarAbs(viewMatrix[SkMatrix::kMSkewX] * radii.fX +
                                   viewMatrix[SkMatrix::kMScaleY] * radii.fY);
    SkASSERT(SkScalarNearlyEqual(xRadius, yRadius));

    SkStrokeRec::Style style = stroke.getStyle();

    // Do (potentially) anisotropic mapping of stroke. Use -1s to indicate fill-only draws.
    SkVector scaledStroke = {-1, -1};
    SkScalar strokeWidth = stroke.getWidth();

    bool isStrokeOnly =
            SkStrokeRec::kStroke_Style == style || SkStrokeRec::kHairline_Style == style;
    bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == style;

    if (hasStroke) {
        if (SkStrokeRec::kHairline_Style == style) {
            scaledStroke.set(1, 1);
        } else {
            scaledStroke.fX = SkScalarAbs(
                    strokeWidth * (viewMatrix[SkMatrix::kMScaleX] + viewMatrix[SkMatrix::kMSkewY]));
            scaledStroke.fY = SkScalarAbs(
                    strokeWidth * (viewMatrix[SkMatrix::kMSkewX] + viewMatrix[SkMatrix::kMScaleY]));
        }

        // we don't handle anisotropic strokes
        if (!SkScalarNearlyEqual(scaledStroke.fX, scaledStroke.fY)) {
            return nullptr;
        }
    }

    // The way the effect interpolates the offset-to-ellipse/circle-center attribute only works on
    // the interior of the rrect if the radii are >= 0.5. Otherwise, the inner rect of the nine-
    // patch will have fractional coverage. This only matters when the interior is actually filled.
    // We could consider falling back to rect rendering here, since a tiny radius is
    // indistinguishable from a square corner.
    if (!isStrokeOnly && (SK_ScalarHalf > xRadius || SK_ScalarHalf > yRadius)) {
        return nullptr;
    }

    return std::unique_ptr<GrLegacyMeshDrawOp>(new ShadowCircularRRectOp(
            color, viewMatrix, bounds, xRadius, blurRadius, scaledStroke.fX, isStrokeOnly));
}

namespace GrShadowRRectOp {
std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRRect& rrect,
                                         const SkScalar blurRadius,
                                         const SkStrokeRec& stroke,
                                         const GrShaderCaps* shaderCaps) {
    if (rrect.isOval()) {
        return make_shadow_circle_op(color, viewMatrix, rrect.getBounds(), blurRadius, stroke,
                                     shaderCaps);
    }

    if (!viewMatrix.rectStaysRect() || !rrect.isSimple()) {
        return nullptr;
    }

    return make_shadow_rrect_op(color, viewMatrix, rrect, blurRadius, stroke);
}
}
///////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

DRAW_OP_TEST_DEFINE(ShadowCircleOp) {
    do {
        SkScalar rotate = random->nextSScalar1() * 360.f;
        SkScalar translateX = random->nextSScalar1() * 1000.f;
        SkScalar translateY = random->nextSScalar1() * 1000.f;
        SkScalar scale = random->nextSScalar1() * 100.f;
        SkMatrix viewMatrix;
        viewMatrix.setRotate(rotate);
        viewMatrix.postTranslate(translateX, translateY);
        viewMatrix.postScale(scale, scale);
        GrColor color = GrRandomColor(random);
        SkRect circle = GrTest::TestSquare(random);
        SkPoint center = {circle.centerX(), circle.centerY()};
        SkScalar radius = circle.width() / 2.f;
        SkStrokeRec stroke = GrTest::TestStrokeRec(random);
        SkScalar blurRadius = random->nextSScalar1() * 72.f;
        std::unique_ptr<GrLegacyMeshDrawOp> op = ShadowCircleOp::Make(
                color, viewMatrix, center, radius, blurRadius, GrStyle(stroke, nullptr));
        if (op) {
            return op;
        }
    } while (true);
}

DRAW_OP_TEST_DEFINE(ShadowRRectOp) {
    SkMatrix viewMatrix = GrTest::TestMatrixRectStaysRect(random);
    GrColor color = GrRandomColor(random);
    const SkRRect& rrect = GrTest::TestRRectSimple(random);
    SkScalar blurRadius = random->nextSScalar1() * 72.f;
    return make_shadow_rrect_op(color, viewMatrix, rrect, blurRadius,
                                GrTest::TestStrokeRec(random));
}

#endif
