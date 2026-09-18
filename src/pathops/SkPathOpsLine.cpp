/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkPathOpsLine.h"

#include "src/pathops/SkPathOpsTypes.h"

#include <cmath>
#include <algorithm>

SkDPoint SkDLine::ptAtT(double t) const {
    if (0 == t) {
        return fPts[0];
    }
    if (1 == t) {
        return fPts[1];
    }
    double one_t = 1 - t;
    SkDPoint result = { one_t * fPts[0].fX + t * fPts[1].fX, one_t * fPts[0].fY + t * fPts[1].fY };
    return result;
}

double SkDLine::exactPoint(const SkDPoint& xy) const {
    if (xy == fPts[0]) {  // do cheapest test first
        return 0;
    }
    if (xy == fPts[1]) {
        return 1;
    }
    return -1;
}

// Scale tolerance by interpolating the endpoint coordinate magnitudes at t (matching propagated
// endpoint rounding error), rather than the global maximum across the entire segment, so a huge
// coordinate at one endpoint does not inflate tolerance near the other endpoint. Clamp to a floor
// based on segment extent so tolerance does not drop precipitously when an endpoint is near the
// origin.
static double interpolated_largest(const SkDPoint& p0, const SkDPoint& p1, double t) {
    constexpr double kSegmentExtentFloorScale = 0.001;
    double largest0 = std::max(std::abs(p0.fX), std::abs(p0.fY));
    double largest1 = std::max(std::abs(p1.fX), std::abs(p1.fY));
    double segExtent = std::max(std::abs(p1.fX - p0.fX), std::abs(p1.fY - p0.fY));
    double minFloor = segExtent * kSegmentExtentFloorScale;
    return std::max((1 - t) * largest0 + t * largest1, minFloor);
}

double SkDLine::nearPoint(const SkDPoint& xy, bool* unequal) const {
    // For axis-aligned lines, the perpendicular distance is purely along the constant
    // coordinate (which has no slope interpolation error). Check that the point is within
    // 2 ULPs (AlmostBequalUlps) of that constant coordinate, matching NearPointV/NearPointH.
    // Otherwise, the distance check below scales tolerance by `largest` (the maximum magnitude
    // across both X and Y), which would allow too much perpendicular drift when the constant
    // coordinate is smaller in magnitude than the varying coordinate.
    if (fPts[0].fX == fPts[1].fX && !AlmostBequalUlps(xy.fX, fPts[0].fX)) {
        return -1;
    }
    if (fPts[0].fY == fPts[1].fY && !AlmostBequalUlps(xy.fY, fPts[0].fY)) {
        return -1;
    }
    // project a perpendicular ray from the point to the line; find the T on the line
    SkDVector len = fPts[1] - fPts[0]; // the x/y magnitudes of the line
    double denom = len.fX * len.fX + len.fY * len.fY;  // see DLine intersectRay
    SkDVector ab0 = xy - fPts[0];
    double numer = len.fX * ab0.fX + ab0.fY * len.fY;
    if (!between(0, numer, denom)) {
        return -1;
    }
    double t = denom ? numer / denom : 0;
    SkDPoint realPt = ptAtT(t);
    double dist = realPt.distance(xy);   // OPTIMIZATION: can we compare against distSq instead ?
    double largest = interpolated_largest(fPts[0], fPts[1], t);
    if (!AlmostEqualUlps_Pin(largest, largest + dist)) { // is the dist within ULPS tolerance?
        return -1;
    }
    if (unequal) {
        *unequal = (float) largest != (float) (largest + dist);
    }
    t = SkPinT(t);  // a looser pin breaks skpwww_lptemp_com_3
    SkASSERT(between(0, t, 1));
    return t;
}

bool SkDLine::nearRay(const SkDPoint& xy) const {
    // project a perpendicular ray from the point to the line; find the T on the line
    SkDVector len = fPts[1] - fPts[0]; // the x/y magnitudes of the line
    double denom = len.fX * len.fX + len.fY * len.fY;  // see DLine intersectRay
    SkDVector ab0 = xy - fPts[0];
    double numer = len.fX * ab0.fX + ab0.fY * len.fY;
    double t = numer / denom;
    SkDPoint realPt = ptAtT(t);
    double dist = realPt.distance(xy);   // OPTIMIZATION: can we compare against distSq instead ?
    // find the ordinal in the original line with the largest unsigned exponent
    double tiniest = std::min(std::min(std::min(fPts[0].fX, fPts[0].fY), fPts[1].fX), fPts[1].fY);
    double largest = std::max(std::max(std::max(fPts[0].fX, fPts[0].fY), fPts[1].fX), fPts[1].fY);
    largest = std::max(largest, -tiniest);
    return RoughlyEqualUlps(largest, largest + dist); // is the dist within ULPS tolerance?
}

double SkDLine::ExactPointH(const SkDPoint& xy, double left, double right, double y) {
    if (xy.fY == y) {
        if (xy.fX == left) {
            return 0;
        }
        if (xy.fX == right) {
            return 1;
        }
    }
    return -1;
}

double SkDLine::NearPointH(const SkDPoint& xy, double left, double right, double y) {
    if (!AlmostBequalUlps(xy.fY, y)) {
        return -1;
    }
    if (!AlmostBetweenUlps(left, xy.fX, right)) {
        return -1;
    }
    double t = (xy.fX - left) / (right - left);
    t = SkPinT(t);
    SkASSERT(between(0, t, 1));
    double realPtX = (1 - t) * left + t * right;
    SkDVector distU = {xy.fY - y, xy.fX - realPtX};
    double distSq = distU.fX * distU.fX + distU.fY * distU.fY;
    double dist = sqrt(distSq); // OPTIMIZATION: can we compare against distSq instead ?
    double largest = interpolated_largest({left, y}, {right, y}, t);
    if (!AlmostEqualUlps_Pin(largest, largest + dist)) { // is the dist within ULPS tolerance?
        return -1;
    }
    return t;
}

double SkDLine::ExactPointV(const SkDPoint& xy, double top, double bottom, double x) {
    if (xy.fX == x) {
        if (xy.fY == top) {
            return 0;
        }
        if (xy.fY == bottom) {
            return 1;
        }
    }
    return -1;
}

double SkDLine::NearPointV(const SkDPoint& xy, double top, double bottom, double x) {
    if (!AlmostBequalUlps(xy.fX, x)) {
        return -1;
    }
    if (!AlmostBetweenUlps(top, xy.fY, bottom)) {
        return -1;
    }
    double t = (xy.fY - top) / (bottom - top);
    t = SkPinT(t);
    SkASSERT(between(0, t, 1));
    double realPtY = (1 - t) * top + t * bottom;
    SkDVector distU = {xy.fX - x, xy.fY - realPtY};
    double distSq = distU.fX * distU.fX + distU.fY * distU.fY;
    double dist = sqrt(distSq); // OPTIMIZATION: can we compare against distSq instead ?
    double largest = interpolated_largest({x, top}, {x, bottom}, t);
    if (!AlmostEqualUlps_Pin(largest, largest + dist)) { // is the dist within ULPS tolerance?
        return -1;
    }
    return t;
}
