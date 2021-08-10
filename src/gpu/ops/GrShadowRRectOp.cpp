/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrShadowRRectOp.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkRRectPriv.h"
#include "src/gpu/GrDrawOpTest.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrShadowGeoProc.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

///////////////////////////////////////////////////////////////////////////////
// Circle Data
//
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
// RoundRect Data
//
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
    SK_ABORT("Invalid type");
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
    SK_ABORT("Invalid type");
}

static const uint16_t* rrect_type_to_indices(RRectType type) {
    switch (type) {
        case kFill_RRectType:
        case kStroke_RRectType:
            return gRRectIndices + 6*4;
        case kOverstroke_RRectType:
            return gRRectIndices;
    }
    SK_ABORT("Invalid type");
}

///////////////////////////////////////////////////////////////////////////////
namespace {

class ShadowCircularRRectOp final : public GrMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID

    // An insetWidth > 1/2 rect width or height indicates a simple fill.
    ShadowCircularRRectOp(GrColor color, const SkRect& devRect,
                          float devRadius, bool isCircle, float blurRadius, float insetWidth,
                          GrSurfaceProxyView falloffView)
            : INHERITED(ClassID())
            , fFalloffView(std::move(falloffView)) {
        SkRect bounds = devRect;
        SkASSERT(insetWidth > 0);
        SkScalar innerRadius = 0.0f;
        SkScalar outerRadius = devRadius;
        SkScalar umbraInset;

        RRectType type = kFill_RRectType;
        if (isCircle) {
            umbraInset = 0;
        } else {
            umbraInset = std::max(outerRadius, blurRadius);
        }

        // If stroke is greater than width or height, this is still a fill,
        // otherwise we compute stroke params.
        if (isCircle) {
            innerRadius = devRadius - insetWidth;
            type = innerRadius > 0 ? kStroke_RRectType : kFill_RRectType;
        } else {
            if (insetWidth <= 0.5f*std::min(devRect.width(), devRect.height())) {
                // We don't worry about a real inner radius, we just need to know if we
                // need to create overstroke vertices.
                innerRadius = std::max(insetWidth - umbraInset, 0.0f);
                type = innerRadius > 0 ? kOverstroke_RRectType : kStroke_RRectType;
            }
        }

        this->setBounds(bounds, HasAABloat::kNo, IsHairline::kNo);

        fGeoData.emplace_back(Geometry{color, outerRadius, umbraInset, innerRadius,
                                       blurRadius, bounds, type, isCircle});
        if (isCircle) {
            fVertCount = circle_type_to_vert_count(kStroke_RRectType == type);
            fIndexCount = circle_type_to_index_count(kStroke_RRectType == type);
        } else {
            fVertCount = rrect_type_to_vert_count(type);
            fIndexCount = rrect_type_to_index_count(type);
        }
    }

    const char* name() const override { return "ShadowCircularRRectOp"; }

    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }

    GrProcessorSet::Analysis finalize(const GrCaps&, const GrAppliedClip*, GrClampType) override {
        return GrProcessorSet::EmptySetAnalysis();
    }

private:
    struct Geometry {
        GrColor   fColor;
        SkScalar  fOuterRadius;
        SkScalar  fUmbraInset;
        SkScalar  fInnerRadius;
        SkScalar  fBlurRadius;
        SkRect    fDevBounds;
        RRectType fType;
        bool      fIsCircle;
    };

    struct CircleVertex {
        SkPoint fPos;
        GrColor fColor;
        SkPoint fOffset;
        SkScalar fDistanceCorrection;
    };

    void fillInCircleVerts(const Geometry& args, bool isStroked, CircleVertex** verts) const {

        GrColor color = args.fColor;
        SkScalar outerRadius = args.fOuterRadius;
        SkScalar innerRadius = args.fInnerRadius;
        SkScalar blurRadius = args.fBlurRadius;
        SkScalar distanceCorrection = outerRadius / blurRadius;

        const SkRect& bounds = args.fDevBounds;

        // The inner radius in the vertex data must be specified in normalized space.
        innerRadius = innerRadius / outerRadius;

        SkPoint center = SkPoint::Make(bounds.centerX(), bounds.centerY());
        SkScalar halfWidth = 0.5f * bounds.width();
        SkScalar octOffset = 0.41421356237f;  // sqrt(2) - 1

        (*verts)->fPos = center + SkPoint::Make(-octOffset * halfWidth, -halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(-octOffset, -1);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        (*verts)->fPos = center + SkPoint::Make(octOffset * halfWidth, -halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(octOffset, -1);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        (*verts)->fPos = center + SkPoint::Make(halfWidth, -octOffset * halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(1, -octOffset);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        (*verts)->fPos = center + SkPoint::Make(halfWidth, octOffset * halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(1, octOffset);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        (*verts)->fPos = center + SkPoint::Make(octOffset * halfWidth, halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(octOffset, 1);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        (*verts)->fPos = center + SkPoint::Make(-octOffset * halfWidth, halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(-octOffset, 1);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        (*verts)->fPos = center + SkPoint::Make(-halfWidth, octOffset * halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(-1, octOffset);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        (*verts)->fPos = center + SkPoint::Make(-halfWidth, -octOffset * halfWidth);
        (*verts)->fColor = color;
        (*verts)->fOffset = SkPoint::Make(-1, -octOffset);
        (*verts)->fDistanceCorrection = distanceCorrection;
        (*verts)++;

        if (isStroked) {
            // compute the inner ring

            // cosine and sine of pi/8
            SkScalar c = 0.923579533f;
            SkScalar s = 0.382683432f;
            SkScalar r = args.fInnerRadius;

            (*verts)->fPos = center + SkPoint::Make(-s * r, -c * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(-s * innerRadius, -c * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = center + SkPoint::Make(s * r, -c * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(s * innerRadius, -c * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = center + SkPoint::Make(c * r, -s * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(c * innerRadius, -s * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = center + SkPoint::Make(c * r, s * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(c * innerRadius, s * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = center + SkPoint::Make(s * r, c * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(s * innerRadius, c * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = center + SkPoint::Make(-s * r, c * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(-s * innerRadius, c * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = center + SkPoint::Make(-c * r, s * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(-c * innerRadius, s * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = center + SkPoint::Make(-c * r, -s * r);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(-c * innerRadius, -s * innerRadius);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;
        } else {
            // filled
            (*verts)->fPos = center;
            (*verts)->fColor = color;
            (*verts)->fOffset = SkPoint::Make(0, 0);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;
        }
    }

    void fillInRRectVerts(const Geometry& args, CircleVertex** verts) const {
        GrColor color = args.fColor;
        SkScalar outerRadius = args.fOuterRadius;

        const SkRect& bounds = args.fDevBounds;

        SkScalar umbraInset = args.fUmbraInset;
        SkScalar minDim = 0.5f*std::min(bounds.width(), bounds.height());
        if (umbraInset > minDim) {
            umbraInset = minDim;
        }

        SkScalar xInner[4] = { bounds.fLeft + umbraInset, bounds.fRight - umbraInset,
            bounds.fLeft + umbraInset, bounds.fRight - umbraInset };
        SkScalar xMid[4] = { bounds.fLeft + outerRadius, bounds.fRight - outerRadius,
            bounds.fLeft + outerRadius, bounds.fRight - outerRadius };
        SkScalar xOuter[4] = { bounds.fLeft, bounds.fRight,
            bounds.fLeft, bounds.fRight };
        SkScalar yInner[4] = { bounds.fTop + umbraInset, bounds.fTop + umbraInset,
            bounds.fBottom - umbraInset, bounds.fBottom - umbraInset };
        SkScalar yMid[4] = { bounds.fTop + outerRadius, bounds.fTop + outerRadius,
            bounds.fBottom - outerRadius, bounds.fBottom - outerRadius };
        SkScalar yOuter[4] = { bounds.fTop, bounds.fTop,
            bounds.fBottom, bounds.fBottom };

        SkScalar blurRadius = args.fBlurRadius;

        // In the case where we have to inset more for the umbra, our two triangles in the
        // corner get skewed to a diamond rather than a square. To correct for that,
        // we also skew the vectors we send to the shader that help define the circle.
        // By doing so, we end up with a quarter circle in the corner rather than the
        // elliptical curve.

        // This is a bit magical, but it gives us the correct results at extrema:
        //   a) umbraInset == outerRadius produces an orthogonal vector
        //   b) outerRadius == 0 produces a diagonal vector
        // And visually the corner looks correct.
        SkVector outerVec = SkVector::Make(outerRadius - umbraInset, -outerRadius - umbraInset);
        outerVec.normalize();
        // We want the circle edge to fall fractionally along the diagonal at
        //      (sqrt(2)*(umbraInset - outerRadius) + outerRadius)/sqrt(2)*umbraInset
        //
        // Setting the components of the diagonal offset to the following value will give us that.
        SkScalar diagVal = umbraInset / (SK_ScalarSqrt2*(outerRadius - umbraInset) - outerRadius);
        SkVector diagVec = SkVector::Make(diagVal, diagVal);
        SkScalar distanceCorrection = umbraInset / blurRadius;

        // build corner by corner
        for (int i = 0; i < 4; ++i) {
            // inner point
            (*verts)->fPos = SkPoint::Make(xInner[i], yInner[i]);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkVector::Make(0, 0);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            // outer points
            (*verts)->fPos = SkPoint::Make(xOuter[i], yInner[i]);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkVector::Make(0, -1);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = SkPoint::Make(xOuter[i], yMid[i]);
            (*verts)->fColor = color;
            (*verts)->fOffset = outerVec;
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = SkPoint::Make(xOuter[i], yOuter[i]);
            (*verts)->fColor = color;
            (*verts)->fOffset = diagVec;
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = SkPoint::Make(xMid[i], yOuter[i]);
            (*verts)->fColor = color;
            (*verts)->fOffset = outerVec;
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;

            (*verts)->fPos = SkPoint::Make(xInner[i], yOuter[i]);
            (*verts)->fColor = color;
            (*verts)->fOffset = SkVector::Make(0, -1);
            (*verts)->fDistanceCorrection = distanceCorrection;
            (*verts)++;
        }

        // Add the additional vertices for overstroked rrects.
        // Effectively this is an additional stroked rrect, with its
        // parameters equal to those in the center of the 9-patch. This will
        // give constant values across this inner ring.
        if (kOverstroke_RRectType == args.fType) {
            SkASSERT(args.fInnerRadius > 0.0f);

            SkScalar inset =  umbraInset + args.fInnerRadius;

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

    }

    GrProgramInfo* programInfo() override { return fProgramInfo; }

    void onCreateProgramInfo(const GrCaps* caps,
                             SkArenaAlloc* arena,
                             const GrSurfaceProxyView& writeView,
                             bool usesMSAASurface,
                             GrAppliedClip&& appliedClip,
                             const GrDstProxyView& dstProxyView,
                             GrXferBarrierFlags renderPassXferBarriers,
                             GrLoadOp colorLoadOp) override {
        GrGeometryProcessor* gp = GrRRectShadowGeoProc::Make(arena, fFalloffView);
        SkASSERT(sizeof(CircleVertex) == gp->vertexStride());

        fProgramInfo = GrSimpleMeshDrawOpHelper::CreateProgramInfo(caps, arena, writeView,
                                                                   std::move(appliedClip),
                                                                   dstProxyView, gp,
                                                                   GrProcessorSet::MakeEmptySet(),
                                                                   GrPrimitiveType::kTriangles,
                                                                   renderPassXferBarriers,
                                                                   colorLoadOp,
                                                                   GrPipeline::InputFlags::kNone,
                                                                   &GrUserStencilSettings::kUnused);
    }

    void onPrepareDraws(GrMeshDrawTarget* target) override {
        int instanceCount = fGeoData.count();

        sk_sp<const GrBuffer> vertexBuffer;
        int firstVertex;
        CircleVertex* verts = (CircleVertex*)target->makeVertexSpace(
                sizeof(CircleVertex), fVertCount, &vertexBuffer, &firstVertex);
        if (!verts) {
            SkDebugf("Could not allocate vertices\n");
            return;
        }

        sk_sp<const GrBuffer> indexBuffer;
        int firstIndex = 0;
        uint16_t* indices = target->makeIndexSpace(fIndexCount, &indexBuffer, &firstIndex);
        if (!indices) {
            SkDebugf("Could not allocate indices\n");
            return;
        }

        int currStartVertex = 0;
        for (int i = 0; i < instanceCount; i++) {
            const Geometry& args = fGeoData[i];

            if (args.fIsCircle) {
                bool isStroked = SkToBool(kStroke_RRectType == args.fType);
                this->fillInCircleVerts(args, isStroked, &verts);

                const uint16_t* primIndices = circle_type_to_indices(isStroked);
                const int primIndexCount = circle_type_to_index_count(isStroked);
                for (int j = 0; j < primIndexCount; ++j) {
                    *indices++ = primIndices[j] + currStartVertex;
                }

                currStartVertex += circle_type_to_vert_count(isStroked);

            } else {
                this->fillInRRectVerts(args, &verts);

                const uint16_t* primIndices = rrect_type_to_indices(args.fType);
                const int primIndexCount = rrect_type_to_index_count(args.fType);
                for (int j = 0; j < primIndexCount; ++j) {
                    *indices++ = primIndices[j] + currStartVertex;
                }

                currStartVertex += rrect_type_to_vert_count(args.fType);
            }
        }

        fMesh = target->allocMesh();
        fMesh->setIndexed(std::move(indexBuffer), fIndexCount, firstIndex, 0, fVertCount - 1,
                          GrPrimitiveRestart::kNo, std::move(vertexBuffer), firstVertex);
    }

    void onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) override {
        if (!fProgramInfo) {
            this->createProgramInfo(flushState);
        }

        if (!fProgramInfo || !fMesh) {
            return;
        }

        flushState->bindPipelineAndScissorClip(*fProgramInfo, chainBounds);
        flushState->bindTextures(fProgramInfo->geomProc(), *fFalloffView.proxy(),
                                 fProgramInfo->pipeline());
        flushState->drawMesh(*fMesh);
    }

    CombineResult onCombineIfPossible(GrOp* t, SkArenaAlloc*, const GrCaps& caps) override {
        ShadowCircularRRectOp* that = t->cast<ShadowCircularRRectOp>();
        fGeoData.push_back_n(that->fGeoData.count(), that->fGeoData.begin());
        fVertCount += that->fVertCount;
        fIndexCount += that->fIndexCount;
        return CombineResult::kMerged;
    }

#if GR_TEST_UTILS
    SkString onDumpInfo() const override {
        SkString string;
        for (int i = 0; i < fGeoData.count(); ++i) {
            string.appendf(
                    "Color: 0x%08x Rect [L: %.2f, T: %.2f, R: %.2f, B: %.2f],"
                    "OuterRad: %.2f, Umbra: %.2f, InnerRad: %.2f, BlurRad: %.2f\n",
                    fGeoData[i].fColor, fGeoData[i].fDevBounds.fLeft, fGeoData[i].fDevBounds.fTop,
                    fGeoData[i].fDevBounds.fRight, fGeoData[i].fDevBounds.fBottom,
                    fGeoData[i].fOuterRadius, fGeoData[i].fUmbraInset,
                    fGeoData[i].fInnerRadius, fGeoData[i].fBlurRadius);
        }
        return string;
    }
#endif

    void visitProxies(const GrVisitProxyFunc& func) const override {
        func(fFalloffView.proxy(), GrMipmapped(false));
        if (fProgramInfo) {
            fProgramInfo->visitFPProxies(func);
        }
    }

    SkSTArray<1, Geometry, true> fGeoData;
    int fVertCount;
    int fIndexCount;
    GrSurfaceProxyView fFalloffView;

    GrSimpleMesh*      fMesh = nullptr;
    GrProgramInfo*     fProgramInfo = nullptr;

    using INHERITED = GrMeshDrawOp;
};

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

namespace GrShadowRRectOp {

static GrSurfaceProxyView create_falloff_texture(GrRecordingContext* rContext) {
    static const GrUniqueKey::Domain kDomain = GrUniqueKey::GenerateDomain();
    GrUniqueKey key;
    GrUniqueKey::Builder builder(&key, kDomain, 0, "Shadow Gaussian Falloff");
    builder.finish();

    auto threadSafeCache = rContext->priv().threadSafeCache();

    GrSurfaceProxyView view = threadSafeCache->find(key);
    if (view) {
        SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
        return view;
    }

    static const int kWidth = 128;
    static const size_t kRowBytes = kWidth * GrColorTypeBytesPerPixel(GrColorType::kAlpha_8);
    SkImageInfo ii = SkImageInfo::MakeA8(kWidth, 1);

    SkBitmap bitmap;
    bitmap.allocPixels(ii, kRowBytes);

    unsigned char* values = (unsigned char*)bitmap.getPixels();
    for (int i = 0; i < 128; ++i) {
        SkScalar d = SK_Scalar1 - i / SkIntToScalar(127);
        values[i] = SkScalarRoundToInt((SkScalarExp(-4 * d * d) - 0.018f) * 255);
    }
    bitmap.setImmutable();

    view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bitmap));
    if (!view) {
        return {};
    }

    view = threadSafeCache->add(key, view);
    SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
    return view;
}


GrOp::Owner Make(GrRecordingContext* context,
                 GrColor color,
                 const SkMatrix& viewMatrix,
                 const SkRRect& rrect,
                 SkScalar blurWidth,
                 SkScalar insetWidth) {
    // Shadow rrect ops only handle simple circular rrects.
    SkASSERT(viewMatrix.isSimilarity() && SkRRectPriv::EqualRadii(rrect));

    GrSurfaceProxyView falloffView = create_falloff_texture(context);
    if (!falloffView) {
        return nullptr;
    }

    // Do any matrix crunching before we reset the draw state for device coords.
    const SkRect& rrectBounds = rrect.getBounds();
    SkRect bounds;
    viewMatrix.mapRect(&bounds, rrectBounds);

    // Map radius and inset. As the matrix is a similarity matrix, this should be isotropic.
    SkScalar radius = SkRRectPriv::GetSimpleRadii(rrect).fX;
    SkScalar matrixFactor = viewMatrix[SkMatrix::kMScaleX] + viewMatrix[SkMatrix::kMSkewX];
    SkScalar scaledRadius = SkScalarAbs(radius*matrixFactor);
    SkScalar scaledInsetWidth = SkScalarAbs(insetWidth*matrixFactor);

    if (scaledInsetWidth <= 0) {
        return nullptr;
    }

    return GrOp::Make<ShadowCircularRRectOp>(context,
                                             color,
                                             bounds,
                                             scaledRadius,
                                             rrect.isOval(),
                                             blurWidth,
                                             scaledInsetWidth,
                                             std::move(falloffView));
}
}  // namespace GrShadowRRectOp

///////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS

GR_DRAW_OP_TEST_DEFINE(ShadowRRectOp) {
    // We may choose matrix and inset values that cause the factory to fail. We loop until we find
    // an acceptable combination.
    do {
        // create a similarity matrix
        SkScalar rotate = random->nextSScalar1() * 360.f;
        SkScalar translateX = random->nextSScalar1() * 1000.f;
        SkScalar translateY = random->nextSScalar1() * 1000.f;
        SkScalar scale = random->nextSScalar1() * 100.f;
        SkMatrix viewMatrix;
        viewMatrix.setRotate(rotate);
        viewMatrix.postTranslate(translateX, translateY);
        viewMatrix.postScale(scale, scale);
        SkScalar insetWidth = random->nextSScalar1() * 72.f;
        SkScalar blurWidth = random->nextSScalar1() * 72.f;
        bool isCircle = random->nextBool();
        // This op doesn't use a full GrPaint, just a color.
        GrColor color = paint.getColor4f().toBytes_RGBA();
        if (isCircle) {
            SkRect circle = GrTest::TestSquare(random);
            SkRRect rrect = SkRRect::MakeOval(circle);
            if (auto op = GrShadowRRectOp::Make(
                    context, color, viewMatrix, rrect, blurWidth, insetWidth)) {
                return op;
            }
        } else {
            SkRRect rrect;
            do {
                // This may return a rrect with elliptical corners, which will cause an assert.
                rrect = GrTest::TestRRectSimple(random);
            } while (!SkRRectPriv::IsSimpleCircular(rrect));
            if (auto op = GrShadowRRectOp::Make(
                    context, color, viewMatrix, rrect, blurWidth, insetWidth)) {
                return op;
            }
        }
    } while (true);
}

#endif
