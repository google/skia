/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOpEdgeBuilder.h"
#include "SkReduceOrder.h"

void SkOpEdgeBuilder::init() {
    fCurrentContour = NULL;
    fOperand = false;
    fXorMask[0] = fXorMask[1] = (fPath->getFillType() & 1) ? kEvenOdd_PathOpsMask
            : kWinding_PathOpsMask;
#if DEBUG_DUMP
    gContourID = 0;
    gSegmentID = 0;
#endif
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

void SkOpEdgeBuilder::finish() {
    walk();
    complete();
    if (fCurrentContour && !fCurrentContour->segments().count()) {
        fContours.pop_back();
    }
    // correct pointers in contours since fReducePts may have moved as it grew
    int cIndex = 0;
    int extraCount = fExtra.count();
    SkASSERT(extraCount == 0 || fExtra[0] == -1);
    int eIndex = 0;
    int rIndex = 0;
    while (++eIndex < extraCount) {
        int offset = fExtra[eIndex];
        if (offset < 0) {
            ++cIndex;
            continue;
        }
        fCurrentContour = &fContours[cIndex];
        rIndex += fCurrentContour->updateSegment(offset - 1,
                &fReducePts[rIndex]);
    }
    fExtra.reset();  // we're done with this
}

// FIXME:remove once we can access path pts directly
int SkOpEdgeBuilder::preFetch() {
    SkPath::RawIter iter(*fPath);  // FIXME: access path directly when allowed
    SkPoint pts[4];
    SkPath::Verb verb;
    do {
        verb = iter.next(pts);
        *fPathVerbs.append() = verb;
        if (verb == SkPath::kMove_Verb) {
            *fPathPts.append() = pts[0];
        } else if (verb >= SkPath::kLine_Verb && verb <= SkPath::kCubic_Verb) {
            fPathPts.append(verb, &pts[1]);
        }
    } while (verb != SkPath::kDone_Verb);
    return fPathVerbs.count() - 1;
}

void SkOpEdgeBuilder::walk() {
    SkPath::Verb reducedVerb;
    uint8_t* verbPtr = fPathVerbs.begin();
    uint8_t* endOfFirstHalf = &verbPtr[fSecondHalf];
    const SkPoint* pointsPtr = fPathPts.begin();
    const SkPoint* finalCurveStart = NULL;
    const SkPoint* finalCurveEnd = NULL;
    SkPath::Verb verb;
    while ((verb = (SkPath::Verb) *verbPtr++) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
                complete();
                if (!fCurrentContour) {
                    fCurrentContour = fContours.push_back_n(1);
                    fCurrentContour->setOperand(fOperand);
                    fCurrentContour->setXor(fXorMask[fOperand] == kEvenOdd_PathOpsMask);
                    *fExtra.append() = -1;  // start new contour
                }
                finalCurveEnd = pointsPtr++;
                goto nextVerb;
            case SkPath::kLine_Verb:
                // skip degenerate points
                if (pointsPtr[-1].fX != pointsPtr[0].fX || pointsPtr[-1].fY != pointsPtr[0].fY) {
                    fCurrentContour->addLine(&pointsPtr[-1]);
                }
                break;
            case SkPath::kQuad_Verb:
                reducedVerb = SkReduceOrder::Quad(&pointsPtr[-1], &fReducePts);
                if (reducedVerb == 0) {
                    break;  // skip degenerate points
                }
                if (reducedVerb == 1) {
                    *fExtra.append() =
                            fCurrentContour->addLine(fReducePts.end() - 2);
                    break;
                }
                fCurrentContour->addQuad(&pointsPtr[-1]);
                break;
            case SkPath::kCubic_Verb:
                reducedVerb = SkReduceOrder::Cubic(&pointsPtr[-1], &fReducePts);
                if (reducedVerb == 0) {
                    break;  // skip degenerate points
                }
                if (reducedVerb == 1) {
                    *fExtra.append() = fCurrentContour->addLine(fReducePts.end() - 2);
                    break;
                }
                if (reducedVerb == 2) {
                    *fExtra.append() = fCurrentContour->addQuad(fReducePts.end() - 3);
                    break;
                }
                fCurrentContour->addCubic(&pointsPtr[-1]);
                break;
            case SkPath::kClose_Verb:
                SkASSERT(fCurrentContour);
                if (finalCurveStart && finalCurveEnd
                        && *finalCurveStart != *finalCurveEnd) {
                    *fReducePts.append() = *finalCurveStart;
                    *fReducePts.append() = *finalCurveEnd;
                    *fExtra.append() = fCurrentContour->addLine(fReducePts.end() - 2);
                }
                complete();
                goto nextVerb;
            default:
                SkDEBUGFAIL("bad verb");
                return;
        }
        finalCurveStart = &pointsPtr[verb - 1];
        pointsPtr += verb;
        SkASSERT(fCurrentContour);
    nextVerb:
        if (verbPtr == endOfFirstHalf) {
            fOperand = true;
        }
    }
}
