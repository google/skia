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
    SkBaseShadowTessellator(SkShadowTessellator::HeightFunc, bool transparent);
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
    int vertexCount() const { return fPositions.count(); }
    int indexCount() const { return fIndices.count(); }

    virtual void handleLine(const SkPoint& p) = 0;
    void handleLine(const SkMatrix& m, SkPoint* p);

    void handleQuad(const SkPoint pts[3]);
    void handleQuad(const SkMatrix& m, SkPoint pts[3]);

    void handleCubic(const SkMatrix& m, SkPoint pts[4]);

    void handleConic(const SkMatrix& m, SkPoint pts[3], SkScalar w);

    void addArc(const SkVector& nextNormal);
    void finishArcAndAddEdge(const SkVector& nextPoint, const SkVector& nextNormal);
    virtual void addEdge(const SkVector& nextPoint, const SkVector& nextNormal) = 0;

    SkShadowTessellator::HeightFunc fHeightFunc;

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

SkBaseShadowTessellator::SkBaseShadowTessellator(SkShadowTessellator::HeightFunc heightFunc,
                                                 bool transparent)
        : fHeightFunc(heightFunc)
        , fFirstVertex(-1)
        , fSucceeded(false)
        , fTransparent(transparent)
        , fDirection(1)
        , fPrevUmbraIndex(-1) {
    fInitPoints.setReserve(3);

    // child classes will set reserve for positions, colors and indices
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

void SkBaseShadowTessellator::addArc(const SkVector& nextNormal) {
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

void SkBaseShadowTessellator::finishArcAndAddEdge(const SkPoint& nextPoint,
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

class SkAmbientShadowTessellator : public SkBaseShadowTessellator {
public:
    SkAmbientShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                               SkShadowTessellator::HeightFunc heightFunc,
                               SkScalar ambientAlpha, bool transparent);

private:
    void handleLine(const SkPoint& p) override;
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal) override;

    int                 fCentroidCount;

    typedef SkBaseShadowTessellator INHERITED;
};

static const float kHeightFactor = 1.0f / 128.0f;
static const float kGeomFactor = 64.0f;

SkAmbientShadowTessellator::SkAmbientShadowTessellator(const SkPath& path,
                                                       const SkMatrix& ctm,
                                                       SkShadowTessellator::HeightFunc heightFunc,
                                                       SkScalar ambientAlpha,
                                                       bool transparent)
        : INHERITED(heightFunc, transparent) {
    // Set radius and colors
    // TODO: vary colors and radius based on heightFunc
    SkScalar occluderHeight = heightFunc(0, 0);
    fRadius = occluderHeight * kHeightFactor * kGeomFactor;
    SkScalar umbraAlpha = SkScalarInvert((1.0f + SkTMax(occluderHeight*kHeightFactor, 0.0f)));
    // umbraColor is the interior value, penumbraColor the exterior value.
    // umbraAlpha is the factor that is linearly interpolated from outside to inside, and
    // then "blurred" by the GrBlurredEdgeFP. It is then multiplied by fAmbientAlpha to get
    // the final alpha.
    fUmbraColor = SkColorSetARGB(255, 0, ambientAlpha * 255.9999f, umbraAlpha * 255.9999f);
    fPenumbraColor = SkColorSetARGB(255, 0, ambientAlpha * 255.9999f, 0);

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

class SkSpotShadowTessellator : public SkBaseShadowTessellator {
public:
    SkSpotShadowTessellator(const SkPath& path, const SkMatrix& ctm,
                            SkShadowTessellator::HeightFunc heightFunc,
                            const SkPoint3& lightPos, SkScalar lightRadius,
                            SkScalar spotAlpha, bool transparent);

private:
    void computeClipAndPathPolygons(const SkPath& path, const SkMatrix& ctm,
                                    SkScalar scale, const SkVector& xlate);
    void computeClipVectorsAndTestCentroid();
    bool clipUmbraPoint(const SkPoint& umbraPoint, const SkPoint& centroid, SkPoint* clipPoint);
    int getClosestUmbraPoint(const SkPoint& point);

    void handleLine(const SkPoint& p) override;
    void handlePolyPoint(const SkPoint& p);

    void mapPoints(SkScalar scale, const SkVector& xlate, SkPoint* pts, int count);
    bool addInnerPoint(const SkPoint& pathPoint);
    void addEdge(const SkVector& nextPoint, const SkVector& nextNormal) override;

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
                                                 SkShadowTessellator::HeightFunc heightFunc,
                                                 const SkPoint3& lightPos, SkScalar lightRadius,
                                                 SkScalar spotAlpha, bool transparent)
        : INHERITED(heightFunc, transparent)
        , fCurrClipPoint(0)
        , fPrevUmbraOutside(false)
        , fFirstUmbraOutside(false)
        , fValidUmbra(true) {

    // Set radius and colors
    // TODO: vary colors and radius based on heightFunc
    SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
    SkScalar occluderHeight = heightFunc(center.fX, center.fY);
    float zRatio = SkTPin(occluderHeight / (lightPos.fZ - occluderHeight), 0.0f, 0.95f);
    SkScalar radius = lightRadius * zRatio;
    fRadius = radius;
    fUmbraColor = SkColorSetARGB(255, 0, spotAlpha * 255.9999f, 255);
    fPenumbraColor = SkColorSetARGB(255, 0, spotAlpha * 255.9999f, 0);

    // Compute the scale and translation for the spot shadow.
    SkScalar scale = lightPos.fZ / (lightPos.fZ - occluderHeight);
    ctm.mapPoints(&center, 1);
    SkVector translate = SkVector::Make(zRatio * (center.fX - lightPos.fX),
                                        zRatio * (center.fY - lightPos.fY));

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
    this->computeClipAndPathPolygons(path, ctm, scale, translate);
    if (fClipPolygon.count() < 3 || fPathPolygon.count() < 3) {
        return;
    }

    // check to see if umbra collapses
    SkScalar minDistSq = fCentroid.distanceToLineSegmentBetweenSqd(fPathPolygon[0],
                                                                   fPathPolygon[1]);
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
        SkScalar ratio = 256 * newRadius / radius;
        // they aren't PMColors, but the interpolation algorithm is the same
        fUmbraColor = SkPMLerp(fUmbraColor, fPenumbraColor, (unsigned)ratio);
        radius = newRadius;
    }

    // compute vectors for clip tests
    this->computeClipVectorsAndTestCentroid();

    // generate inner ring
    if (!SkInsetConvexPolygon(&fPathPolygon[0], fPathPolygon.count(), radius, &fUmbraPolygon)) {
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
        this->handlePolyPoint(fPathPolygon[i]);
    }

    if (!this->indexCount()) {
        return;
    }

    // finish up the final verts
    SkVector normal;
    if (compute_normal(fPrevPoint, fFirstPoint, fRadius, fDirection, &normal)) {
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

void SkSpotShadowTessellator::computeClipAndPathPolygons(const SkPath& path, const SkMatrix& ctm,
                                                         SkScalar scale, const SkVector& xlate) {
    // For the path polygon we are going to apply 'scale' and 'xlate' (in that order) to each
    // computed path point. We want the effect to be to scale the points relative to the path
    // bounds center and then translate them by the 'xlate' param we were passed.
    SkPoint center = SkPoint::Make(path.getBounds().centerX(), path.getBounds().centerY());
    ctm.mapPoints(&center, 1);
    SkVector translate = center * (1.f - scale) + xlate;
    SkMatrix shadowTransform;
    shadowTransform.setScaleTranslate(scale, scale, translate.fX, translate.fY);

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
            if (s_num < 0) {
                return false;
            } else {
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

static bool is_collinear(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2) {
    SkVector v0 = p1 - p0;
    SkVector v1 = p2 - p0;
    return (SkScalarNearlyZero(v0.cross(v1)));
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

void SkSpotShadowTessellator::handlePolyPoint(const SkPoint& p) {
    if (fInitPoints.count() < 2) {
        *fInitPoints.push() = p;
        return;
    }

    if (fInitPoints.count() == 2) {
        // determine if cw or ccw
        SkVector v0 = fInitPoints[1] - fInitPoints[0];
        SkVector v1 = p - fInitPoints[0];
        SkScalar perpDot = v0.cross(v1);
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
    if (fPrevUmbraIndex == fFirstVertex ||
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
    fPrevNormal = nextNormal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkVertices> SkShadowTessellator::MakeAmbient(const SkPath& path, const SkMatrix& ctm,
                                                   HeightFunc heightFunc, SkScalar ambientAlpha,
                                                   bool transparent) {
    SkAmbientShadowTessellator ambientTess(path, ctm, heightFunc, ambientAlpha, transparent);
    return ambientTess.releaseVertices();
}

sk_sp<SkVertices> SkShadowTessellator::MakeSpot(const SkPath& path, const SkMatrix& ctm,
                                                HeightFunc heightFunc,
                                                const SkPoint3& lightPos, SkScalar lightRadius,
                                                SkScalar spotAlpha, bool transparent) {
    SkSpotShadowTessellator spotTess(path, ctm, heightFunc, lightPos, lightRadius,
                                     spotAlpha, transparent);
    return spotTess.releaseVertices();
}
