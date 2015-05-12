/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrStrokePathRenderer.h"

#include "GrDrawTarget.h"
#include "SkPath.h"
#include "SkStrokeRec.h"

static bool is_clockwise(const SkVector& before, const SkVector& after) {
    return before.cross(after) > 0;
}

enum IntersectionType {
    kNone_IntersectionType,
    kIn_IntersectionType,
    kOut_IntersectionType
};

static IntersectionType intersection(const SkPoint& p1, const SkPoint& p2,
                                     const SkPoint& p3, const SkPoint& p4,
                                     SkPoint& res) {
    // Store the values for fast access and easy
    // equations-to-code conversion
    SkScalar x1 = p1.x(), x2 = p2.x(), x3 = p3.x(), x4 = p4.x();
    SkScalar y1 = p1.y(), y2 = p2.y(), y3 = p3.y(), y4 = p4.y();

    SkScalar d = SkScalarMul(x1 - x2, y3 - y4) - SkScalarMul(y1 - y2, x3 - x4);
    // If d is zero, there is no intersection
    if (SkScalarNearlyZero(d)) {
        return kNone_IntersectionType;
    }

    // Get the x and y
    SkScalar pre  = SkScalarMul(x1, y2) - SkScalarMul(y1, x2),
             post = SkScalarMul(x3, y4) - SkScalarMul(y3, x4);
    // Compute the point of intersection
    res.set(SkScalarDiv(SkScalarMul(pre, x3 - x4) - SkScalarMul(x1 - x2, post), d),
            SkScalarDiv(SkScalarMul(pre, y3 - y4) - SkScalarMul(y1 - y2, post), d));

    // Check if the x and y coordinates are within both lines
    return (res.x() < GrMin(x1, x2) || res.x() > GrMax(x1, x2) ||
            res.x() < GrMin(x3, x4) || res.x() > GrMax(x3, x4) ||
            res.y() < GrMin(y1, y2) || res.y() > GrMax(y1, y2) ||
            res.y() < GrMin(y3, y4) || res.y() > GrMax(y3, y4)) ?
            kOut_IntersectionType : kIn_IntersectionType;
}

GrStrokePathRenderer::GrStrokePathRenderer() {
}

bool GrStrokePathRenderer::canDrawPath(const SkPath& path,
                                       const SkStrokeRec& stroke,
                                       const GrDrawTarget* target,
                                       bool antiAlias) const {
    // FIXME : put the proper condition once GrDrawTarget::isOpaque is implemented
    const bool isOpaque = true; // target->isOpaque();

    // FIXME : remove this requirement once we have AA circles and implement the
    //         circle joins/caps appropriately in the ::onDrawPath() function.
    const bool requiresAACircle = (stroke.getCap()  == SkPaint::kRound_Cap) ||
                                  (stroke.getJoin() == SkPaint::kRound_Join);

    // Indices being stored in uint16, we don't want to overflow the indices capacity
    static const int maxVBSize = 1 << 16;
    const int maxNbVerts = (path.countPoints() + 1) * 5;

    // Check that the path contains no curved lines, only straight lines
    static const uint32_t unsupportedMask = SkPath::kQuad_SegmentMask | SkPath::kCubic_SegmentMask;

    // Must not be filled nor hairline nor semi-transparent
    // Note : May require a check to path.isConvex() if AA is supported
    return ((stroke.getStyle() == SkStrokeRec::kStroke_Style) && (maxNbVerts < maxVBSize) &&
            !path.isInverseFillType() && isOpaque && !requiresAACircle && !antiAlias &&
            ((path.getSegmentMasks() & unsupportedMask) == 0));
}

bool GrStrokePathRenderer::onDrawPath(const SkPath& origPath,
                                      const SkStrokeRec& stroke,
                                      GrDrawTarget* target,
                                      bool antiAlias) {
    if (origPath.isEmpty()) {
        return true;
    }

    SkScalar width = stroke.getWidth();
    if (width <= 0) {
        return false;
    }

    // Get the join type
    SkPaint::Join join = stroke.getJoin();
    SkScalar miterLimit = stroke.getMiter();
    SkScalar sqMiterLimit = SkScalarMul(miterLimit, miterLimit);
    if ((join == SkPaint::kMiter_Join) && (miterLimit <= SK_Scalar1)) {
        // If the miter limit is small, treat it as a bevel join
        join = SkPaint::kBevel_Join;
    }
    const bool isMiter       = (join == SkPaint::kMiter_Join);
    const bool isBevel       = (join == SkPaint::kBevel_Join);
    SkScalar invMiterLimit   = isMiter ? SK_Scalar1 / miterLimit : 0;
    SkScalar invMiterLimitSq = SkScalarMul(invMiterLimit, invMiterLimit);

    // Allocate vertices
    const int nbQuads     = origPath.countPoints() + 1; // Could be "-1" if path is not closed
    const int extraVerts  = isMiter || isBevel ? 1 : 0;
    const int maxVertexCount = nbQuads * (4 + extraVerts);
    const int maxIndexCount  = nbQuads * (6 + extraVerts * 3); // Each extra vert adds a triangle
    target->drawState()->setDefaultVertexAttribs();
    GrDrawTarget::AutoReleaseGeometry arg(target, maxVertexCount, maxIndexCount);
    if (!arg.succeeded()) {
        return false;
    }
    SkPoint* verts = reinterpret_cast<SkPoint*>(arg.vertices());
    uint16_t* idxs = reinterpret_cast<uint16_t*>(arg.indices());
    int vCount = 0, iCount = 0;

    // Transform the path into a list of triangles
    SkPath::Iter iter(origPath, false);
    SkPoint pts[4];
    const SkScalar radius = SkScalarMul(width, 0.5f);
    SkPoint *firstPt = verts, *lastPt = NULL;
    SkVector firstDir, dir;
    firstDir.set(0, 0);
    dir.set(0, 0);
    bool isOpen = true;
    for(SkPath::Verb v = iter.next(pts); v != SkPath::kDone_Verb; v = iter.next(pts)) {
        switch(v) {
            case SkPath::kMove_Verb:
                // This will already be handled as pts[0] of the 1st line
                break;
            case SkPath::kClose_Verb:
                isOpen = (lastPt == NULL);
                break;
            case SkPath::kLine_Verb:
            {
                SkVector v0 = dir;
                dir = pts[1] - pts[0];
                if (dir.setLength(radius)) {
                    SkVector dirT;
                    dirT.set(dir.fY, -dir.fX); // Get perpendicular direction
                    SkPoint l1a = pts[0]+dirT, l1b = pts[1]+dirT,
                            l2a = pts[0]-dirT, l2b = pts[1]-dirT;
                    SkPoint miterPt[2];
                    bool useMiterPoint = false;
                    int idx0(-1), idx1(-1);
                    if (NULL == lastPt) {
                        firstDir = dir;
                    } else {
                        SkVector v1 = dir;
                        if (v0.normalize() && v1.normalize()) {
                            SkScalar dotProd = v0.dot(v1);
                            // No need for bevel or miter join if the angle
                            // is either 0 or 180 degrees
                            if (!SkScalarNearlyZero(dotProd + SK_Scalar1) &&
                                !SkScalarNearlyZero(dotProd - SK_Scalar1)) {
                                bool ccw = !is_clockwise(v0, v1);
                                int offset = ccw ? 1 : 0;
                                idx0 = vCount-2+offset;
                                idx1 = vCount+offset;
                                const SkPoint* pt0 = &(lastPt[offset]);
                                const SkPoint* pt1 = ccw ? &l2a : &l1a;
                                switch(join) {
                                    case SkPaint::kMiter_Join:
                                    {
                                        // *Note : Logic is from MiterJoiner

                                        // FIXME : Special case if we have a right angle ?
                                        // if (SkScalarNearlyZero(dotProd)) {...}

                                        SkScalar sinHalfAngleSq =
                                                SkScalarHalf(SK_Scalar1 + dotProd);
                                        if (sinHalfAngleSq >= invMiterLimitSq) {
                                            // Find the miter point (or points if it is further
                                            // than the miter limit)
                                            const SkPoint pt2 = *pt0+v0, pt3 = *pt1+v1;
                                            if (intersection(*pt0, pt2, *pt1, pt3, miterPt[0]) !=
                                                kNone_IntersectionType) {
                                                SkPoint miterPt0 = miterPt[0] - *pt0;
                                                SkPoint miterPt1 = miterPt[0] - *pt1;
                                                SkScalar sqDist0 = miterPt0.dot(miterPt0);
                                                SkScalar sqDist1 = miterPt1.dot(miterPt1);
                                                const SkScalar rSq =
                                                        SkScalarDiv(SkScalarMul(radius, radius),
                                                                    sinHalfAngleSq);
                                                const SkScalar sqRLimit =
                                                        SkScalarMul(sqMiterLimit, rSq);
                                                if (sqDist0 > sqRLimit || sqDist1 > sqRLimit) {
                                                    if (sqDist1 > sqRLimit) {
                                                        v1.setLength(SkScalarSqrt(sqRLimit));
                                                        miterPt[1] = *pt1+v1;
                                                    } else {
                                                        miterPt[1] = miterPt[0];
                                                    }
                                                    if (sqDist0 > sqRLimit) {
                                                        v0.setLength(SkScalarSqrt(sqRLimit));
                                                        miterPt[0] = *pt0+v0;
                                                    }
                                                } else {
                                                    miterPt[1] = miterPt[0];
                                                }
                                                useMiterPoint = true;
                                            }
                                        }
                                        if (useMiterPoint && (miterPt[1] == miterPt[0])) {
                                            break;
                                        }
                                    }
                                    default:
                                    case SkPaint::kBevel_Join:
                                    {
                                        // Note : This currently causes some overdraw where both
                                        //        lines initially intersect. We'd need to add
                                        //        another line intersection check here if the
                                        //        overdraw becomes an issue instead of using the
                                        //        current point directly.

                                        // Add center point
                                        *verts++ = pts[0]; // Use current point directly
                                        // This idx is passed the current point so increment it
                                        ++idx1;
                                        // Add center triangle
                                        *idxs++ = idx0;
                                        *idxs++ = vCount;
                                        *idxs++ = idx1;
                                        vCount++;
                                        iCount += 3;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    *verts++ = l1a;
                    *verts++ = l2a;
                    lastPt   = verts;
                    *verts++ = l1b;
                    *verts++ = l2b;

                    if (useMiterPoint && (idx0 >= 0) && (idx1 >= 0)) {
                        firstPt[idx0] = miterPt[0];
                        firstPt[idx1] = miterPt[1];
                    }

                    // 1st triangle
                    *idxs++  = vCount+0;
                    *idxs++  = vCount+2;
                    *idxs++  = vCount+1;
                    // 2nd triangle
                    *idxs++  = vCount+1;
                    *idxs++  = vCount+2;
                    *idxs++  = vCount+3;

                    vCount += 4;
                    iCount += 6;
                }
            }
                break;
            case SkPath::kQuad_Verb:
            case SkPath::kCubic_Verb:
                SkDEBUGFAIL("Curves not supported!");
            default:
                // Unhandled cases
                SkASSERT(false);
        }
    }

    if (isOpen) {
        // Add caps
        switch (stroke.getCap()) {
            case SkPaint::kSquare_Cap:
                firstPt[0] -= firstDir;
                firstPt[1] -= firstDir;
                lastPt [0] += dir;
                lastPt [1] += dir;
                break;
            case SkPaint::kRound_Cap:
                SkDEBUGFAIL("Round caps not supported!");
            default: // No cap
                break;
        }
    }

    SkASSERT(vCount <= maxVertexCount);
    SkASSERT(iCount <= maxIndexCount);

    if (vCount > 0) {
        target->drawIndexed(kTriangles_GrPrimitiveType,
                            0,        // start vertex
                            0,        // start index
                            vCount,
                            iCount);
    }

    return true;
}
