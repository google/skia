/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PerfectStroke.h"
#include "SkGeometry.h"
#include "SkAddIntersections.h"
#include "SkArenaAlloc.h"
#include "SkIntersections.h"
#include "SkOpCoincidence.h"
#include "SkPathOpsCommon.h"
#include "SkOpEdgeBuilder.h"
#include "SkPathOpsDebug.h"
#include "SkPointPriv.h"
#include "SkStroke.h"
#include "SkStrokeRec.h"

static inline bool degenerate_vector(const SkVector& v) {
    return !SkPointPriv::CanNormalize(v.fX, v.fY);
}

// to create an overlap-free stroke, it is necessary to intersect every inner and outer stroke
// with all of the other inner and outer strokes. Any one could intersect any other.
// it's possible to special case consecutive straight lines with fixed width, but even then, the
// lines may intersect other inner and outer strokes later.

// other than rectangles (and regular convex polygons) the hard work needs to be done, and
// rectangles as well if the width is variable. in this experiment, create inner and outer
// paths for everything, including lines. Given all inner and outer paths, and their bounds,
// intersect everything with everything. To keep this marginally bounded, also compute the bounds
// of pairs of consecutive stroke edges so that more gross rejection is possible


CurveData* StrokeData::curveData() {
    size_t strokeDataOffset = StrokeType::kInner == fStrokeType ?
            offsetof(CurveData, fInner) : offsetof(CurveData, fOuter);
    return (CurveData* ) ((char*) this - strokeDataOffset);
}

    void StrokePath::setPointArray(const SkPath& path, SkScalar width) {
        fPath = path;
        int verbCount = path.countVerbs();
        if (!verbCount) {
            return;
        }
        fCurveData = SkTArray<CurveData>(verbCount);
        SkPath::Iter iter(path, false);
        int ptIndex = 0;
        CurveData* curCurve = &fCurveData.push_back();
        bool degenerate;
        CurveData* firstCurve = nullptr;
        CurveData* prevCurve = nullptr;
        while ((curCurve->fVerb = iter.next(curCurve->fPts)) != SkPath::kDone_Verb) {
            int ptCount = 3;
            switch (curCurve->fVerb) {
            case SkPath::kMove_Verb:
                firstCurve = nullptr;
                break;
            case SkPath::kClose_Verb:
                if (!firstCurve) {
                    break;
                }
                SkASSERT(prevCurve);
                prevCurve->fPos = CurvePos::kLast;
                firstCurve->fPrev = prevCurve;
                break;
            case SkPath::kLine_Verb:
                ptCount -= 1;
            case SkPath::kQuad_Verb:
            case SkPath::kConic_Verb:
                ptCount -= 1;
                if (SkPath::kConic_Verb == curCurve->fVerb) {
                    curCurve->fWeight = iter.conicWeight();
                }
            case SkPath::kCubic_Verb:
                for (int index = 1; index <= ptCount; ++index) {
                    curCurve->fTangents[0] = curCurve->fPts[index] - curCurve->fPts[0];
                    if (!degenerate_vector(curCurve->fTangents[0])) {
                        break;
                    }
                }
                degenerate = !curCurve->fTangents[0].setLength(width);
                if (SkPath::kLine_Verb == curCurve->fVerb) {
                    curCurve->fTangents[1] = -curCurve->fTangents[0];
                } else {
                    for (int index = ptCount - 1; index >= 0; --index) {
                        curCurve->fTangents[1] = curCurve->fPts[ptCount] - curCurve->fPts[index];
                        if (!degenerate_vector(curCurve->fTangents[1])) {
                            break;
                        }
                    }
                    curCurve->fTangents[1].setLength(width);
                }
                if (!degenerate) {
                    curCurve->fPrev = prevCurve;
                    if (!firstCurve) {
                        curCurve->fPos = CurvePos::kFirst;
                        firstCurve = curCurve;
                    } else {
                        curCurve->fPos = CurvePos::kMid;
                    }
                    prevCurve = curCurve;
                    curCurve = &fCurveData.push_back();
                }
                ptIndex += ptCount;
                break;
            }
        }
        if (!fCurveData.count()) {
            return;
        }
        SkASSERT(SkPath::kDone_Verb == curCurve->fVerb);
        SkASSERT(curCurve == &fCurveData.back());
        fCurveData.pop_back();
    }

    // given a pair of paths, find their intersections
    // trim the first part up to the first intersection
    // trim out every other pair afterwards
    // if an intersection is the start of coincidence or nearly so, tread carefully
        // to avoid trimming out the wrong pair accidentally,
        // maybe check midway to see if either side is in the expected direction
        // relative to the midpoint
    bool intersect_paths(const SkPath& path1, const SkPath& path2,
            SkPath* trim1, SkPath* trim2) {

        return false;
    }

    // call when both are not lines, for both inner paths
    // given a path pair, the end of 1 extends past the start of 2
    // find the intersection point, trim both paths to common point
    bool StrokePath::intersectEnds() {
        // construct segments from each path
        SkSTArenaAlloc<4096> allocator;  // FIXME: constant-ize, tune
        SkOpContourHead* contourList = static_cast<SkOpContourHead*>(&fCurveData[0].fContour);
        SkDEBUGCODE(bool skipAssert = false);
        SkDEBUGCODE(const char* testName = nullptr);
        SkOpGlobalState globalState(contourList, &allocator
                SkDEBUGPARAMS(skipAssert) SkDEBUGPARAMS(testName));
        SkOpEdgeBuilder builder(fCurveData[0].fInner.fPath, contourList, &globalState,
                SkOpEdgeBuilder::kOpen);
        for (int ptIndex = 1; ptIndex < fCurveData.count(); ++ptIndex) {
            CurveData* curPtData = &fCurveData[ptIndex];
            builder.addOperand(curPtData->fInner.fPath);
        }
        if (!builder.finish()) {
            return false;
        }
        SkOpCoincidence coincidence(&globalState);
        AddIntersectTs(&fCurveData[0].fContour, &fCurveData[0].fContour, &coincidence);
        fCurveData[0].fContour.dump();
        // remove the start of curve1, and the end of curve2

        bool success = HandleCoincidence(contourList, &coincidence);

        return success;
    }

    // given a pair of inset paths (or outset paths)
    // find the intersection point(s)
    // discard the parts that intersect : to intersect 1, then discard from i 2 to i 3, etc.
    void StrokePath::calcBisects(SkScalar width) {
        SkPathStroker pathStroker(fPath, width, 4, SkPaint::kButt_Cap, SkPaint::kMiter_Join,
                1, true);
        for (int ptIndex = 0; ptIndex < fCurveData.count(); ++ptIndex) {
            CurveData* curPtData = &fCurveData[ptIndex];
            const SkPoint* pts = curPtData->fPts;
            // always compute the inner and outer strokes
            // may be worth optimizing the code for computing lines in the future
            // since the code to compute the tangents and bisect may be all we need
            for (SkPathStroker::StrokeType strokeType : { SkPathStroker::kInner_StrokeType,
                    SkPathStroker::kOuter_StrokeType } ) {
                StrokeData& strokeData = SkPathStroker::kInner_StrokeType == strokeType ?
                        curPtData->fInner : curPtData->fOuter;
                SkVector* tangents = curPtData->fTangents;
                SkVector normal = {-tangents[0].fY, tangents[0].fX};
                if (SkPathStroker::kOuter_StrokeType == strokeType) {
                    normal = -normal;
                }
                switch (curPtData->fVerb) {
                    case SkPath::kLine_Verb:
                        strokeData.fPath.moveTo(pts[0] + normal);
                        strokeData.fPath.lineTo(pts[1] + normal);
                        break;
                    case SkPath::kQuad_Verb: {
                        SkQuadConstruct quadConstruct;
                        // get next tangent
                        pathStroker.init(strokeType, &quadConstruct, 0, 1);
                        SkPath* path = SkPathStroker::kInner_StrokeType == strokeType ?
                                pathStroker.inner() : pathStroker.outer();
                        path->moveTo(pts[0] + normal);
                        pathStroker.quadStroke(pts, &quadConstruct);
                        } break;
                    default:
                        // unimplemented
                        SkASSERT(0);
                }
                if (SkPath::kLine_Verb != curPtData->fVerb) {
                    strokeData.fPath = pathStroker.outer()->isEmpty() ? *pathStroker.inner() :
                            *pathStroker.outer();
                }
            }
        }
        this->intersectEnds();
        // collect inner and outer paths into bounds trees
            // collect every pair in contour iteratively until contour bounds is collected
            // collect inner and outer into one pair
            // contour bounds in pairs until path is collected

        // always can use tangent (line) bisect calc for outer
        // for inner and curves, need to intersect segment pair
        for (int ptIndex = 0; ptIndex < fCurveData.count(); ++ptIndex) {
            CurveData* curPtData = &fCurveData[ptIndex];
            if (SkPath::kLine_Verb == curPtData->fPrev->fVerb &&
                    SkPath::kLine_Verb == curPtData->fVerb) {
                curPtData->calcLineBisects(width);
            }


            // note that the inner of one curve can intersect the outer of the next
            // even if the curves don't overlap

            // in general, need to keep bounds of everything, and find intersections
            // with any path with bounds that intersect any other path
            // can do some special casing with lines with const width, but not variable width
            // may also benefit from bounds of runs
            // build up binary search tree of bounds?

        }
    }
