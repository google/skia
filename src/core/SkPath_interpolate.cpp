/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkSpan.h"

#include <optional>
#include <vector>

namespace {

template <typename T> bool span_equal(SkSpan<T> a, SkSpan<T> b) {
    if (a.size() != b.size()) {
        return false;
    }
    if (a.empty()) {
        return true;
    }
    return (a.data() == b.data()) || std::equal(a.begin(), a.end(), b.begin());
}

SkPoint operator*(float s, SkPoint p) {
    return {
        p.fX * s,
        p.fY * s
    };
}

SkPoint lerp(SkPoint from, SkPoint to, float t) {
    return from + t * (to - from);
}

// These can be copy/pasted into Chrome / Android
// - CanInterpolate()
// - Interpolate()
//
// That would allow us to remove the associated SkPath methods.
//
// Note: they require no private access to SkPath internals, and are not used/called by
//       anthing in Skia.

/**
 *  Returns true if the two paths can be interpolated.
 *
 *  Returns true iff the paths have the same verbs and conicWeights,
 *  and the same number of points.
 */
bool CanInterpolate(const SkPath& from, const SkPath& to) {
    return from.points().size() == to.points().size() &&
           span_equal(from.verbs(), to.verbs()) &&
           span_equal(from.conicWeights(), to.conicWeights());
}

/**
 *  Return a path whose points are lineraly interpolated between from and to,
 *  using t (typically between 0 and 1).
 *
 *  (logically) newPoint = (1 - t) * fromPoint + t * toPoint
 *
 *  The result has the same fillType as the 'from' path.
 *
 *  If the two paths are not interpolatable, this returns {}.
 */
std::optional<SkPath> Interpolate(const SkPath& from, const SkPath& to, float t) {
    if (!CanInterpolate(from, to)) {
        return {};
    }

    // note, if from and to have different fill-types, that's up to the client to deal with.
    const SkPathFillType fillType = from.getFillType();

    const SkSpan<const SkPoint> fromPts = from.points(),
                                  toPts = to.points();

    std::vector<SkPoint> dst(fromPts.size());

    for (size_t i = 0; i < fromPts.size(); ++i) {
        dst[i] = lerp(fromPts[i], toPts[i], t);
    }
    return SkPath::Raw({dst.data(), dst.size()}, from.verbs(), from.conicWeights(), fillType);
}

} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////

bool SkPath::isInterpolatable(const SkPath& compare) const {
    return CanInterpolate(*this, compare);
}

SkPath SkPath::makeInterpolate(const SkPath& ending, SkScalar weight) const {
    return Interpolate(*this, ending, 1 - weight).value_or(SkPath());
}

bool SkPath::interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const {
    if (auto result = Interpolate(*this, ending, 1 - weight)) {
        *out = *result;
        return true;
    }
    return false;
}
