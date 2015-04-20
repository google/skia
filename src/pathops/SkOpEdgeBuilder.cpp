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
    fCurrentContour = fContoursHead;
    fOperand = false;
    fXorMask[0] = fXorMask[1] = (fPath->getFillType() & 1) ? kEvenOdd_PathOpsMask
            : kWinding_PathOpsMask;
    fUnparseable = false;
    fSecondHalf = preFetch();
}

void SkOpEdgeBuilder::addOperand(const SkPath& path) {
    SkASSERT(fPathVerbs.count() > 0 && fPathVerbs.end()[-1] == SkPath::kDone_Verb);
    fPathVerbs.pop();
    fPath = &path;
    fXorMask[1] = (fPath->getFillType() & 1) ? kEvenOdd_PathOpsMask
            : kWinding_PathOpsMask;
    preFetch();
}

int SkOpEdgeBuilder::count() const {
    SkOpContour* contour = fContoursHead;
    int count = 0;
    while (contour) {
        count += contour->count() > 0;
        contour = contour->next();
    }
    return count;
}

bool SkOpEdgeBuilder::finish(SkChunkAlloc* allocator) {
    fOperand = false;
    if (fUnparseable || !walk(allocator)) {
        return false;
    }
    complete();
    if (fCurrentContour && !fCurrentContour->count()) {
        fContoursHead->remove(fCurrentContour);
    }
    return true;
}

void SkOpEdgeBuilder::closeContour(const SkPoint& curveEnd, const SkPoint& curveStart) {
    if (!SkDPoint::ApproximatelyEqual(curveEnd, curveStart)) {
        *fPathVerbs.append() = SkPath::kLine_Verb;
        *fPathPts.append() = curveStart;
    } else {
        fPathPts[fPathPts.count() - 1] = curveStart;
    }
    *fPathVerbs.append() = SkPath::kClose_Verb;
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
                *fPathVerbs.append() = verb;
                force_small_to_zero(&pts[0]);
                *fPathPts.append() = pts[0];
                curveStart = curve[0] = pts[0];
                lastCurve = false;
                continue;
            case SkPath::kLine_Verb:
                force_small_to_zero(&pts[1]);
                if (SkDPoint::ApproximatelyEqual(curve[0], pts[1])) {
                    uint8_t lastVerb = fPathVerbs.top();
                    if (lastVerb != SkPath::kLine_Verb && lastVerb != SkPath::kMove_Verb) {
                        fPathPts.top() = pts[1];
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
            case SkPath::kConic_Verb:
                force_small_to_zero(&pts[1]);
                force_small_to_zero(&pts[2]);
                curve[1] = pts[1];
                curve[2] = pts[2];
                verb = SkReduceOrder::Conic(curve, iter.conicWeight(), pts);
                if (verb == SkPath::kMove_Verb) {
                    continue;  // skip degenerate points
                }
                break;
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
        *fPathVerbs.append() = verb;
        int ptCount = SkPathOpsVerbToPoints(verb);
        fPathPts.append(ptCount, &pts[1]);
        if (verb == SkPath::kConic_Verb) {
            *fWeights.append() = iter.conicWeight();
        }
        curve[0] = pts[ptCount];
        lastCurve = true;
    } while (verb != SkPath::kDone_Verb);
    if (!fAllowOpenContours && lastCurve) {
        closeContour(curve[0], curveStart);
    }
    *fPathVerbs.append() = SkPath::kDone_Verb;
    return fPathVerbs.count() - 1;
}

bool SkOpEdgeBuilder::close() {
    complete();
    return true;
}

bool SkOpEdgeBuilder::walk(SkChunkAlloc* allocator) {
    uint8_t* verbPtr = fPathVerbs.begin();
    uint8_t* endOfFirstHalf = &verbPtr[fSecondHalf];
    SkPoint* pointsPtr = fPathPts.begin() - 1;
    SkScalar* weightPtr = fWeights.begin();
    SkPath::Verb verb;
    while ((verb = (SkPath::Verb) *verbPtr) != SkPath::kDone_Verb) {
        if (verbPtr == endOfFirstHalf) {
            fOperand = true;
        }
        verbPtr++;
        switch (verb) {
            case SkPath::kMove_Verb:
                if (fCurrentContour && fCurrentContour->count()) {
                    if (fAllowOpenContours) {
                        complete();
                    } else if (!close()) {
                        return false;
                    }
                }
                if (!fCurrentContour) {
                    fCurrentContour = fContoursHead->appendContour(allocator);
                }
                fCurrentContour->init(fGlobalState, fOperand,
                    fXorMask[fOperand] == kEvenOdd_PathOpsMask);
                pointsPtr += 1;
                continue;
            case SkPath::kLine_Verb:
                fCurrentContour->addLine(pointsPtr, fAllocator);
                break;
            case SkPath::kQuad_Verb:
                fCurrentContour->addQuad(pointsPtr, fAllocator);
                break;
            case SkPath::kConic_Verb:
                fCurrentContour->addConic(pointsPtr, *weightPtr++, fAllocator);
                break;
            case SkPath::kCubic_Verb: {
                // split self-intersecting cubics in two before proceeding
                // if the cubic is convex, it doesn't self intersect.
                SkScalar loopT;
                if (SkDCubic::ComplexBreak(pointsPtr, &loopT)) {
                    SkPoint cubicPair[7]; 
                    SkChopCubicAt(pointsPtr, cubicPair, loopT);
                    if (!SkScalarsAreFinite(&cubicPair[0].fX, SK_ARRAY_COUNT(cubicPair) * 2)) {
                        return false;
                    }
                    SkPoint cStorage[2][4];
                    SkPath::Verb v1 = SkReduceOrder::Cubic(&cubicPair[0], cStorage[0]);
                    SkPath::Verb v2 = SkReduceOrder::Cubic(&cubicPair[3], cStorage[1]);
                    if (v1 != SkPath::kMove_Verb && v2 != SkPath::kMove_Verb) {
                        SkPoint* curve1 = v1 == SkPath::kCubic_Verb ? &cubicPair[0] : cStorage[0];
                        SkPoint* curve2 = v2 == SkPath::kCubic_Verb ? &cubicPair[3] : cStorage[1];
                        for (int index = 0; index < SkPathOpsVerbToPoints(v1); ++index) {
                            force_small_to_zero(&curve1[index]);
                        }
                        for (int index = 0; index < SkPathOpsVerbToPoints(v2); ++index) {
                            force_small_to_zero(&curve2[index]);
                        }
                        fCurrentContour->addCurve(v1, curve1, fAllocator);
                        fCurrentContour->addCurve(v2, curve2, fAllocator);
                    } else {
                        fCurrentContour->addCubic(pointsPtr, fAllocator);
                    }
                } else {
                    fCurrentContour->addCubic(pointsPtr, fAllocator);
                }
                } break;
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
        SkASSERT(fCurrentContour);
        fCurrentContour->debugValidate();
        pointsPtr += SkPathOpsVerbToPoints(verb);
    }
   if (fCurrentContour && fCurrentContour->count() &&!fAllowOpenContours && !close()) {
       return false;
   }
   return true;
}
