/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkStrokerPriv.h"
#include "SkGeometry.h"
#include "SkPath.h"

#define kMaxQuadSubdivide   5
#define kMaxCubicSubdivide  7

static inline bool degenerate_vector(const SkVector& v) {
    return !SkPoint::CanNormalize(v.fX, v.fY);
}

static inline bool normals_too_curvy(const SkVector& norm0, SkVector& norm1) {
    /*  root2/2 is a 45-degree angle
        make this constant bigger for more subdivisions (but not >= 1)
    */
    static const SkScalar kFlatEnoughNormalDotProd =
                                            SK_ScalarSqrt2/2 + SK_Scalar1/10;

    SkASSERT(kFlatEnoughNormalDotProd > 0 &&
             kFlatEnoughNormalDotProd < SK_Scalar1);

    return SkPoint::DotProduct(norm0, norm1) <= kFlatEnoughNormalDotProd;
}

static inline bool normals_too_pinchy(const SkVector& norm0, SkVector& norm1) {
    static const SkScalar kTooPinchyNormalDotProd = -SK_Scalar1 * 999 / 1000;

    return SkPoint::DotProduct(norm0, norm1) <= kTooPinchyNormalDotProd;
}

static bool set_normal_unitnormal(const SkPoint& before, const SkPoint& after,
                                  SkScalar radius,
                                  SkVector* normal, SkVector* unitNormal) {
    if (!unitNormal->setNormalize(after.fX - before.fX, after.fY - before.fY)) {
        return false;
    }
    unitNormal->rotateCCW();
    unitNormal->scale(radius, normal);
    return true;
}

static bool set_normal_unitnormal(const SkVector& vec,
                                  SkScalar radius,
                                  SkVector* normal, SkVector* unitNormal) {
    if (!unitNormal->setNormalize(vec.fX, vec.fY)) {
        return false;
    }
    unitNormal->rotateCCW();
    unitNormal->scale(radius, normal);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

class SkPathStroker {
public:
    SkPathStroker(const SkPath& src,
                  SkScalar radius, SkScalar miterLimit, SkPaint::Cap cap,
                  SkPaint::Join join);

    void moveTo(const SkPoint&);
    void lineTo(const SkPoint&);
    void quadTo(const SkPoint&, const SkPoint&);
    void cubicTo(const SkPoint&, const SkPoint&, const SkPoint&);
    void close(bool isLine) { this->finishContour(true, isLine); }

    void done(SkPath* dst, bool isLine) {
        this->finishContour(false, isLine);
        fOuter.addPath(fExtra);
        dst->swap(fOuter);
    }

private:
    SkScalar    fRadius;
    SkScalar    fInvMiterLimit;

    SkVector    fFirstNormal, fPrevNormal, fFirstUnitNormal, fPrevUnitNormal;
    SkPoint     fFirstPt, fPrevPt;  // on original path
    SkPoint     fFirstOuterPt;
    int         fSegmentCount;
    bool        fPrevIsLine;

    SkStrokerPriv::CapProc  fCapper;
    SkStrokerPriv::JoinProc fJoiner;

    SkPath  fInner, fOuter; // outer is our working answer, inner is temp
    SkPath  fExtra;         // added as extra complete contours

    void    finishContour(bool close, bool isLine);
    void    preJoinTo(const SkPoint&, SkVector* normal, SkVector* unitNormal,
                      bool isLine);
    void    postJoinTo(const SkPoint&, const SkVector& normal,
                       const SkVector& unitNormal);

    void    line_to(const SkPoint& currPt, const SkVector& normal);
    void    quad_to(const SkPoint pts[3],
                    const SkVector& normalAB, const SkVector& unitNormalAB,
                    SkVector* normalBC, SkVector* unitNormalBC,
                    int subDivide);
    void    cubic_to(const SkPoint pts[4],
                    const SkVector& normalAB, const SkVector& unitNormalAB,
                    SkVector* normalCD, SkVector* unitNormalCD,
                    int subDivide);
};

///////////////////////////////////////////////////////////////////////////////

void SkPathStroker::preJoinTo(const SkPoint& currPt, SkVector* normal,
                              SkVector* unitNormal, bool currIsLine) {
    SkASSERT(fSegmentCount >= 0);

    SkScalar    prevX = fPrevPt.fX;
    SkScalar    prevY = fPrevPt.fY;

    SkAssertResult(set_normal_unitnormal(fPrevPt, currPt, fRadius, normal,
                                         unitNormal));

    if (fSegmentCount == 0) {
        fFirstNormal = *normal;
        fFirstUnitNormal = *unitNormal;
        fFirstOuterPt.set(prevX + normal->fX, prevY + normal->fY);

        fOuter.moveTo(fFirstOuterPt.fX, fFirstOuterPt.fY);
        fInner.moveTo(prevX - normal->fX, prevY - normal->fY);
    } else {    // we have a previous segment
        fJoiner(&fOuter, &fInner, fPrevUnitNormal, fPrevPt, *unitNormal,
                fRadius, fInvMiterLimit, fPrevIsLine, currIsLine);
    }
    fPrevIsLine = currIsLine;
}

void SkPathStroker::postJoinTo(const SkPoint& currPt, const SkVector& normal,
                               const SkVector& unitNormal) {
    fPrevPt = currPt;
    fPrevUnitNormal = unitNormal;
    fPrevNormal = normal;
    fSegmentCount += 1;
}

void SkPathStroker::finishContour(bool close, bool currIsLine) {
    if (fSegmentCount > 0) {
        SkPoint pt;

        if (close) {
            fJoiner(&fOuter, &fInner, fPrevUnitNormal, fPrevPt,
                    fFirstUnitNormal, fRadius, fInvMiterLimit,
                    fPrevIsLine, currIsLine);
            fOuter.close();
            // now add fInner as its own contour
            fInner.getLastPt(&pt);
            fOuter.moveTo(pt.fX, pt.fY);
            fOuter.reversePathTo(fInner);
            fOuter.close();
        } else {    // add caps to start and end
            // cap the end
            fInner.getLastPt(&pt);
            fCapper(&fOuter, fPrevPt, fPrevNormal, pt,
                    currIsLine ? &fInner : NULL);
            fOuter.reversePathTo(fInner);
            // cap the start
            fCapper(&fOuter, fFirstPt, -fFirstNormal, fFirstOuterPt,
                    fPrevIsLine ? &fInner : NULL);
            fOuter.close();
        }
    }
    // since we may re-use fInner, we rewind instead of reset, to save on
    // reallocating its internal storage.
    fInner.rewind();
    fSegmentCount = -1;
}

///////////////////////////////////////////////////////////////////////////////

SkPathStroker::SkPathStroker(const SkPath& src,
                             SkScalar radius, SkScalar miterLimit,
                             SkPaint::Cap cap, SkPaint::Join join)
        : fRadius(radius) {

    /*  This is only used when join is miter_join, but we initialize it here
        so that it is always defined, to fis valgrind warnings.
    */
    fInvMiterLimit = 0;

    if (join == SkPaint::kMiter_Join) {
        if (miterLimit <= SK_Scalar1) {
            join = SkPaint::kBevel_Join;
        } else {
            fInvMiterLimit = SkScalarInvert(miterLimit);
        }
    }
    fCapper = SkStrokerPriv::CapFactory(cap);
    fJoiner = SkStrokerPriv::JoinFactory(join);
    fSegmentCount = -1;
    fPrevIsLine = false;

    // Need some estimate of how large our final result (fOuter)
    // and our per-contour temp (fInner) will be, so we don't spend
    // extra time repeatedly growing these arrays.
    //
    // 3x for result == inner + outer + join (swag)
    // 1x for inner == 'wag' (worst contour length would be better guess)
    fOuter.incReserve(src.countPoints() * 3);
    fInner.incReserve(src.countPoints());
}

void SkPathStroker::moveTo(const SkPoint& pt) {
    if (fSegmentCount > 0) {
        this->finishContour(false, false);
    }
    fSegmentCount = 0;
    fFirstPt = fPrevPt = pt;
}

void SkPathStroker::line_to(const SkPoint& currPt, const SkVector& normal) {
    fOuter.lineTo(currPt.fX + normal.fX, currPt.fY + normal.fY);
    fInner.lineTo(currPt.fX - normal.fX, currPt.fY - normal.fY);
}

void SkPathStroker::lineTo(const SkPoint& currPt) {
    if (SkPath::IsLineDegenerate(fPrevPt, currPt)) {
        return;
    }
    SkVector    normal, unitNormal;

    this->preJoinTo(currPt, &normal, &unitNormal, true);
    this->line_to(currPt, normal);
    this->postJoinTo(currPt, normal, unitNormal);
}

void SkPathStroker::quad_to(const SkPoint pts[3],
                      const SkVector& normalAB, const SkVector& unitNormalAB,
                      SkVector* normalBC, SkVector* unitNormalBC,
                      int subDivide) {
    if (!set_normal_unitnormal(pts[1], pts[2], fRadius,
                               normalBC, unitNormalBC)) {
        // pts[1] nearly equals pts[2], so just draw a line to pts[2]
        this->line_to(pts[2], normalAB);
        *normalBC = normalAB;
        *unitNormalBC = unitNormalAB;
        return;
    }

    if (--subDivide >= 0 && normals_too_curvy(unitNormalAB, *unitNormalBC)) {
        SkPoint     tmp[5];
        SkVector    norm, unit;

        SkChopQuadAtHalf(pts, tmp);
        this->quad_to(&tmp[0], normalAB, unitNormalAB, &norm, &unit, subDivide);
        this->quad_to(&tmp[2], norm, unit, normalBC, unitNormalBC, subDivide);
    } else {
        SkVector    normalB;

        normalB = pts[2] - pts[0];
        normalB.rotateCCW();
        SkScalar dot = SkPoint::DotProduct(unitNormalAB, *unitNormalBC);
        SkAssertResult(normalB.setLength(SkScalarDiv(fRadius,
                                     SkScalarSqrt((SK_Scalar1 + dot)/2))));

        fOuter.quadTo(  pts[1].fX + normalB.fX, pts[1].fY + normalB.fY,
                        pts[2].fX + normalBC->fX, pts[2].fY + normalBC->fY);
        fInner.quadTo(  pts[1].fX - normalB.fX, pts[1].fY - normalB.fY,
                        pts[2].fX - normalBC->fX, pts[2].fY - normalBC->fY);
    }
}

void SkPathStroker::cubic_to(const SkPoint pts[4],
                      const SkVector& normalAB, const SkVector& unitNormalAB,
                      SkVector* normalCD, SkVector* unitNormalCD,
                      int subDivide) {
    SkVector    ab = pts[1] - pts[0];
    SkVector    cd = pts[3] - pts[2];
    SkVector    normalBC, unitNormalBC;

    bool    degenerateAB = degenerate_vector(ab);
    bool    degenerateCD = degenerate_vector(cd);

    if (degenerateAB && degenerateCD) {
DRAW_LINE:
        this->line_to(pts[3], normalAB);
        *normalCD = normalAB;
        *unitNormalCD = unitNormalAB;
        return;
    }

    if (degenerateAB) {
        ab = pts[2] - pts[0];
        degenerateAB = degenerate_vector(ab);
    }
    if (degenerateCD) {
        cd = pts[3] - pts[1];
        degenerateCD = degenerate_vector(cd);
    }
    if (degenerateAB || degenerateCD) {
        goto DRAW_LINE;
    }
    SkAssertResult(set_normal_unitnormal(cd, fRadius, normalCD, unitNormalCD));
    bool degenerateBC = !set_normal_unitnormal(pts[1], pts[2], fRadius,
                                               &normalBC, &unitNormalBC);
#ifndef SK_IGNORE_CUBIC_STROKE_FIX
    if (--subDivide < 0) {
        goto DRAW_LINE;
    }
#endif
    if (degenerateBC || normals_too_curvy(unitNormalAB, unitNormalBC) ||
             normals_too_curvy(unitNormalBC, *unitNormalCD)) {
#ifdef SK_IGNORE_CUBIC_STROKE_FIX
        // subdivide if we can
        if (--subDivide < 0) {
            goto DRAW_LINE;
        }
#endif
        SkPoint     tmp[7];
        SkVector    norm, unit, dummy, unitDummy;

        SkChopCubicAtHalf(pts, tmp);
        this->cubic_to(&tmp[0], normalAB, unitNormalAB, &norm, &unit,
                       subDivide);
        // we use dummys since we already have a valid (and more accurate)
        // normals for CD
        this->cubic_to(&tmp[3], norm, unit, &dummy, &unitDummy, subDivide);
    } else {
        SkVector    normalB, normalC;

        // need normals to inset/outset the off-curve pts B and C

        SkVector    unitBC = pts[2] - pts[1];
        unitBC.normalize();
        unitBC.rotateCCW();

        normalB = unitNormalAB + unitBC;
        normalC = *unitNormalCD + unitBC;

        SkScalar dot = SkPoint::DotProduct(unitNormalAB, unitBC);
        SkAssertResult(normalB.setLength(SkScalarDiv(fRadius,
                                    SkScalarSqrt((SK_Scalar1 + dot)/2))));
        dot = SkPoint::DotProduct(*unitNormalCD, unitBC);
        SkAssertResult(normalC.setLength(SkScalarDiv(fRadius,
                                    SkScalarSqrt((SK_Scalar1 + dot)/2))));

        fOuter.cubicTo( pts[1].fX + normalB.fX, pts[1].fY + normalB.fY,
                        pts[2].fX + normalC.fX, pts[2].fY + normalC.fY,
                        pts[3].fX + normalCD->fX, pts[3].fY + normalCD->fY);

        fInner.cubicTo( pts[1].fX - normalB.fX, pts[1].fY - normalB.fY,
                        pts[2].fX - normalC.fX, pts[2].fY - normalC.fY,
                        pts[3].fX - normalCD->fX, pts[3].fY - normalCD->fY);
    }
}

void SkPathStroker::quadTo(const SkPoint& pt1, const SkPoint& pt2) {
    bool    degenerateAB = SkPath::IsLineDegenerate(fPrevPt, pt1);
    bool    degenerateBC = SkPath::IsLineDegenerate(pt1, pt2);

    if (degenerateAB | degenerateBC) {
        if (degenerateAB ^ degenerateBC) {
            this->lineTo(pt2);
        }
        return;
    }

    SkVector    normalAB, unitAB, normalBC, unitBC;

    this->preJoinTo(pt1, &normalAB, &unitAB, false);

    {
        SkPoint pts[3], tmp[5];
        pts[0] = fPrevPt;
        pts[1] = pt1;
        pts[2] = pt2;

        if (SkChopQuadAtMaxCurvature(pts, tmp) == 2) {
            unitBC.setNormalize(pts[2].fX - pts[1].fX, pts[2].fY - pts[1].fY);
            unitBC.rotateCCW();
            if (normals_too_pinchy(unitAB, unitBC)) {
                normalBC = unitBC;
                normalBC.scale(fRadius);

                fOuter.lineTo(tmp[2].fX + normalAB.fX, tmp[2].fY + normalAB.fY);
                fOuter.lineTo(tmp[2].fX + normalBC.fX, tmp[2].fY + normalBC.fY);
                fOuter.lineTo(tmp[4].fX + normalBC.fX, tmp[4].fY + normalBC.fY);

                fInner.lineTo(tmp[2].fX - normalAB.fX, tmp[2].fY - normalAB.fY);
                fInner.lineTo(tmp[2].fX - normalBC.fX, tmp[2].fY - normalBC.fY);
                fInner.lineTo(tmp[4].fX - normalBC.fX, tmp[4].fY - normalBC.fY);

                fExtra.addCircle(tmp[2].fX, tmp[2].fY, fRadius,
                                 SkPath::kCW_Direction);
            } else {
                this->quad_to(&tmp[0], normalAB, unitAB, &normalBC, &unitBC,
                              kMaxQuadSubdivide);
                SkVector n = normalBC;
                SkVector u = unitBC;
                this->quad_to(&tmp[2], n, u, &normalBC, &unitBC,
                              kMaxQuadSubdivide);
            }
        } else {
            this->quad_to(pts, normalAB, unitAB, &normalBC, &unitBC,
                          kMaxQuadSubdivide);
        }
    }

    this->postJoinTo(pt2, normalBC, unitBC);
}

void SkPathStroker::cubicTo(const SkPoint& pt1, const SkPoint& pt2,
                            const SkPoint& pt3) {
    bool    degenerateAB = SkPath::IsLineDegenerate(fPrevPt, pt1);
    bool    degenerateBC = SkPath::IsLineDegenerate(pt1, pt2);
    bool    degenerateCD = SkPath::IsLineDegenerate(pt2, pt3);

    if (degenerateAB + degenerateBC + degenerateCD >= 2) {
        this->lineTo(pt3);
        return;
    }

    SkVector    normalAB, unitAB, normalCD, unitCD;

    // find the first tangent (which might be pt1 or pt2
    {
        const SkPoint*  nextPt = &pt1;
        if (degenerateAB)
            nextPt = &pt2;
        this->preJoinTo(*nextPt, &normalAB, &unitAB, false);
    }

    {
        SkPoint pts[4], tmp[13];
        int         i, count;
        SkVector    n, u;
        SkScalar    tValues[3];

        pts[0] = fPrevPt;
        pts[1] = pt1;
        pts[2] = pt2;
        pts[3] = pt3;

        count = SkChopCubicAtMaxCurvature(pts, tmp, tValues);
        n = normalAB;
        u = unitAB;
        for (i = 0; i < count; i++) {
            this->cubic_to(&tmp[i * 3], n, u, &normalCD, &unitCD,
                           kMaxCubicSubdivide);
            if (i == count - 1) {
                break;
            }
            n = normalCD;
            u = unitCD;

        }
    }

    this->postJoinTo(pt3, normalCD, unitCD);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "SkPaintDefaults.h"

SkStroke::SkStroke() {
    fWidth      = SK_Scalar1;
    fMiterLimit = SkPaintDefaults_MiterLimit;
    fCap        = SkPaint::kDefault_Cap;
    fJoin       = SkPaint::kDefault_Join;
    fDoFill     = false;
}

SkStroke::SkStroke(const SkPaint& p) {
    fWidth      = p.getStrokeWidth();
    fMiterLimit = p.getStrokeMiter();
    fCap        = (uint8_t)p.getStrokeCap();
    fJoin       = (uint8_t)p.getStrokeJoin();
    fDoFill     = SkToU8(p.getStyle() == SkPaint::kStrokeAndFill_Style);
}

SkStroke::SkStroke(const SkPaint& p, SkScalar width) {
    fWidth      = width;
    fMiterLimit = p.getStrokeMiter();
    fCap        = (uint8_t)p.getStrokeCap();
    fJoin       = (uint8_t)p.getStrokeJoin();
    fDoFill     = SkToU8(p.getStyle() == SkPaint::kStrokeAndFill_Style);
}

void SkStroke::setWidth(SkScalar width) {
    SkASSERT(width >= 0);
    fWidth = width;
}

void SkStroke::setMiterLimit(SkScalar miterLimit) {
    SkASSERT(miterLimit >= 0);
    fMiterLimit = miterLimit;
}

void SkStroke::setCap(SkPaint::Cap cap) {
    SkASSERT((unsigned)cap < SkPaint::kCapCount);
    fCap = SkToU8(cap);
}

void SkStroke::setJoin(SkPaint::Join join) {
    SkASSERT((unsigned)join < SkPaint::kJoinCount);
    fJoin = SkToU8(join);
}

///////////////////////////////////////////////////////////////////////////////

// If src==dst, then we use a tmp path to record the stroke, and then swap
// its contents with src when we're done.
class AutoTmpPath {
public:
    AutoTmpPath(const SkPath& src, SkPath** dst) : fSrc(src) {
        if (&src == *dst) {
            *dst = &fTmpDst;
            fSwapWithSrc = true;
        } else {
            (*dst)->reset();
            fSwapWithSrc = false;
        }
    }

    ~AutoTmpPath() {
        if (fSwapWithSrc) {
            fTmpDst.swap(*const_cast<SkPath*>(&fSrc));
        }
    }

private:
    SkPath          fTmpDst;
    const SkPath&   fSrc;
    bool            fSwapWithSrc;
};

void SkStroke::strokePath(const SkPath& src, SkPath* dst) const {
    SkASSERT(&src != NULL && dst != NULL);

    SkScalar radius = SkScalarHalf(fWidth);

    AutoTmpPath tmp(src, &dst);

    if (radius <= 0) {
        return;
    }

    // If src is really a rect, call our specialty strokeRect() method
    {
        bool isClosed;
        SkPath::Direction dir;
        if (src.isRect(&isClosed, &dir) && isClosed) {
            this->strokeRect(src.getBounds(), dst, dir);
            // our answer should preserve the inverseness of the src
            if (src.isInverseFillType()) {
                SkASSERT(!dst->isInverseFillType());
                dst->toggleInverseFillType();
            }
            return;
        }
    }

    SkAutoConicToQuads converter;
    const SkScalar conicTol = SK_Scalar1 / 4;

    SkPathStroker   stroker(src, radius, fMiterLimit, this->getCap(),
                            this->getJoin());
    SkPath::Iter    iter(src, false);
    SkPath::Verb    lastSegment = SkPath::kMove_Verb;

    for (;;) {
        SkPoint  pts[4];
        switch (iter.next(pts, false)) {
            case SkPath::kMove_Verb:
                stroker.moveTo(pts[0]);
                break;
            case SkPath::kLine_Verb:
                stroker.lineTo(pts[1]);
                lastSegment = SkPath::kLine_Verb;
                break;
            case SkPath::kQuad_Verb:
                stroker.quadTo(pts[1], pts[2]);
                lastSegment = SkPath::kQuad_Verb;
                break;
            case SkPath::kConic_Verb: {
                // todo: if we had maxcurvature for conics, perhaps we should
                // natively extrude the conic instead of converting to quads.
                const SkPoint* quadPts =
                    converter.computeQuads(pts, iter.conicWeight(), conicTol);
                for (int i = 0; i < converter.countQuads(); ++i) {
                    stroker.quadTo(quadPts[1], quadPts[2]);
                    quadPts += 2;
                }
                lastSegment = SkPath::kQuad_Verb;
            } break;
            case SkPath::kCubic_Verb:
                stroker.cubicTo(pts[1], pts[2], pts[3]);
                lastSegment = SkPath::kCubic_Verb;
                break;
            case SkPath::kClose_Verb:
                stroker.close(lastSegment == SkPath::kLine_Verb);
                break;
            case SkPath::kDone_Verb:
                goto DONE;
        }
    }
DONE:
    stroker.done(dst, lastSegment == SkPath::kLine_Verb);

    if (fDoFill) {
        if (src.cheapIsDirection(SkPath::kCCW_Direction)) {
            dst->reverseAddPath(src);
        } else {
            dst->addPath(src);
        }
    } else {
        //  Seems like we can assume that a 2-point src would always result in
        //  a convex stroke, but testing has proved otherwise.
        //  TODO: fix the stroker to make this assumption true (without making
        //  it slower that the work that will be done in computeConvexity())
#if 0
        // this test results in a non-convex stroke :(
        static void test(SkCanvas* canvas) {
            SkPoint pts[] = { 146.333328,  192.333328, 300.333344, 293.333344 };
            SkPaint paint;
            paint.setStrokeWidth(7);
            paint.setStrokeCap(SkPaint::kRound_Cap);
            canvas->drawLine(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, paint);
        }
#endif
#if 0
        if (2 == src.countPoints()) {
            dst->setIsConvex(true);
        }
#endif
    }

    // our answer should preserve the inverseness of the src
    if (src.isInverseFillType()) {
        SkASSERT(!dst->isInverseFillType());
        dst->toggleInverseFillType();
    }
}

static SkPath::Direction reverse_direction(SkPath::Direction dir) {
    SkASSERT(SkPath::kUnknown_Direction != dir);
    return SkPath::kCW_Direction == dir ? SkPath::kCCW_Direction : SkPath::kCW_Direction;
}

static void addBevel(SkPath* path, const SkRect& r, const SkRect& outer, SkPath::Direction dir) {
    SkPoint pts[8];

    if (SkPath::kCW_Direction == dir) {
        pts[0].set(r.fLeft, outer.fTop);
        pts[1].set(r.fRight, outer.fTop);
        pts[2].set(outer.fRight, r.fTop);
        pts[3].set(outer.fRight, r.fBottom);
        pts[4].set(r.fRight, outer.fBottom);
        pts[5].set(r.fLeft, outer.fBottom);
        pts[6].set(outer.fLeft, r.fBottom);
        pts[7].set(outer.fLeft, r.fTop);
    } else {
        pts[7].set(r.fLeft, outer.fTop);
        pts[6].set(r.fRight, outer.fTop);
        pts[5].set(outer.fRight, r.fTop);
        pts[4].set(outer.fRight, r.fBottom);
        pts[3].set(r.fRight, outer.fBottom);
        pts[2].set(r.fLeft, outer.fBottom);
        pts[1].set(outer.fLeft, r.fBottom);
        pts[0].set(outer.fLeft, r.fTop);
    }
    path->addPoly(pts, 8, true);
}

void SkStroke::strokeRect(const SkRect& origRect, SkPath* dst,
                          SkPath::Direction dir) const {
    SkASSERT(dst != NULL);
    dst->reset();

    SkScalar radius = SkScalarHalf(fWidth);
    if (radius <= 0) {
        return;
    }

    SkScalar rw = origRect.width();
    SkScalar rh = origRect.height();
    if ((rw < 0) ^ (rh < 0)) {
        dir = reverse_direction(dir);
    }
    SkRect rect(origRect);
    rect.sort();
    // reassign these, now that we know they'll be >= 0
    rw = rect.width();
    rh = rect.height();

    SkRect r(rect);
    r.outset(radius, radius);

    SkPaint::Join join = (SkPaint::Join)fJoin;
    if (SkPaint::kMiter_Join == join && fMiterLimit < SK_ScalarSqrt2) {
        join = SkPaint::kBevel_Join;
    }

    switch (join) {
        case SkPaint::kMiter_Join:
            dst->addRect(r, dir);
            break;
        case SkPaint::kBevel_Join:
            addBevel(dst, rect, r, dir);
            break;
        case SkPaint::kRound_Join:
            dst->addRoundRect(r, radius, radius, dir);
            break;
        default:
            break;
    }

    if (fWidth < SkMinScalar(rw, rh) && !fDoFill) {
        r = rect;
        r.inset(radius, radius);
        dst->addRect(r, reverse_direction(dir));
    }
}
