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

bool SkOpEdgeBuilder::finish() {
    if (fUnparseable || !walk()) {
        return false;
    }
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
    return true;
}

// Note that copying the points here avoids copying the resulting path later.
// To allow Op() to take one of the input paths as an output parameter, either the source data
// must be copied (as implemented below) or the result must be copied.
// OPTIMIZATION: This copies both sets of input points every time. If the input data was read
// directly, the output path would only need to be copied if it was also one of the input paths.
int SkOpEdgeBuilder::preFetch() {
    if (!fPath->isFinite()) {
        fUnparseable = true;
        return 0;
    }
    SkPath::RawIter iter(*fPath);
    SkPoint pts[4];
    SkPath::Verb verb;
    do {
        verb = iter.next(pts);
        *fPathVerbs.append() = verb;
        if (verb == SkPath::kMove_Verb) {
            *fPathPts.append() = pts[0];
        } else if (verb >= SkPath::kLine_Verb && verb <= SkPath::kCubic_Verb) {
            fPathPts.append(SkPathOpsVerbToPoints(verb), &pts[1]);
        }
    } while (verb != SkPath::kDone_Verb);
    return fPathVerbs.count() - 1;
}

bool SkOpEdgeBuilder::close() {
    if (fFinalCurveStart && fFinalCurveEnd && *fFinalCurveStart != *fFinalCurveEnd) {
        *fReducePts.append() = *fFinalCurveStart;
        *fReducePts.append() = *fFinalCurveEnd;
        const SkPoint* lineStart = fReducePts.end() - 2;
        *fExtra.append() = fCurrentContour->addLine(lineStart);
    }
    complete();
    return true;
}

bool SkOpEdgeBuilder::walk() {
    SkPath::Verb reducedVerb;
    uint8_t* verbPtr = fPathVerbs.begin();
    uint8_t* endOfFirstHalf = &verbPtr[fSecondHalf];
    const SkPoint* pointsPtr = fPathPts.begin();
    SkPath::Verb verb;
    fFinalCurveStart = NULL;
    fFinalCurveEnd = NULL;
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
                    *fExtra.append() = -1;  // start new contour
                }
                fFinalCurveEnd = pointsPtr++;
                continue;
            case SkPath::kLine_Verb: {
                const SkPoint& lineEnd = pointsPtr[0];
                const SkPoint& lineStart = pointsPtr[-1];
                // skip degenerate points
                if (lineStart.fX != lineEnd.fX || lineStart.fY != lineEnd.fY) {
                    fCurrentContour->addLine(&lineStart);
                }
                } break;
            case SkPath::kQuad_Verb: {
                const SkPoint* quadStart = &pointsPtr[-1];
                reducedVerb = SkReduceOrder::Quad(quadStart, &fReducePts);
                if (reducedVerb == 0) {
                    break;  // skip degenerate points
                }
                if (reducedVerb == SkPath::kLine_Verb) {
                    const SkPoint* lineStart = fReducePts.end() - 2;
                   *fExtra.append() = fCurrentContour->addLine(lineStart);
                    break;
                }
                fCurrentContour->addQuad(quadStart);
                } break;
            case SkPath::kCubic_Verb: {
                const SkPoint* cubicStart = &pointsPtr[-1];
                reducedVerb = SkReduceOrder::Cubic(cubicStart, &fReducePts);
                if (reducedVerb == 0) {
                    break;  // skip degenerate points
                }
                if (reducedVerb == SkPath::kLine_Verb) {
                    const SkPoint* lineStart = fReducePts.end() - 2;
                    *fExtra.append() = fCurrentContour->addLine(lineStart);
                    break;
                }
                if (reducedVerb == SkPath::kQuad_Verb) {
                    const SkPoint* quadStart = fReducePts.end() - 3;
                    *fExtra.append() = fCurrentContour->addQuad(quadStart);
                    break;
                }
                fCurrentContour->addCubic(cubicStart);
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
        fFinalCurveStart = &pointsPtr[SkPathOpsVerbToPoints(verb) - 1];
        pointsPtr += SkPathOpsVerbToPoints(verb);
        SkASSERT(fCurrentContour);
    }
   if (fCurrentContour && !fAllowOpenContours && !close()) {
       return false;
   }
   return true;
}
