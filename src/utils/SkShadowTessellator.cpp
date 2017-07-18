/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShadowTessellator.h"
#include "SkColorPriv.h"
#include "SkGeometry.h"
#include "SkInsetConvexPolygon.h"
#include "SkPath.h"
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

    virtual void handleLine(const SkPoint& p) = 0;
    void handleLine(const SkMatrix& m, SkPoint* p);

    void handleQuad(const SkPoint pts[3]);
    void handleQuad(const SkMatrix& m, SkPoint pts[3]);

    void handleCubic(const SkMatrix& m, SkPoint pts[4]);

    void handleConic(const SkMatrix& m, SkPoint pts[3], SkScalar w);

    bool setTransformedHeightFunc(const SkMatrix& ctm);

    bool addArc(const SkVector& nextNormal, bool finishArc);

    SkScalar heightFunc(SkScalar x, SkScalar y) {
        return fZPlaneParams.fX*x + fZPlaneParams.fY*y + fZPlaneParams.fZ;
    }

    SkPoint3                                fZPlaneParams;
    std::function<SkScalar(const SkPoint&)> fTransformedHeightFunc;
    SkScalar                                fZOffset;
    // members for perspective height function
    SkPoint3                                fTransformedZParams;
    SkScalar                                fPartialDeterminants[3];

    // first two points
    SkTDArray<SkPoint>  fInitPoints;
    // temporary buffer
    SkTDArray<SkPoint>  fPointBuffer;

    SkTDArray<SkPoint>  fPositions;
    SkTDArray<SkColor>  fColors;
    SkTDArray<uint16_t> fIndices;

    int                 fFirstVertexIndex;
    SkVector            fFirstOutset;
    SkPoint             fFirstPoint;

    bool                fSucceeded;
    bool                fTransparent;

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

static void compute_radial_steps(const SkVector& v1, const SkVector& v2, SkScalar r,
                                 SkScalar* rotSin, SkScalar* rotCos, int* n) {
    const SkScalar kRecipPixelsPerArcSegment = 0.125f;

    SkScalar rCos = v1.dot(v2);
    SkScalar rSin = v1.cross(v2);
    SkScalar theta = SkScalarATan2(rSin, rCos);

    int steps = SkScalarFloorToInt(r*theta*kRecipPixelsPerArcSegment);

    SkScalar dTheta = theta / steps;
    *rotSin = SkScalarSinCos(dTheta, rotCos);
    *n = steps;
}

SkBaseShadowTessellator::SkBaseShadowTessellator(const SkPoint3& zPlaneParams, bool transparent)
        : fZPlaneParams(zPlaneParams)
        , fZOffset(0)
        , fFirstVertexIndex(-1)
        , fSucceeded(false)
        , fTransparent(transparent)
        , fDirection(1)
        , fPrevUmbraIndex(-1) {
    fInitPoints.setReserve(3);

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

// tesselation tolerance values, in device space pixels
#if SK_SUPPORT_GPU
static const SkScalar kQuadTolerance = 0.2f;
static const SkScalar kCubicTolerance = 0.2f;
#endif
static const SkScalar kConicTolerance = 0.5f;

void SkBaseShadowTessellator::handleLine(const SkMatrix& m, SkPoint* p) {
    m.mapPoints(p, 1);
    this->handleLine(*p);
}

void SkBaseShadowTessellator::handleQuad(const SkPoint pts[3]) {
#if SK_SUPPORT_GPU
    // TODO: Pull PathUtils out of Ganesh?
    int maxCount = GrPathUtils::quadraticPointCount(pts, kQuadTolerance);
    fPointBuffer.setReserve(maxCount);
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
    fPointBuffer.setReserve(maxCount);
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
    compute_radial_steps(fPrevOutset, nextNormal, fRadius, &rotSin, &rotCos, &numSteps);
    SkVector prevNormal = fPrevOutset;
    for (int i = 0; i < numSteps-1; ++i) {
        SkVector currNormal;
        currNormal.fX = prevNormal.fX*rotCos - prevNormal.fY*rotSin;
        currNormal.fY = prevNormal.fY*rotCos + prevNormal.fX*rotSin;
        *fPositions.push() = fPrevPoint + currNormal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fPositions.count() - 2;

        prevNormal = currNormal;
    }
    if (finishArc && numSteps) {
        *fPositions.push() = fPrevPoint + nextNormal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fPositions.count() - 2;
    }
    fPrevOutset = nextNormal;

    return (numSteps > 0);
}

bool SkBaseShadowTessellator::setTransformedHeightFunc(const SkMatrix& ctm) {
    if (SkScalarNearlyZero(fZPlaneParams.fX) && SkScalarNearlyZero(fZPlaneParams.fY)) {
        fTransformedHeightFunc = [this](const SkPoint& p) {
            return fZPlaneParams.fZ;
        };
    } else {
        SkMatrix ctmInverse;
        if (!ctm.invert(&ctmInverse)) {
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
    void handleLine(const SkPoint& p) override;
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal);

    static constexpr auto kHeightFactor = 1.0f / 128.0f;
    static constexpr auto kGeomFactor = 64.0f;
    static constexpr auto kMaxEdgeLenSqr = 20 * 20;
    static constexpr auto kInsetFactor = -0.5f;

    SkScalar offset(SkScalar z) {
        return z * kHeightFactor * kGeomFactor;
    }
    SkColor umbraColor(SkScalar z) {
        SkScalar umbraAlpha = SkScalarInvert((1.0f + SkTMax(z*kHeightFactor, 0.0f)));
        return SkColorSetARGB(umbraAlpha * 255.9999f, 0, 0, 0);
    }

    int                 fCentroidCount;
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
    SkScalar occluderHeight = heightFunc(0, 0);
    SkScalar umbraAlpha = SkScalarInvert((1.0f + SkTMax(occluderHeight*kHeightFactor, 0.0f)));
    // umbraColor is the interior value, penumbraColor the exterior value.
    // umbraAlpha is the factor that is linearly interpolated from outside to inside, and
    // then "blurred" by the GrBlurredEdgeFP. It is then multiplied by fAmbientAlpha to get
    // the final alpha.
    fUmbraColor = SkColorSetARGB(umbraAlpha * 255.9999f, 0, 0, 0);
    fPenumbraColor = SkColorSetARGB(0, 0, 0, 0);

    // make sure we're not below the canvas plane
    this->setZOffset(path.getBounds(), ctm.hasPerspective());

    this->setTransformedHeightFunc(ctm);

    // Outer ring: 3*numPts
    // Middle ring: numPts
    fPositions.setReserve(4 * path.countPoints());
    fColors.setReserve(4 * path.countPoints());
    // Outer ring: 12*numPts
    // Middle ring: 0
    fIndices.setReserve(12 * path.countPoints());

    // walk around the path, tessellate and generate outer ring
    // if original path is transparent, will accumulate sum of points for centroid
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    if (fTransparent) {
        *fPositions.push() = SkPoint::Make(0, 0);
        *fColors.push() = fUmbraColor;
        fCentroidCount = 0;
    }
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->INHERITED::handleLine(ctm, &pts[1]);
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
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }

    if (!this->indexCount()) {
        return;
    }

    // Finish up
    SkVector normal;
    if (compute_normal(fPrevPoint, fFirstPoint, fDirection, &normal)) {
        SkScalar z = fTransformedHeightFunc(fPrevPoint);
        fRadius = this->offset(z);
        SkVector scaledNormal(normal);
        scaledNormal *= fRadius;
        this->addArc(scaledNormal, true);

        // fix-up the last and first umbra points
        SkVector inset = normal;
        // adding to an average, so multiply by an additional half
        inset *= 0.5f*kInsetFactor;
        fPositions[fPrevUmbraIndex] += inset;
        fPositions[fFirstVertexIndex] += inset;
        // we multiply by another half because now we're adding to an average of an average
        inset *= 0.5f;
        if (fSplitPreviousEdge) {
            fPositions[fPrevUmbraIndex - 2] += inset;
        }
        if (fSplitFirstEdge) {
            fPositions[fFirstVertexIndex + 2] += inset;
        }

        // set up for final edge
        z = fTransformedHeightFunc(fFirstPoint);
        normal *= this->offset(z);

        // make sure we don't end up with a sharp alpha edge along the quad diagonal
        if (fColors[fPrevUmbraIndex] != fColors[fFirstVertexIndex] &&
            fFirstPoint.distanceToSqd(fPositions[fPrevUmbraIndex]) > kMaxEdgeLenSqr) {
            SkPoint centerPoint = fPositions[fPrevUmbraIndex] + fPositions[fFirstVertexIndex];
            centerPoint *= 0.5f;
            *fPositions.push() = centerPoint;
            *fColors.push() = SkPMLerp(fColors[fFirstVertexIndex], fColors[fPrevUmbraIndex], 128);
            centerPoint = fPositions[fPositions.count()-2] + fPositions[fFirstVertexIndex+1];
            centerPoint *= 0.5f;
            *fPositions.push() = centerPoint;
            *fColors.push() = fPenumbraColor;

            if (fColors[fPrevUmbraIndex] > fColors[fPositions.count() - 2]) {
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fPositions.count() - 3;
                *fIndices.push() = fPositions.count() - 2;

                *fIndices.push() = fPositions.count() - 3;
                *fIndices.push() = fPositions.count() - 1;
                *fIndices.push() = fPositions.count() - 2;
            } else {
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fPositions.count() - 2;
                *fIndices.push() = fPositions.count() - 1;

                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fPositions.count() - 1;
                *fIndices.push() = fPositions.count() - 3;
            }

            // if transparent, add point to first one in array and add to center fan
            if (fTransparent) {
                fPositions[0] += centerPoint;
                ++fCentroidCount;

                *fIndices.push() = 0;
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fPositions.count() - 2;
            }

            fPrevUmbraIndex = fPositions.count() - 2;
        }

        // final edge
        *fPositions.push() = fFirstPoint + normal;
        *fColors.push() = fPenumbraColor;

        if (fColors[fPrevUmbraIndex] > fColors[fFirstVertexIndex]) {
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fPositions.count() - 2;
            *fIndices.push() = fFirstVertexIndex;

            *fIndices.push() = fPositions.count() - 2;
            *fIndices.push() = fPositions.count() - 1;
            *fIndices.push() = fFirstVertexIndex;
        } else {
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fPositions.count() - 2;
            *fIndices.push() = fPositions.count() - 1;

            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fPositions.count() - 1;
            *fIndices.push() = fFirstVertexIndex;
        }
        fPrevOutset = normal;
    }

    // finalize centroid
    if (fTransparent) {
        fPositions[0] *= SkScalarFastInvert(fCentroidCount);
        fColors[0] = this->umbraColor(fTransformedHeightFunc(fPositions[0]));

        *fIndices.push() = 0;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fFirstVertexIndex;
    }

    // final fan
    if (fPositions.count() >= 3) {
        fPrevUmbraIndex = fFirstVertexIndex;
        fPrevPoint = fFirstPoint;
        fRadius = this->offset(fTransformedHeightFunc(fPrevPoint));
        if (this->addArc(fFirstOutset, false)) {
            *fIndices.push() = fFirstVertexIndex;
            *fIndices.push() = fPositions.count() - 1;
            *fIndices.push() = fFirstVertexIndex + 1;
        } else {
            // arc is too small, set the first penumbra point to be the same position
            // as the last one
            fPositions[fFirstVertexIndex + 1] = fPositions[fPositions.count() - 1];
        }
    }
    fSucceeded = true;
}

void SkAmbientShadowTessellator::handleLine(const SkPoint& p)  {
    if (fInitPoints.count() < 2) {
        *fInitPoints.push() = p;
        return;
    }

    if (fInitPoints.count() == 2) {
        // determine if cw or ccw
        SkVector v0 = fInitPoints[1] - fInitPoints[0];
        SkVector v1 = p - fInitPoints[0];
        SkScalar perpDot = v0.fX*v1.fY - v0.fY*v1.fX;
        if (SkScalarNearlyZero(perpDot)) {
            // nearly parallel, just treat as straight line and continue
            fInitPoints[1] = p;
            return;
        }

        // if perpDot > 0, winding is ccw
        fDirection = (perpDot > 0) ? -1 : 1;

        // add first quad
        SkVector normal;
        if (!compute_normal(fInitPoints[0], fInitPoints[1], fDirection, &normal)) {
            // first two points are incident, make the third point the second and continue
            fInitPoints[1] = p;
            return;
        }

        fFirstPoint = fInitPoints[0];
        fFirstVertexIndex = fPositions.count();
        SkScalar z = fTransformedHeightFunc(fFirstPoint);
        fFirstOutset = normal;
        fFirstOutset *= this->offset(z);

        fPrevOutset = fFirstOutset;
        fPrevPoint = fFirstPoint;
        fPrevUmbraIndex = fFirstVertexIndex;

        *fPositions.push() = fFirstPoint;
        *fColors.push() = this->umbraColor(z);
        *fPositions.push() = fFirstPoint + fFirstOutset;
        *fColors.push() = fPenumbraColor;
        if (fTransparent) {
            fPositions[0] += fFirstPoint;
            fCentroidCount = 1;
        }

        // add the first quad
        z = fTransformedHeightFunc(fInitPoints[1]);
        fRadius = this->offset(z);
        fUmbraColor = this->umbraColor(z);
        this->addEdge(fInitPoints[1], normal);

        // to ensure we skip this block next time
        *fInitPoints.push() = p;
    }

    SkVector normal;
    if (compute_normal(fPrevPoint, p, fDirection, &normal)) {
        SkVector scaledNormal = normal;
        scaledNormal *= fRadius;
        this->addArc(scaledNormal, true);
        SkScalar z = fTransformedHeightFunc(p);
        fRadius = this->offset(z);
        fUmbraColor = this->umbraColor(z);
        this->addEdge(p, normal);
    }
}

void SkAmbientShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal) {
    // We compute the inset in two stages: first we inset by half the current normal,
    // then on the next addEdge() we add half of the next normal to get an average of the two
    SkVector insetNormal = nextNormal;
    insetNormal *= 0.5f*kInsetFactor;

    // Adding the other half of the average for the previous edge
    fPositions[fPrevUmbraIndex] += insetNormal;

    SkPoint umbraPoint = nextPoint + insetNormal;
    SkVector outsetNormal = nextNormal;
    outsetNormal *= fRadius;
    SkPoint penumbraPoint = nextPoint + outsetNormal;

    // For split edges, we're adding an average of two averages, so we multiply by another half
    if (fSplitPreviousEdge) {
        insetNormal *= 0.5f;
        fPositions[fPrevUmbraIndex - 2] += insetNormal;
    }

    // Split the edge to make sure we don't end up with a sharp alpha edge along the quad diagonal
    if (fColors[fPrevUmbraIndex] != fUmbraColor &&
        nextPoint.distanceToSqd(fPositions[fPrevUmbraIndex]) > kMaxEdgeLenSqr) {

        // This is lacking 1/4 of the next inset -- we'll add it the next time we call addEdge()
        SkPoint centerPoint = fPositions[fPrevUmbraIndex] + umbraPoint;
        centerPoint *= 0.5f;
        *fPositions.push() = centerPoint;
        *fColors.push() = SkPMLerp(fUmbraColor, fColors[fPrevUmbraIndex], 128);
        centerPoint = fPositions[fPositions.count()-2] + penumbraPoint;
        centerPoint *= 0.5f;
        *fPositions.push() = centerPoint;
        *fColors.push() = fPenumbraColor;

        // set triangularization to get best interpolation of color
        if (fColors[fPrevUmbraIndex] > fColors[fPositions.count() - 2]) {
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fPositions.count() - 3;
            *fIndices.push() = fPositions.count() - 2;

            *fIndices.push() = fPositions.count() - 3;
            *fIndices.push() = fPositions.count() - 1;
            *fIndices.push() = fPositions.count() - 2;
        } else {
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fPositions.count() - 2;
            *fIndices.push() = fPositions.count() - 1;

            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fPositions.count() - 1;
            *fIndices.push() = fPositions.count() - 3;
        }

        // if transparent, add point to first one in array and add to center fan
        if (fTransparent) {
            fPositions[0] += centerPoint;
            ++fCentroidCount;

            *fIndices.push() = 0;
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fPositions.count() - 2;
        }

        fSplitPreviousEdge = true;
        if (fPrevUmbraIndex == fFirstVertexIndex) {
            fSplitFirstEdge = true;
        }
        fPrevUmbraIndex = fPositions.count() - 2;
    } else {
        fSplitPreviousEdge = false;
    }

    // add next quad
    *fPositions.push() = umbraPoint;
    *fColors.push() = fUmbraColor;
    *fPositions.push() = penumbraPoint;
    *fColors.push() = fPenumbraColor;

    // set triangularization to get best interpolation of color
    if (fColors[fPrevUmbraIndex] > fColors[fPositions.count() - 2]) {
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 3;
        *fIndices.push() = fPositions.count() - 2;

        *fIndices.push() = fPositions.count() - 3;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fPositions.count() - 2;
    } else {
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fPositions.count() - 3;
    }

    // if transparent, add point to first one in array and add to center fan
    if (fTransparent) {
        fPositions[0] += nextPoint;
        ++fCentroidCount;

        *fIndices.push() = 0;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
    }

    fPrevUmbraIndex = fPositions.count() - 2;
    fPrevPoint = nextPoint;
    fPrevOutset = outsetNormal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkSpotShadowTessellator : public SkBaseShadowTessellator {
public:
    SkSpotShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                            const SkPoint3& zPlaneParams, const SkPoint3& lightPos,
                            SkScalar lightRadius, bool transparent);

private:
    void computeClipAndPathPolygons(const SkPath& path, const SkMatrix& ctm,
                                    const SkMatrix& shadowTransform);
    void computeClipVectorsAndTestCentroid();
    bool clipUmbraPoint(const SkPoint& umbraPoint, const SkPoint& centroid, SkPoint* clipPoint);
    int getClosestUmbraPoint(const SkPoint& point);

    void handleLine(const SkPoint& p) override;
    bool handlePolyPoint(const SkPoint& p);

    void mapPoints(SkScalar scale, const SkVector& xlate, SkPoint* pts, int count);
    bool addInnerPoint(const SkPoint& pathPoint);
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal);

    SkScalar offset(SkScalar z) {
        float zRatio = SkTPin(z / (fLightZ - z), 0.0f, 0.95f);
        return fLightRadius*zRatio;
    }

    SkScalar            fLightZ;
    SkScalar            fLightRadius;
    SkScalar            fOffsetAdjust;

    SkTDArray<SkPoint>  fClipPolygon;
    SkTDArray<SkVector> fClipVectors;
    SkPoint             fCentroid;
    SkScalar            fArea;

    SkTDArray<SkPoint>  fPathPolygon;
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
    float zRatio = SkTPin(occluderHeight / (fLightZ - occluderHeight), 0.0f, 0.95f);
    SkScalar radius = lightRadius * zRatio;
    fRadius = radius;
    fUmbraColor = SkColorSetARGB(255, 0, 0, 0);
    fPenumbraColor = SkColorSetARGB(0, 0, 0, 0);

    // Compute the scale and translation for the spot shadow.
    SkMatrix shadowTransform;
    if (!ctm.hasPerspective()) {
        SkScalar scale = fLightZ / (fLightZ - occluderHeight);
        SkVector translate = SkVector::Make(-zRatio * lightPos.fX, -zRatio * lightPos.fY);
        shadowTransform.setScaleTranslate(scale, scale, translate.fX, translate.fY);
    } else {
        // For perspective, we have a scale, a z-shear, and another projective divide --
        // this varies at each point so we can't use an affine transform.
        // We'll just apply this to each generated point in turn.
        shadowTransform.reset();
        // Also can't cull the center (for now).
        fTransparent = true;
    }
    SkMatrix fullTransform = SkMatrix::Concat(shadowTransform, ctm);

    // Set up our reverse mapping
    this->setTransformedHeightFunc(fullTransform);

    // TODO: calculate these reserves better
    // Penumbra ring: 3*numPts
    // Umbra ring: numPts
    // Inner ring: numPts
    fPositions.setReserve(5 * path.countPoints());
    fColors.setReserve(5 * path.countPoints());
    // Penumbra ring: 12*numPts
    // Umbra ring: 3*numPts
    fIndices.setReserve(15 * path.countPoints());
    fClipPolygon.setReserve(path.countPoints());

    // compute rough clip bounds for umbra, plus offset polygon, plus centroid
    this->computeClipAndPathPolygons(path, ctm, shadowTransform);
    if (fClipPolygon.count() < 3 || fPathPolygon.count() < 3) {
        return;
    }

    // check to see if umbra collapses
    SkScalar minDistSq = fCentroid.distanceToLineSegmentBetweenSqd(fPathPolygon[0],
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
        SkScalar distSq = fCentroid.distanceToLineSegmentBetweenSqd(currPoint, nextPoint);
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

    // compute vectors for clip tests
    this->computeClipVectorsAndTestCentroid();

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
    for (int i = 0; i < fPathPolygon.count(); ++i) {
        if (!this->handlePolyPoint(fPathPolygon[i])) {
            return;
        }
    }

    if (!this->indexCount()) {
        return;
    }

    // finish up the final verts
    SkVector normal;
    if (compute_normal(fPrevPoint, fFirstPoint, fDirection, &normal)) {
        normal *= fRadius;
        this->addArc(normal, true);

        // add to center fan
        if (fTransparent) {
            *fIndices.push() = 0;
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fFirstVertexIndex;
            // or to clip ring
        } else {
            if (fFirstUmbraOutside) {
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fFirstVertexIndex;
                *fIndices.push() = fFirstVertexIndex + 1;
                if (fPrevUmbraOutside) {
                    // fill out quad
                    *fIndices.push() = fPrevUmbraIndex;
                    *fIndices.push() = fFirstVertexIndex + 1;
                    *fIndices.push() = fPrevUmbraIndex + 1;
                }
            } else if (fPrevUmbraOutside) {
                // add tri
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fFirstVertexIndex;
                *fIndices.push() = fPrevUmbraIndex + 1;
            }
        }

        // add final edge
        *fPositions.push() = fFirstPoint + normal;
        *fColors.push() = fPenumbraColor;

        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fFirstVertexIndex;

        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertexIndex;

        fPrevOutset = normal;
    }

    // final fan
    if (fPositions.count() >= 3) {
        fPrevUmbraIndex = fFirstVertexIndex;
        fPrevPoint = fFirstPoint;
        if (this->addArc(fFirstOutset, false)) {
            *fIndices.push() = fFirstVertexIndex;
            *fIndices.push() = fPositions.count() - 1;
            if (fFirstUmbraOutside) {
                *fIndices.push() = fFirstVertexIndex + 2;
            } else {
                *fIndices.push() = fFirstVertexIndex + 1;
            }
        } else {
            // no arc added, fix up by setting first penumbra point position to last one
            if (fFirstUmbraOutside) {
                fPositions[fFirstVertexIndex + 2] = fPositions[fPositions.count() - 1];
            } else {
                fPositions[fFirstVertexIndex + 1] = fPositions[fPositions.count() - 1];
            }
        }
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

    *fIndices.push() = fPositions.count() - 4;
    *fIndices.push() = fPositions.count() - 2;
    *fIndices.push() = fPositions.count() - 1;

    *fIndices.push() = fPositions.count() - 4;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = fPositions.count() - 3;
#endif

    fSucceeded = true;
}

void SkSpotShadowTessellator::computeClipAndPathPolygons(const SkPath& path, const SkMatrix& ctm,
                                                         const SkMatrix& shadowTransform) {

    fPathPolygon.setReserve(path.countPoints());

    // Walk around the path and compute clip polygon and path polygon.
    // Will also accumulate sum of areas for centroid.
    // For Bezier curves, we compute additional interior points on curve.
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;

    fClipPolygon.reset();

    // init centroid
    fCentroid = SkPoint::Make(0, 0);
    fArea = 0;

    // coefficients to compute cubic Bezier at t = 5/16
    static constexpr SkScalar kA = 0.32495117187f;
    static constexpr SkScalar kB = 0.44311523437f;
    static constexpr SkScalar kC = 0.20141601562f;
    static constexpr SkScalar kD = 0.03051757812f;

    SkPoint curvePoint;
    SkScalar w;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                ctm.mapPoints(&pts[1], 1);
                *fClipPolygon.push() = pts[1];
                this->INHERITED::handleLine(shadowTransform, &pts[1]);
                break;
            case SkPath::kQuad_Verb:
                ctm.mapPoints(pts, 3);
                // point at t = 1/2
                curvePoint.fX = 0.25f*pts[0].fX + 0.5f*pts[1].fX + 0.25f*pts[2].fX;
                curvePoint.fY = 0.25f*pts[0].fY + 0.5f*pts[1].fY + 0.25f*pts[2].fY;
                *fClipPolygon.push() = curvePoint;
                *fClipPolygon.push() = pts[2];
                this->handleQuad(shadowTransform, pts);
                break;
            case SkPath::kConic_Verb:
                ctm.mapPoints(pts, 3);
                w = iter.conicWeight();
                // point at t = 1/2
                curvePoint.fX = 0.25f*pts[0].fX + w*0.5f*pts[1].fX + 0.25f*pts[2].fX;
                curvePoint.fY = 0.25f*pts[0].fY + w*0.5f*pts[1].fY + 0.25f*pts[2].fY;
                curvePoint *= SkScalarInvert(0.5f + 0.5f*w);
                *fClipPolygon.push() = curvePoint;
                *fClipPolygon.push() = pts[2];
                this->handleConic(shadowTransform, pts, w);
                break;
            case SkPath::kCubic_Verb:
                ctm.mapPoints(pts, 4);
                // point at t = 5/16
                curvePoint.fX = kA*pts[0].fX + kB*pts[1].fX + kC*pts[2].fX + kD*pts[3].fX;
                curvePoint.fY = kA*pts[0].fY + kB*pts[1].fY + kC*pts[2].fY + kD*pts[3].fY;
                *fClipPolygon.push() = curvePoint;
                // point at t = 11/16
                curvePoint.fX = kD*pts[0].fX + kC*pts[1].fX + kB*pts[2].fX + kA*pts[3].fX;
                curvePoint.fY = kD*pts[0].fY + kC*pts[1].fY + kB*pts[2].fY + kA*pts[3].fY;
                *fClipPolygon.push() = curvePoint;
                *fClipPolygon.push() = pts[3];
                this->handleCubic(shadowTransform, pts);
                break;
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
    }

    // finish centroid
    if (fPathPolygon.count() > 0) {
        SkPoint currPoint = fPathPolygon[fPathPolygon.count() - 1];
        SkPoint nextPoint = fPathPolygon[0];
        SkScalar quadArea = currPoint.cross(nextPoint);
        fCentroid.fX += (currPoint.fX + nextPoint.fX) * quadArea;
        fCentroid.fY += (currPoint.fY + nextPoint.fY) * quadArea;
        fArea += quadArea;
        fCentroid *= SK_Scalar1 / (3 * fArea);
    }

    fCurrClipPoint = fClipPolygon.count() - 1;
}

void SkSpotShadowTessellator::computeClipVectorsAndTestCentroid() {
    SkASSERT(fClipPolygon.count() >= 3);

    // init clip vectors
    SkVector v0 = fClipPolygon[1] - fClipPolygon[0];
    *fClipVectors.push() = v0;

    // init centroid check
    bool hiddenCentroid = true;
    SkVector v1 = fCentroid - fClipPolygon[0];
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
    SkScalar minDistance = p.distanceToSqd(fUmbraPolygon[fCurrUmbraPoint]);
    int index = fCurrUmbraPoint;
    int dir = 1;
    int next = (index + dir) % fUmbraPolygon.count();

    // init travel direction
    SkScalar distance = p.distanceToSqd(fUmbraPolygon[next]);
    if (distance < minDistance) {
        index = next;
        minDistance = distance;
    } else {
        dir = fUmbraPolygon.count()-1;
    }

    // iterate until we find a point that increases the distance
    next = (index + dir) % fUmbraPolygon.count();
    distance = p.distanceToSqd(fUmbraPolygon[next]);
    while (distance < minDistance) {
        index = next;
        minDistance = distance;
        next = (index + dir) % fUmbraPolygon.count();
        distance = p.distanceToSqd(fUmbraPolygon[next]);
    }

    fCurrUmbraPoint = index;
    return index;
}

void SkSpotShadowTessellator::mapPoints(SkScalar scale, const SkVector& xlate,
                                        SkPoint* pts, int count) {
    // TODO: vectorize
    for (int i = 0; i < count; ++i) {
        pts[i] *= scale;
        pts[i] += xlate;
    }
}

static bool duplicate_pt(const SkPoint& p0, const SkPoint& p1) {
    static constexpr SkScalar kClose = (SK_Scalar1 / 16);
    static constexpr SkScalar kCloseSqd = kClose*kClose;

    SkScalar distSq = p0.distanceToSqd(p1);
    return distSq < kCloseSqd;
}

static SkScalar perp_dot(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2) {
    SkVector v0 = p1 - p0;
    SkVector v1 = p2 - p0;
    return v0.cross(v1);
}

static bool is_collinear(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2) {
    return (SkScalarNearlyZero(perp_dot(p0, p1, p2)));
}

void SkSpotShadowTessellator::handleLine(const SkPoint& p) {
    // remove coincident points and add to centroid
    if (fPathPolygon.count() > 0) {
        const SkPoint& lastPoint = fPathPolygon[fPathPolygon.count() - 1];
        if (duplicate_pt(p, lastPoint)) {
            return;
        }
        SkScalar quadArea = lastPoint.cross(p);
        fCentroid.fX += (p.fX + lastPoint.fX) * quadArea;
        fCentroid.fY += (p.fY + lastPoint.fY) * quadArea;
        fArea += quadArea;
    }

    // try to remove collinear points
    if (fPathPolygon.count() > 1 && is_collinear(fPathPolygon[fPathPolygon.count()-2],
                                                 fPathPolygon[fPathPolygon.count()-1],
                                                 p)) {
        fPathPolygon[fPathPolygon.count() - 1] = p;
    } else {
        *fPathPolygon.push() = p;
    }
}

bool SkSpotShadowTessellator::handlePolyPoint(const SkPoint& p) {
    if (fInitPoints.count() < 2) {
        *fInitPoints.push() = p;
        return true;
    }

    if (fInitPoints.count() == 2) {
        // determine if cw or ccw
        SkScalar perpDot = perp_dot(fInitPoints[0], fInitPoints[1], p);
        if (SkScalarNearlyZero(perpDot)) {
            // nearly parallel, just treat as straight line and continue
            fInitPoints[1] = p;
            return true;
        }

        // if perpDot > 0, winding is ccw
        fDirection = (perpDot > 0) ? -1 : 1;

        // add first quad
        if (!compute_normal(fInitPoints[0], fInitPoints[1], fDirection, &fFirstOutset)) {
            // first two points are incident, make the third point the second and continue
            fInitPoints[1] = p;
            return true;
        }

        fFirstOutset *= fRadius;
        fFirstPoint = fInitPoints[0];
        fFirstVertexIndex = fPositions.count();
        fPrevOutset = fFirstOutset;
        fPrevPoint = fFirstPoint;
        fPrevUmbraIndex = -1;

        this->addInnerPoint(fFirstPoint);
        fPrevUmbraIndex = fFirstVertexIndex;

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
        this->addEdge(fInitPoints[1], fFirstOutset);

        // to ensure we skip this block next time
        *fInitPoints.push() = p;
    }

    // if concave, abort
    SkScalar perpDot = perp_dot(fInitPoints[1], fInitPoints[2], p);
    if (fDirection*perpDot > 0) {
        return false;
    }

    SkVector normal;
    if (compute_normal(fPrevPoint, p, fDirection, &normal)) {
        normal *= fRadius;
        this->addArc(normal, true);
        this->addEdge(p, normal);
        fInitPoints[1] = fInitPoints[2];
        fInitPoints[2] = p;
    }

    return true;
}

bool SkSpotShadowTessellator::addInnerPoint(const SkPoint& pathPoint) {
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
        *fPositions.push() = umbraPoint;
        *fColors.push() = fUmbraColor;

        return false;
    } else {
        return true;
    }
}

void SkSpotShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal) {
    // add next umbra point
    bool duplicate = this->addInnerPoint(nextPoint);
    int prevPenumbraIndex = duplicate ? fPositions.count()-1 : fPositions.count()-2;
    int currUmbraIndex = duplicate ? fPrevUmbraIndex : fPositions.count()-1;

    if (!duplicate) {
        // add to center fan if transparent or centroid showing
        if (fTransparent) {
            *fIndices.push() = 0;
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = currUmbraIndex;
        // otherwise add to clip ring
        } else {
            SkPoint clipPoint;
            bool isOutside = this->clipUmbraPoint(fPositions[currUmbraIndex], fCentroid,
                                                  &clipPoint);
            if (isOutside) {
                *fPositions.push() = clipPoint;
                *fColors.push() = fUmbraColor;

                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = currUmbraIndex;
                *fIndices.push() = currUmbraIndex + 1;
                if (fPrevUmbraOutside) {
                    // fill out quad
                    *fIndices.push() = fPrevUmbraIndex;
                    *fIndices.push() = currUmbraIndex + 1;
                    *fIndices.push() = fPrevUmbraIndex + 1;
                }
            } else if (fPrevUmbraOutside) {
                // add tri
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = currUmbraIndex;
                *fIndices.push() = fPrevUmbraIndex + 1;
            }
            fPrevUmbraOutside = isOutside;
        }
    }

    // add next penumbra point and quad
    SkPoint newPoint = nextPoint + nextNormal;
    *fPositions.push() = newPoint;
    *fColors.push() = fPenumbraColor;

    if (!duplicate) {
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = prevPenumbraIndex;
        *fIndices.push() = currUmbraIndex;
    }

    *fIndices.push() = prevPenumbraIndex;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = currUmbraIndex;

    fPrevUmbraIndex = currUmbraIndex;
    fPrevOutset = nextNormal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkVertices> SkShadowTessellator::MakeAmbient(const SkPath& path, const SkMatrix& ctm,
                                                   const SkPoint3& zPlane, bool transparent) {
    SkAmbientShadowTessellator ambientTess(path, ctm, zPlane, transparent);
    return ambientTess.releaseVertices();
}

sk_sp<SkVertices> SkShadowTessellator::MakeSpot(const SkPath& path, const SkMatrix& ctm,
                                                const SkPoint3& zPlane, const SkPoint3& lightPos,
                                                SkScalar lightRadius,  bool transparent) {
    SkSpotShadowTessellator spotTess(path, ctm, zPlane, lightPos, lightRadius, transparent);
    return spotTess.releaseVertices();
}
