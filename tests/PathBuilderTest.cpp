/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "src/core/SkPathPriv.h"
#include "tests/Test.h"

static void is_empty(skiatest::Reporter* reporter, const SkPath& p) {
    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, p.countPoints() == 0);
}

DEF_TEST(pathbuilder, reporter) {
    SkPathBuilder b;

    is_empty(reporter, b.snapshot());
    is_empty(reporter, b.detach());

    b.moveTo(10, 10).lineTo(20, 20).quadTo(30, 10, 10, 20);

    SkPath p0 = b.snapshot();
    SkPath p1 = b.snapshot();
    SkPath p2 = b.detach();

    // Builders should always precompute the path's bounds, so there is no race condition later
    REPORTER_ASSERT(reporter, SkPathPriv::HasComputedBounds(p0));
    REPORTER_ASSERT(reporter, SkPathPriv::HasComputedBounds(p1));
    REPORTER_ASSERT(reporter, SkPathPriv::HasComputedBounds(p2));

    REPORTER_ASSERT(reporter, p0.getBounds() == SkRect::MakeLTRB(10, 10, 30, 20));
    REPORTER_ASSERT(reporter, p0.countPoints() == 4);

    REPORTER_ASSERT(reporter, p0 == p1);
    REPORTER_ASSERT(reporter, p0 == p2);

    is_empty(reporter, b.snapshot());
    is_empty(reporter, b.detach());
}

DEF_TEST(pathbuilder_filltype, reporter) {
    for (auto fillType : { SkPathFillType::kWinding,
                           SkPathFillType::kEvenOdd,
                           SkPathFillType::kInverseWinding,
                           SkPathFillType::kInverseEvenOdd }) {
        SkPathBuilder b(fillType);

        REPORTER_ASSERT(reporter, b.fillType() == fillType);

        for (const SkPath& path : { b.snapshot(), b.detach() }) {
            REPORTER_ASSERT(reporter, path.getFillType() == fillType);
            is_empty(reporter, path);
        }
    }
}

static bool check_points(const SkPath& path, const SkPoint expected[], size_t count) {
    std::vector<SkPoint> iter_pts;

    for (auto [v, p, w] : SkPathPriv::Iterate(path)) {
        switch (v) {
            case SkPathVerb::kMove:
                iter_pts.push_back(p[0]);
                break;
            case SkPathVerb::kLine:
                iter_pts.push_back(p[1]);
                break;
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
                iter_pts.push_back(p[1]);
                iter_pts.push_back(p[2]);
                break;
            case SkPathVerb::kCubic:
                iter_pts.push_back(p[1]);
                iter_pts.push_back(p[2]);
                iter_pts.push_back(p[3]);
                break;
            case SkPathVerb::kClose:
                break;
        }
    }
    if (iter_pts.size() != count) {
        return false;
    }
    for (size_t i = 0; i < count; ++i) {
        if (iter_pts[i] != expected[i]) {
            return false;
        }
    }
    return true;
}

DEF_TEST(pathbuilder_missing_move, reporter) {
    SkPathBuilder b;

    b.lineTo(10, 10).lineTo(20, 30);
    const SkPoint pts0[] = {
        {0, 0}, {10, 10}, {20, 30},
    };
    REPORTER_ASSERT(reporter, check_points(b.snapshot(), pts0, SK_ARRAY_COUNT(pts0)));

    b.reset().moveTo(20, 20).lineTo(10, 10).lineTo(20, 30).close().lineTo(60, 60);
    const SkPoint pts1[] = {
        {20, 20}, {10, 10}, {20, 30},
        {20, 20}, {60, 60},
    };
    REPORTER_ASSERT(reporter, check_points(b.snapshot(), pts1, SK_ARRAY_COUNT(pts1)));
}

DEF_TEST(pathbuilder_addRect, reporter) {
    const SkRect r = { 10, 20, 30, 40 };

    for (int i = 0; i < 4; ++i) {
        for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            SkPathBuilder b;
            b.addRect(r, dir, i);
            auto bp = b.detach();

            SkRect r2;
            bool   closed = false;
            SkPathDirection dir2;
            REPORTER_ASSERT(reporter, bp.isRect(&r2, &closed, &dir2));
            REPORTER_ASSERT(reporter, r2 == r);
            REPORTER_ASSERT(reporter, closed);
            REPORTER_ASSERT(reporter, dir == dir2);

            SkPath p;
            p.addRect(r, dir, i);
            REPORTER_ASSERT(reporter, p == bp);
        }
    }
}

static bool is_eq(const SkPath& a, const SkPath& b) {
    if (a != b) {
        return false;
    }

    {
        SkRect ra, rb;
        bool is_a = a.isOval(&ra);
        bool is_b = b.isOval(&rb);
        if (is_a != is_b) {
            return false;
        }
        if (is_a && (ra != rb)) {
            return false;
        }
    }

    {
        SkRRect rra, rrb;
        bool is_a = a.isRRect(&rra);
        bool is_b = b.isRRect(&rrb);
        if (is_a != is_b) {
            return false;
        }
        if (is_a && (rra != rrb)) {
            return false;
        }
    }

    // getConvextity() should be sufficient to test, but internally we sometimes don't want
    // to trigger computing it, so this is the stronger test for equality.
    {
        SkPathConvexity ca = SkPathPriv::GetConvexityOrUnknown(a),
                        cb = SkPathPriv::GetConvexityOrUnknown(b);
        if (ca != cb) {
            return false;
        }
    }

    return true;
}

DEF_TEST(pathbuilder_addOval, reporter) {
    const SkRect r = { 10, 20, 30, 40 };
    SkRect tmp;

    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int i = 0; i < 4; ++i) {
            auto bp = SkPathBuilder().addOval(r, dir, i).detach();
            SkPath p;
            p.addOval(r, dir, i);
            REPORTER_ASSERT(reporter, is_eq(p, bp));
        }
        auto bp = SkPathBuilder().addOval(r, dir).detach();
        SkPath p;
        p.addOval(r, dir);
        REPORTER_ASSERT(reporter, is_eq(p, bp));

        // test negative case -- can't have any other segments
        bp = SkPathBuilder().addOval(r, dir).lineTo(10, 10).detach();
        REPORTER_ASSERT(reporter, !bp.isOval(&tmp));
        bp = SkPathBuilder().lineTo(10, 10).addOval(r, dir).detach();
        REPORTER_ASSERT(reporter, !bp.isOval(&tmp));
    }
}

DEF_TEST(pathbuilder_addRRect, reporter) {
    const SkRRect rr = SkRRect::MakeRectXY({ 10, 20, 30, 40 }, 5, 6);

    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int i = 0; i < 4; ++i) {
            SkPathBuilder b;
            b.addRRect(rr, dir, i);
            auto bp = b.detach();

            SkPath p;
            p.addRRect(rr, dir, i);
            REPORTER_ASSERT(reporter, is_eq(p, bp));
        }
        auto bp = SkPathBuilder().addRRect(rr, dir).detach();
        SkPath p;
        p.addRRect(rr, dir);
        REPORTER_ASSERT(reporter, is_eq(p, bp));

        // test negative case -- can't have any other segments
        SkRRect tmp;
        bp = SkPathBuilder().addRRect(rr, dir).lineTo(10, 10).detach();
        REPORTER_ASSERT(reporter, !bp.isRRect(&tmp));
        bp = SkPathBuilder().lineTo(10, 10).addRRect(rr, dir).detach();
        REPORTER_ASSERT(reporter, !bp.isRRect(&tmp));
    }
}

#include "include/utils/SkRandom.h"

DEF_TEST(pathbuilder_make, reporter) {
    constexpr int N = 100;
    uint8_t vbs[N];
    SkPoint pts[N];

    SkRandom rand;
    SkPathBuilder b;
    b.moveTo(0, 0);
    pts[0] = {0, 0}; vbs[0] = (uint8_t)SkPathVerb::kMove;
    for (int i = 1; i < N; ++i) {
        float x = rand.nextF();
        float y = rand.nextF();
        b.lineTo(x, y);
        pts[i] = {x, y}; vbs[i] = (uint8_t)SkPathVerb::kLine;
    }
    auto p0 = b.detach();
    auto p1 = SkPath::Make(pts, N, vbs, N, nullptr, 0, p0.getFillType());
    REPORTER_ASSERT(reporter, p0 == p1);
}

DEF_TEST(pathbuilder_genid, r) {
    SkPathBuilder builder;

    builder.lineTo(10, 10);
    auto p1 = builder.snapshot();

    builder.lineTo(10, 20);
    auto p2 = builder.snapshot();

    REPORTER_ASSERT(r, p1.getGenerationID() != p2.getGenerationID());
}

DEF_TEST(pathbuilder_addPolygon, reporter) {
    SkPoint pts[] = {{1, 2}, {3, 4}, {5, 6}, {7, 8}};

    auto addpoly = [](const SkPoint pts[], int count, bool isClosed) {
        SkPathBuilder builder;
        if (count > 0) {
            builder.moveTo(pts[0]);
            for (int i = 1; i < count; ++i) {
                builder.lineTo(pts[i]);
            }
            if (isClosed) {
                builder.close();
            }
        }
        return builder.detach();
    };

    for (bool isClosed : {false, true}) {
        for (size_t i = 0; i <= SK_ARRAY_COUNT(pts); ++i) {
            auto path0 = SkPathBuilder().addPolygon(pts, i, isClosed).detach();
            auto path1 = addpoly(pts, i, isClosed);
            REPORTER_ASSERT(reporter, path0 == path1);
        }
    }
}

DEF_TEST(pathbuilder_shrinkToFit, reporter) {
    // SkPathBuilder::snapshot() creates copies of its arrays for perfectly sized paths,
    // where SkPathBuilder::detach() moves its larger scratch arrays for speed.
    bool any_smaller = false;
    for (int pts = 0; pts < 10; pts++) {

        SkPathBuilder b;
        for (int i = 0; i < pts; i++) {
            b.lineTo(i,i);
        }
        b.close();

        SkPath s = b.snapshot(),
               d = b.detach();
        REPORTER_ASSERT(reporter, s.approximateBytesUsed() <= d.approximateBytesUsed());
        any_smaller |=            s.approximateBytesUsed() <  d.approximateBytesUsed();
    }
    REPORTER_ASSERT(reporter, any_smaller);
}

DEF_TEST(pathbuilder_addPath, reporter) {
    const auto p = SkPath()
        .moveTo(10, 10)
        .lineTo(100, 10)
        .quadTo(200, 100, 100, 200)
        .close()
        .moveTo(200, 200)
        .cubicTo(210, 200, 210, 300, 200, 300)
        .conicTo(150, 250, 100, 200, 1.4f);

    REPORTER_ASSERT(reporter, p == SkPathBuilder().addPath(p).detach());
}
