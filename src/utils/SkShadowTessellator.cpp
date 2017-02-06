/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShadowTessellator.h"
#include "SkColorPriv.h"
#include "SkGeometry.h"
#include "SkPath.h"

#if SK_SUPPORT_GPU
#include "GrPathUtils.h"
#endif

template <typename T> using UniqueArray = SkShadowVertices::UniqueArray<T>;

/**
 * Base class
 */
class SkShadowTessellator {
public:
    SkShadowTessellator(SkScalar radius, SkColor umbraColor,
                        SkColor penumbraColor, bool transparent);
    virtual ~SkShadowTessellator() {}

    int vertexCount() const { return fPositions.count(); }
    int indexCount() const { return fIndices.count(); }

    bool succeeded() const { return fSucceeded; }

    // The casts are needed to work around a, older GCC issue where the fact that the pointers are
    // T* and not const T* causes calls to a deleted unique_ptr constructor.
    UniqueArray<SkPoint> releasePositions() {
        return UniqueArray<SkPoint>(static_cast<const SkPoint*>(fPositions.release()));
    }
    UniqueArray<SkColor> releaseColors() {
        return UniqueArray<SkColor>(static_cast<const SkColor*>(fColors.release()));
    }
    UniqueArray<uint16_t> releaseIndices() {
        return UniqueArray<uint16_t>(static_cast<const uint16_t*>(fIndices.release()));
    }

protected:
    virtual void handleLine(const SkPoint& p) = 0;
    void handleLine(const SkMatrix& m, SkPoint* p);

    void handleQuad(const SkPoint pts[3]);
    void handleQuad(const SkMatrix& m, SkPoint pts[3]);

    void handleCubic(const SkMatrix& m, SkPoint pts[4]);

    void handleConic(const SkMatrix& m, SkPoint pts[3], SkScalar w);

    void addArc(const SkVector& nextNormal);
    void finishArcAndAddEdge(const SkVector& nextPoint, const SkVector& nextNormal);
    virtual void addEdge(const SkVector& nextPoint, const SkVector& nextNormal) = 0;

    // first three points
    SkTDArray<SkPoint>  fInitPoints;
    // temporary buffer
    SkTDArray<SkPoint>  fPointBuffer;

    SkTDArray<SkPoint>  fPositions;
    SkTDArray<SkColor>  fColors;
    SkTDArray<uint16_t> fIndices;

    int                 fFirstVertex;
    SkVector            fFirstNormal;
    SkPoint             fFirstPoint;

    bool                fSucceeded;
    bool                fTransparent;

    SkColor             fUmbraColor;
    SkColor             fPenumbraColor;

    SkScalar            fRadius;
    SkScalar            fDirection;
    int                 fPrevUmbraIndex;
    SkVector            fPrevNormal;
    SkPoint             fPrevPoint;
};

static bool compute_normal(const SkPoint& p0, const SkPoint& p1, SkScalar radius, SkScalar dir,
                           SkVector* newNormal) {
    SkVector normal;
    // compute perpendicular
    normal.fX = p0.fY - p1.fY;
    normal.fY = p1.fX - p0.fX;
    if (!normal.normalize()) {
        return false;
    }
    normal *= radius*dir;
    *newNormal = normal;
    return true;
}

static void compute_radial_steps(const SkVector& v1, const SkVector& v2, SkScalar r,
                                 SkScalar* rotSin, SkScalar* rotCos, int* n) {
    const SkScalar kRecipPixelsPerArcSegment = 0.25f;

    SkScalar rCos = v1.dot(v2);
    SkScalar rSin = v1.cross(v2);
    SkScalar theta = SkScalarATan2(rSin, rCos);

    SkScalar steps = r*theta*kRecipPixelsPerArcSegment;

    SkScalar dTheta = theta / steps;
    *rotSin = SkScalarSinCos(dTheta, rotCos);
    *n = SkScalarFloorToInt(steps);
}

SkShadowTessellator::SkShadowTessellator(SkScalar radius, SkColor umbraColor,
                                         SkColor penumbraColor, bool transparent)
    : fFirstVertex(-1)
    , fSucceeded(false)
    , fTransparent(transparent)
    , fUmbraColor(umbraColor)
    , fPenumbraColor(penumbraColor)
    , fRadius(radius)
    , fDirection(1)
    , fPrevUmbraIndex(-1) {

    fInitPoints.setReserve(3);

    // child classes will set reserve for positions, colors and indices
}

// tesselation tolerance values, in device space pixels
static const SkScalar kQuadTolerance = 0.2f;
static const SkScalar kCubicTolerance = 0.2f;
static const SkScalar kConicTolerance = 0.5f;

void SkShadowTessellator::handleLine(const SkMatrix& m, SkPoint* p) {
    m.mapPoints(p, 1);
    this->handleLine(*p);
}

void SkShadowTessellator::handleQuad(const SkPoint pts[3]) {
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

void SkShadowTessellator::handleQuad(const SkMatrix& m, SkPoint pts[3]) {
    m.mapPoints(pts, 3);
    this->handleQuad(pts);
}

void SkShadowTessellator::handleCubic(const SkMatrix& m, SkPoint pts[4]) {
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

void SkShadowTessellator::handleConic(const SkMatrix& m, SkPoint pts[3], SkScalar w) {
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

void SkShadowTessellator::addArc(const SkVector& nextNormal) {
    // fill in fan from previous quad
    SkScalar rotSin, rotCos;
    int numSteps;
    compute_radial_steps(fPrevNormal, nextNormal, fRadius, &rotSin, &rotCos, &numSteps);
    SkVector prevNormal = fPrevNormal;
    for (int i = 0; i < numSteps; ++i) {
        SkVector nextNormal;
        nextNormal.fX = prevNormal.fX*rotCos - prevNormal.fY*rotSin;
        nextNormal.fY = prevNormal.fY*rotCos + prevNormal.fX*rotSin;
        *fPositions.push() = fPrevPoint + nextNormal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        prevNormal = nextNormal;
    }
}

void SkShadowTessellator::finishArcAndAddEdge(const SkPoint& nextPoint,
                                              const SkVector& nextNormal) {
    // close out previous arc
    *fPositions.push() = fPrevPoint + nextNormal;
    *fColors.push() = fPenumbraColor;
    *fIndices.push() = fPrevUmbraIndex;
    *fIndices.push() = fPositions.count() - 2;
    *fIndices.push() = fPositions.count() - 1;

    this->addEdge(nextPoint, nextNormal);
}


//////////////////////////////////////////////////////////////////////////////////////////////////

class SkAmbientShadowTessellator : public SkShadowTessellator {
public:
    SkAmbientShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                               SkScalar radius, SkColor umbraColor,
                               SkColor penumbraColor, bool transparent);

private:
    void handleLine(const SkPoint& p) override;
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal) override;

    int                 fCentroidCount;

    typedef SkShadowTessellator INHERITED;
};

SkAmbientShadowTessellator::SkAmbientShadowTessellator(const SkPath& path,
                                                       const SkMatrix& ctm,
                                                       SkScalar radius,
                                                       SkColor umbraColor,
                                                       SkColor penumbraColor,
                                                       bool transparent)
        : INHERITED(radius, umbraColor, penumbraColor, transparent) {
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
        *fColors.push() = umbraColor;
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

    SkVector normal;
    if (compute_normal(fPrevPoint, fFirstPoint, fRadius, fDirection,
                       &normal)) {
        this->addArc(normal);

        // close out previous arc
        *fPositions.push() = fPrevPoint + normal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        // add final edge
        *fPositions.push() = fFirstPoint + normal;
        *fColors.push() = fPenumbraColor;

        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fFirstVertex;

        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertex;
    }

    // finalize centroid
    if (fTransparent) {
        fPositions[0] *= SkScalarFastInvert(fCentroidCount);

        *fIndices.push() = 0;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fFirstVertex;
    }

    // final fan
    if (fPositions.count() >= 3) {
        fPrevUmbraIndex = fFirstVertex;
        fPrevNormal = normal;
        fPrevPoint = fFirstPoint;
        this->addArc(fFirstNormal);

        *fIndices.push() = fFirstVertex;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertex + 1;
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
        if (!compute_normal(fInitPoints[0], fInitPoints[1], fRadius, fDirection,
                            &fFirstNormal)) {
            // first two points are incident, make the third point the second and continue
            fInitPoints[1] = p;
            return;
        }

        fFirstPoint = fInitPoints[0];
        fFirstVertex = fPositions.count();
        fPrevNormal = fFirstNormal;
        fPrevPoint = fFirstPoint;
        fPrevUmbraIndex = fFirstVertex;

        *fPositions.push() = fInitPoints[0];
        *fColors.push() = fUmbraColor;
        *fPositions.push() = fInitPoints[0] + fFirstNormal;
        *fColors.push() = fPenumbraColor;
        if (fTransparent) {
            fPositions[0] += fInitPoints[0];
            fCentroidCount = 1;
        }
        this->addEdge(fInitPoints[1], fFirstNormal);

        // to ensure we skip this block next time
        *fInitPoints.push() = p;
    }

    SkVector normal;
    if (compute_normal(fPositions[fPrevUmbraIndex], p, fRadius, fDirection, &normal)) {
        this->addArc(normal);
        this->finishArcAndAddEdge(p, normal);
    }
}

void SkAmbientShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal) {
    // add next quad
    *fPositions.push() = nextPoint;
    *fColors.push() = fUmbraColor;
    *fPositions.push() = nextPoint + nextNormal;
    *fColors.push() = fPenumbraColor;

    *fIndices.push() = fPrevUmbraIndex;
    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 2;

    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = fPositions.count() - 2;

    // if transparent, add point to first one in array and add to center fan
    if (fTransparent) {
        fPositions[0] += nextPoint;
        ++fCentroidCount;

        *fIndices.push() = 0;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
    }

    fPrevUmbraIndex = fPositions.count() - 2;
    fPrevNormal = nextNormal;
    fPrevPoint = nextPoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkSpotShadowTessellator : public SkShadowTessellator {
public:
    SkSpotShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                            SkScalar scale, const SkVector& translate,
                            SkScalar radius, SkColor umbraColor, SkColor penumbraColor,
                            bool transparent);

private:
    void computeClipBounds(const SkPath& path, const SkMatrix& ctm, SkPath* devPath);
    void checkUmbraAndTransformCentroid(SkScalar scale, const SkVector& xlate,
                                        bool useDistanceToPoint);
    bool clipUmbraPoint(const SkPoint& umbraPoint, const SkPoint& centroid, SkPoint* clipPoint);

    void handleLine(const SkPoint& p) override;

    void mapPoints(SkScalar scale, const SkVector& xlate, SkPoint* pts, int count);
    void addInnerPoint(const SkPoint& pathPoint);
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal) override;

    SkTDArray<SkPoint>  fClipPolygon;
    SkTDArray<SkVector> fClipVectors;
    SkPoint             fCentroid;

    int                 fCurrPolyPoint;
    bool                fPrevUmbraOutside;
    bool                fFirstUmbraOutside;
    bool                fValidUmbra;

    typedef SkShadowTessellator INHERITED;
};

SkSpotShadowTessellator::SkSpotShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                                                 SkScalar scale, const SkVector& translate,
                                                 SkScalar radius, SkColor umbraColor,
                                                 SkColor penumbraColor, bool transparent)
        : INHERITED(radius, umbraColor, penumbraColor, transparent)
        , fCurrPolyPoint(0)
        , fPrevUmbraOutside(false)
        , fFirstUmbraOutside(false)
        , fValidUmbra(true) {
    
    // TODO: calculate these better
    // Penumbra ring: 3*numPts
    // Umbra ring: numPts
    // Inner ring: numPts
    fPositions.setReserve(5 * path.countPoints());
    fColors.setReserve(5 * path.countPoints());
    // Penumbra ring: 12*numPts
    // Umbra ring: 3*numPts
    fIndices.setReserve(15 * path.countPoints());

    fClipPolygon.setReserve(path.countPoints());
    // compute rough clip bounds for umbra, plus centroid
    SkPath devPath;
    this->computeClipBounds(path, ctm, &devPath);
    if (fClipPolygon.count() < 3) {
        return;
    }
    // We are going to apply 'scale' and 'xlate' (in that order) to each computed path point. We
    // want the effect to be to scale the points relative to the path centroid and then translate
    // them by the 'translate' param we were passed.
    SkVector xlate = fCentroid * (1.f - scale) + translate;

    // check to see if we have a valid umbra at all
    bool usePointCheck = path.isRRect(nullptr) || path.isRect(nullptr) || path.isOval(nullptr);
    this->checkUmbraAndTransformCentroid(scale, translate, usePointCheck);

    // walk around the path, tessellate and generate inner and outer rings
    SkPath::Iter iter(devPath, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    if (fTransparent) {
        *fPositions.push() = fCentroid;
        *fColors.push() = fUmbraColor;
    }
    SkMatrix shadowTransform;
    shadowTransform.setScaleTranslate(scale, scale, xlate.fX, xlate.fY);
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->INHERITED::handleLine(shadowTransform, &pts[1]);
                break;
            case SkPath::kQuad_Verb:
                this->handleQuad(shadowTransform, pts);
                break;
            case SkPath::kCubic_Verb:
                this->handleCubic(shadowTransform, pts);
                break;
            case SkPath::kConic_Verb:
                this->handleConic(shadowTransform, pts, iter.conicWeight());
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

    SkVector normal;
    if (compute_normal(fPrevPoint, fFirstPoint, fRadius, fDirection,
                        &normal)) {
        this->addArc(normal);

        // close out previous arc
        *fPositions.push() = fPrevPoint + normal;
        *fColors.push() = fPenumbraColor;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;

        // add to center fan
        if (fTransparent) {
            *fIndices.push() = 0;
            *fIndices.push() = fPrevUmbraIndex;
            *fIndices.push() = fFirstVertex;
            // or to clip ring
        } else {
            if (fFirstUmbraOutside) {
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fFirstVertex;
                *fIndices.push() = fFirstVertex + 1;
                if (fPrevUmbraOutside) {
                    // fill out quad
                    *fIndices.push() = fPrevUmbraIndex;
                    *fIndices.push() = fFirstVertex + 1;
                    *fIndices.push() = fPrevUmbraIndex + 1;
                }
            } else if (fPrevUmbraOutside) {
                // add tri
                *fIndices.push() = fPrevUmbraIndex;
                *fIndices.push() = fFirstVertex;
                *fIndices.push() = fPrevUmbraIndex + 1;
            }
        }

        // add final edge
        *fPositions.push() = fFirstPoint + normal;
        *fColors.push() = fPenumbraColor;

        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fFirstVertex;

        *fIndices.push() = fPositions.count() - 2;
        *fIndices.push() = fPositions.count() - 1;
        *fIndices.push() = fFirstVertex;
    }

    // final fan
    if (fPositions.count() >= 3) {
        fPrevUmbraIndex = fFirstVertex;
        fPrevPoint = fFirstPoint;
        fPrevNormal = normal;
        this->addArc(fFirstNormal);

        *fIndices.push() = fFirstVertex;
        *fIndices.push() = fPositions.count() - 1;
        if (fFirstUmbraOutside) {
            *fIndices.push() = fFirstVertex + 2;
        } else {
            *fIndices.push() = fFirstVertex + 1;
        }
    }
    fSucceeded = true;
}

void SkSpotShadowTessellator::computeClipBounds(const SkPath& path, const SkMatrix& ctm,
                                                SkPath* devPath) {
    // walk around the path and compute clip polygon
    // if original path is transparent, will accumulate sum of points for centroid
    // for Bezier curves, we compute additional interior points on curve
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;

    fCentroid = SkPoint::Make(0, 0);
    int centroidCount = 0;
    fClipPolygon.reset();

    // coefficients to compute cubic Bezier at t = 5/16
    const SkScalar kA = 0.32495117187f;
    const SkScalar kB = 0.44311523437f;
    const SkScalar kC = 0.20141601562f;
    const SkScalar kD = 0.03051757812f;

    SkPoint curvePoint;
    SkScalar w;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                ctm.mapPoints(&pts[0], 1);
                devPath->moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                ctm.mapPoints(&pts[1], 1);
                devPath->lineTo(pts[1]);
                fCentroid += pts[1];
                centroidCount++;
                *fClipPolygon.push() = pts[1];
                break;
            case SkPath::kQuad_Verb:
                ctm.mapPoints(pts, 3);
                devPath->quadTo(pts[1], pts[2]);
                // point at t = 1/2
                curvePoint.fX = 0.25f*pts[0].fX + 0.5f*pts[1].fX + 0.25f*pts[2].fX;
                curvePoint.fY = 0.25f*pts[0].fY + 0.5f*pts[1].fY + 0.25f*pts[2].fY;
                *fClipPolygon.push() = curvePoint;
                fCentroid += curvePoint;
                *fClipPolygon.push() = pts[2];
                fCentroid += pts[2];
                centroidCount += 2;
                break;
            case SkPath::kConic_Verb:
                ctm.mapPoints(pts, 3);
                w = iter.conicWeight();
                devPath->conicTo(pts[1], pts[2], w);
                // point at t = 1/2
                curvePoint.fX = 0.25f*pts[0].fX + w*0.5f*pts[1].fX + 0.25f*pts[2].fX;
                curvePoint.fY = 0.25f*pts[0].fY + w*0.5f*pts[1].fY + 0.25f*pts[2].fY;
                curvePoint *= SkScalarInvert(0.5f + 0.5f*w);
                *fClipPolygon.push() = curvePoint;
                fCentroid += curvePoint;
                *fClipPolygon.push() = pts[2];
                fCentroid += pts[2];
                centroidCount += 2;
                break;
            case SkPath::kCubic_Verb:
                ctm.mapPoints(pts, 4);
                devPath->cubicTo(pts[1], pts[2], pts[3]);
                // point at t = 5/16
                curvePoint.fX = kA*pts[0].fX + kB*pts[1].fX + kC*pts[2].fX + kD*pts[3].fX;
                curvePoint.fY = kA*pts[0].fY + kB*pts[1].fY + kC*pts[2].fY + kD*pts[3].fY;
                *fClipPolygon.push() = curvePoint;
                fCentroid += curvePoint;
                // point at t = 11/16
                curvePoint.fX = kD*pts[0].fX + kC*pts[1].fX + kB*pts[2].fX + kA*pts[3].fX;
                curvePoint.fY = kD*pts[0].fY + kC*pts[1].fY + kB*pts[2].fY + kA*pts[3].fY;
                *fClipPolygon.push() = curvePoint;
                fCentroid += curvePoint;
                *fClipPolygon.push() = pts[3];
                fCentroid += pts[3];
                centroidCount += 3;
                break;
            case SkPath::kClose_Verb:
                devPath->close();
                break;
            default:
                SkDEBUGFAIL("unknown verb");
        }
    }

    fCentroid *= SkScalarInvert(centroidCount);
    fCurrPolyPoint = fClipPolygon.count() - 1;
}

void SkSpotShadowTessellator::checkUmbraAndTransformCentroid(SkScalar scale,
                                                             const SkVector& xlate,
                                                             bool useDistanceToPoint) {
    SkASSERT(fClipPolygon.count() >= 3);
    SkPoint transformedCentroid = fCentroid;
    transformedCentroid += xlate;

    SkScalar localRadius = fRadius / scale;
    localRadius *= localRadius;

    // init umbra check
    SkVector w = fCentroid - fClipPolygon[0];
    SkVector v0 = fClipPolygon[1] - fClipPolygon[0];
    *fClipVectors.push() = v0;
    bool validUmbra;
    SkScalar minDistance;
    // check distance against line segment
    if (useDistanceToPoint) {
        minDistance = w.lengthSqd();
    } else {
        SkScalar vSq = v0.dot(v0);
        SkScalar wDotV = w.dot(v0);
        minDistance = w.dot(w) - wDotV*wDotV/vSq;
    }
    validUmbra = (minDistance >= localRadius);

    // init centroid check
    bool hiddenCentroid = true;
    SkVector v1 = transformedCentroid - fClipPolygon[0];
    SkScalar initCross = v0.cross(v1);

    for (int p = 1; p < fClipPolygon.count(); ++p) {
        // Determine whether we have a real umbra by insetting clipPolygon by radius/scale
        // and see if it extends past centroid.
        // TODO: adjust this later for more accurate umbra calcs
        w = fCentroid - fClipPolygon[p];
        v0 = fClipPolygon[(p + 1) % fClipPolygon.count()] - fClipPolygon[p];
        *fClipVectors.push() = v0;
        // check distance against line segment
        SkScalar distance;
        if (useDistanceToPoint) {
            distance = w.lengthSqd();
        } else {
            SkScalar vSq = v0.dot(v0);
            SkScalar wDotV = w.dot(v0);
            distance = w.dot(w) - wDotV*wDotV/vSq;
        }
        if (distance < localRadius) {
            validUmbra = false;
        }
        if (distance < minDistance) {
            minDistance = distance;
        }
        // Determine if transformed centroid is inside clipPolygon.
        v1 = transformedCentroid - fClipPolygon[p];
        if (initCross*v0.cross(v1) <= 0) {
            hiddenCentroid = false;
        }
    }
    SkASSERT(fClipVectors.count() == fClipPolygon.count());

    if (!validUmbra) {
        SkScalar ratio = 256 * SkScalarSqrt(minDistance / localRadius);
        // they aren't PMColors, but the interpolation algorithm is the same
        fUmbraColor = SkPMLerp(fUmbraColor, fPenumbraColor, (unsigned)ratio);
    }

    fTransparent = fTransparent || !hiddenCentroid || !validUmbra;
    fValidUmbra = validUmbra;
    fCentroid = transformedCentroid;
}

bool SkSpotShadowTessellator::clipUmbraPoint(const SkPoint& umbraPoint, const SkPoint& centroid,
                                             SkPoint* clipPoint) {
    SkVector segmentVector = centroid - umbraPoint;

    int startPolyPoint = fCurrPolyPoint;
    do {
        SkVector dp = umbraPoint - fClipPolygon[fCurrPolyPoint];
        SkScalar denom = fClipVectors[fCurrPolyPoint].cross(segmentVector);
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
            SkScalar s_num = dp.cross(fClipVectors[fCurrPolyPoint]);
            // if umbra point is inside the clip polygon
            if (s_num < 0) {
                return false;
            } else {
                segmentVector *= s_num/denom;
                *clipPoint = umbraPoint + segmentVector;
                return true;
            }
        }
        fCurrPolyPoint = (fCurrPolyPoint + 1) % fClipPolygon.count();
    } while (fCurrPolyPoint != startPolyPoint);

    return false;
}

void SkSpotShadowTessellator::mapPoints(SkScalar scale, const SkVector& xlate,
                                        SkPoint* pts, int count) {
    // TODO: vectorize
    for (int i = 0; i < count; ++i) {
        pts[i] *= scale;
        pts[i] += xlate;
    }
}

void SkSpotShadowTessellator::handleLine(const SkPoint& p) {
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
        if (!compute_normal(fInitPoints[0], fInitPoints[1], fRadius, fDirection,
                            &fFirstNormal)) {
            // first two points are incident, make the third point the second and continue
            fInitPoints[1] = p;
            return;
        }

        fFirstPoint = fInitPoints[0];
        fFirstVertex = fPositions.count();
        fPrevNormal = fFirstNormal;
        fPrevPoint = fFirstPoint;
        fPrevUmbraIndex = fFirstVertex;

        this->addInnerPoint(fFirstPoint);

        if (!fTransparent) {
            SkPoint clipPoint;
            bool isOutside = this->clipUmbraPoint(fPositions[fFirstVertex], fCentroid, &clipPoint);
            if (isOutside) {
                *fPositions.push() = clipPoint;
                *fColors.push() = fUmbraColor;
            }
            fPrevUmbraOutside = isOutside;
            fFirstUmbraOutside = isOutside;
        }

        SkPoint newPoint = fFirstPoint + fFirstNormal;
        *fPositions.push() = newPoint;
        *fColors.push() = fPenumbraColor;
        this->addEdge(fInitPoints[1], fFirstNormal);

        // to ensure we skip this block next time
        *fInitPoints.push() = p;
    }

    SkVector normal;
    if (compute_normal(fPrevPoint, p, fRadius, fDirection, &normal)) {
        this->addArc(normal);
        this->finishArcAndAddEdge(p, normal);
    }
}

void SkSpotShadowTessellator::addInnerPoint(const SkPoint& pathPoint) {
    SkVector v = fCentroid - pathPoint;
    SkScalar distance = v.length();
    SkScalar t;
    if (fValidUmbra) {
        SkASSERT(distance >= fRadius);
        t = fRadius / distance;
    } else {
        t = 0.95f;
    }
    v *= t;
    SkPoint umbraPoint = pathPoint + v;
    *fPositions.push() = umbraPoint;
    *fColors.push() = fUmbraColor;

    fPrevPoint = pathPoint;
}

void SkSpotShadowTessellator::addEdge(const SkPoint& nextPoint, const SkVector& nextNormal) {
    // add next umbra point
    this->addInnerPoint(nextPoint);
    int prevPenumbraIndex = fPositions.count() - 2;
    int currUmbraIndex = fPositions.count() - 1;

    // add to center fan if transparent or centroid showing
    if (fTransparent) {
        *fIndices.push() = 0;
        *fIndices.push() = fPrevUmbraIndex;
        *fIndices.push() = currUmbraIndex;
    // otherwise add to clip ring
    } else {
        if (!fTransparent) {
            SkPoint clipPoint;
            bool isOutside = clipUmbraPoint(fPositions[currUmbraIndex], fCentroid, &clipPoint);
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

    *fIndices.push() = fPrevUmbraIndex;
    *fIndices.push() = prevPenumbraIndex;
    *fIndices.push() = currUmbraIndex;

    *fIndices.push() = prevPenumbraIndex;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = currUmbraIndex;

    fPrevUmbraIndex = currUmbraIndex;
    fPrevNormal = nextNormal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShadowVertices> SkShadowVertices::MakeAmbient(const SkPath& path, const SkMatrix& ctm,
                                                      SkScalar radius, SkColor umbra,
                                                      SkColor penumbra, bool transparent) {
    SkAmbientShadowTessellator ambientTess(path, ctm, radius, umbra, penumbra, transparent);
    if (!ambientTess.succeeded()) {
        return nullptr;
    }
    int vcount = ambientTess.vertexCount();
    int icount = ambientTess.indexCount();
    return sk_sp<SkShadowVertices>(new SkShadowVertices(ambientTess.releasePositions(),
                                                        ambientTess.releaseColors(),
                                                        ambientTess.releaseIndices(), vcount,
                                                        icount));
}

sk_sp<SkShadowVertices> SkShadowVertices::MakeSpot(const SkPath& path, const SkMatrix& ctm,
                                                   SkScalar scale, const SkVector& translate,
                                                   SkScalar radius, SkColor umbraColor,
                                                   SkColor penumbraColor, bool transparent) {
    SkSpotShadowTessellator spotTess(path, ctm, scale, translate, radius, umbraColor,
                                     penumbraColor, transparent);
    if (!spotTess.succeeded()) {
        return nullptr;
    }
    int vcount = spotTess.vertexCount();
    int icount = spotTess.indexCount();
    return sk_sp<SkShadowVertices>(new SkShadowVertices(spotTess.releasePositions(),
                                                        spotTess.releaseColors(),
                                                        spotTess.releaseIndices(), vcount, icount));
}
