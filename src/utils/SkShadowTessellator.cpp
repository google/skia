/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShadowTessellator.h"
#include "SkColorData.h"
#include "SkDrawShadowInfo.h"
#include "SkGeometry.h"
#include "SkPolyUtils.h"
#include "SkPath.h"
#include "SkPoint3.h"
#include "SkPointPriv.h"
#include "SkVertices.h"

#if SK_SUPPORT_GPU
#include "GrPathUtils.h"
#endif


/**
 * Base class
 */
class SkBaseShadowTessellator {
public:
    SkBaseShadowTessellator(const SkPoint3& zPlaneParams, bool transparent);
    virtual ~SkBaseShadowTessellator() {}

    sk_sp<SkVertices> releaseVertices() {
        if (!fSucceeded) {
            return nullptr;
        }
        return SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode, this->vertexCount(),
                                    fPositions.begin(), nullptr, fColors.begin(),
                                    this->indexCount(), fIndices.begin());
    }

protected:
    static constexpr auto kMinHeight = 0.1f;

    int vertexCount() const { return fPositions.count(); }
    int indexCount() const { return fIndices.count(); }

    bool setZOffset(const SkRect& bounds, bool perspective);

    bool accumulateCentroid(const SkPoint& c, const SkPoint& n);
    bool checkConvexity(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2);
    void finishPathPolygon();
    void stitchConcaveRings(const SkTDArray<SkPoint>& umbraPolygon,
                            SkTDArray<int>* umbraIndices,
                            const SkTDArray<SkPoint>& penumbraPolygon,
                            SkTDArray<int>* penumbraIndices);

    void handleLine(const SkPoint& p);
    void handleLine(const SkMatrix& m, SkPoint* p);

    void handleQuad(const SkPoint pts[3]);
    void handleQuad(const SkMatrix& m, SkPoint pts[3]);

    void handleCubic(const SkMatrix& m, SkPoint pts[4]);

    void handleConic(const SkMatrix& m, SkPoint pts[3], SkScalar w);

    bool setTransformedHeightFunc(const SkMatrix& ctm);

    bool addArc(const SkVector& nextNormal, bool finishArc);

    void appendTriangle(uint16_t index0, uint16_t index1, uint16_t index2);
    void appendQuad(uint16_t index0, uint16_t index1, uint16_t index2, uint16_t index3);

    SkScalar heightFunc(SkScalar x, SkScalar y) {
        return fZPlaneParams.fX*x + fZPlaneParams.fY*y + fZPlaneParams.fZ;
    }

    SkPoint3                                fZPlaneParams;
    std::function<SkScalar(const SkPoint&)> fTransformedHeightFunc;
    SkScalar                                fZOffset;
    // members for perspective height function
    SkPoint3                                fTransformedZParams;
    SkScalar                                fPartialDeterminants[3];

    // temporary buffer
    SkTDArray<SkPoint>  fPointBuffer;

    SkTDArray<SkPoint>  fPositions;
    SkTDArray<SkColor>  fColors;
    SkTDArray<uint16_t> fIndices;

    SkTDArray<SkPoint>  fPathPolygon;
    SkPoint             fCentroid;
    SkScalar            fArea;
    SkScalar            fLastArea;
    SkScalar            fLastCross;

    int                 fFirstVertexIndex;
    SkVector            fFirstOutset;
    SkPoint             fFirstPoint;

    bool                fSucceeded;
    bool                fTransparent;
    bool                fIsConvex;

    SkColor             fUmbraColor;
    SkColor             fPenumbraColor;

    SkScalar            fRadius;
    SkScalar            fDirection;
    int                 fPrevUmbraIndex;
    SkVector            fPrevOutset;
    SkPoint             fPrevPoint;
};

static bool compute_normal(const SkPoint& p0, const SkPoint& p1, SkScalar dir,
                           SkVector* newNormal) {
    SkVector normal;
    // compute perpendicular
    normal.fX = p0.fY - p1.fY;
    normal.fY = p1.fX - p0.fX;
    normal *= dir;
    if (!normal.normalize()) {
        return false;
    }
    *newNormal = normal;
    return true;
}

static bool duplicate_pt(const SkPoint& p0, const SkPoint& p1) {
    static constexpr SkScalar kClose = (SK_Scalar1 / 16);
    static constexpr SkScalar kCloseSqd = kClose * kClose;

    SkScalar distSq = SkPointPriv::DistanceToSqd(p0, p1);
    return distSq < kCloseSqd;
}

static SkScalar perp_dot(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2) {
    SkVector v0 = p1 - p0;
    SkVector v1 = p2 - p1;
    return v0.cross(v1);
}

SkBaseShadowTessellator::SkBaseShadowTessellator(const SkPoint3& zPlaneParams, bool transparent)
        : fZPlaneParams(zPlaneParams)
        , fZOffset(0)
        , fCentroid({0, 0})
        , fArea(0)
        , fLastArea(0)
        , fLastCross(0)
        , fFirstVertexIndex(-1)
        , fSucceeded(false)
        , fTransparent(transparent)
        , fIsConvex(true)
        , fDirection(1)
        , fPrevUmbraIndex(-1) {
    // child classes will set reserve for positions, colors and indices
}

bool SkBaseShadowTessellator::setZOffset(const SkRect& bounds, bool perspective) {
    SkScalar minZ = this->heightFunc(bounds.fLeft, bounds.fTop);
    if (perspective) {
        SkScalar z = this->heightFunc(bounds.fLeft, bounds.fBottom);
        if (z < minZ) {
            minZ = z;
        }
        z = this->heightFunc(bounds.fRight, bounds.fTop);
        if (z < minZ) {
            minZ = z;
        }
        z = this->heightFunc(bounds.fRight, bounds.fBottom);
        if (z < minZ) {
            minZ = z;
        }
    }

    if (minZ < kMinHeight) {
        fZOffset = -minZ + kMinHeight;
        return true;
    }

    return false;
}

bool SkBaseShadowTessellator::accumulateCentroid(const SkPoint& curr, const SkPoint& next) {
    if (duplicate_pt(curr, next)) {
        return false;
    }

    SkASSERT(fPathPolygon.count() > 0);
    SkVector v0 = curr - fPathPolygon[0];
    SkVector v1 = next - fPathPolygon[0];
    SkScalar quadArea = v0.cross(v1);
    fCentroid.fX += (v0.fX + v1.fX) * quadArea;
    fCentroid.fY += (v0.fY + v1.fY) * quadArea;
    fArea += quadArea;
    // convexity check
    if (quadArea*fLastArea < 0) {
        fIsConvex = false;
    }
    if (0 != quadArea) {
        fLastArea = quadArea;
    }

    return true;
}

bool SkBaseShadowTessellator::checkConvexity(const SkPoint& p0,
                                             const SkPoint& p1,
                                             const SkPoint& p2) {
    SkScalar cross = perp_dot(p0, p1, p2);
    // skip collinear point
    if (SkScalarNearlyZero(cross)) {
        return false;
    }

    // check for convexity
    if (fLastCross*cross < 0) {
        fIsConvex = false;
    }
    if (0 != cross) {
        fLastCross = cross;
    }

    return true;
}

void SkBaseShadowTessellator::finishPathPolygon() {
    if (fPathPolygon.count() > 1) {
        if (!this->accumulateCentroid(fPathPolygon[fPathPolygon.count() - 1], fPathPolygon[0])) {
            // remove coincident point
            fPathPolygon.pop();
        }
    }

    if (fPathPolygon.count() > 2) {
        // do this before the final convexity check, so we use the correct fPathPolygon[0]
        fCentroid *= sk_ieee_float_divide(1, 3 * fArea);
        fCentroid += fPathPolygon[0];
        if (!checkConvexity(fPathPolygon[fPathPolygon.count() - 2],
                            fPathPolygon[fPathPolygon.count() - 1],
                            fPathPolygon[0])) {
            // remove collinear point
            fPathPolygon[0] = fPathPolygon[fPathPolygon.count() - 1];
            fPathPolygon.pop();
        }
    }

    // if area is positive, winding is ccw
    fDirection = fArea > 0 ? -1 : 1;
}

void SkBaseShadowTessellator::stitchConcaveRings(const SkTDArray<SkPoint>& umbraPolygon,
                                                 SkTDArray<int>* umbraIndices,
                                                 const SkTDArray<SkPoint>& penumbraPolygon,
                                                 SkTDArray<int>* penumbraIndices) {
    // TODO: only create and fill indexMap when fTransparent is true?
    SkAutoSTMalloc<64, uint16_t> indexMap(umbraPolygon.count());

    // find minimum indices
    int minIndex = 0;
    int min = (*penumbraIndices)[0];
    for (int i = 1; i < (*penumbraIndices).count(); ++i) {
        if ((*penumbraIndices)[i] < min) {
            min = (*penumbraIndices)[i];
            minIndex = i;
        }
    }
    int currPenumbra = minIndex;

    minIndex = 0;
    min = (*umbraIndices)[0];
    for (int i = 1; i < (*umbraIndices).count(); ++i) {
        if ((*umbraIndices)[i] < min) {
            min = (*umbraIndices)[i];
            minIndex = i;
        }
    }
    int currUmbra = minIndex;

    // now find a case where the indices are equal (there should be at least one)
    int maxPenumbraIndex = fPathPolygon.count() - 1;
    int maxUmbraIndex = fPathPolygon.count() - 1;
    while ((*penumbraIndices)[currPenumbra] != (*umbraIndices)[currUmbra]) {
        if ((*penumbraIndices)[currPenumbra] < (*umbraIndices)[currUmbra]) {
            (*penumbraIndices)[currPenumbra] += fPathPolygon.count();
            maxPenumbraIndex = (*penumbraIndices)[currPenumbra];
            currPenumbra = (currPenumbra + 1) % penumbraPolygon.count();
        } else {
            (*umbraIndices)[currUmbra] += fPathPolygon.count();
            maxUmbraIndex = (*umbraIndices)[currUmbra];
            currUmbra = (currUmbra + 1) % umbraPolygon.count();
        }
    }

    *fPositions.push() = penumbraPolygon[currPenumbra];
    *fColors.push() = fPenumbraColor;
    int prevPenumbraIndex = 0;
    *fPositions.push() = umbraPolygon[currUmbra];
    *fColors.push() = fUmbraColor;
    fPrevUmbraIndex = 1;
    indexMap[currUmbra] = 1;

    int nextPenumbra = (currPenumbra + 1) % penumbraPolygon.count();
    int nextUmbra = (currUmbra + 1) % umbraPolygon.count();
    while ((*penumbraIndices)[nextPenumbra] <= maxPenumbraIndex ||
           (*umbraIndices)[nextUmbra] <= maxUmbraIndex) {

        if ((*umbraIndices)[nextUmbra] == (*penumbraIndices)[nextPenumbra]) {
            // advance both one step
            *fPositions.push() = penumbraPolygon[nextPenumbra];
            *fColors.push() = fPenumbraColor;
            int currPenumbraIndex = fPositions.count() - 1;

            *fPositions.push() = umbraPolygon[nextUmbra];
            *fColors.push() = fUmbraColor;
            int currUmbraIndex = fPositions.count() - 1;
            indexMap[nextUmbra] = currUmbraIndex;

            this->appendQuad(prevPenumbraIndex, currPenumbraIndex,
                             fPrevUmbraIndex, currUmbraIndex);

            prevPenumbraIndex = currPenumbraIndex;
            (*penumbraIndices)[currPenumbra] += fPathPolygon.count();
            currPenumbra = nextPenumbra;
            nextPenumbra = (currPenumbra + 1) % penumbraPolygon.count();

            fPrevUmbraIndex = currUmbraIndex;
            (*umbraIndices)[currUmbra] += fPathPolygon.count();
            currUmbra = nextUmbra;
            nextUmbra = (currUmbra + 1) % umbraPolygon.count();
        }

        while ((*penumbraIndices)[nextPenumbra] < (*umbraIndices)[nextUmbra] &&
               (*penumbraIndices)[nextPenumbra] <= maxPenumbraIndex) {
            // fill out penumbra arc
            *fPositions.push() = penumbraPolygon[nextPenumbra];
            *fColors.push() = fPenumbraColor;
            int currPenumbraIndex = fPositions.count() - 1;

            this->appendTriangle(prevPenumbraIndex, currPenumbraIndex, fPrevUmbraIndex);

            prevPenumbraIndex = currPenumbraIndex;
            // this ensures the ordering when we wrap around
            (*penumbraIndices)[currPenumbra] += fPathPolygon.count();
            currPenumbra = nextPenumbra;
            nextPenumbra = (currPenumbra + 1) % penumbraPolygon.count();
        }

        while ((*umbraIndices)[nextUmbra] < (*penumbraIndices)[nextPenumbra] &&
               (*umbraIndices)[nextUmbra] <= maxUmbraIndex) {
            // fill out umbra arc
            *fPositions.push() = umbraPolygon[nextUmbra];
            *fColors.push() = fUmbraColor;
            int currUmbraIndex = fPositions.count() - 1;
            indexMap[nextUmbra] = currUmbraIndex;

            this->appendTriangle(fPrevUmbraIndex, prevPenumbraIndex, currUmbraIndex);

            fPrevUmbraIndex = currUmbraIndex;
            // this ensures the ordering when we wrap around
            (*umbraIndices)[currUmbra] += fPathPolygon.count();
            currUmbra = nextUmbra;
            nextUmbra = (currUmbra + 1) % umbraPolygon.count();
        }
    }
    // finish up by advancing both one step
    *fPositions.push() = penumbraPolygon[nextPenumbra];
    *fColors.push() = fPenumbraColor;
    int currPenumbraIndex = fPositions.count() - 1;

    *fPositions.push() = umbraPolygon[nextUmbra];
    *fColors.push() = fUmbraColor;
    int currUmbraIndex = fPositions.count() - 1;
    indexMap[nextUmbra] = currUmbraIndex;

    this->appendQuad(prevPenumbraIndex, currPenumbraIndex,
                     fPrevUmbraIndex, currUmbraIndex);

    if (fTransparent) {
        SkTriangulateSimplePolygon(umbraPolygon.begin(), indexMap, umbraPolygon.count(),
                                   &fIndices);
    }
}


// tesselation tolerance values, in device space pixels
#if SK_SUPPORT_GPU
static const SkScalar kQuadTolerance = 0.2f;
static const SkScalar kCubicTolerance = 0.2f;
#endif
static const SkScalar kConicTolerance = 0.5f;

// clamps the point to the nearest 16th of a pixel
static void sanitize_point(const SkPoint& in, SkPoint* out) {
    out->fX = SkScalarRoundToScalar(16.f*in.fX)*0.0625f;
    out->fY = SkScalarRoundToScalar(16.f*in.fY)*0.0625f;
}

void SkBaseShadowTessellator::handleLine(const SkPoint& p) {
    SkPoint pSanitized;
    sanitize_point(p, &pSanitized);

    if (fPathPolygon.count() > 0) {
        if (!this->accumulateCentroid(fPathPolygon[fPathPolygon.count() - 1], pSanitized)) {
            // skip coincident point
            return;
        }
    }

    if (fPathPolygon.count() > 1) {
        if (!checkConvexity(fPathPolygon[fPathPolygon.count() - 2],
                            fPathPolygon[fPathPolygon.count() - 1],
                            pSanitized)) {
            // remove collinear point
            fPathPolygon.pop();
        }
    }

    *fPathPolygon.push() = pSanitized;
}

void SkBaseShadowTessellator::handleLine(const SkMatrix& m, SkPoint* p) {
    m.mapPoints(p, 1);

    this->handleLine(*p);
}

void SkBaseShadowTessellator::handleQuad(const SkPoint pts[3]) {
#if SK_SUPPORT_GPU
    // check for degeneracy
    SkVector v0 = pts[1] - pts[0];
    SkVector v1 = pts[2] - pts[0];
    if (SkScalarNearlyZero(v0.cross(v1))) {
        return;
    }
    // TODO: Pull PathUtils out of Ganesh?
    int maxCount = GrPathUtils::quadraticPointCount(pts, kQuadTolerance);
    fPointBuffer.setCount(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2],
                                                     kQuadTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count; i++) {
        this->handleLine(fPointBuffer[i]);
    }
#else
    // for now, just to draw something
    this->handleLine(pts[1]);
    this->handleLine(pts[2]);
#endif
}

void SkBaseShadowTessellator::handleQuad(const SkMatrix& m, SkPoint pts[3]) {
    m.mapPoints(pts, 3);
    this->handleQuad(pts);
}

void SkBaseShadowTessellator::handleCubic(const SkMatrix& m, SkPoint pts[4]) {
    m.mapPoints(pts, 4);
#if SK_SUPPORT_GPU
    // TODO: Pull PathUtils out of Ganesh?
    int maxCount = GrPathUtils::cubicPointCount(pts, kCubicTolerance);
    fPointBuffer.setCount(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateCubicPoints(pts[0], pts[1], pts[2], pts[3],
                                                 kCubicTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count; i++) {
        this->handleLine(fPointBuffer[i]);
    }
#else
    // for now, just to draw something
    this->handleLine(pts[1]);
    this->handleLine(pts[2]);
    this->handleLine(pts[3]);
#endif
}

void SkBaseShadowTessellator::handleConic(const SkMatrix& m, SkPoint pts[3], SkScalar w) {
    if (m.hasPerspective()) {
        w = SkConic::TransformW(pts, w, m);
    }
    m.mapPoints(pts, 3);
    SkAutoConicToQuads quadder;
    const SkPoint* quads = quadder.computeQuads(pts, w, kConicTolerance);
    SkPoint lastPoint = *(quads++);
    int count = quadder.countQuads();
    for (int i = 0; i < count; ++i) {
        SkPoint quadPts[3];
        quadPts[0] = lastPoint;
        quadPts[1] = quads[0];
        quadPts[2] = i == count - 1 ? pts[2] : quads[1];
        this->handleQuad(quadPts);
        lastPoint = quadPts[2];
        quads += 2;
    }
}

bool SkBaseShadowTessellator::addArc(const SkVector& nextNormal, bool finishArc) {
    // fill in fan from previous quad
    SkScalar rotSin, rotCos;
    int numSteps;
    if (!SkComputeRadialSteps(fPrevOutset, nextNormal, fRadius, &rotSin, &rotCos, &numSteps)) {
        // recover as best we can
        numSteps = 0;
    }
    SkVector prevNormal = fPrevOutset;
    for (int i = 0; i < numSteps-1; ++i) {
        SkVector currNormal;
        currNormal.fX = prevNormal.fX*rotCos - prevNormal.fY*rotSin;
        currNormal.fY = prevNormal.fY*rotCos + prevNormal.fX*rotSin;
        *fPositions.push() = fPrevPoint + currNormal;
        *fColors.push() = fPenumbraColor;
        this->appendTriangle(fPrevUmbraIndex, fPositions.count() - 1, fPositions.count() - 2);

        prevNormal = currNormal;
    }
    if (finishArc && numSteps) {
        *fPositions.push() = fPrevPoint + nextNormal;
        *fColors.push() = fPenumbraColor;
        this->appendTriangle(fPrevUmbraIndex, fPositions.count() - 1, fPositions.count() - 2);
    }
    fPrevOutset = nextNormal;

    return (numSteps > 0);
}

void SkBaseShadowTessellator::appendTriangle(uint16_t index0, uint16_t index1, uint16_t index2) {
    auto indices = fIndices.append(3);

    indices[0] = index0;
    indices[1] = index1;
    indices[2] = index2;
}

void SkBaseShadowTessellator::appendQuad(uint16_t index0, uint16_t index1,
                                         uint16_t index2, uint16_t index3) {
    auto indices = fIndices.append(6);

    indices[0] = index0;
    indices[1] = index1;
    indices[2] = index2;

    indices[3] = index2;
    indices[4] = index1;
    indices[5] = index3;
}

bool SkBaseShadowTessellator::setTransformedHeightFunc(const SkMatrix& ctm) {
    if (SkScalarNearlyZero(fZPlaneParams.fX) && SkScalarNearlyZero(fZPlaneParams.fY)) {
        fTransformedHeightFunc = [this](const SkPoint& p) {
            return fZPlaneParams.fZ;
        };
    } else {
        SkMatrix ctmInverse;
        if (!ctm.invert(&ctmInverse) || !ctmInverse.isFinite()) {
            return false;
        }
        // multiply by transpose
        fTransformedZParams = SkPoint3::Make(
            ctmInverse[SkMatrix::kMScaleX] * fZPlaneParams.fX +
            ctmInverse[SkMatrix::kMSkewY] * fZPlaneParams.fY +
            ctmInverse[SkMatrix::kMPersp0] * fZPlaneParams.fZ,

            ctmInverse[SkMatrix::kMSkewX] * fZPlaneParams.fX +
            ctmInverse[SkMatrix::kMScaleY] * fZPlaneParams.fY +
            ctmInverse[SkMatrix::kMPersp1] * fZPlaneParams.fZ,

            ctmInverse[SkMatrix::kMTransX] * fZPlaneParams.fX +
            ctmInverse[SkMatrix::kMTransY] * fZPlaneParams.fY +
            ctmInverse[SkMatrix::kMPersp2] * fZPlaneParams.fZ
        );

        if (ctm.hasPerspective()) {
            // We use Cramer's rule to solve for the W value for a given post-divide X and Y,
            // so pre-compute those values that are independent of X and Y.
            // W is det(ctmInverse)/(PD[0]*X + PD[1]*Y + PD[2])
            fPartialDeterminants[0] = ctm[SkMatrix::kMSkewY] * ctm[SkMatrix::kMPersp1] -
                                      ctm[SkMatrix::kMScaleY] * ctm[SkMatrix::kMPersp0];
            fPartialDeterminants[1] = ctm[SkMatrix::kMPersp0] * ctm[SkMatrix::kMSkewX] -
                                      ctm[SkMatrix::kMPersp1] * ctm[SkMatrix::kMScaleX];
            fPartialDeterminants[2] = ctm[SkMatrix::kMScaleX] * ctm[SkMatrix::kMScaleY] -
                                      ctm[SkMatrix::kMSkewX] * ctm[SkMatrix::kMSkewY];
            SkScalar ctmDeterminant = ctm[SkMatrix::kMTransX] * fPartialDeterminants[0] +
                                      ctm[SkMatrix::kMTransY] * fPartialDeterminants[1] +
                                      ctm[SkMatrix::kMPersp2] * fPartialDeterminants[2];

            // Pre-bake the numerator of Cramer's rule into the zParams to avoid another multiply.
            // TODO: this may introduce numerical instability, but I haven't seen any issues yet.
            fTransformedZParams.fX *= ctmDeterminant;
            fTransformedZParams.fY *= ctmDeterminant;
            fTransformedZParams.fZ *= ctmDeterminant;

            fTransformedHeightFunc = [this](const SkPoint& p) {
                SkScalar denom = p.fX * fPartialDeterminants[0] +
                                 p.fY * fPartialDeterminants[1] +
                                 fPartialDeterminants[2];
                SkScalar w = SkScalarFastInvert(denom);
                return fZOffset + w*(fTransformedZParams.fX * p.fX +
                                     fTransformedZParams.fY * p.fY +
                                     fTransformedZParams.fZ);
            };
        } else {
            fTransformedHeightFunc = [this](const SkPoint& p) {
                return fZOffset + fTransformedZParams.fX * p.fX +
                       fTransformedZParams.fY * p.fY + fTransformedZParams.fZ;
            };
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////

class SkAmbientShadowTessellator : public SkBaseShadowTessellator {
public:
    SkAmbientShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                               const SkPoint3& zPlaneParams, bool transparent);

private:
    bool computePathPolygon(const SkPath& path, const SkMatrix& ctm);
    bool computeConvexShadow();
    bool computeConcaveShadow();

    void handlePolyPoint(const SkPoint& p, bool finalPoint);
    void addEdge(const SkPoint& nextPoint, const SkVector& nextNormal, bool finalEdge);
    void splitEdge(const SkPoint& nextPoint, const SkVector& insetNormal,
                   const SkPoint& penumbraPoint, const SkPoint& umbraPoint, SkColor umbraColor);

    static constexpr auto kMaxEdgeLenSqr = 20 * 20;
    static constexpr auto kInsetFactor = -0.5f;

    SkScalar offset(SkScalar z) {
        return SkDrawShadowMetrics::AmbientBlurRadius(z);
    }
    SkColor umbraColor(SkScalar z) {
        SkScalar umbraAlpha = SkScalarInvert(SkDrawShadowMetrics::AmbientRecipAlpha(z));
        return SkColorSetARGB(umbraAlpha * 255.9999f, 0, 0, 0);
    }

    bool                fSplitFirstEdge;
    bool                fSplitPreviousEdge;

    typedef SkBaseShadowTessellator INHERITED;
};

SkAmbientShadowTessellator::SkAmbientShadowTessellator(const SkPath& path,
                                                       const SkMatrix& ctm,
                                                       const SkPoint3& zPlaneParams,
                                                       bool transparent)
        : INHERITED(zPlaneParams, transparent)
        , fSplitFirstEdge(false)
        , fSplitPreviousEdge(false) {
    // Set base colors
    SkScalar umbraAlpha = SkScalarInvert(SkDrawShadowMetrics::AmbientRecipAlpha(heightFunc(0, 0)));
    // umbraColor is the interior value, penumbraColor the exterior value.
    // umbraAlpha is the factor that is linearly interpolated from outside to inside, and
    // then "blurred" by the GrBlurredEdgeFP. It is then multiplied by fAmbientAlpha to get
    // the final alpha.
    fUmbraColor = SkColorSetARGB(umbraAlpha * 255.9999f, 0, 0, 0);
    fPenumbraColor = SkColorSetARGB(0, 0, 0, 0);

    // make sure we're not below the canvas plane
    this->setZOffset(path.getBounds(), ctm.hasPerspective());

    if (!this->setTransformedHeightFunc(ctm)) {
        return;
    }

    if (!this->computePathPolygon(path, ctm)) {
        return;
    }
    if (fPathPolygon.count() < 3 || !SkScalarIsFinite(fArea)) {
        fSucceeded = true; // We don't want to try to blur these cases, so we will
                           // return an empty SkVertices instead.
        return;
    }

    // Outer ring: 3*numPts
    // Middle ring: numPts
    fPositions.setReserve(4 * path.countPoints());
    fColors.setReserve(4 * path.countPoints());
    // Outer ring: 12*numPts
    // Middle ring: 0
    fIndices.setReserve(12 * path.countPoints());

    if (fIsConvex) {
        fSucceeded = this->computeConvexShadow();
    } else {
        fSucceeded = this->computeConcaveShadow();
    }
}

bool SkAmbientShadowTessellator::computePathPolygon(const SkPath& path, const SkMatrix& ctm) {
    fPathPolygon.setReserve(path.countPoints());

    // walk around the path, tessellate and generate outer ring
    // if original path is transparent, will accumulate sum of points for centroid
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    bool verbSeen = false;
    bool closeSeen = false;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (closeSeen) {
            return false;
        }
        switch (verb) {
            case SkPath::kLine_Verb:
                this->handleLine(ctm, &pts[1]);
                break;
            case SkPath::kQuad_Verb:
                this->handleQuad(ctm, pts);
                break;
            case SkPath::kCubic_Verb:
                this->handleCubic(ctm, pts);
                break;
            case SkPath::kConic_Verb:
                this->handleConic(ctm, pts, iter.conicWeight());
                break;
            case SkPath::kMove_Verb:
                if (verbSeen) {
                    return false;
                }
                break;
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                closeSeen = true;
                break;
        }
        verbSeen = true;
    }

    this->finishPathPolygon();
    return true;
}

bool SkAmbientShadowTessellator::computeConvexShadow() {
    int polyCount = fPathPolygon.count();
    if (polyCount < 3) {
        return false;
    }

    // Add center point for fan if needed
    if (fTransparent) {
        *fPositions.push() = fCentroid;
        *fColors.push() = this->umbraColor(fTransformedHeightFunc(fCentroid));
    }

    // Initialize
    SkVector normal;
    if (!compute_normal(fPathPolygon[polyCount-1], fPathPolygon[0], fDirection, &normal)) {
        // the polygon should be sanitized, so any issues at this point are unrecoverable
        return false;
    }
    fFirstPoint = fPathPolygon[polyCount - 1];
    fFirstVertexIndex = fPositions.count();
    SkScalar z = fTransformedHeightFunc(fFirstPoint);
    fFirstOutset = normal;
    fFirstOutset *= this->offset(z);

    fPrevOutset = fFirstOutset;
    fPrevPoint = fFirstPoint;
    fPrevUmbraIndex = fFirstVertexIndex;

    // Add the first quad
    *fPositions.push() = fFirstPoint;
    *fColors.push() = this->umbraColor(z);
    *fPositions.push() = fFirstPoint + fFirstOutset;
    *fColors.push() = fPenumbraColor;

    z = fTransformedHeightFunc(fPathPolygon[0]);
    fRadius = this->offset(z);
    fUmbraColor = this->umbraColor(z);
    this->addEdge(fPathPolygon[0], normal, false);

    // Process the remaining points
    for (int i = 1; i < fPathPolygon.count(); ++i) {
        this->handlePolyPoint(fPathPolygon[i], i == fPathPolygon.count()-1);
    }
    SkASSERT(this->indexCount());

    // Final fan
    SkASSERT(fPositions.count() >= 3);
    if (this->addArc(fFirstOutset, false)) {
        this->appendTriangle(fFirstVertexIndex, fPositions.count() - 1, fFirstVertexIndex + 1);
    } else {
        // arc is too small, set the first penumbra point to be the same position
        // as the last one
        fPositions[fFirstVertexIndex + 1] = fPositions[fPositions.count() - 1];
    }

    return true;
}

bool SkAmbientShadowTessellator::computeConcaveShadow() {
    if (!SkIsSimplePolygon(&fPathPolygon[0], fPathPolygon.count())) {
        return false;
    }

    // generate inner ring
    SkTDArray<SkPoint> umbraPolygon;
    SkTDArray<int> umbraIndices;
    umbraIndices.setReserve(fPathPolygon.count());
    if (!SkOffsetSimplePolygon(&fPathPolygon[0], fPathPolygon.count(), 0.5f,
                               &umbraPolygon, &umbraIndices)) {
        // TODO: figure out how to handle this case
        return false;
    }

    // generate outer ring
    SkTDArray<SkPoint> penumbraPolygon;
    SkTDArray<int> penumbraIndices;
    penumbraPolygon.setReserve(umbraPolygon.count());
    penumbraIndices.setReserve(umbraPolygon.count());

    auto offsetFunc = [this](const SkPoint& p) { return -this->offset(fTransformedHeightFunc(p)); };
    if (!SkOffsetSimplePolygon(&fPathPolygon[0], fPathPolygon.count(), offsetFunc,
                               &penumbraPolygon, &penumbraIndices)) {
        // TODO: figure out how to handle this case
        return false;
    }

    if (!umbraPolygon.count() || !penumbraPolygon.count()) {
        return false;
    }

    // attach the rings together
    this->stitchConcaveRings(umbraPolygon, &umbraIndices, penumbraPolygon, &penumbraIndices);

    return true;
}

void SkAmbientShadowTessellator::handlePolyPoint(const SkPoint& p, bool finalPoint)  {
    SkVector normal;
    if (compute_normal(fPrevPoint, p, fDirection, &normal)) {
        SkVector scaledNormal = normal;
        scaledNormal *= fRadius;
        this->addArc(scaledNormal, true);
        SkScalar z = fTransformedHeightFunc(p);
        fRadius = this->offset(z);
        fUmbraColor = this->umbraColor(z);
        this->addEdge(p, normal, finalPoint);
    }
}

void SkAmbientShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal,
                                         bool finalEdge) {
    // We compute the inset in two stages: first we inset by half the current normal,
    // then on the next addEdge() we add half of the next normal to get an average of the two
    SkVector insetNormal = nextNormal;
    insetNormal *= 0.5f*kInsetFactor;

    // Adding the other half of the average for the previous edge
    fPositions[fPrevUmbraIndex] += insetNormal;

    SkPoint umbraPoint;
    if (finalEdge) {
        // Again, adding the other half of the average for the previous edge
        fPositions[fFirstVertexIndex] += insetNormal;
        // we multiply by another half because now we're adding to an average of an average
        if (fSplitFirstEdge) {
            fPositions[fFirstVertexIndex + 2] += insetNormal * 0.5f;
        }
        umbraPoint = fPositions[fFirstVertexIndex];
    } else {
        umbraPoint = nextPoint + insetNormal;
    }
    SkVector outsetNormal = nextNormal;
    outsetNormal *= fRadius;
    SkPoint penumbraPoint = nextPoint + outsetNormal;

    // make sure we don't end up with a sharp alpha edge along the quad diagonal
    this->splitEdge(nextPoint, insetNormal, penumbraPoint, umbraPoint, fUmbraColor);

    // add next quad
    int prevPenumbraIndex;
    int currUmbraIndex;
    if (finalEdge) {
        prevPenumbraIndex = fPositions.count() - 1;
        currUmbraIndex = fFirstVertexIndex;
    } else {
        prevPenumbraIndex = fPositions.count() - 1;
        *fPositions.push() = umbraPoint;
        *fColors.push() = fUmbraColor;
        currUmbraIndex = fPositions.count() - 1;
    }

    *fPositions.push() = penumbraPoint;
    *fColors.push() = fPenumbraColor;

    // set triangularization to get best interpolation of color
    if (fColors[fPrevUmbraIndex] > fUmbraColor) {
        this->appendQuad(fPrevUmbraIndex, prevPenumbraIndex,
                         currUmbraIndex, fPositions.count() - 1);
    } else {
        this->appendQuad(currUmbraIndex, fPositions.count() - 1,
                         fPrevUmbraIndex, prevPenumbraIndex);
    }

    // if transparent, add to center fan
    if (fTransparent) {
        this->appendTriangle(0, fPrevUmbraIndex, currUmbraIndex);
    }

    fPrevUmbraIndex = currUmbraIndex;
    fPrevPoint = nextPoint;
    fPrevOutset = outsetNormal;
}

void SkAmbientShadowTessellator::splitEdge(const SkPoint& nextPoint, const SkVector& insetNormal,
                                           const SkPoint& penumbraPoint, const SkPoint& umbraPoint,
                                           SkColor umbraColor) {
    // For split edges, we're adding an average of two averages, so we multiply by another half
    if (fSplitPreviousEdge) {
        fPositions[fPrevUmbraIndex - 2] += insetNormal*SK_ScalarHalf;
    }

    // Split the edge to make sure we don't end up with a sharp alpha edge along the quad diagonal
    if (fColors[fPrevUmbraIndex] != umbraColor &&
        SkPointPriv::DistanceToSqd(nextPoint, fPositions[fPrevUmbraIndex]) > kMaxEdgeLenSqr) {

        // This is lacking 1/4 of the next inset -- we'll add it the next time we call addEdge()
        SkPoint centerPoint = fPositions[fPrevUmbraIndex] + umbraPoint;
        centerPoint *= 0.5f;
        *fPositions.push() = centerPoint;
        *fColors.push() = SkPMLerp(umbraColor, fColors[fPrevUmbraIndex], 128);
        centerPoint = fPositions[fPositions.count() - 2] + penumbraPoint;
        centerPoint *= 0.5f;
        *fPositions.push() = centerPoint;
        *fColors.push() = fPenumbraColor;

        // set triangularization to get best interpolation of color
        if (fColors[fPrevUmbraIndex] > fColors[fPositions.count() - 2]) {
            this->appendQuad(fPrevUmbraIndex, fPositions.count() - 3,
                             fPositions.count() - 2, fPositions.count() - 1);
        } else {
            this->appendQuad(fPositions.count() - 2, fPositions.count() - 1,
                             fPrevUmbraIndex, fPositions.count() - 3);
        }

        // if transparent, add to center fan
        if (fTransparent) {
            this->appendTriangle(0, fPrevUmbraIndex, fPositions.count() - 2);
        }

        fSplitPreviousEdge = true;
        if (fPrevUmbraIndex == fFirstVertexIndex) {
            fSplitFirstEdge = true;
        }
        fPrevUmbraIndex = fPositions.count() - 2;
    } else {
        fSplitPreviousEdge = false;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////

class SkSpotShadowTessellator : public SkBaseShadowTessellator {
public:
    SkSpotShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                            const SkPoint3& zPlaneParams, const SkPoint3& lightPos,
                            SkScalar lightRadius, bool transparent);

private:
    bool computeClipAndPathPolygons(const SkPath& path, const SkMatrix& ctm,
                                    const SkMatrix& shadowTransform);
    void computeClipVectorsAndTestCentroid();
    bool clipUmbraPoint(const SkPoint& umbraPoint, const SkPoint& centroid, SkPoint* clipPoint);
    int getClosestUmbraPoint(const SkPoint& point);

    bool computeConvexShadow(SkScalar radius);
    bool computeConcaveShadow(SkScalar radius);

    bool handlePolyPoint(const SkPoint& p, bool lastPoint);

    void mapPoints(SkScalar scale, const SkVector& xlate, SkPoint* pts, int count);
    bool addInnerPoint(const SkPoint& pathPoint, int* currUmbraIndex);
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal, bool lastEdge);
    void addToClip(const SkVector& nextPoint);

    SkScalar offset(SkScalar z) {
        float zRatio = SkTPin(z / (fLightZ - z), 0.0f, 0.95f);
        return fLightRadius*zRatio;
    }

    SkScalar            fLightZ;
    SkScalar            fLightRadius;
    SkScalar            fOffsetAdjust;

    SkTDArray<SkPoint>  fClipPolygon;
    SkTDArray<SkVector> fClipVectors;

    SkTDArray<SkPoint>  fUmbraPolygon;
    int                 fCurrClipPoint;
    int                 fCurrUmbraPoint;
    bool                fPrevUmbraOutside;
    bool                fFirstUmbraOutside;
    bool                fValidUmbra;

    typedef SkBaseShadowTessellator INHERITED;
};

SkSpotShadowTessellator::SkSpotShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                                                 const SkPoint3& zPlaneParams,
                                                 const SkPoint3& lightPos, SkScalar lightRadius,
                                                 bool transparent)
    : INHERITED(zPlaneParams, transparent)
    , fLightZ(lightPos.fZ)
    , fLightRadius(lightRadius)
    , fOffsetAdjust(0)
    , fCurrClipPoint(0)
    , fPrevUmbraOutside(false)
    , fFirstUmbraOutside(false)
    , fValidUmbra(true) {

    // make sure we're not below the canvas plane
    if (this->setZOffset(path.getBounds(), ctm.hasPerspective())) {
        // Adjust light height and radius
        fLightRadius *= (fLightZ + fZOffset) / fLightZ;
        fLightZ += fZOffset;
    }

    // Set radius and colors
    SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
    SkScalar occluderHeight = this->heightFunc(center.fX, center.fY) + fZOffset;
    fUmbraColor = SkColorSetARGB(255, 0, 0, 0);
    fPenumbraColor = SkColorSetARGB(0, 0, 0, 0);

    // Compute the blur radius, scale and translation for the spot shadow.
    SkScalar radius;
    SkMatrix shadowTransform;
    if (!ctm.hasPerspective()) {
        SkScalar scale;
        SkVector translate;
        SkDrawShadowMetrics::GetSpotParams(occluderHeight, lightPos.fX, lightPos.fY, fLightZ,
                                           lightRadius, &radius, &scale, &translate);
        shadowTransform.setScaleTranslate(scale, scale, translate.fX, translate.fY);
    } else {
        // For perspective, we have a scale, a z-shear, and another projective divide --
        // this varies at each point so we can't use an affine transform.
        // We'll just apply this to each generated point in turn.
        shadowTransform.reset();
        // Also can't cull the center (for now).
        fTransparent = true;
        radius = SkDrawShadowMetrics::SpotBlurRadius(occluderHeight, lightPos.fZ, lightRadius);
    }
    fRadius = radius;
    SkMatrix fullTransform = SkMatrix::Concat(shadowTransform, ctm);

    // Set up our reverse mapping
    if (!this->setTransformedHeightFunc(fullTransform)) {
        return;
    }

    // compute rough clip bounds for umbra, plus offset polygon, plus centroid
    if (!this->computeClipAndPathPolygons(path, ctm, shadowTransform)) {
        return;
    }
    if (fClipPolygon.count() < 3 || fPathPolygon.count() < 3 || !SkScalarIsFinite(fArea)) {
        fSucceeded = true; // We don't want to try to blur these cases, so we will
                           // return an empty SkVertices instead.
        return;
    }

    // compute vectors for clip tests
    this->computeClipVectorsAndTestCentroid();

    // check to see if umbra collapses
    if (fIsConvex) {
        SkScalar minDistSq = SkPointPriv::DistanceToLineSegmentBetweenSqd(fCentroid,
                                                                          fPathPolygon[0],
                                                                          fPathPolygon[1]);
        SkRect bounds;
        bounds.setBounds(&fPathPolygon[0], fPathPolygon.count());
        for (int i = 1; i < fPathPolygon.count(); ++i) {
            int j = i + 1;
            if (i == fPathPolygon.count() - 1) {
                j = 0;
            }
            SkPoint currPoint = fPathPolygon[i];
            SkPoint nextPoint = fPathPolygon[j];
            SkScalar distSq = SkPointPriv::DistanceToLineSegmentBetweenSqd(fCentroid, currPoint,
                                                                           nextPoint);
            if (distSq < minDistSq) {
                minDistSq = distSq;
            }
        }
        static constexpr auto kTolerance = 1.0e-2f;
        if (minDistSq < (radius + kTolerance)*(radius + kTolerance)) {
            // if the umbra would collapse, we back off a bit on inner blur and adjust the alpha
            SkScalar newRadius = SkScalarSqrt(minDistSq) - kTolerance;
            fOffsetAdjust = newRadius - radius;
            SkScalar ratio = 128 * (newRadius + radius) / radius;
            // they aren't PMColors, but the interpolation algorithm is the same
            fUmbraColor = SkPMLerp(fUmbraColor, fPenumbraColor, (unsigned)ratio);
            radius = newRadius;
        }
    }

    // TODO: calculate these reserves better
    // Penumbra ring: 3*numPts
    // Umbra ring: numPts
    // Inner ring: numPts
    fPositions.setReserve(5 * path.countPoints());
    fColors.setReserve(5 * path.countPoints());
    // Penumbra ring: 12*numPts
    // Umbra ring: 3*numPts
    fIndices.setReserve(15 * path.countPoints());

    if (fIsConvex) {
        fSucceeded = this->computeConvexShadow(radius);
    } else {
        fSucceeded = this->computeConcaveShadow(radius);
    }

    if (!fSucceeded) {
        return;
    }

    if (ctm.hasPerspective()) {
        for (int i = 0; i < fPositions.count(); ++i) {
            SkScalar pathZ = fTransformedHeightFunc(fPositions[i]);
            SkScalar factor = SkScalarInvert(fLightZ - pathZ);
            fPositions[i].fX = (fPositions[i].fX*fLightZ - lightPos.fX*pathZ)*factor;
            fPositions[i].fY = (fPositions[i].fY*fLightZ - lightPos.fY*pathZ)*factor;
        }
#ifdef DRAW_CENTROID
        SkScalar pathZ = fTransformedHeightFunc(fCentroid);
        SkScalar factor = SkScalarInvert(fLightZ - pathZ);
        fCentroid.fX = (fCentroid.fX*fLightZ - lightPos.fX*pathZ)*factor;
        fCentroid.fY = (fCentroid.fY*fLightZ - lightPos.fY*pathZ)*factor;
#endif
    }
#ifdef DRAW_CENTROID
    *fPositions.push() = fCentroid + SkVector::Make(-2, -2);
    *fColors.push() = SkColorSetARGB(255, 0, 255, 255);
    *fPositions.push() = fCentroid + SkVector::Make(2, -2);
    *fColors.push() = SkColorSetARGB(255, 0, 255, 255);
    *fPositions.push() = fCentroid + SkVector::Make(-2, 2);
    *fColors.push() = SkColorSetARGB(255, 0, 255, 255);
    *fPositions.push() = fCentroid + SkVector::Make(2, 2);
    *fColors.push() = SkColorSetARGB(255, 0, 255, 255);

    this->appendQuad(fPositions.count() - 2, fPositions.count() - 1,
                     fPositions.count() - 4, fPositions.count() - 3);
#endif

    fSucceeded = true;
}

void SkSpotShadowTessellator::addToClip(const SkPoint& point) {
    if (fClipPolygon.isEmpty() || !duplicate_pt(point, fClipPolygon[fClipPolygon.count()-1])) {
        *fClipPolygon.push() = point;
    }
}

bool SkSpotShadowTessellator::computeClipAndPathPolygons(const SkPath& path, const SkMatrix& ctm,
                                                         const SkMatrix& shadowTransform) {

    fPathPolygon.setReserve(path.countPoints());
    fClipPolygon.setReserve(path.countPoints());

    // Walk around the path and compute clip polygon and path polygon.
    // Will also accumulate sum of areas for centroid.
    // For Bezier curves, we compute additional interior points on curve.
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;

    // coefficients to compute cubic Bezier at t = 5/16
    static constexpr SkScalar kA = 0.32495117187f;
    static constexpr SkScalar kB = 0.44311523437f;
    static constexpr SkScalar kC = 0.20141601562f;
    static constexpr SkScalar kD = 0.03051757812f;

    SkPoint curvePoint;
    SkScalar w;
    bool closeSeen = false;
    bool verbSeen = false;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        if (closeSeen) {
            return false;
        }
        switch (verb) {
            case SkPath::kLine_Verb:
                ctm.mapPoints(&pts[1], 1);
                this->addToClip(pts[1]);
                this->handleLine(shadowTransform, &pts[1]);
                break;
            case SkPath::kQuad_Verb:
                ctm.mapPoints(pts, 3);
                // point at t = 1/2
                curvePoint.fX = 0.25f*pts[0].fX + 0.5f*pts[1].fX + 0.25f*pts[2].fX;
                curvePoint.fY = 0.25f*pts[0].fY + 0.5f*pts[1].fY + 0.25f*pts[2].fY;
                this->addToClip(curvePoint);
                this->addToClip(pts[2]);
                this->handleQuad(shadowTransform, pts);
                break;
            case SkPath::kConic_Verb:
                ctm.mapPoints(pts, 3);
                w = iter.conicWeight();
                // point at t = 1/2
                curvePoint.fX = 0.25f*pts[0].fX + w*0.5f*pts[1].fX + 0.25f*pts[2].fX;
                curvePoint.fY = 0.25f*pts[0].fY + w*0.5f*pts[1].fY + 0.25f*pts[2].fY;
                curvePoint *= SkScalarInvert(0.5f + 0.5f*w);
                this->addToClip(curvePoint);
                this->addToClip(pts[2]);
                this->handleConic(shadowTransform, pts, w);
                break;
            case SkPath::kCubic_Verb:
                ctm.mapPoints(pts, 4);
                // point at t = 5/16
                curvePoint.fX = kA*pts[0].fX + kB*pts[1].fX + kC*pts[2].fX + kD*pts[3].fX;
                curvePoint.fY = kA*pts[0].fY + kB*pts[1].fY + kC*pts[2].fY + kD*pts[3].fY;
                this->addToClip(curvePoint);
                // point at t = 11/16
                curvePoint.fX = kD*pts[0].fX + kC*pts[1].fX + kB*pts[2].fX + kA*pts[3].fX;
                curvePoint.fY = kD*pts[0].fY + kC*pts[1].fY + kB*pts[2].fY + kA*pts[3].fY;
                this->addToClip(curvePoint);
                this->addToClip(pts[3]);
                this->handleCubic(shadowTransform, pts);
                break;
            case SkPath::kMove_Verb:
                if (verbSeen) {
                    return false;
                }
                break;
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                closeSeen = true;
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
        verbSeen = true;
    }

    this->finishPathPolygon();
    fCurrClipPoint = fClipPolygon.count() - 1;
    return true;
}

void SkSpotShadowTessellator::computeClipVectorsAndTestCentroid() {
    SkASSERT(fClipPolygon.count() >= 3);

    // init clip vectors
    SkVector v0 = fClipPolygon[1] - fClipPolygon[0];
    SkVector v1 = fClipPolygon[2] - fClipPolygon[0];
    *fClipVectors.push() = v0;

    // init centroid check
    bool hiddenCentroid = true;
    v1 = fCentroid - fClipPolygon[0];
    SkScalar initCross = v0.cross(v1);

    for (int p = 1; p < fClipPolygon.count(); ++p) {
        // add to clip vectors
        v0 = fClipPolygon[(p + 1) % fClipPolygon.count()] - fClipPolygon[p];
        *fClipVectors.push() = v0;
        // Determine if transformed centroid is inside clipPolygon.
        v1 = fCentroid - fClipPolygon[p];
        if (initCross*v0.cross(v1) <= 0) {
            hiddenCentroid = false;
        }
    }
    SkASSERT(fClipVectors.count() == fClipPolygon.count());

    fTransparent = fTransparent || !hiddenCentroid;
}

bool SkSpotShadowTessellator::clipUmbraPoint(const SkPoint& umbraPoint, const SkPoint& centroid,
                                             SkPoint* clipPoint) {
    SkVector segmentVector = centroid - umbraPoint;

    int startClipPoint = fCurrClipPoint;
    do {
        SkVector dp = umbraPoint - fClipPolygon[fCurrClipPoint];
        SkScalar denom = fClipVectors[fCurrClipPoint].cross(segmentVector);
        SkScalar t_num = dp.cross(segmentVector);
        // if line segments are nearly parallel
        if (SkScalarNearlyZero(denom)) {
            // and collinear
            if (SkScalarNearlyZero(t_num)) {
                return false;
            }
            // otherwise are separate, will try the next poly segment
        // else if crossing lies within poly segment
        } else if (t_num >= 0 && t_num <= denom) {
            SkScalar s_num = dp.cross(fClipVectors[fCurrClipPoint]);
            // if umbra point is inside the clip polygon
            if (s_num >= 0 && s_num <= denom) {
                segmentVector *= s_num/denom;
                *clipPoint = umbraPoint + segmentVector;
                return true;
            }
        }
        fCurrClipPoint = (fCurrClipPoint + 1) % fClipPolygon.count();
    } while (fCurrClipPoint != startClipPoint);

    return false;
}

int SkSpotShadowTessellator::getClosestUmbraPoint(const SkPoint& p) {
    SkScalar minDistance = SkPointPriv::DistanceToSqd(p, fUmbraPolygon[fCurrUmbraPoint]);
    int index = fCurrUmbraPoint;
    int dir = 1;
    int next = (index + dir) % fUmbraPolygon.count();

    // init travel direction
    SkScalar distance = SkPointPriv::DistanceToSqd(p, fUmbraPolygon[next]);
    if (distance < minDistance) {
        index = next;
        minDistance = distance;
    } else {
        dir = fUmbraPolygon.count()-1;
    }

    // iterate until we find a point that increases the distance
    next = (index + dir) % fUmbraPolygon.count();
    distance = SkPointPriv::DistanceToSqd(p, fUmbraPolygon[next]);
    while (distance < minDistance) {
        index = next;
        minDistance = distance;
        next = (index + dir) % fUmbraPolygon.count();
        distance = SkPointPriv::DistanceToSqd(p, fUmbraPolygon[next]);
    }

    fCurrUmbraPoint = index;
    return index;
}

bool SkSpotShadowTessellator::computeConvexShadow(SkScalar radius) {
    // generate inner ring
    if (!SkInsetConvexPolygon(&fPathPolygon[0], fPathPolygon.count(), radius,
                              &fUmbraPolygon)) {
        // this shouldn't happen, but just in case we'll inset using the centroid
        fValidUmbra = false;
    }

    // walk around the path polygon, generate outer ring and connect to inner ring
    if (fTransparent) {
        *fPositions.push() = fCentroid;
        *fColors.push() = fUmbraColor;
    }
    fCurrUmbraPoint = 0;

    // initial setup
    // add first quad
    int polyCount = fPathPolygon.count();
    if (!compute_normal(fPathPolygon[polyCount-1], fPathPolygon[0], fDirection, &fFirstOutset)) {
        // polygon should be sanitized by this point, so this is unrecoverable
        return false;
    }

    fFirstOutset *= fRadius;
    fFirstPoint = fPathPolygon[polyCount - 1];
    fFirstVertexIndex = fPositions.count();
    fPrevOutset = fFirstOutset;
    fPrevPoint = fFirstPoint;
    fPrevUmbraIndex = -1;

    this->addInnerPoint(fFirstPoint, &fPrevUmbraIndex);

    if (!fTransparent) {
        SkPoint clipPoint;
        bool isOutside = this->clipUmbraPoint(fPositions[fFirstVertexIndex],
                                              fCentroid, &clipPoint);
        if (isOutside) {
            *fPositions.push() = clipPoint;
            *fColors.push() = fUmbraColor;
        }
        fPrevUmbraOutside = isOutside;
        fFirstUmbraOutside = isOutside;
    }

    SkPoint newPoint = fFirstPoint + fFirstOutset;
    *fPositions.push() = newPoint;
    *fColors.push() = fPenumbraColor;
    this->addEdge(fPathPolygon[0], fFirstOutset, false);

    for (int i = 1; i < polyCount; ++i) {
        if (!this->handlePolyPoint(fPathPolygon[i], i == polyCount-1)) {
            return false;
        }
    }
    SkASSERT(this->indexCount());

    // final fan
    SkASSERT(fPositions.count() >= 3);
    if (this->addArc(fFirstOutset, false)) {
        if (fFirstUmbraOutside) {
            this->appendTriangle(fFirstVertexIndex, fPositions.count() - 1,
                                    fFirstVertexIndex + 2);
        } else {
            this->appendTriangle(fFirstVertexIndex, fPositions.count() - 1,
                                    fFirstVertexIndex + 1);
        }
    } else {
        // no arc added, fix up by setting first penumbra point position to last one
        if (fFirstUmbraOutside) {
            fPositions[fFirstVertexIndex + 2] = fPositions[fPositions.count() - 1];
        } else {
            fPositions[fFirstVertexIndex + 1] = fPositions[fPositions.count() - 1];
        }
    }

    return true;
}

bool SkSpotShadowTessellator::computeConcaveShadow(SkScalar radius) {
    if (!SkIsSimplePolygon(&fPathPolygon[0], fPathPolygon.count())) {
        return false;
    }

    // generate inner ring
    SkTDArray<int> umbraIndices;
    umbraIndices.setReserve(fPathPolygon.count());
    if (!SkOffsetSimplePolygon(&fPathPolygon[0], fPathPolygon.count(), radius,
                               &fUmbraPolygon, &umbraIndices)) {
        // TODO: figure out how to handle this case
        return false;
    }

    // generate outer ring
    SkTDArray<SkPoint> penumbraPolygon;
    SkTDArray<int> penumbraIndices;
    penumbraPolygon.setReserve(fUmbraPolygon.count());
    penumbraIndices.setReserve(fUmbraPolygon.count());
    if (!SkOffsetSimplePolygon(&fPathPolygon[0], fPathPolygon.count(), -radius,
                               &penumbraPolygon, &penumbraIndices)) {
        // TODO: figure out how to handle this case
        return false;
    }

    if (!fUmbraPolygon.count() || !penumbraPolygon.count()) {
        return false;
    }

    // attach the rings together
    this->stitchConcaveRings(fUmbraPolygon, &umbraIndices, penumbraPolygon, &penumbraIndices);

    return true;
}

void SkSpotShadowTessellator::mapPoints(SkScalar scale, const SkVector& xlate,
                                        SkPoint* pts, int count) {
    // TODO: vectorize
    for (int i = 0; i < count; ++i) {
        pts[i] *= scale;
        pts[i] += xlate;
    }
}

bool SkSpotShadowTessellator::handlePolyPoint(const SkPoint& p, bool lastPoint) {
    SkVector normal;
    if (compute_normal(fPrevPoint, p, fDirection, &normal)) {
        normal *= fRadius;
        this->addArc(normal, true);
        this->addEdge(p, normal, lastPoint);
    }

    return true;
}

bool SkSpotShadowTessellator::addInnerPoint(const SkPoint& pathPoint, int* currUmbraIndex) {
    SkPoint umbraPoint;
    if (!fValidUmbra) {
        SkVector v = fCentroid - pathPoint;
        v *= 0.95f;
        umbraPoint = pathPoint + v;
    } else {
        umbraPoint = fUmbraPolygon[this->getClosestUmbraPoint(pathPoint)];
    }

    fPrevPoint = pathPoint;

    // merge "close" points
    if (fPrevUmbraIndex == -1 ||
        !duplicate_pt(umbraPoint, fPositions[fPrevUmbraIndex])) {
        // if we've wrapped around, don't add a new point
        if (fPrevUmbraIndex >= 0 && duplicate_pt(umbraPoint, fPositions[fFirstVertexIndex])) {
            *currUmbraIndex = fFirstVertexIndex;
        } else {
            *currUmbraIndex = fPositions.count();
            *fPositions.push() = umbraPoint;
            *fColors.push() = fUmbraColor;
        }
        return false;
    } else {
        *currUmbraIndex = fPrevUmbraIndex;
        return true;
    }
}

void SkSpotShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal,
                                      bool lastEdge) {
    // add next umbra point
    int currUmbraIndex;
    bool duplicate;
    if (lastEdge) {
        duplicate = false;
        currUmbraIndex = fFirstVertexIndex;
        fPrevPoint = nextPoint;
    } else {
        duplicate = this->addInnerPoint(nextPoint, &currUmbraIndex);
    }
    int prevPenumbraIndex = duplicate || (currUmbraIndex == fFirstVertexIndex)
                          ? fPositions.count()-1
                          : fPositions.count()-2;
    if (!duplicate) {
        // add to center fan if transparent or centroid showing
        if (fTransparent) {
            this->appendTriangle(0, fPrevUmbraIndex, currUmbraIndex);
        // otherwise add to clip ring
        } else {
            SkPoint clipPoint;
            bool isOutside = lastEdge ? fFirstUmbraOutside
                                      : this->clipUmbraPoint(fPositions[currUmbraIndex], fCentroid,
                                                             &clipPoint);
            if (isOutside) {
                if (!lastEdge) {
                    *fPositions.push() = clipPoint;
                    *fColors.push() = fUmbraColor;
                }
                this->appendTriangle(fPrevUmbraIndex, currUmbraIndex, currUmbraIndex + 1);
                if (fPrevUmbraOutside) {
                    // fill out quad
                    this->appendTriangle(fPrevUmbraIndex, currUmbraIndex + 1,
                                         fPrevUmbraIndex + 1);
                }
            } else if (fPrevUmbraOutside) {
                // add tri
                this->appendTriangle(fPrevUmbraIndex, currUmbraIndex, fPrevUmbraIndex + 1);
            }

            fPrevUmbraOutside = isOutside;
        }
    }

    // add next penumbra point and quad
    SkPoint newPoint = nextPoint + nextNormal;
    *fPositions.push() = newPoint;
    *fColors.push() = fPenumbraColor;

    if (!duplicate) {
        this->appendTriangle(fPrevUmbraIndex, prevPenumbraIndex, currUmbraIndex);
    }
    this->appendTriangle(prevPenumbraIndex, fPositions.count() - 1, currUmbraIndex);

    fPrevUmbraIndex = currUmbraIndex;
    fPrevOutset = nextNormal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkVertices> SkShadowTessellator::MakeAmbient(const SkPath& path, const SkMatrix& ctm,
                                                   const SkPoint3& zPlane, bool transparent) {
    if (!ctm.mapRect(path.getBounds()).isFinite() || !zPlane.isFinite()) {
        return nullptr;
    }
    SkAmbientShadowTessellator ambientTess(path, ctm, zPlane, transparent);
    return ambientTess.releaseVertices();
}

sk_sp<SkVertices> SkShadowTessellator::MakeSpot(const SkPath& path, const SkMatrix& ctm,
                                                const SkPoint3& zPlane, const SkPoint3& lightPos,
                                                SkScalar lightRadius,  bool transparent) {
    if (!ctm.mapRect(path.getBounds()).isFinite() || !zPlane.isFinite() ||
        !lightPos.isFinite() || !(lightPos.fZ >= SK_ScalarNearlyZero) ||
        !SkScalarIsFinite(lightRadius) || !(lightRadius >= SK_ScalarNearlyZero)) {
        return nullptr;
    }
    SkSpotShadowTessellator spotTess(path, ctm, zPlane, lightPos, lightRadius, transparent);
    return spotTess.releaseVertices();
}
