/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_Oracle_DEFINED
#define skgpu_graphite_sparse_strips_Oracle_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "src/core/SkVx.h"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace skgpu::graphite {

// Used as the source of ground truth for testing the entire SparseStrips pipeline end to end.
// Builds the subsample windings for a single pixel row by raycasting at each of the 8 subsample
// locations in the SparseStrips MSAA pattern.
class ScanlineOracle8x {
public:
    struct CurveIntersection {
        skvx::float8 x;
        skvx::int8 dir;  // +1 for downward, -1 for upward, 0 for no hit in this lane
    };

    struct RowWindingInterval {
        int x;
        skvx::int8 winding;
    };

    static skvx::int8 GetOracleWinding(int px, const std::vector<RowWindingInterval>& intervals) {
        if (intervals.empty() || px < intervals.front().x || px >= intervals.back().x) {
            return skvx::int8(0);
        }
        auto it = std::upper_bound(
                intervals.begin(),
                intervals.end(),
                px,
                [](int val, const RowWindingInterval& interval) { return val < interval.x; });
        if (it == intervals.begin()) {
            return skvx::int8(0);
        }
        return std::prev(it)->winding;
    }

    ScanlineOracle8x() = default;
    explicit ScanlineOracle8x(const SkPath& path) : fPath(path) {}

    void setPath(const SkPath& path) {
        if (fPath != path) {
            fPath = path;
            fBuilt = false;
            fCurrentRow = -1;
            fHits.clear();
            fIntervals.clear();
        }
    }

    const SkPath& path() const { return fPath; }

    void buildRow(int py);
    void buildRow(int py, const SkPath& path);

    skvx::int8 evaluatePixelWinding(float px) const;
    skvx::int8 evaluate(float px) const { return this->evaluatePixelWinding(px); }

    skvx::int8 evaluate(SkPoint pt);
    skvx::int8 evaluate(SkPoint pt, const SkPath& path);

    bool isBuilt() const { return fBuilt; }
    int currentRow() const { return fCurrentRow; }
    const std::vector<CurveIntersection>& hits() const {
        SkASSERT(fBuilt);
        return fHits;
    }
    const std::vector<RowWindingInterval>& rowIntervals() const {
        SkASSERT(fBuilt);
        return fIntervals;
    }

    std::vector<RowWindingInterval> buildRowIntervals(int py) {
        this->buildRow(py);
        return fIntervals;
    }
    std::vector<RowWindingInterval> buildRowIntervals(int py, const SkPath& path) {
        this->buildRow(py, path);
        return fIntervals;
    }

private:
    void intersectLine(const SkPoint pts[2], const skvx::float8& targetY);
    void intersectQuad(const SkPoint pts[3], const skvx::float8& targetY);
    void intersectConic(const SkPoint pts[3], float weight, const skvx::float8& targetY);
    void intersectCubic(const SkPoint pts[4], const skvx::float8& targetY);
    void collectPathIntersections(const SkPath& path, const skvx::float8& targetY);

    SkPath fPath;
    int fCurrentRow = -1;
    bool fBuilt = false;
    std::vector<CurveIntersection> fHits;
    std::vector<RowWindingInterval> fIntervals;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_Oracle_DEFINED
