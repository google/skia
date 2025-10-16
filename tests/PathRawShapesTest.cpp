/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"

#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRaw.h"
#include "src/core/SkPathRawShapes.h"

#include "tests/Test.h"

const SkPathDirection gDirections[] = {
    SkPathDirection::kCW,
    SkPathDirection::kCCW,
};

static SkPath path_from_raw(const SkPathRaw& raw) {
    return SkPath::Raw(raw.points(), raw.verbs(), raw.conics(), raw.fillType());
}

template <typename T> bool span_eq(SkSpan<T> a, SkSpan<T> b) {
    if (a.size() != b.size()) {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin());
}

static void check_path_is_raw(skiatest::Reporter* reporter,
                              const SkPath& path, const SkPathRaw& raw) {
    auto praw = SkPathPriv::Raw(path, SkResolveConvexity::kNo);
    REPORTER_ASSERT(reporter, praw.has_value());

    REPORTER_ASSERT(reporter, span_eq(praw->fPoints, raw.fPoints));
    REPORTER_ASSERT(reporter, span_eq(praw->fVerbs,  raw.fVerbs));
    REPORTER_ASSERT(reporter, span_eq(praw->fConics, raw.fConics));
    REPORTER_ASSERT(reporter, praw->fBounds == raw.fBounds);
}

DEF_TEST(pathrawshapes_rect, reporter) {
    const SkRect r = {1, 2, 3, 4};

    for (auto dir : gDirections) {
        SkPathRawShapes::Rect shape(r, dir);

        REPORTER_ASSERT(reporter, shape.bounds() == r);
        REPORTER_ASSERT(reporter, shape.isKnownToBeConvex());
        REPORTER_ASSERT(reporter, shape.segmentMasks() == kLine_SkPathSegmentMask);

        const SkPath path = path_from_raw(shape);

        check_path_is_raw(reporter, path, shape);
    }
}

DEF_TEST(pathrawshapes_oval, reporter) {
    const SkRect r = {1, 2, 3, 4};

    for (auto dir : gDirections) {
        SkPathRawShapes::Oval shape(r, dir);

        REPORTER_ASSERT(reporter, shape.bounds() == r);
        REPORTER_ASSERT(reporter, shape.isKnownToBeConvex());
        REPORTER_ASSERT(reporter, shape.segmentMasks() == kConic_SkPathSegmentMask);

        const SkPath path = SkPath::Oval(r, dir);

        check_path_is_raw(reporter, path, shape);
    }
}

DEF_TEST(pathrawshapes_rrect, reporter) {
    const SkRect r = {0, 0, 4, 4};
    const SkRRect rr = SkRRect::MakeRectXY(r, 1, 1);

    for (auto dir : gDirections) {
        SkPathRawShapes::RRect shape(rr, dir);

        REPORTER_ASSERT(reporter, shape.bounds() == r);
        REPORTER_ASSERT(reporter, shape.isKnownToBeConvex());
        REPORTER_ASSERT(reporter, shape.segmentMasks() == (kLine_SkPathSegmentMask |
                                                           kConic_SkPathSegmentMask));

        const SkPath path = SkPath::RRect(rr, dir);

        check_path_is_raw(reporter, path, shape);
    }
}

