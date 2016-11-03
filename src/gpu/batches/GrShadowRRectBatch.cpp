/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShadowRRectBatch.h"

#include "GrBatchFlushState.h"
#include "GrBatchTest.h"
#include "GrResourceProvider.h"
#include "GrStyle.h"

#include "effects/GrShadowGeoProc.h"

///////////////////////////////////////////////////////////////////////////////

// We have two possible cases for geometry for a circle:

// In the case of a normal fill, we draw geometry for the circle as an octagon.
static const uint16_t gFillCircleIndices[] = {
    // enter the octagon
    0, 1, 8, 1, 2, 8,
    2, 3, 8, 3, 4, 8,
    4, 5, 8, 5, 6, 8,
    6, 7, 8, 7, 0, 8,
};

// For stroked circles, we use two nested octagons.
static const uint16_t gStrokeCircleIndices[] = {
    // enter the octagon
    0, 1, 9, 0, 9, 8,
    1, 2, 10, 1, 10, 9,
    2, 3, 11, 2, 11, 10,
    3, 4, 12, 3, 12, 11,
    4, 5, 13, 4, 13, 12,
    5, 6, 14, 5, 14, 13,
    6, 7, 15, 6, 15, 14,
    7, 0, 8, 7, 8, 15,
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

class ShadowCircleBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    static GrDrawBatch* Create(GrColor color, const SkMatrix& viewMatrix, SkPoint center,
                               SkScalar radius, SkScalar blurRadius, const GrStyle& style) {
        SkASSERT(viewMatrix.isSimilarity());
        const SkStrokeRec& stroke = style.strokeRec();
        if (style.hasPathEffect()) {
            return nullptr;
        }
        SkStrokeRec::Style recStyle = stroke.getStyle();

        viewMatrix.mapPoints(&center, 1);
        radius = viewMatrix.mapRadius(radius);
        SkScalar strokeWidth = viewMatrix.mapRadius(stroke.getWidth());

        bool isStrokeOnly = SkStrokeRec::kStroke_Style == recStyle ||
            SkStrokeRec::kHairline_Style == recStyle;
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

        // TODO: still needed?
        // The radii are outset for two reasons. First, it allows the shader to simply perform
        // simpler computation because the computed alpha is zero, rather than 50%, at the radius.
        // Second, the outer radius is used to compute the verts of the bounding box that is
        // rendered and the outset ensures the box will cover all partially covered by the circle.
        outerRadius += SK_ScalarHalf;
        innerRadius -= SK_ScalarHalf;
        bool stroked = isStrokeOnly && innerRadius > 0.0f;
        ShadowCircleBatch* batch = new ShadowCircleBatch();
        batch->fViewMatrixIfUsingLocalCoords = viewMatrix;

        // This makes every point fully inside the intersection plane.
        SkRect devBounds = SkRect::MakeLTRB(center.fX - outerRadius, center.fY - outerRadius,
                                            center.fX + outerRadius, center.fY + outerRadius);

        batch->fGeoData.emplace_back(Geometry{
            color,
            outerRadius,
            innerRadius,
            blurRadius,
            devBounds,
            stroked
        });

        // Use the original radius and stroke radius for the bounds so that it does not include the
        // AA bloat.
        radius += halfWidth;
        batch->setBounds({ center.fX - radius, center.fY - radius,
                         center.fX + radius, center.fY + radius },
                         HasAABloat::kYes, IsZeroArea::kNo);
        batch->fVertCount = circle_type_to_vert_count(stroked);
        batch->fIndexCount = circle_type_to_index_count(stroked);
        batch->fAllFill = !stroked;
        return batch;
    }

    const char* name() const override { return "CircleBatch"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fGeoData.count(); ++i) {
            string.appendf("Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f], "
                           "OuterRad: %.2f, InnerRad: %.2f, BlurRad: %.2f\n",
                           fGeoData[i].fColor,
                           fGeoData[i].fDevBounds.fLeft, fGeoData[i].fDevBounds.fTop,
                           fGeoData[i].fDevBounds.fRight, fGeoData[i].fDevBounds.fBottom,
                           fGeoData[i].fOuterRadius, fGeoData[i].fInnerRadius,
                           fGeoData[i].fBlurRadius);
        }
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    ShadowCircleBatch() : INHERITED(ClassID()) {}
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any overrides that affect our GP.
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        if (!overrides.readsLocalCoords()) {
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
            SkPoint  fPos;
            GrColor  fColor;
            SkPoint  fOffset;
            SkScalar fOuterRadius;
            SkScalar fBlurRadius;
        };

        int instanceCount = fGeoData.count();
        size_t vertexStride = gp->getVertexStride();
        SkASSERT(vertexStride == sizeof(CircleVertex));

        const GrBuffer* vertexBuffer;
        int firstVertex;
        char* vertices = (char*)target->makeVertexSpace(vertexStride, fVertCount,
                                                        &vertexBuffer, &firstVertex);
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
            const Geometry& geom = fGeoData[i];

            GrColor color = geom.fColor;
            SkScalar outerRadius = geom.fOuterRadius;
            SkScalar innerRadius = geom.fInnerRadius;
            SkScalar blurRadius = geom.fBlurRadius;

            const SkRect& bounds = geom.fDevBounds;
            CircleVertex* v0 = reinterpret_cast<CircleVertex*>(vertices + 0 * vertexStride);
            CircleVertex* v1 = reinterpret_cast<CircleVertex*>(vertices + 1 * vertexStride);
            CircleVertex* v2 = reinterpret_cast<CircleVertex*>(vertices + 2 * vertexStride);
            CircleVertex* v3 = reinterpret_cast<CircleVertex*>(vertices + 3 * vertexStride);
            CircleVertex* v4 = reinterpret_cast<CircleVertex*>(vertices + 4 * vertexStride);
            CircleVertex* v5 = reinterpret_cast<CircleVertex*>(vertices + 5 * vertexStride);
            CircleVertex* v6 = reinterpret_cast<CircleVertex*>(vertices + 6 * vertexStride);
            CircleVertex* v7 = reinterpret_cast<CircleVertex*>(vertices + 7 * vertexStride);

            // The inner radius in the vertex data must be specified in normalized space.
            innerRadius = innerRadius / outerRadius;
            
            SkPoint center = SkPoint::Make(bounds.centerX(), bounds.centerY());
            SkScalar halfWidth = 0.5f*bounds.width();
            SkScalar octOffset = 0.41421356237f;  // sqrt(2) - 1

            v0->fPos = center + SkPoint::Make(-octOffset*halfWidth, -halfWidth);
            v0->fColor = color;
            v0->fOffset = SkPoint::Make(-octOffset, -1);
            v0->fOuterRadius = outerRadius;
            v0->fBlurRadius = blurRadius;

            v1->fPos = center + SkPoint::Make(octOffset*halfWidth, -halfWidth);
            v1->fColor = color;
            v1->fOffset = SkPoint::Make(octOffset, -1);
            v1->fOuterRadius = outerRadius;
            v1->fBlurRadius = blurRadius;

            v2->fPos = center + SkPoint::Make(halfWidth, -octOffset*halfWidth);
            v2->fColor = color;
            v2->fOffset = SkPoint::Make(1, -octOffset);
            v2->fOuterRadius = outerRadius;
            v2->fBlurRadius = blurRadius;

            v3->fPos = center + SkPoint::Make(halfWidth, octOffset*halfWidth);
            v3->fColor = color;
            v3->fOffset = SkPoint::Make(1, octOffset);
            v3->fOuterRadius = outerRadius;
            v3->fBlurRadius = blurRadius;

            v4->fPos = center + SkPoint::Make(octOffset*halfWidth, halfWidth);
            v4->fColor = color;
            v4->fOffset = SkPoint::Make(octOffset, 1);
            v4->fOuterRadius = outerRadius;
            v4->fBlurRadius = blurRadius;

            v5->fPos = center + SkPoint::Make(-octOffset*halfWidth, halfWidth);
            v5->fColor = color;
            v5->fOffset = SkPoint::Make(-octOffset, 1);
            v5->fOuterRadius = outerRadius;
            v5->fBlurRadius = blurRadius;

            v6->fPos = center + SkPoint::Make(-halfWidth, octOffset*halfWidth);
            v6->fColor = color;
            v6->fOffset = SkPoint::Make(-1, octOffset);
            v6->fOuterRadius = outerRadius;
            v6->fBlurRadius = blurRadius;

            v7->fPos = center + SkPoint::Make(-halfWidth, -octOffset*halfWidth);
            v7->fColor = color;
            v7->fOffset = SkPoint::Make(-1, -octOffset);
            v7->fOuterRadius = outerRadius;
            v7->fBlurRadius = blurRadius;

            if (geom.fStroked) {
                // compute the inner ring
                CircleVertex* v0 = reinterpret_cast<CircleVertex*>(vertices + 8 * vertexStride);
                CircleVertex* v1 = reinterpret_cast<CircleVertex*>(vertices + 9 * vertexStride);
                CircleVertex* v2 = reinterpret_cast<CircleVertex*>(vertices + 10 * vertexStride);
                CircleVertex* v3 = reinterpret_cast<CircleVertex*>(vertices + 11 * vertexStride);
                CircleVertex* v4 = reinterpret_cast<CircleVertex*>(vertices + 12 * vertexStride);
                CircleVertex* v5 = reinterpret_cast<CircleVertex*>(vertices + 13 * vertexStride);
                CircleVertex* v6 = reinterpret_cast<CircleVertex*>(vertices + 14 * vertexStride);
                CircleVertex* v7 = reinterpret_cast<CircleVertex*>(vertices + 15 * vertexStride);

                // cosine and sine of pi/8
                SkScalar c = 0.923579533f;
                SkScalar s = 0.382683432f;
                SkScalar r = geom.fInnerRadius;

                v0->fPos = center + SkPoint::Make(-s*r, -c*r);
                v0->fColor = color;
                v0->fOffset = SkPoint::Make(-s*innerRadius, -c*innerRadius);
                v0->fOuterRadius = outerRadius;
                v0->fBlurRadius = blurRadius;

                v1->fPos = center + SkPoint::Make(s*r, -c*r);
                v1->fColor = color;
                v1->fOffset = SkPoint::Make(s*innerRadius, -c*innerRadius);
                v1->fOuterRadius = outerRadius;
                v1->fBlurRadius = blurRadius;

                v2->fPos = center + SkPoint::Make(c*r, -s*r);
                v2->fColor = color;
                v2->fOffset = SkPoint::Make(c*innerRadius, -s*innerRadius);
                v2->fOuterRadius = outerRadius;
                v2->fBlurRadius = blurRadius;

                v3->fPos = center + SkPoint::Make(c*r, s*r);
                v3->fColor = color;
                v3->fOffset = SkPoint::Make(c*innerRadius, s*innerRadius);
                v3->fOuterRadius = outerRadius;
                v3->fBlurRadius = blurRadius;

                v4->fPos = center + SkPoint::Make(s*r, c*r);
                v4->fColor = color;
                v4->fOffset = SkPoint::Make(s*innerRadius, c*innerRadius);
                v4->fOuterRadius = outerRadius;
                v4->fBlurRadius = blurRadius;

                v5->fPos = center + SkPoint::Make(-s*r, c*r);
                v5->fColor = color;
                v5->fOffset = SkPoint::Make(-s*innerRadius, c*innerRadius);
                v5->fOuterRadius = outerRadius;
                v5->fBlurRadius = blurRadius;

                v6->fPos = center + SkPoint::Make(-c*r, s*r);
                v6->fColor = color;
                v6->fOffset = SkPoint::Make(-c*innerRadius, s*innerRadius);
                v6->fOuterRadius = outerRadius;
                v6->fBlurRadius = blurRadius;

                v7->fPos = center + SkPoint::Make(-c*r, -s*r);
                v7->fColor = color;
                v7->fOffset = SkPoint::Make(-c*innerRadius, -s*innerRadius);
                v7->fOuterRadius = outerRadius;
                v7->fBlurRadius = blurRadius;
            } else {
                // filled
                CircleVertex* v8 = reinterpret_cast<CircleVertex*>(vertices + 8 * vertexStride);
                v8->fPos = center;
                v8->fColor = color;
                v8->fOffset = SkPoint::Make(0, 0);
                v8->fOuterRadius = outerRadius;
                v8->fBlurRadius = blurRadius;
            }

            const uint16_t* primIndices = circle_type_to_indices(geom.fStroked);
            const int primIndexCount = circle_type_to_index_count(geom.fStroked);
            for (int i = 0; i < primIndexCount; ++i) {
                *indices++ = primIndices[i] + currStartVertex;
            }

            currStartVertex += circle_type_to_vert_count(geom.fStroked);
            vertices += circle_type_to_vert_count(geom.fStroked)*vertexStride;
        }

        GrMesh mesh;
        mesh.initIndexed(kTriangles_GrPrimitiveType, vertexBuffer, indexBuffer, firstVertex,
                         firstIndex, fVertCount, fIndexCount);
        target->draw(gp.get(), mesh);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        ShadowCircleBatch* that = t->cast<ShadowCircleBatch>();
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
        fAllFill = fAllFill && that->fAllFill;
        return true;
    }

    struct Geometry {
        GrColor  fColor;
        SkScalar fOuterRadius;
        SkScalar fInnerRadius;
        SkScalar fBlurRadius;
        SkRect   fDevBounds;
        bool     fStroked;
    };

    SkSTArray<1, Geometry, true> fGeoData;
    SkMatrix                     fViewMatrixIfUsingLocalCoords;
    int                          fVertCount;
    int                          fIndexCount;
    bool                         fAllFill;

    typedef GrVertexBatch INHERITED;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

// We have three possible cases for geometry for a roundrect.
//
// In the case of a normal fill or a stroke, we draw the roundrect as a 9-patch:
//    ____________
//   |_|________|_|
//   | |        | |
//   | |        | |
//   | |        | |
//   |_|________|_|
//   |_|________|_|
//
// For strokes, we don't draw the center quad.
//
// For circular roundrects, in the case where the stroke width is greater than twice
// the corner radius (overstroke), we add additional geometry to mark out the rectangle
// in the center. The shared vertices are duplicated so we can set a different outer radius
// for the fill calculation.
//    ____________
//   |_|________|_|
//   | |\ ____ /| |
//   | | |    | | |
//   | | |____| | |
//   |_|/______\|_|
//   |_|________|_|
//
// We don't draw the center quad from the fill rect in this case.
//
// For filled rrects that need to provide a distance vector we resuse the overstroke
// geometry but make the inner rect degenerate (either a point or a horizontal or
// vertical line).

static const uint16_t gOverstrokeRRectIndices[] = {
    // overstroke quads
    // we place this at the beginning so that we can skip these indices when rendering normally
    16, 17, 19, 16, 19, 18,
    19, 17, 23, 19, 23, 21,
    21, 23, 22, 21, 22, 20,
    22, 16, 18, 22, 18, 20,

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

    // center
    // we place this at the end so that we can ignore these indices when not rendering as filled
    5, 6, 10, 5, 10, 9,
};
// fill and standard stroke indices skip the overstroke "ring"
static const uint16_t* gStandardRRectIndices = gOverstrokeRRectIndices + 6 * 4;

// overstroke count is arraysize minus the center indices
static const int kIndicesPerOverstrokeRRect = SK_ARRAY_COUNT(gOverstrokeRRectIndices) - 6;
// fill count skips overstroke indices and includes center
static const int kIndicesPerFillRRect = kIndicesPerOverstrokeRRect - 6 * 4 + 6;
// stroke count is fill count minus center indices
static const int kIndicesPerStrokeRRect = kIndicesPerFillRRect - 6;
static const int kVertsPerStandardRRect = 16;
static const int kVertsPerOverstrokeRRect = 24;

enum RRectType {
    kFill_RRectType,
    kStroke_RRectType,
    kOverstroke_RRectType,
    kFillWithDist_RRectType
};

static int rrect_type_to_vert_count(RRectType type) {
    static const int kTypeToVertCount[] = {
        kVertsPerStandardRRect,
        kVertsPerStandardRRect,
        kVertsPerOverstrokeRRect,
        kVertsPerOverstrokeRRect,
    };

    return kTypeToVertCount[type];
}

static int rrect_type_to_index_count(RRectType type) {
    static const int kTypeToIndexCount[] = {
        kIndicesPerFillRRect,
        kIndicesPerStrokeRRect,
        kIndicesPerOverstrokeRRect,
        kIndicesPerOverstrokeRRect,
    };

    return kTypeToIndexCount[type];
}

static const uint16_t* rrect_type_to_indices(RRectType type) {
    static const uint16_t* kTypeToIndices[] = {
        gStandardRRectIndices,
        gStandardRRectIndices,
        gOverstrokeRRectIndices,
        gOverstrokeRRectIndices,
    };

    return kTypeToIndices[type];
}

// For distance computations in the interior of filled rrects we:
//
//   add a interior degenerate (point or line) rect
//   each vertex of that rect gets -outerRad as its radius
//      this makes the computation of the distance to the outer edge be negative
//      negative values are caught and then handled differently in the GP's onEmitCode
//   each vertex is also given the normalized x & y distance from the interior rect's edge
//      the GP takes the min of those depths +1 to get the normalized distance to the outer edge

class ShadowCircularRRectBatch : public GrVertexBatch {
public:
    DEFINE_BATCH_CLASS_ID

    // A devStrokeWidth <= 0 indicates a fill only. If devStrokeWidth > 0 then strokeOnly indicates
    // whether the rrect is only stroked or stroked and filled.
    ShadowCircularRRectBatch(GrColor color, bool needsDistance, const SkMatrix& viewMatrix,
                             const SkRect& devRect, float devRadius, float blurRadius,
                             float devStrokeWidth, bool strokeOnly)
        : INHERITED(ClassID())
        , fViewMatrixIfUsingLocalCoords(viewMatrix) {
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
                // Outset stroke by 1/4 pixel
                devStrokeWidth += 0.25f;
                // If stroke is greater than width or height, this is still a fill
                // Otherwise we compute stroke params
                if (devStrokeWidth <= devRect.width() &&
                    devStrokeWidth <= devRect.height()) {
                    innerRadius = devRadius - halfWidth;
                    type = (innerRadius >= 0) ? kStroke_RRectType : kOverstroke_RRectType;
                }
            }
            outerRadius += halfWidth;
            bounds.outset(halfWidth, halfWidth);
        }
        if (kFill_RRectType == type && needsDistance) {
            type = kFillWithDist_RRectType;
        }

        // TODO: still needed?
        // The radii are outset for two reasons. First, it allows the shader to simply perform
        // simpler computation because the computed alpha is zero, rather than 50%, at the radius.
        // Second, the outer radius is used to compute the verts of the bounding box that is
        // rendered and the outset ensures the box will cover all partially covered by the rrect
        // corners.
        outerRadius += SK_ScalarHalf;
        innerRadius -= SK_ScalarHalf;

        this->setBounds(bounds, HasAABloat::kYes, IsZeroArea::kNo);

        // Expand the rect for aa to generate correct vertices.
        bounds.outset(SK_ScalarHalf, SK_ScalarHalf);

        fGeoData.emplace_back(Geometry{color, outerRadius, innerRadius, blurRadius, bounds, type});
        fVertCount = rrect_type_to_vert_count(type);
        fIndexCount = rrect_type_to_index_count(type);
        fAllFill = (kFill_RRectType == type);
    }

    const char* name() const override { return "RRectCircleBatch"; }

    SkString dumpInfo() const override {
        SkString string;
        for (int i = 0; i < fGeoData.count(); ++i) {
            string.appendf("Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f],"
                           "OuterRad: %.2f, InnerRad: %.2f, BlurRad: %.2f\n",
                           fGeoData[i].fColor,
                           fGeoData[i].fDevBounds.fLeft, fGeoData[i].fDevBounds.fTop,
                           fGeoData[i].fDevBounds.fRight, fGeoData[i].fDevBounds.fBottom,
                           fGeoData[i].fOuterRadius, fGeoData[i].fInnerRadius,
                           fGeoData[i].fBlurRadius);
        }
        string.append(INHERITED::dumpInfo());
        return string;
    }

    void computePipelineOptimizations(GrInitInvariantOutput* color,
                                      GrInitInvariantOutput* coverage,
                                      GrBatchToXPOverrides* overrides) const override {
        // When this is called on a batch, there is only one geometry bundle
        color->setKnownFourComponents(fGeoData[0].fColor);
        coverage->setUnknownSingleComponent();
    }

private:
    void initBatchTracker(const GrXPOverridesForBatch& overrides) override {
        // Handle any overrides that affect our GP.
        overrides.getOverrideColorIfSet(&fGeoData[0].fColor);
        if (!overrides.readsLocalCoords()) {
            fViewMatrixIfUsingLocalCoords.reset();
        }
    }

    struct CircleVertex {
        SkPoint  fPos;
        GrColor  fColor;
        SkPoint  fOffset;
        SkScalar fOuterRadius;
        SkScalar fBlurRadius;
    };

    static void FillInOverstrokeVerts(CircleVertex** verts, const SkRect& bounds,
                                      SkScalar smInset, SkScalar bigInset, SkScalar xOffset,
                                      SkScalar outerRadius, GrColor color, SkScalar blurRadius) {
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

            SkScalar yCoords[4] = {
                bounds.fTop,
                bounds.fTop + outerRadius,
                bounds.fBottom - outerRadius,
                bounds.fBottom
            };

            SkScalar yOuterRadii[4] = { -1, 0, 0, 1 };
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

                FillInOverstrokeVerts(&verts, bounds, outerRadius, overstrokeOuterRadius,
                                      maxOffset, overstrokeOuterRadius, color, blurRadius);
            }

            if (kFillWithDist_RRectType == args.fType) {
                SkScalar halfMinDim = 0.5f * SkTMin(bounds.width(), bounds.height());

                SkScalar xOffset = 1.0f - outerRadius / halfMinDim;

                FillInOverstrokeVerts(&verts, bounds, outerRadius, halfMinDim,
                                      xOffset, halfMinDim, color, blurRadius);
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
        target->draw(gp.get(), mesh);
    }

    bool onCombineIfPossible(GrBatch* t, const GrCaps& caps) override {
        ShadowCircularRRectBatch* that = t->cast<ShadowCircularRRectBatch>();
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
        fAllFill = fAllFill && that->fAllFill;
        return true;
    }

    struct Geometry {
        GrColor  fColor;
        SkScalar fOuterRadius;
        SkScalar fInnerRadius;
        SkScalar fBlurRadius;
        SkRect fDevBounds;
        RRectType fType;
    };

    SkSTArray<1, Geometry, true> fGeoData;
    SkMatrix                     fViewMatrixIfUsingLocalCoords;
    int                          fVertCount;
    int                          fIndexCount;
    bool                         fAllFill;

    typedef GrVertexBatch INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GrDrawBatch* create_shadow_circle_batch(GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const SkRect& oval,
                                               const SkStrokeRec& stroke,
                                               SkScalar blurRadius,
                                               const GrShaderCaps* shaderCaps) {
    // we can only draw circles
    SkScalar width = oval.width();
    SkASSERT(SkScalarNearlyEqual(width, oval.height()) && viewMatrix.isSimilarity());
    SkPoint center = { oval.centerX(), oval.centerY() };
    return ShadowCircleBatch::Create(color, viewMatrix, center, width / 2.f,
                                     blurRadius, GrStyle(stroke, nullptr));
}

static GrDrawBatch* create_shadow_rrect_batch(GrColor color,
                                              bool needsDistance,
                                              const SkMatrix& viewMatrix,
                                              const SkRRect& rrect,
                                              SkScalar blurRadius,
                                              const SkStrokeRec& stroke) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(rrect.isSimple());
    SkASSERT(!rrect.isOval());

    // Shadow rrect batchs only handle simple circular rrects
    // do any matrix crunching before we reset the draw state for device coords
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
    SkVector scaledStroke = { -1, -1 };
    SkScalar strokeWidth = stroke.getWidth();

    bool isStrokeOnly = SkStrokeRec::kStroke_Style == style ||
        SkStrokeRec::kHairline_Style == style;
    bool hasStroke = isStrokeOnly || SkStrokeRec::kStrokeAndFill_Style == style;

    if (hasStroke) {
        if (SkStrokeRec::kHairline_Style == style) {
            scaledStroke.set(1, 1);
        } else {
            scaledStroke.fX = SkScalarAbs(strokeWidth*(viewMatrix[SkMatrix::kMScaleX] +
                                                       viewMatrix[SkMatrix::kMSkewY]));
            scaledStroke.fY = SkScalarAbs(strokeWidth*(viewMatrix[SkMatrix::kMSkewX] +
                                                       viewMatrix[SkMatrix::kMScaleY]));
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

    return new ShadowCircularRRectBatch(color, needsDistance, viewMatrix, bounds, xRadius,
                                        blurRadius, scaledStroke.fX, isStrokeOnly);
}

GrDrawBatch* CreateShadowRRectBatch(GrColor color,
                                    bool needsDistance,
                                    const SkMatrix& viewMatrix,
                                    const SkRRect& rrect,
                                    const SkStrokeRec& stroke,
                                    const SkScalar blurRadius,
                                    const GrShaderCaps* shaderCaps) {
    if (rrect.isOval()) {
        return create_shadow_circle_batch(color, viewMatrix, rrect.getBounds(),
                                          stroke, blurRadius, shaderCaps);
    }

    if (!viewMatrix.rectStaysRect() || !rrect.isSimple()) {
        return nullptr;
    }

    return create_shadow_rrect_batch(color, needsDistance, viewMatrix, rrect, blurRadius, stroke);
}

///////////////////////////////////////////////////////////////////////////////

#ifdef GR_TEST_UTILS

DRAW_BATCH_TEST_DEFINE(ShadowCircleBatch) {
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
        SkPoint center = { circle.centerX(), circle.centerY() };
        SkScalar radius = circle.width() / 2.f;
        SkStrokeRec stroke = GrTest::TestStrokeRec(random);
        SkScalar blurRadius = random->nextSScalar1() * 72.f;
        GrDrawBatch* batch = ShadowCircleBatch::Create(color, viewMatrix, center, radius,
                                                       blurRadius, GrStyle(stroke, nullptr));
        if (batch) {
            return batch;
        }
    } while (true);
}

DRAW_BATCH_TEST_DEFINE(ShadowRRectBatch) {
    SkMatrix viewMatrix = GrTest::TestMatrixRectStaysRect(random);
    GrColor color = GrRandomColor(random);
    const SkRRect& rrect = GrTest::TestRRectSimple(random);
    bool needsDistance = random->nextBool();
    SkScalar blurRadius = random->nextSScalar1() * 72.f;
    return create_shadow_rrect_batch(color, needsDistance, viewMatrix, rrect,
                                     blurRadius, GrTest::TestStrokeRec(random));
}

#endif
