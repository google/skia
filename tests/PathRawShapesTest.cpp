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

static void check_path_is_raw(skiatest::Reporter* reporter,
                              const SkPath& path, const SkPathRaw& raw) {
    size_t pIndex = 0, vIndex = 0, cIndex = 0;
    SkPath::RawIter iter(path);
    bool done = false;

    while (!done) {
        SkPoint pts[4];
        switch (iter.next(pts)) {
            case SkPath::kMove_Verb:
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[0]);
                REPORTER_ASSERT(reporter, raw.verbs()[vIndex++] == SkPathVerb::kMove);
                break;
            case SkPath::kLine_Verb:
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[1]);
                REPORTER_ASSERT(reporter, raw.verbs()[vIndex++] == SkPathVerb::kLine);
                break;
            case SkPath::kQuad_Verb:
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[1]);
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[2]);
                REPORTER_ASSERT(reporter, raw.verbs()[vIndex++] == SkPathVerb::kQuad);
                break;
            case SkPath::kConic_Verb:
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[1]);
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[2]);
                REPORTER_ASSERT(reporter, raw.verbs()[vIndex++] == SkPathVerb::kConic);
                REPORTER_ASSERT(reporter, raw.conics()[cIndex++] == iter.conicWeight());
                break;
            case SkPath::kCubic_Verb:
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[1]);
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[2]);
                REPORTER_ASSERT(reporter, raw.points()[pIndex++] == pts[3]);
                REPORTER_ASSERT(reporter, raw.verbs()[vIndex++] == SkPathVerb::kCubic);
                break;
            case SkPath::kClose_Verb:
                REPORTER_ASSERT(reporter, raw.verbs()[vIndex++] == SkPathVerb::kClose);
                break;
            case SkPath::kDone_Verb:
                done = true;
                break;
        }
    }
    REPORTER_ASSERT(reporter, pIndex == raw.points().size());
    REPORTER_ASSERT(reporter, vIndex == raw.verbs().size());
    REPORTER_ASSERT(reporter, cIndex == raw.conics().size());
}

DEF_TEST(pathrawshapes_rect, reporter) {
    const SkRect r = {1, 2, 3, 4};

    for (auto dir : gDirections) {
        SkPathRawShapes::Rect shape(r, dir);

        REPORTER_ASSERT(reporter, shape.bounds() == r);
        REPORTER_ASSERT(reporter, shape.isConvex());
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
        REPORTER_ASSERT(reporter, shape.isConvex());
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
        REPORTER_ASSERT(reporter, shape.isConvex());
        REPORTER_ASSERT(reporter, shape.segmentMasks() == (kLine_SkPathSegmentMask |
                                                           kConic_SkPathSegmentMask));

        const SkPath path = SkPath::RRect(rr, dir);

        check_path_is_raw(reporter, path, shape);
    }
}

