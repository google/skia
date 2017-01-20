/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkShadowTessellator.h"
#include "SkGeometry.h"
#include "GrPathUtils.h"

SkAmbientShadowTessellator::SkAmbientShadowTessellator(const SkMatrix& viewMatrix,
                                                       const SkPath& path,
                                                       SkScalar radius,
                                                       GrColor umbraColor,
                                                       bool transparent)
    : fRadius(radius)
    , fUmbraColor(umbraColor)
    , fTransparent(transparent)
    , fPrevInnerIndex(-1)
    , fPrevOuterIndex(-1) {

    // Outer ring: 3*numPts
    // Middle ring: numPts
    fPositions.setReserve(4 * path.countPoints());
    fColors.setReserve(4 * path.countPoints());
    // Outer ring: 12*numPts
    // Middle ring: 0
    fIndices.setReserve(12 * path.countPoints());

    fInitPoints.setReserve(3);

    // walk around the path, tessellate and generate outer ring
    // if original path is transparent, will accumulate sum of points for centroid
    SkPath::Iter iter(path, true);
    SkPoint pts[4];
    SkPath::Verb verb;
    if (transparent) {
        *fPositions.push() = SkPoint::Make(0, 0);
        *fColors.push() = umbraColor;
    }
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kLine_Verb:
                this->lineTo(viewMatrix, pts[1]);
                break;
            case SkPath::kQuad_Verb:
                this->quadTo(viewMatrix, pts);
                break;
            case SkPath::kCubic_Verb:
                this->cubicTo(viewMatrix, pts);
                break;
            case SkPath::kConic_Verb:
                this->conicTo(viewMatrix, pts, iter.conicWeight());
                break;
            case SkPath::kMove_Verb:
            case SkPath::kClose_Verb:
            case SkPath::kDone_Verb:
                break;
        }
    }

    SkVector normal;
    normal.fX = fPositions[fPrevInnerIndex].fY - fPositions[fFirstVertex].fY;
    normal.fY = fPositions[fFirstVertex].fX - fPositions[fPrevInnerIndex].fX;
    normal *= fDirection;
    if (!normal.normalize()) {
        //*** better error recovery?
        return;
    }
    normal *= fRadius;

    // fill in fan from previous quad
    //*** TODO: step full curve
    SkVector midNormal = fPrevNormal + normal;
    midNormal *= SK_ScalarHalf;
    *fPositions.push() = fPositions[fPrevInnerIndex] + midNormal;
    *fColors.push() = 0;
    *fPositions.push() = fPositions[fPrevInnerIndex] + normal;
    *fColors.push() = 0;

    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPrevOuterIndex;
    *fIndices.push() = fPrevOuterIndex + 1;
    
    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPrevOuterIndex + 1;
    *fIndices.push() = fPositions.count() - 1;

    // add final quad
    *fPositions.push() = fPositions[fFirstVertex] + normal;
    *fColors.push() = 0;

    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPositions.count() - 2;
    *fIndices.push() = fFirstVertex;

    *fIndices.push() = fPositions.count() - 2;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = fFirstVertex;
        
    // final fan
    //*** TODO: step full curve
    midNormal = normal + fFirstNormal;
    midNormal *= SK_ScalarHalf;
    *fPositions.push() = fPositions[fPrevInnerIndex] + midNormal;
    *fColors.push() = 0;
        
    *fIndices.push() = fFirstVertex;
    *fIndices.push() = fPositions.count() - 2;
    *fIndices.push() = fPositions.count() - 1;
    
    *fIndices.push() = fFirstVertex;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = fFirstVertex + 1;
        
    // finalize centroid

}

// tesselation tolerance values, in device space pixels
static const SkScalar kQuadTolerance = 0.2f;
//static const SkScalar kCubicTolerance = 0.2f;
static const SkScalar kConicTolerance = 0.5f;

void SkAmbientShadowTessellator::lineTo(const SkPoint& p)  {
    //*** need to handle incident points

    if (fInitPoints.count() < 2) {
        *fInitPoints.push() = p;
        return;
    } else if (fInitPoints.count() == 2) {
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
        fFirstNormal.fX = fInitPoints[0].fY - fInitPoints[1].fY;
        fFirstNormal.fY = fInitPoints[1].fX - fInitPoints[0].fX;
        fFirstNormal *= fDirection;
        if (!fFirstNormal.normalize()) {
            //*** better error recovery?
            return;
        }
        fFirstNormal *= fRadius;

        fFirstVertex = fPositions.count();
        fPrevNormal = fFirstNormal;
        fPrevInnerIndex = fFirstVertex + 2;  //*** may not need these
        fPrevOuterIndex = fFirstVertex + 3;

        *fPositions.push() = fInitPoints[0];
        *fColors.push() = fUmbraColor;
        *fPositions.push() = fInitPoints[0] + fFirstNormal;
        *fColors.push() = 0;
        *fPositions.push() = fInitPoints[1];
        *fColors.push() = fUmbraColor;
        *fPositions.push() = fInitPoints[1] + fFirstNormal;
        *fColors.push() = 0;

        *fIndices.push() = fFirstVertex;
        *fIndices.push() = fFirstVertex + 1;
        *fIndices.push() = fPrevInnerIndex;

        *fIndices.push() = fFirstVertex + 1;
        *fIndices.push() = fPrevOuterIndex;
        *fIndices.push() = fPrevInnerIndex;

        // if transparent, add points to first one in array
        if (fTransparent) {
            fPositions[0] += fInitPoints[0];
            fPositions[0] += fInitPoints[1];
        }

        // to ensure we skip this block next time
        *fInitPoints.push() = p;
    }

    SkVector normal;
    normal.fX = fPositions[fPrevInnerIndex].fY - p.fY;
    normal.fY = p.fX - fPositions[fPrevInnerIndex].fX;
    normal *= fDirection;
    if (!normal.normalize()) {
        //*** better error recovery?
        return;
    }
    normal *= fRadius;

    // fill in fan from previous quad
    //*** TODO: step full curve
    SkVector midNormal = fPrevNormal + normal;
    midNormal *= SK_ScalarHalf;
    *fPositions.push() = fPositions[fPrevInnerIndex] + midNormal;
    *fColors.push() = 0;
    *fPositions.push() = fPositions[fPrevInnerIndex] + normal;
    *fColors.push() = 0;
    
    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPrevOuterIndex;
    *fIndices.push() = fPrevOuterIndex + 1;
    
    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPrevOuterIndex + 1;
    *fIndices.push() = fPositions.count() - 1;
    
    // add next quad
    *fPositions.push() = p;
    *fColors.push() = fUmbraColor;
    *fPositions.push() = p + normal;
    *fColors.push() = 0;

    *fIndices.push() = fPrevInnerIndex;
    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 2;

    *fIndices.push() = fPositions.count() - 3;
    *fIndices.push() = fPositions.count() - 1;
    *fIndices.push() = fPositions.count() - 2;

    fPrevInnerIndex = fPositions.count() - 2;
    fPrevOuterIndex = fPositions.count() - 1;
    fPrevNormal = normal;

    // if transparent, add point to first one in array
    if (fTransparent) {
        fPositions[0] += p;
    }
}

void SkAmbientShadowTessellator::lineTo(const SkMatrix& m, SkPoint p)  {
    m.mapPoints(&p, 1);
    this->lineTo(p);
}

void SkAmbientShadowTessellator::quadTo(const SkPoint pts[3]) {
    int maxCount = GrPathUtils::quadraticPointCount(pts, kQuadTolerance);
    fPointBuffer.setReserve(maxCount);
    SkPoint* target = fPointBuffer.begin();
    int count = GrPathUtils::generateQuadraticPoints(pts[0], pts[1], pts[2],
                                                     kQuadTolerance, &target, maxCount);
    fPointBuffer.setCount(count);
    for (int i = 0; i < count - 1; i++) {
        this->lineTo(fPointBuffer[i]);
    }
    this->lineTo(fPointBuffer[count - 1]);

}

void SkAmbientShadowTessellator::quadTo(const SkMatrix& m, SkPoint pts[3]) {
    m.mapPoints(pts, 3);
    this->quadTo(pts);
}

void SkAmbientShadowTessellator::cubicTo(const SkMatrix& m, SkPoint pts[4]) {

}

void SkAmbientShadowTessellator::conicTo(const SkMatrix& m, SkPoint pts[3], SkScalar w) {
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
        this->quadTo(quadPts);
        lastPoint = quadPts[2];
        quads += 2;
    }
}
