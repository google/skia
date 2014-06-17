/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkAddIntersections.h"
#include "SkPathOpsBounds.h"

#if DEBUG_ADD_INTERSECTING_TS

static void debugShowLineIntersection(int pts, const SkIntersectionHelper& wt,
                                      const SkIntersectionHelper& wn, const SkIntersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " LINE_DEBUG_STR " " LINE_DEBUG_STR "\n",
                __FUNCTION__, LINE_DEBUG_DATA(wt.pts()), LINE_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " LINE_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i[0][0], LINE_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    if (pts == 2) {
        SkDebugf(" " T_DEBUG_STR(wtTs, 1) " " PT_DEBUG_STR, i[0][1], PT_DEBUG_DATA(i, 1));
    }
    SkDebugf(" wnTs[0]=%g " LINE_DEBUG_STR, i[1][0], LINE_DEBUG_DATA(wn.pts()));
    if (pts == 2) {
        SkDebugf(" " T_DEBUG_STR(wnTs, 1), i[1][1]);
    }
    SkDebugf("\n");
}

static void debugShowQuadLineIntersection(int pts, const SkIntersectionHelper& wt,
                                          const SkIntersectionHelper& wn,
                                          const SkIntersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " QUAD_DEBUG_STR " " LINE_DEBUG_STR "\n",
                __FUNCTION__, QUAD_DEBUG_DATA(wt.pts()), LINE_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " QUAD_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i[0][0], QUAD_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " LINE_DEBUG_STR, i[1][0], LINE_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowQuadIntersection(int pts, const SkIntersectionHelper& wt,
        const SkIntersectionHelper& wn, const SkIntersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " QUAD_DEBUG_STR " " QUAD_DEBUG_STR "\n",
                __FUNCTION__, QUAD_DEBUG_DATA(wt.pts()), QUAD_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " QUAD_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i[0][0], QUAD_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " QUAD_DEBUG_STR, i[1][0], QUAD_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicLineIntersection(int pts, const SkIntersectionHelper& wt,
        const SkIntersectionHelper& wn, const SkIntersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " CUBIC_DEBUG_STR " " LINE_DEBUG_STR "\n",
                __FUNCTION__, CUBIC_DEBUG_DATA(wt.pts()), LINE_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " LINE_DEBUG_STR, i[1][0], LINE_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicQuadIntersection(int pts, const SkIntersectionHelper& wt,
        const SkIntersectionHelper& wn, const SkIntersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " CUBIC_DEBUG_STR " " QUAD_DEBUG_STR "\n",
                __FUNCTION__, CUBIC_DEBUG_DATA(wt.pts()), QUAD_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " QUAD_DEBUG_STR, i[1][0], QUAD_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicIntersection(int pts, const SkIntersectionHelper& wt,
        const SkIntersectionHelper& wn, const SkIntersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no intersect " CUBIC_DEBUG_STR " " CUBIC_DEBUG_STR "\n",
                __FUNCTION__, CUBIC_DEBUG_DATA(wt.pts()), CUBIC_DEBUG_DATA(wn.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wtTs) " " PT_DEBUG_STR, n, i[0][n], PT_DEBUG_DATA(i, n));
    }
    SkDebugf(" wnTs[0]=%g " CUBIC_DEBUG_STR, i[1][0], CUBIC_DEBUG_DATA(wn.pts()));
    for (int n = 1; n < pts; ++n) {
        SkDebugf(" " TX_DEBUG_STR(wnTs), n, i[1][n]);
    }
    SkDebugf("\n");
}

static void debugShowCubicIntersection(int pts, const SkIntersectionHelper& wt,
        const SkIntersections& i) {
    SkASSERT(i.used() == pts);
    if (!pts) {
        SkDebugf("%s no self intersect " CUBIC_DEBUG_STR "\n", __FUNCTION__,
                CUBIC_DEBUG_DATA(wt.pts()));
        return;
    }
    SkDebugf("%s " T_DEBUG_STR(wtTs, 0) " " CUBIC_DEBUG_STR " " PT_DEBUG_STR, __FUNCTION__,
            i[0][0], CUBIC_DEBUG_DATA(wt.pts()), PT_DEBUG_DATA(i, 0));
    SkDebugf(" " T_DEBUG_STR(wtTs, 1), i[1][0]);
    SkDebugf("\n");
}

#else
static void debugShowLineIntersection(int , const SkIntersectionHelper& ,
        const SkIntersectionHelper& , const SkIntersections& ) {
}

static void debugShowQuadLineIntersection(int , const SkIntersectionHelper& ,
        const SkIntersectionHelper& , const SkIntersections& ) {
}

static void debugShowQuadIntersection(int , const SkIntersectionHelper& ,
        const SkIntersectionHelper& , const SkIntersections& ) {
}

static void debugShowCubicLineIntersection(int , const SkIntersectionHelper& ,
        const SkIntersectionHelper& , const SkIntersections& ) {
}

static void debugShowCubicQuadIntersection(int , const SkIntersectionHelper& ,
        const SkIntersectionHelper& , const SkIntersections& ) {
}

static void debugShowCubicIntersection(int , const SkIntersectionHelper& ,
        const SkIntersectionHelper& , const SkIntersections& ) {
}

static void debugShowCubicIntersection(int , const SkIntersectionHelper& ,
        const SkIntersections& ) {
}
#endif

bool AddIntersectTs(SkOpContour* test, SkOpContour* next) {
    if (test != next) {
        if (AlmostLessUlps(test->bounds().fBottom, next->bounds().fTop)) {
            return false;
        }
        // OPTIMIZATION: outset contour bounds a smidgen instead?
        if (!SkPathOpsBounds::Intersects(test->bounds(), next->bounds())) {
            return true;
        }
    }
    SkIntersectionHelper wt;
    wt.init(test);
    bool foundCommonContour = test == next;
    do {
        SkIntersectionHelper wn;
        wn.init(next);
        if (test == next && !wn.startAfter(wt)) {
            continue;
        }
        do {
            if (!SkPathOpsBounds::Intersects(wt.bounds(), wn.bounds())) {
                continue;
            }
            int pts = 0;
            SkIntersections ts;
            bool swap = false;
            switch (wt.segmentType()) {
                case SkIntersectionHelper::kHorizontalLine_Segment:
                    swap = true;
                    switch (wn.segmentType()) {
                        case SkIntersectionHelper::kHorizontalLine_Segment:
                        case SkIntersectionHelper::kVerticalLine_Segment:
                        case SkIntersectionHelper::kLine_Segment: {
                            pts = ts.lineHorizontal(wn.pts(), wt.left(),
                                    wt.right(), wt.y(), wt.xFlipped());
                            debugShowLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        case SkIntersectionHelper::kQuad_Segment: {
                            pts = ts.quadHorizontal(wn.pts(), wt.left(),
                                    wt.right(), wt.y(), wt.xFlipped());
                            debugShowQuadLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        case SkIntersectionHelper::kCubic_Segment: {
                            pts = ts.cubicHorizontal(wn.pts(), wt.left(),
                                    wt.right(), wt.y(), wt.xFlipped());
                            debugShowCubicLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case SkIntersectionHelper::kVerticalLine_Segment:
                    swap = true;
                    switch (wn.segmentType()) {
                        case SkIntersectionHelper::kHorizontalLine_Segment:
                        case SkIntersectionHelper::kVerticalLine_Segment:
                        case SkIntersectionHelper::kLine_Segment: {
                            pts = ts.lineVertical(wn.pts(), wt.top(),
                                    wt.bottom(), wt.x(), wt.yFlipped());
                            debugShowLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        case SkIntersectionHelper::kQuad_Segment: {
                            pts = ts.quadVertical(wn.pts(), wt.top(),
                                    wt.bottom(), wt.x(), wt.yFlipped());
                            debugShowQuadLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        case SkIntersectionHelper::kCubic_Segment: {
                            pts = ts.cubicVertical(wn.pts(), wt.top(),
                                    wt.bottom(), wt.x(), wt.yFlipped());
                            debugShowCubicLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case SkIntersectionHelper::kLine_Segment:
                    switch (wn.segmentType()) {
                        case SkIntersectionHelper::kHorizontalLine_Segment:
                            pts = ts.lineHorizontal(wt.pts(), wn.left(),
                                    wn.right(), wn.y(), wn.xFlipped());
                            debugShowLineIntersection(pts, wt, wn, ts);
                            break;
                        case SkIntersectionHelper::kVerticalLine_Segment:
                            pts = ts.lineVertical(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped());
                            debugShowLineIntersection(pts, wt, wn, ts);
                            break;
                        case SkIntersectionHelper::kLine_Segment: {
                            pts = ts.lineLine(wt.pts(), wn.pts());
                            debugShowLineIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case SkIntersectionHelper::kQuad_Segment: {
                            swap = true;
                            pts = ts.quadLine(wn.pts(), wt.pts());
                            debugShowQuadLineIntersection(pts, wn, wt, ts);
                            break;
                        }
                        case SkIntersectionHelper::kCubic_Segment: {
                            swap = true;
                            pts = ts.cubicLine(wn.pts(), wt.pts());
                            debugShowCubicLineIntersection(pts, wn, wt,  ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case SkIntersectionHelper::kQuad_Segment:
                    switch (wn.segmentType()) {
                        case SkIntersectionHelper::kHorizontalLine_Segment:
                            pts = ts.quadHorizontal(wt.pts(), wn.left(),
                                    wn.right(), wn.y(), wn.xFlipped());
                            debugShowQuadLineIntersection(pts, wt, wn, ts);
                            break;
                        case SkIntersectionHelper::kVerticalLine_Segment:
                            pts = ts.quadVertical(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped());
                            debugShowQuadLineIntersection(pts, wt, wn, ts);
                            break;
                        case SkIntersectionHelper::kLine_Segment: {
                            pts = ts.quadLine(wt.pts(), wn.pts());
                            debugShowQuadLineIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case SkIntersectionHelper::kQuad_Segment: {
                            pts = ts.quadQuad(wt.pts(), wn.pts());
                            debugShowQuadIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case SkIntersectionHelper::kCubic_Segment: {
                            swap = true;
                            pts = ts.cubicQuad(wn.pts(), wt.pts());
                            debugShowCubicQuadIntersection(pts, wn, wt, ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                case SkIntersectionHelper::kCubic_Segment:
                    switch (wn.segmentType()) {
                        case SkIntersectionHelper::kHorizontalLine_Segment:
                            pts = ts.cubicHorizontal(wt.pts(), wn.left(),
                                    wn.right(), wn.y(), wn.xFlipped());
                            debugShowCubicLineIntersection(pts, wt, wn, ts);
                            break;
                        case SkIntersectionHelper::kVerticalLine_Segment:
                            pts = ts.cubicVertical(wt.pts(), wn.top(),
                                    wn.bottom(), wn.x(), wn.yFlipped());
                            debugShowCubicLineIntersection(pts, wt, wn, ts);
                            break;
                        case SkIntersectionHelper::kLine_Segment: {
                            pts = ts.cubicLine(wt.pts(), wn.pts());
                            debugShowCubicLineIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case SkIntersectionHelper::kQuad_Segment: {
                            pts = ts.cubicQuad(wt.pts(), wn.pts());
                            debugShowCubicQuadIntersection(pts, wt, wn, ts);
                            break;
                        }
                        case SkIntersectionHelper::kCubic_Segment: {
                            pts = ts.cubicCubic(wt.pts(), wn.pts());
                            debugShowCubicIntersection(pts, wt, wn, ts);
                            break;
                        }
                        default:
                            SkASSERT(0);
                    }
                    break;
                default:
                    SkASSERT(0);
            }
            if (!foundCommonContour && pts > 0) {
                test->addCross(next);
                next->addCross(test);
                foundCommonContour = true;
            }
            // in addition to recording T values, record matching segment
            if (pts == 2) {
                if (wn.segmentType() <= SkIntersectionHelper::kLine_Segment
                        && wt.segmentType() <= SkIntersectionHelper::kLine_Segment) {
                    if (wt.addCoincident(wn, ts, swap)) {
                        continue;
                    }
                    ts.cleanUpCoincidence();  // prefer (t == 0 or t == 1)
                    pts = 1;
                } else if (wn.segmentType() >= SkIntersectionHelper::kQuad_Segment
                        && wt.segmentType() >= SkIntersectionHelper::kQuad_Segment
                        && ts.isCoincident(0)) {
                    SkASSERT(ts.coincidentUsed() == 2);
                    if (wt.addCoincident(wn, ts, swap)) {
                        continue;
                    }
                    ts.cleanUpCoincidence();  // prefer (t == 0 or t == 1)
                    pts = 1;
                }
            }
            if (pts >= 2) {
                for (int pt = 0; pt < pts - 1; ++pt) {
                    const SkDPoint& point = ts.pt(pt);
                    const SkDPoint& next = ts.pt(pt + 1);
                    if (wt.isPartial(ts[swap][pt], ts[swap][pt + 1], point, next)
                            && wn.isPartial(ts[!swap][pt], ts[!swap][pt + 1], point, next)) {
                        if (!wt.addPartialCoincident(wn, ts, pt, swap)) {
                            // remove extra point if two map to same float values
                            ts.cleanUpCoincidence();  // prefer (t == 0 or t == 1)
                            pts = 1;
                        }
                    }
                }
            }
            for (int pt = 0; pt < pts; ++pt) {
                SkASSERT(ts[0][pt] >= 0 && ts[0][pt] <= 1);
                SkASSERT(ts[1][pt] >= 0 && ts[1][pt] <= 1);
                SkPoint point = ts.pt(pt).asSkPoint();
                wt.alignTPt(wn, swap, pt, &ts, &point);
                int testTAt = wt.addT(wn, point, ts[swap][pt]);
                int nextTAt = wn.addT(wt, point, ts[!swap][pt]);
                wt.addOtherT(testTAt, ts[!swap][pt], nextTAt);
                wn.addOtherT(nextTAt, ts[swap][pt], testTAt);
            }
        } while (wn.advance());
    } while (wt.advance());
    return true;
}

void AddSelfIntersectTs(SkOpContour* test) {
    SkIntersectionHelper wt;
    wt.init(test);
    do {
        if (wt.segmentType() != SkIntersectionHelper::kCubic_Segment) {
            continue;
        }
        SkIntersections ts;
        int pts = ts.cubic(wt.pts());
        debugShowCubicIntersection(pts, wt, ts);
        if (!pts) {
            continue;
        }
        SkASSERT(pts == 1);
        SkASSERT(ts[0][0] >= 0 && ts[0][0] <= 1);
        SkASSERT(ts[1][0] >= 0 && ts[1][0] <= 1);
        SkPoint point = ts.pt(0).asSkPoint();
        int testTAt = wt.addSelfT(point, ts[0][0]);
        int nextTAt = wt.addSelfT(point, ts[1][0]);
        wt.addOtherT(testTAt, ts[1][0], nextTAt);
        wt.addOtherT(nextTAt, ts[0][0], testTAt);
    } while (wt.advance());
}

// resolve any coincident pairs found while intersecting, and
// see if coincidence is formed by clipping non-concident segments
void CoincidenceCheck(SkTArray<SkOpContour*, true>* contourList, int total) {
    int contourCount = (*contourList).count();
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = (*contourList)[cIndex];
        contour->resolveNearCoincidence();
    }
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = (*contourList)[cIndex];
        contour->addCoincidentPoints();
    }
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = (*contourList)[cIndex];
        contour->calcCoincidentWinding();
    }
    for (int cIndex = 0; cIndex < contourCount; ++cIndex) {
        SkOpContour* contour = (*contourList)[cIndex];
        contour->calcPartialCoincidentWinding();
    }
}
