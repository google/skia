/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkGeometry.h"
#include "SkOpEdgeBuilder.h"
#include "SkReduceOrder.h"

void SkOpEdgeBuilder::init() {
    fCurrentContour = NULL;
    fOperand = false;
    fXorMask[0] = fXorMask[1] = (fPath->getFillType() & 1) ? kEvenOdd_PathOpsMask
            : kWinding_PathOpsMask;
    fUnparseable = false;
    fSecondHalf = preFetch();
}

void SkOpEdgeBuilder::addOperand(const SkPath& path) {
    SkASSERT(fPathVerbs.count() > 0 && fPathVerbs.end()[-1] == SkPath::kDone_Verb);
    fPathVerbs.pop_back();
    fPath = &path;
    fXorMask[1] = (fPath->getFillType() & 1) ? kEvenOdd_PathOpsMask
            : kWinding_PathOpsMask;
    preFetch();
}

bool SkOpEdgeBuilder::finish() {
    if (fUnparseable || !walk()) {
        return false;
    }
    complete();
    if (fCurrentContour && !fCurrentContour->segments().count()) {
        fContours.pop_back();
    }
    return true;
}

void SkOpEdgeBuilder::closeContour(const SkPoint& curveEnd, const SkPoint& curveStart) {
    if (!SkDPoint::ApproximatelyEqual(curveEnd, curveStart)) {
        fPathVerbs.push_back(SkPath::kLine_Verb);
        fPathPts.push_back_n(1, &curveStart);
    } else {
        fPathPts[fPathPts.count() - 1] = curveStart;
    }
    fPathVerbs.push_back(SkPath::kClose_Verb);
}

// very tiny points cause numerical instability : don't allow them
static void force_small_to_zero(SkPoint* pt) {
    if (SkScalarAbs(pt->fX) < FLT_EPSILON_ORDERABLE_ERR) {
        pt->fX = 0;
    }
    if (SkScalarAbs(pt->fY) < FLT_EPSILON_ORDERABLE_ERR) {
        pt->fY = 0;
    }
}


int SkOpEdgeBuilder::preFetch() {
    if (!fPath->isFinite()) {
        fUnparseable = true;
        return 0;
    }
    SkAutoConicToQuads quadder;
    const SkScalar quadderTol = SK_Scalar1 / 16;
    SkPath::RawIter iter(*fPath);
    SkPoint curveStart;
    SkPoint curve[4];
    SkPoint pts[4];
    SkPath::Verb verb;
    bool lastCurve = false;
    do {
        verb = iter.next(pts);
        switch (verb) {
            case SkPath::kMove_Verb:
                if (!fAllowOpenContours && lastCurve) {
                    closeContour(curve[0], curveStart);
                }
                fPathVerbs.push_back(verb);
                force_small_to_zero(&pts[0]);
                fPathPts.push_back(pts[0]);
                curveStart = curve[0] = pts[0];
                lastCurve = false;
                continue;
            case SkPath::kLine_Verb:
                force_small_to_zero(&pts[1]);
                if (SkDPoint::ApproximatelyEqual(curve[0], pts[1])) {
                    uint8_t lastVerb = fPathVerbs.back();
                    if (lastVerb != SkPath::kLine_Verb && lastVerb != SkPath::kMove_Verb) {
                        fPathPts.back() = pts[1];
                    }
                    continue;  // skip degenerate points
                }
                break;
            case SkPath::kQuad_Verb:
                force_small_to_zero(&pts[1]);
                force_small_to_zero(&pts[2]);
                curve[1] = pts[1];
                curve[2] = pts[2];
                verb = SkReduceOrder::Quad(curve, pts);
                if (verb == SkPath::kMove_Verb) {
                    continue;  // skip degenerate points
                }
                break;
            case SkPath::kConic_Verb: {
                    const SkPoint* quadPts = quadder.computeQuads(pts, iter.conicWeight(),
                            quadderTol);
                    const int nQuads = quadder.countQuads();
                    for (int i = 0; i < nQuads; ++i) {
                       fPathVerbs.push_back(SkPath::kQuad_Verb);
                    }
                    fPathPts.push_back_n(nQuads * 2, quadPts);
                    curve[0] = quadPts[nQuads * 2 - 1];
                    lastCurve = true;
                }
                continue;
            case SkPath::kCubic_Verb:
                force_small_to_zero(&pts[1]);
                force_small_to_zero(&pts[2]);
                force_small_to_zero(&pts[3]);
                curve[1] = pts[1];
                curve[2] = pts[2];
                curve[3] = pts[3];
                verb = SkReduceOrder::Cubic(curve, pts);
                if (verb == SkPath::kMove_Verb) {
                    continue;  // skip degenerate points
                }
                break;
            case SkPath::kClose_Verb:
                closeContour(curve[0], curveStart);
                lastCurve = false;
                continue;
            case SkPath::kDone_Verb:
                continue;
        }
        fPathVerbs.push_back(verb);
        int ptCount = SkPathOpsVerbToPoints(verb);
        fPathPts.push_back_n(ptCount, &pts[1]);
        curve[0] = pts[ptCount];
        lastCurve = true;
    } while (verb != SkPath::kDone_Verb);
    if (!fAllowOpenContours && lastCurve) {
        closeContour(curve[0], curveStart);
    }
    fPathVerbs.push_back(SkPath::kDone_Verb);
    return fPathVerbs.count() - 1;
}

bool SkOpEdgeBuilder::close() {
    complete();
    return true;
}

bool SkOpEdgeBuilder::walk() {
    uint8_t* verbPtr = fPathVerbs.begin();
    uint8_t* endOfFirstHalf = &verbPtr[fSecondHalf];
    const SkPoint* pointsPtr = fPathPts.begin() - 1;
    SkPath::Verb verb;
    while ((verb = (SkPath::Verb) *verbPtr) != SkPath::kDone_Verb) {
        if (verbPtr == endOfFirstHalf) {
            fOperand = true;
        }
        verbPtr++;
        switch (verb) {
            case SkPath::kMove_Verb:
                if (fCurrentContour) {
                    if (fAllowOpenContours) {
                        complete();
                    } else if (!close()) {
                        return false;
                    }
                }
                if (!fCurrentContour) {
                    fCurrentContour = fContours.push_back_n(1);
                    fCurrentContour->setOperand(fOperand);
                    fCurrentContour->setXor(fXorMask[fOperand] == kEvenOdd_PathOpsMask);
                }
                pointsPtr += 1;
                continue;
            case SkPath::kLine_Verb:
                fCurrentContour->addLine(pointsPtr);
                break;
            case SkPath::kQuad_Verb:
                fCurrentContour->addQuad(pointsPtr);
                break;
            case SkPath::kCubic_Verb:
                fCurrentContour->addCubic(pointsPtr);
                break;
            case SkPath::kClose_Verb:
                SkASSERT(fCurrentContour);
                if (!close()) {
                    return false;
                }
                continue;
            default:
                SkDEBUGFAIL("bad verb");
                return false;
        }
        pointsPtr += SkPathOpsVerbToPoints(verb);
        SkASSERT(fCurrentContour);
    }
   if (fCurrentContour && !fAllowOpenContours && !close()) {
       return false;
   }
   return true;
}
