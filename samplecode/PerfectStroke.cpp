/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PerfectStroke.h"
#include "SkGeometry.h"
#include "SkIntersections.h"
#include "SkOpEdgeBuilder.h"
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
                SkQuadConstruct quadConstruct;
                // get next tangent
                pathStroker.init(strokeType, &quadConstruct, 0, 1);
                pathStroker.quadStroke(pts, &quadConstruct);
                StrokeData& strokeData = SkPathStroker::kInner_StrokeType == strokeType ?
                        curPtData->fInner : curPtData->fOuter;
                strokeData.fPath = pathStroker.outer().isEmpty() ? pathStroker.inner() :
                        pathStroker.outer();
            }
        }
 //       start here;
        // always can use tangent (line) bisect calc for outer
        // for inner and curves, need to intersect segment pair
        for (int ptIndex = 0; ptIndex < fCurveData.count(); ++ptIndex) {
            CurveData* curPtData = &fCurveData[ptIndex];
            if (SkPath::kLine_Verb == curPtData->fPrev->fVerb &&
                    SkPath::kLine_Verb == curPtData->fVerb) {
                curPtData->calcLineBisects(width);
                continue;
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
