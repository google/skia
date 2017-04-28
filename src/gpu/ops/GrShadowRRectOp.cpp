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
            SkScalar fDistanceCorrection;
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
            SkScalar distanceCorrection = outerRadius / blurRadius;

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
            ov0->fDistanceCorrection = distanceCorrection;

            ov1->fPos = center + SkPoint::Make(octOffset * halfWidth, -halfWidth);
            ov1->fColor = color;
            ov1->fOffset = SkPoint::Make(octOffset, -1);
            ov1->fDistanceCorrection = distanceCorrection;

            ov2->fPos = center + SkPoint::Make(halfWidth, -octOffset * halfWidth);
            ov2->fColor = color;
            ov2->fOffset = SkPoint::Make(1, -octOffset);
            ov2->fDistanceCorrection = distanceCorrection;

            ov3->fPos = center + SkPoint::Make(halfWidth, octOffset * halfWidth);
            ov3->fColor = color;
            ov3->fOffset = SkPoint::Make(1, octOffset);
            ov3->fDistanceCorrection = distanceCorrection;

            ov4->fPos = center + SkPoint::Make(octOffset * halfWidth, halfWidth);
            ov4->fColor = color;
            ov4->fOffset = SkPoint::Make(octOffset, 1);
            ov4->fDistanceCorrection = distanceCorrection;

            ov5->fPos = center + SkPoint::Make(-octOffset * halfWidth, halfWidth);
            ov5->fColor = color;
            ov5->fOffset = SkPoint::Make(-octOffset, 1);
            ov5->fDistanceCorrection = distanceCorrection;

            ov6->fPos = center + SkPoint::Make(-halfWidth, octOffset * halfWidth);
            ov6->fColor = color;
            ov6->fOffset = SkPoint::Make(-1, octOffset);
            ov6->fDistanceCorrection = distanceCorrection;

            ov7->fPos = center + SkPoint::Make(-halfWidth, -octOffset * halfWidth);
            ov7->fColor = color;
            ov7->fOffset = SkPoint::Make(-1, -octOffset);
            ov7->fDistanceCorrection = distanceCorrection;

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
                iv0->fDistanceCorrection = distanceCorrection;

                iv1->fPos = center + SkPoint::Make(s * r, -c * r);
                iv1->fColor = color;
                iv1->fOffset = SkPoint::Make(s * innerRadius, -c * innerRadius);
                iv1->fDistanceCorrection = distanceCorrection;

                iv2->fPos = center + SkPoint::Make(c * r, -s * r);
                iv2->fColor = color;
                iv2->fOffset = SkPoint::Make(c * innerRadius, -s * innerRadius);
                iv2->fDistanceCorrection = distanceCorrection;

                iv3->fPos = center + SkPoint::Make(c * r, s * r);
                iv3->fColor = color;
                iv3->fOffset = SkPoint::Make(c * innerRadius, s * innerRadius);
                iv3->fDistanceCorrection = distanceCorrection;

                iv4->fPos = center + SkPoint::Make(s * r, c * r);
                iv4->fColor = color;
                iv4->fOffset = SkPoint::Make(s * innerRadius, c * innerRadius);
                iv4->fDistanceCorrection = distanceCorrection;

                iv5->fPos = center + SkPoint::Make(-s * r, c * r);
                iv5->fColor = color;
                iv5->fOffset = SkPoint::Make(-s * innerRadius, c * innerRadius);
                iv5->fDistanceCorrection = distanceCorrection;

                iv6->fPos = center + SkPoint::Make(-c * r, s * r);
                iv6->fColor = color;
                iv6->fOffset = SkPoint::Make(-c * innerRadius, s * innerRadius);
                iv6->fDistanceCorrection = distanceCorrection;

                iv7->fPos = center + SkPoint::Make(-c * r, -s * r);
                iv7->fColor = color;
                iv7->fOffset = SkPoint::Make(-c * innerRadius, -s * innerRadius);
                iv7->fDistanceCorrection = distanceCorrection;
            } else {
                // filled
                CircleVertex* iv = reinterpret_cast<CircleVertex*>(vertices + 8 * vertexStride);
                iv->fPos = center;
                iv->fColor = color;
                iv->fOffset = SkPoint::Make(0, 0);
                iv->fDistanceCorrection = distanceCorrection;
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
// The geometry for a shadow roundrect is similar to a 9-patch:
//    ____________
//   |_|________|_|
//   | |        | |
//   | |        | |
//   | |        | |
//   |_|________|_|
//   |_|________|_|
//
// However, each corner is rendered as a fan rather than a simple quad, as below. (The diagram
// shows the upper part of the upper left corner. The bottom triangle would similarly be split
// into two triangles.)
//    ________
//   |\  \   |
//   |  \ \  |
//   |    \\ |
//   |      \|
//   --------
//
// The center of the fan handles the curve of the corner. For roundrects where the stroke width
// is greater than the corner radius, the outer triangles blend from the curve to the straight
// sides. Otherwise these triangles will be degenerate.
//
// In the case where the stroke width is greater than the corner radius and the
// blur radius (overstroke), we add additional geometry to mark out the rectangle in the center.
// This rectangle extends the coverage values of the center edges of the 9-patch.
//    ____________
//   |_|________|_|
//   | |\ ____ /| |
//   | | |    | | |
//   | | |____| | |
//   |_|/______\|_|
//   |_|________|_|
//
// For filled rrects we reuse the stroke geometry but add an additional quad to the center.

static const uint16_t gRRectIndices[] = {
    // clang-format off
    // overstroke quads
    // we place this at the beginning so that we can skip these indices when rendering as filled
    0, 6, 25, 0, 25, 24,
    6, 18, 27, 6, 27, 25,
    18, 12, 26, 18, 26, 27,
    12, 0, 24, 12, 24, 26,

    // corners
    0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5,
    6, 11, 10, 6, 10, 9, 6, 9, 8, 6, 8, 7,
    12, 17, 16, 12, 16, 15, 12, 15, 14, 12, 14, 13,
    18, 19, 20, 18, 20, 21, 18, 21, 22, 18, 22, 23,

    // edges
    0, 5, 11, 0, 11, 6,
    6, 7, 19, 6, 19, 18,
    18, 23, 17, 18, 17, 12,
    12, 13, 1, 12, 1, 0,

    // fill quad
    // we place this at the end so that we can skip these indices when rendering as stroked
    0, 6, 18, 0, 18, 12,
    // clang-format on
};

// overstroke count
static const int kIndicesPerOverstrokeRRect = SK_ARRAY_COUNT(gRRectIndices) - 6;
// simple stroke count skips overstroke indices
static const int kIndicesPerStrokeRRect = kIndicesPerOverstrokeRRect - 6*4;
// fill count adds final quad to stroke count
static const int kIndicesPerFillRRect = kIndicesPerStrokeRRect + 6;
static const int kVertsPerStrokeRRect = 24;
static const int kVertsPerOverstrokeRRect = 28;
static const int kVertsPerFillRRect = 24;

enum RRectType {
    kFill_RRectType,
    kStroke_RRectType,
    kOverstroke_RRectType,
};

static int rrect_type_to_vert_count(RRectType type) {
    switch (type) {
        case kFill_RRectType:
            return kVertsPerFillRRect;
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
            return kIndicesPerFillRRect;
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
        case kStroke_RRectType:
            return gRRectIndices + 6*4;
        case kOverstroke_RRectType:
            return gRRectIndices;
    }
    SkFAIL("Invalid type");
    return nullptr;
}

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
        SkScalar overStrokeRadius = 0.0f;
        SkScalar outerRadius = devRadius;
        SkScalar umbraInset = SkTMax(outerRadius, blurRadius);
        SkScalar halfWidth = 0;
        RRectType type = kFill_RRectType;
        if (devStrokeWidth > 0) {
            if (SkScalarNearlyZero(devStrokeWidth)) {
                halfWidth = SK_ScalarHalf;
            } else {
                halfWidth = SkScalarHalf(devStrokeWidth);
            }
            outerRadius += halfWidth;
            bounds.outset(halfWidth, halfWidth);

            // If the client has requested a stroke smaller than the outer radius,
            // we will assume they want no special umbra inset (this is for ambient shadows).
            if (devStrokeWidth <= outerRadius) {
                umbraInset = outerRadius;
            }

            if (strokeOnly) {
                // If stroke is greater than width or height, this is still a fill,
                // otherwise we compute stroke params.
                if (devStrokeWidth <= devRect.width() && devStrokeWidth <= devRect.height()) {
                    // We don't worry about an inner radius, we just need to know if we
                    // need to create overstroke vertices.
                    overStrokeRadius = SkTMax(devStrokeWidth - umbraInset,
                                              0.0f);
                    type = overStrokeRadius > 0 ? kOverstroke_RRectType : kStroke_RRectType;
                }
            }
        }

        this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);

        fGeoData.emplace_back(Geometry{color, outerRadius, umbraInset, overStrokeRadius,
                                       blurRadius, bounds, type});
        fVertCount = rrect_type_to_vert_count(type);
        fIndexCount = rrect_type_to_index_count(type);
    }

    const char* name() const override { return "ShadowCircularRRectOp"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fGeoData.count(); ++i) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f],"
                    "OuterRad: %.2f, Umbra: %.2f, Overstroke: %.2f, BlurRad: %.2f\n",
                    fGeoData[i].fColor, fGeoData[i].fDevBounds.fLeft, fGeoData[i].fDevBounds.fTop,
                    fGeoData[i].fDevBounds.fRight, fGeoData[i].fDevBounds.fBottom,
                    fGeoData[i].fOuterRadius, fGeoData[i].fUmbraInset,
                    fGeoData[i].fOverstroke, fGeoData[i].fBlurRadius);
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
        SkScalar fDistanceCorrection;
    };

    static void FillInOverstrokeVerts(CircleVertex** verts, const SkRect& bounds, SkScalar inset,
                                      GrColor color, SkScalar distanceCorrection) {
        // TL
        (*verts)->fPos = SkPoint::Make(bounds.fLeft + inset, bounds.fTop + inset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        // TR
        (*verts)->fPos = SkPoint::Make(bounds.fRight - inset, bounds.fTop + inset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        // BL
        (*verts)->fPos = SkPoint::Make(bounds.fLeft + inset, bounds.fBottom - inset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        // BR
        (*verts)->fPos = SkPoint::Make(bounds.fRight - inset, bounds.fBottom - inset);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(0, 0);
        (*verts)->fDistanceCorrection = distanceCorrection;
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

            SkScalar umbraInset = args.fUmbraInset;

            SkScalar minDim = 0.5f*SkTMin(bounds.width(), bounds.height());
            if (umbraInset > minDim) {
                umbraInset = minDim;
            }

            SkScalar xInner[4] = { bounds.fLeft + umbraInset, bounds.fRight - umbraInset,
                                   bounds.fLeft + umbraInset, bounds.fRight - umbraInset };
            SkScalar xMid[4]   = { bounds.fLeft + outerRadius, bounds.fRight - outerRadius,
                                   bounds.fLeft + outerRadius, bounds.fRight - outerRadius};
            SkScalar xOuter[4] = { bounds.fLeft, bounds.fRight,
                                   bounds.fLeft, bounds.fRight };
            SkScalar yInner[4] = { bounds.fTop + umbraInset, bounds.fTop + umbraInset,
                                   bounds.fBottom - umbraInset, bounds.fBottom - umbraInset };
            SkScalar yMid[4]   = { bounds.fTop + outerRadius, bounds.fTop + outerRadius,
                                   bounds.fBottom - outerRadius, bounds.fBottom - outerRadius };
            SkScalar yOuter[4] = { bounds.fTop, bounds.fTop,
                                   bounds.fBottom, bounds.fBottom };

            SkScalar blurRadius = args.fBlurRadius;

            // In the case where we have to inset more for the umbra, our two triangles in the
            // corner get skewed to a diamond rather than a square. To correct for that,
            // we also skew the vectors we send to the shader that help define the circle.
            // By doing so, we end up with a quarter circle in the corner rather than the
            // elliptical curve.
            SkVector outerVec = SkVector::Make(0.5f*(outerRadius - umbraInset), -umbraInset);
            outerVec.normalize();
            SkVector diagVec = SkVector::Make(outerVec.fX + outerVec.fY,
                                              outerVec.fX + outerVec.fY);
            diagVec *= umbraInset / (2 * umbraInset - outerRadius);
            SkScalar distanceCorrection = umbraInset / blurRadius;

            // build corner by corner
            for (int i = 0; i < 4; ++i) {
                // inner point
                verts->fPos = SkPoint::Make(xInner[i], yInner[i]);
                verts->fColor = color;
                verts->fOffset = SkVector::Make(0, 0);
                verts->fDistanceCorrection = distanceCorrection;
                verts++;

                // outer points
                verts->fPos = SkPoint::Make(xOuter[i], yInner[i]);
                verts->fColor = color;
                verts->fOffset = SkVector::Make(0, -1);
                verts->fDistanceCorrection = distanceCorrection;
                verts++;

                verts->fPos = SkPoint::Make(xOuter[i], yMid[i]);
                verts->fColor = color;
                verts->fOffset = outerVec;
                verts->fDistanceCorrection = distanceCorrection;
                verts++;

                verts->fPos = SkPoint::Make(xOuter[i], yOuter[i]);
                verts->fColor = color;
                verts->fOffset = diagVec;
                verts->fDistanceCorrection = distanceCorrection;
                verts++;

                verts->fPos = SkPoint::Make(xMid[i], yOuter[i]);
                verts->fColor = color;
                verts->fOffset = outerVec;
                verts->fDistanceCorrection = distanceCorrection;
                verts++;

                verts->fPos = SkPoint::Make(xInner[i], yOuter[i]);
                verts->fColor = color;
                verts->fOffset = SkVector::Make(0, -1);
                verts->fDistanceCorrection = distanceCorrection;
                verts++;
            }

            // Add the additional vertices for overstroked rrects.
            // Effectively this is an additional stroked rrect, with its
            // parameters equal to those in the center of the 9-patch. This will
            // give constant values across this inner ring.
            if (kOverstroke_RRectType == args.fType) {
                SkASSERT(args.fOverstroke > 0.0f);

                FillInOverstrokeVerts(&verts, bounds, umbraInset + args.fOverstroke,
                                      color, distanceCorrection);
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
        SkScalar fUmbraInset;
        SkScalar fOverstroke;
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
                                   viewMatrix[SkMatrix::kMSkewX] * radii.fY);
    SkDEBUGCODE(SkScalar yRadius = SkScalarAbs(viewMatrix[SkMatrix::kMSkewY] * radii.fX +
                                               viewMatrix[SkMatrix::kMScaleY] * radii.fY));
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
    if (!isStrokeOnly && SK_ScalarHalf > xRadius) {
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
