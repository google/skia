/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "src/base/SkRandom.h"
#include "src/core/SkPathPriv.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>

enum class SkPathConvexity;

static void is_empty(skiatest::Reporter* reporter, const SkPath& p) {
    REPORTER_ASSERT(reporter, p.getBounds().isEmpty());
    REPORTER_ASSERT(reporter, p.countPoints() == 0);
}

DEF_TEST(pathbuilder, reporter) {
    SkPathBuilder b;

    REPORTER_ASSERT(reporter, b.isEmpty());
    is_empty(reporter, b.snapshot());
    is_empty(reporter, b.detach());

    b.moveTo(10, 10).lineTo(20, 20).quadTo(30, 10, 10, 20);
    REPORTER_ASSERT(reporter, b.countPoints() == 4);

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

    REPORTER_ASSERT(reporter, b.isEmpty());
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
        REPORTER_ASSERT(reporter, b.isInverseFillType() == SkPathFillType_IsInverse(fillType));

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
    REPORTER_ASSERT(reporter, check_points(b.snapshot(), pts0, std::size(pts0)));

    b.reset().moveTo(20, 20).lineTo(10, 10).lineTo(20, 30).close().lineTo(60, 60);
    const SkPoint pts1[] = {
        {20, 20}, {10, 10}, {20, 30},
        {20, 20}, {60, 60},
    };
    REPORTER_ASSERT(reporter, check_points(b.snapshot(), pts1, std::size(pts1)));
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
            REPORTER_ASSERT(reporter, bp.isConvex());
            REPORTER_ASSERT(reporter, bp.isRect(&r2, &closed, &dir2));
            REPORTER_ASSERT(reporter, r2 == r);
            REPORTER_ASSERT(reporter, closed);
            REPORTER_ASSERT(reporter, dir == dir2);

            SkPath p = SkPath::Rect(r, dir, i);
            REPORTER_ASSERT(reporter, p == bp);

            // do it again, after the detach
            b.addRect(r, dir, i);
            b.moveTo(3, 4);
            b.lineTo(4, 5);
            bp = b.detach();
            REPORTER_ASSERT(reporter, !bp.isConvex());
            REPORTER_ASSERT(reporter, !bp.isRect(&r2, &closed, &dir2));
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
            SkPath p = SkPath::Oval(r, dir, i);
            REPORTER_ASSERT(reporter, is_eq(p, bp));

            SkRect bounds;
            REPORTER_ASSERT(reporter, p.isOval(&bounds));
            REPORTER_ASSERT(reporter, bp.isOval(&bounds));
            REPORTER_ASSERT(reporter, p.isConvex());
            REPORTER_ASSERT(reporter, bp.isConvex());
        }
        auto bp = SkPathBuilder().addOval(r, dir).detach();
        SkPath p = SkPath::Oval(r, dir);
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

            SkPath p = SkPath::RRect(rr, dir, i);
            REPORTER_ASSERT(reporter, is_eq(p, bp));
        }
        auto bp = SkPathBuilder().addRRect(rr, dir).detach();
        SkPath p = SkPath::RRect(rr, dir);
        REPORTER_ASSERT(reporter, is_eq(p, bp));

        // test negative case -- can't have any other segments
        SkRRect tmp;
        bp = SkPathBuilder().addRRect(rr, dir).lineTo(10, 10).detach();
        REPORTER_ASSERT(reporter, !bp.isRRect(&tmp));
        bp = SkPathBuilder().lineTo(10, 10).addRRect(rr, dir).detach();
        REPORTER_ASSERT(reporter, !bp.isRRect(&tmp));
    }
}

DEF_TEST(pathbuilder_make, reporter) {
    constexpr int N = 100;
    SkPathVerb vbs[N];
    SkPoint pts[N];

    SkRandom rand;
    SkPathBuilder b;
    b.moveTo(0, 0);
    pts[0] = {0, 0}; vbs[0] = SkPathVerb::kMove;
    for (int i = 1; i < N; ++i) {
        float x = rand.nextF();
        float y = rand.nextF();
        b.lineTo(x, y);
        pts[i] = {x, y}; vbs[i] = SkPathVerb::kLine;
    }
    auto p0 = b.detach();
    auto p1 = SkPath::Raw(pts, vbs, {}, p0.getFillType());
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
        for (size_t i = 0; i <= std::size(pts); ++i) {
            auto path0 = SkPathBuilder().addPolygon({pts, i}, isClosed).detach();
            auto path1 = addpoly(pts, i, isClosed);
            REPORTER_ASSERT(reporter, path0 == path1);
        }
    }
}

static void test_addPath(skiatest::Reporter* reporter) {
    SkPathBuilder p, q;
    p.lineTo(1, 2);
    q.moveTo(4, 4);
    q.lineTo(7, 8);
    q.conicTo(8, 7, 6, 5, 0.5f);
    q.quadTo(6, 7, 8, 6);
    q.cubicTo(5, 6, 7, 8, 7, 5);
    q.close();
    p.addPath(q.snapshot(), -4, -4);
    SkRect expected = {0, 0, 4, 4};
    REPORTER_ASSERT(reporter, p.snapshot().getBounds() == expected);
    p.reset();
    SkPathPriv::ReverseAddPath(&p, q.snapshot());
    SkRect reverseExpected = {4, 4, 8, 8};
    REPORTER_ASSERT(reporter, p.snapshot().getBounds() == reverseExpected);
}

static void test_addPathMode(skiatest::Reporter* reporter, bool explicitMoveTo, bool extend) {
    SkPathBuilder p, q;
    if (explicitMoveTo) {
        p.moveTo(1, 1);
    }
    p.lineTo(1, 2);
    if (explicitMoveTo) {
        q.moveTo(2, 1);
    }
    q.lineTo(2, 2);
    p.addPath(q.snapshot(), extend ? SkPath::kExtend_AddPathMode : SkPath::kAppend_AddPathMode);
    auto verbs = SkPathPriv::GetVerbs(p);
    REPORTER_ASSERT(reporter, verbs.size() == 4);
    REPORTER_ASSERT(reporter, verbs[0] == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, verbs[1] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[2] == (extend ? SkPathVerb::kLine : SkPathVerb::kMove));
    REPORTER_ASSERT(reporter, verbs[3] == SkPathVerb::kLine);
}

static void test_extendClosedPath(skiatest::Reporter* reporter) {
    SkPathBuilder p, q;
    p.moveTo(1, 1);
    p.lineTo(1, 2);
    p.lineTo(2, 2);
    p.close();
    q.moveTo(2, 1);
    q.lineTo(2, 3);
    p.addPath(q.detach(), SkPath::kExtend_AddPathMode);
    auto verbs = SkPathPriv::GetVerbs(p);
    REPORTER_ASSERT(reporter, verbs.size() == 7);
    REPORTER_ASSERT(reporter, verbs[0] == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, verbs[1] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[2] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[3] == SkPathVerb::kClose);
    REPORTER_ASSERT(reporter, verbs[4] == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, verbs[5] == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, verbs[6] == SkPathVerb::kLine);

    std::optional<SkPoint> pt = p.getLastPt();
    REPORTER_ASSERT(reporter, pt.has_value());
    REPORTER_ASSERT(reporter, pt.value() == SkPoint::Make(2, 3));
    pt = SkPathPriv::GetPoint(p, 3);
    REPORTER_ASSERT(reporter, pt.has_value());
    REPORTER_ASSERT(reporter, pt == SkPoint::Make(1, 1));
}

static void test_addEmptyPath(skiatest::Reporter* reporter, SkPath::AddPathMode mode) {
    SkPathBuilder p, q, r;
    // case 1: dst is empty
    p.moveTo(2, 1);
    p.lineTo(2, 3);
    q.addPath(p.snapshot(), mode);
    REPORTER_ASSERT(reporter, q.snapshot() == p.snapshot());
    // case 2: src is empty
    p.addPath(r.snapshot(), mode);
    REPORTER_ASSERT(reporter, q.snapshot() == p.snapshot());
    // case 3: src and dst are empty
    q.reset();
    q.addPath(r.snapshot(), mode);
    REPORTER_ASSERT(reporter, q.isEmpty());
}

/*
 *  SkPath allows the caller to "skip" calling moveTo for contours. If lineTo (or a curve) is
 *  called on an empty path, a 'moveTo(0,0)' will automatically be injected. If the path is
 *  not empty, but its last contour has been "closed", then it will inject a moveTo corresponding
 *  to where the last contour itself started (i.e. its moveTo).
 *
 *  This test exercises this in a particular case:
 *      path.moveTo(...)                <-- needed to show the bug
 *      path.moveTo....close()
 *      // at this point, the path's verbs are: M M ... C
 *
 *      path.lineTo(...)
 *      // after lineTo,  the path's verbs are: M M ... C M L
 */
static void test_addPath_and_injected_moveTo(skiatest::Reporter* reporter) {
    /*
     *  Given a path, and the expected last-point and last-move-to in it,
     *  assert that, after a lineTo(), that the injected moveTo corresponds
     *  to the expected value.
     */
    auto test_before_after_lineto = [reporter](SkPathBuilder& path,
                                               SkPoint expectedLastPt,
                                               SkPoint expectedMoveTo) {
        std::optional<SkPoint> p = SkPathPriv::GetPoint(path, path.countPoints() - 1);
        REPORTER_ASSERT(reporter, p.has_value());
        REPORTER_ASSERT(reporter, p.value() == expectedLastPt);

        const SkPoint newLineTo = {1234, 5678};
        path.lineTo(newLineTo);

        p = SkPathPriv::GetPoint(path, path.countPoints() - 2);
        REPORTER_ASSERT(reporter, p.has_value());
        REPORTER_ASSERT(reporter, p.value() == expectedMoveTo); // this was injected by lineTo()

        p = SkPathPriv::GetPoint(path, path.countPoints() - 1);
        REPORTER_ASSERT(reporter, p.has_value());
        REPORTER_ASSERT(reporter, p.value() == newLineTo);
    };

    SkPathBuilder path1;
    path1.moveTo(230, 230); // Needed to show the bug: a moveTo before the addRect
    path1.moveTo(20,30).lineTo(40,30).lineTo(40,50).lineTo(20,50);
    SkPathBuilder path1c(path1.snapshot());
    path1c.close();

    SkPathBuilder path2;
    // If path2 contains zero points, the update calculation isn't tested.
    path2.moveTo(144, 72);
    path2.lineTo(146, 72);
    SkPathBuilder path2c(path2.snapshot());
    path2c.close();
    SkPathBuilder path3(path2.snapshot());
    SkPathBuilder path3c(path2c.snapshot());

    // Test addPath, adding a path that ends with close.
    // The start point of the last contour added,
    // and the internal flag tracking whether it is closed,
    // must be updated correctly.
    path2.addPath(path1c.snapshot());
    path2c.addPath(path1c.snapshot());
    // At this point, path1c, path2, and path2c should end the same way.
    test_before_after_lineto(path1c, {20,50}, {20,30});
    test_before_after_lineto(path2, {20,50}, {20,30});
    test_before_after_lineto(path2c, {20,50}, {20,30});

    // Test addPath, adding a path not ending in close.
    path3.addPath(path1.snapshot());
    path3c.addPath(path1.snapshot());
    // At this point, path1, path3, and path3c should end the same way.
    test_before_after_lineto(path1, {20,50}, {20,50});
    test_before_after_lineto(path3, {20,50}, {20,50});
    test_before_after_lineto(path3c, {20,50}, {20,50});
}

static void test_addPath_convexity(skiatest::Reporter* reporter) {
    auto circle = SkPath::Circle(10, 10, 10);
    REPORTER_ASSERT(reporter, circle.isConvex());

#ifndef SK_HIDE_PATH_EDIT_METHODS
    auto path_add = [&](bool startWithMove, SkPath::AddPathMode mode) {
        SkPath path;
        if (startWithMove) {
            path.moveTo(0, 0);
        }
        path.addPath(circle, mode);
        return path;
    };
#endif

    auto builder_add = [&](bool startWithMove, SkPath::AddPathMode mode) {
        SkPathBuilder builder;
        if (startWithMove) {
            builder.moveTo(0, 0);
        }
        builder.addPath(circle, mode);
        return builder.detach();
    };

    const struct Expect {
        bool                fStartWithMove;
        SkPath::AddPathMode fMode;
        bool                fShouldBeConvex;
    } expectations[] = {
        { false, SkPath::AddPathMode::kAppend_AddPathMode,  true  },
        { true,  SkPath::AddPathMode::kAppend_AddPathMode,  true  },
        { false, SkPath::AddPathMode::kExtend_AddPathMode,  true  },
        { true,  SkPath::AddPathMode::kExtend_AddPathMode,  false },
    };

    for (auto e : expectations) {
        SkPath path;
#ifndef SK_HIDE_PATH_EDIT_METHODS
        path = path_add(e.fStartWithMove, e.fMode);
        REPORTER_ASSERT(reporter, path.isConvex() == e.fShouldBeConvex);
#endif
        path = builder_add(e.fStartWithMove, e.fMode);
        REPORTER_ASSERT(reporter, path.isConvex() == e.fShouldBeConvex);
    }

    SkPathBuilder pb;
    REPORTER_ASSERT(reporter, pb.snapshot().isConvex());
    // Appending to empty preserves convexity.
    pb.addPath(circle);
    REPORTER_ASSERT(reporter, pb.snapshot().isConvex());
    // Appending to non-empty should clear convexity.
    pb.addPath(circle);
    REPORTER_ASSERT(reporter, !pb.snapshot().isConvex());
}

DEF_TEST(pathbuilder_addPath, reporter) {
    const auto p = SkPathBuilder()
                   .moveTo(10, 10)
                   .lineTo(100, 10)
                   .quadTo(200, 100, 100, 200)
                   .close()
                   .moveTo(200, 200)
                   .cubicTo(210, 200, 210, 300, 200, 300)
                   .conicTo(150, 250, 100, 200, 1.4f)
                   .detach();

    REPORTER_ASSERT(reporter, p == SkPathBuilder().addPath(p).detach());

    test_addPath(reporter);
    test_addPathMode(reporter, false, false);
    test_addPathMode(reporter, true, false);
    test_addPathMode(reporter, false, true);
    test_addPathMode(reporter, true, true);
    test_extendClosedPath(reporter);
    test_addEmptyPath(reporter, SkPath::kExtend_AddPathMode);
    test_addEmptyPath(reporter, SkPath::kAppend_AddPathMode);
    test_addPath_and_injected_moveTo(reporter);

    test_addPath_convexity(reporter);
}

DEF_TEST(pathbuilder_addpath_crbug_1153516, r) {
    // When we add a closed path to another path, verify
    // that the result has the right value for last contour start point.
    SkPathBuilder p1, p2;
    p2.lineTo(10,20);
    p1.addRect({143,226,200,241});
    p2.addPath(p1.snapshot());
    p2.lineTo(262,513); // this should not assert
    SkPoint rectangleStart = {143, 226};
    SkPoint lineEnd = {262, 513};
    std::optional<SkPoint> actualMoveTo = SkPathPriv::GetPoint(p2, p2.countPoints() - 2);
    REPORTER_ASSERT(r, actualMoveTo.has_value());
    REPORTER_ASSERT(r, actualMoveTo.value() == rectangleStart );
    std::optional<SkPoint> actualLineTo = SkPathPriv::GetPoint(p2, p2.countPoints() - 1);
    REPORTER_ASSERT(r, actualLineTo.has_value());
    REPORTER_ASSERT(r, actualLineTo.value() == lineEnd);

    // Verify adding a closed path to itself
    p1.addPath(p1.snapshot());
    p1.lineTo(262,513);
    actualMoveTo = SkPathPriv::GetPoint(p1, p1.countPoints() - 2);
    REPORTER_ASSERT(r, actualMoveTo.has_value());
    REPORTER_ASSERT(r, actualMoveTo.value() == rectangleStart );
    actualLineTo = SkPathPriv::GetPoint(p1, p1.countPoints() - 1);
    REPORTER_ASSERT(r, actualLineTo.has_value());
    REPORTER_ASSERT(r, actualLineTo.value() == lineEnd);
}

/*
 *  If paths were immutable, we would not have to track this, but until that day, we need
 *  to ensure that paths are built correctly/consistently with this field, regardless of
 *  either the classic mutable apis, or via SkPathBuilder (SkPath::Polygon uses builder).
 */
DEF_TEST(pathbuilder_lastmoveindex, reporter) {
#ifndef SK_HIDE_PATH_EDIT_METHODS
    const SkPoint pts[] = {
        {0, 1}, {2, 3}, {4, 5},
    };
    const size_t N = std::size(pts);

    for (int ctrCount = 1; ctrCount < 4; ++ctrCount) {
        const int lastMoveToIndex = (ctrCount - 1) * N;

        for (bool isClosed : {false, true}) {
            SkPath a, b;

            SkPathBuilder builder;
            for (int i = 0; i < ctrCount; ++i) {
                builder.addPolygon(pts, isClosed);  // new-school way
                b.addPoly(pts, isClosed);           // old-school way
            }
            a = builder.detach();

            // We track the last moveTo verb index, and we invert it if the last verb was a close
            const int expected = isClosed ? ~lastMoveToIndex : lastMoveToIndex;
            const int a_last = SkPathPriv::LastMoveToIndex(a);
            const int b_last = SkPathPriv::LastMoveToIndex(b);

            REPORTER_ASSERT(reporter, a_last == expected);
            REPORTER_ASSERT(reporter, b_last == expected);
        }
    }
#endif
}

static void assertIsMoveTo(skiatest::Reporter* reporter, SkPathPriv::RangeIter* iter,
                           SkScalar x0, SkScalar y0) {
    auto [v, pts, w] = *(*iter)++;
    REPORTER_ASSERT(reporter, v == SkPathVerb::kMove, "%d != %d (move)",
                    (int)v, (int)SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, pts[0].fX == x0, "X mismatch %f != %f", pts[0].fX, x0);
    REPORTER_ASSERT(reporter, pts[0].fY == y0, "Y mismatch %f != %f", pts[0].fY, y0);
}

static void assertIsLineTo(skiatest::Reporter* reporter, SkPathPriv::RangeIter* iter,
                           SkScalar x1, SkScalar y1) {
    auto [v, pts, w] = *(*iter)++;
    REPORTER_ASSERT(reporter, v == SkPathVerb::kLine, "%d != %d (line)",
                    (int)v, (int)SkPathVerb::kLine);
    // pts[0] is the moveTo before this line. See pts_backset_for_verb in SkPath::RangeIter
    REPORTER_ASSERT(reporter, pts[1].fX == x1, "X mismatch %f != %f", pts[1].fX, x1);
    REPORTER_ASSERT(reporter, pts[1].fY == y1, "Y mismatch %f != %f", pts[1].fY, y1);
}

static void assertIsDone(skiatest::Reporter* reporter, SkPathPriv::RangeIter* iter, SkPath* p) {
    REPORTER_ASSERT(reporter, *iter == SkPathPriv::Iterate(*p).end(), "Iterator is not done yet");
}

DEF_TEST(SkPathBuilder_multipleMoveTos, reporter) {
    SkPathBuilder pb;
    REPORTER_ASSERT(reporter, pb.isEmpty());

    auto check_last_pt = [&](float x, float y) {
        auto lastPt = pb.getLastPt();
        REPORTER_ASSERT(reporter, lastPt.has_value());
        return *lastPt == SkPoint{x, y};
    };

    pb.moveTo(1, 2);
    REPORTER_ASSERT(reporter, pb.points().size() == 1);
    REPORTER_ASSERT(reporter, check_last_pt(1, 2));
    REPORTER_ASSERT(reporter, pb.computeBounds() == SkRect::MakeXYWH(1, 2, 0, 0));

    pb.moveTo(3, 4);
    pb.moveTo(5, 6);
    pb.moveTo(7, 8);
    REPORTER_ASSERT(reporter, pb.points().size() == 1);
    REPORTER_ASSERT(reporter, check_last_pt(7, 8));
    REPORTER_ASSERT(reporter, pb.computeBounds() == SkRect::MakeXYWH(7, 8, 0, 0));
}

DEF_TEST(SkPathBuilder_lineToMoveTo, reporter) {
    SkPathBuilder pb;
    pb.moveTo(20, 3);
    pb.lineTo(7, 11);
    pb.lineTo(8, 12);
    pb.moveTo(2, 3);
    pb.lineTo(20, 30);

    SkPath result = pb.detach();

    auto iter = SkPathPriv::Iterate(result).begin();
    assertIsMoveTo(reporter, &iter, 20, 3);
    assertIsLineTo(reporter, &iter, 7, 11);
    assertIsLineTo(reporter, &iter, 8, 12);
    assertIsMoveTo(reporter, &iter, 2, 3);
    assertIsLineTo(reporter, &iter, 20, 30);
    assertIsDone(reporter, &iter, &result);
}

DEF_TEST(SkPathBuilder_arcToPtPtRad_invalidInputsResultInALine, reporter) {
    auto test = [&](const std::string& name, SkPoint start, SkPoint end, SkScalar radius,
                    SkPoint expectedLineTo) {
        SkPathBuilder pb;
        // Remember there is an implicit moveTo(0, 0) if arcTo is the first command called.
        pb.arcTo(start, end, radius);
        SkPath result = pb.detach();

        reporter->push(name);
        auto iter = SkPathPriv::Iterate(result).begin();
        assertIsMoveTo(reporter, &iter, 0, 0);
        assertIsLineTo(reporter, &iter, expectedLineTo.fX, expectedLineTo.fY);
        assertIsDone(reporter, &iter, &result);
        reporter->pop();
    };
    // From SkPathBuilder docs:
    //   Arc is contained by tangent from last SkPath point to p1, and tangent from p1 to p2. Arc
    //   is part of circle sized to radius, positioned so it touches both tangent lines.
    // If the values cannot construct an arc, a line to the first point is constructed instead.
    test("first point equals previous point", {0, 0}, {1, 2}, 1, {0, 0});
    test("two points equal", {5, 7}, {5, 7}, 1, {5, 7});
    test("radius is zero", {-3, 5}, {-7, 11}, 0, {-3, 5});
    test("second point equals previous point", {5, 4}, {0, 0}, 1, {5, 4});
}

DEF_TEST(SkPathBuilder_assign, reporter) {
    auto check_round_trip = [reporter](const SkPath& src) {
        SkPathBuilder builder;
        builder = src;
        const SkPath dst = builder.detach();
        REPORTER_ASSERT(reporter, src == dst);
        // Our equality test doesn't look at volatility, which is probably correct, but
        // we want to ensure that our builder faithfully can reproduce the path.
        REPORTER_ASSERT(reporter, src.isVolatile() == dst.isVolatile());
    };

    const SkPoint pts[] = {{0, 0}, {1, 1}, {2, 2}};
    const bool isClosed = false; // doesn't matter for the test

    bool isVolatile = false;
    check_round_trip(SkPath::Polygon(pts, isClosed, SkPathFillType::kWinding, isVolatile));
    isVolatile = true;
    check_round_trip(SkPath::Polygon(pts, isClosed, SkPathFillType::kWinding, isVolatile));
}

DEF_TEST(SkPathBuilder_getLastPt, reporter) {
    SkPathBuilder b;
    REPORTER_ASSERT(reporter, b.getLastPt() == std::nullopt);
    b.setLastPt(10, 10);
    std::optional<SkPoint> pt = b.getLastPt();
    REPORTER_ASSERT(reporter, pt);
    REPORTER_ASSERT(reporter, pt == SkPoint::Make(10, 10));
    b.rLineTo(10, 10);
    pt = b.getLastPt();
    REPORTER_ASSERT(reporter, pt == SkPoint::Make(20, 20));
}

DEF_TEST(SkPathBuilder_transform, reporter) {
    SkPathBuilder b;

#define CONIC_PERSPECTIVE_BUG_FIXED 0
    static const SkPoint pts[] = {
        { 0, 0 },  // move
        { SkIntToScalar(10), SkIntToScalar(10) },  // line
        { SkIntToScalar(20), SkIntToScalar(10) }, { SkIntToScalar(20), 0 },  // quad
        { 0, 0 }, { 0, SkIntToScalar(10) }, { SkIntToScalar(1), SkIntToScalar(10) },  // cubic
#if CONIC_PERSPECTIVE_BUG_FIXED
        { 0, 0 }, { SkIntToScalar(20), SkIntToScalar(10) },  // conic
#endif
    };
    const int kPtCount = std::size(pts);

    b.moveTo(pts[0]);
    b.lineTo(pts[1]);
    b.quadTo(pts[2], pts[3]);
    b.cubicTo(pts[4], pts[5], pts[6]);
#if CONIC_PERSPECTIVE_BUG_FIXED
    b.conicTo(pts[4], pts[5], 0.5f);
#endif
    b.close();

    {
        SkMatrix matrix;
        matrix.reset();
        SkPath p1 = SkPathBuilder(b.snapshot()).transform(matrix).detach();
        REPORTER_ASSERT(reporter, b.snapshot() == p1);
    }


    {
        SkMatrix matrix;
        matrix.setScale(SK_Scalar1 * 2, SK_Scalar1 * 3);

        SkPath p1 = SkPathBuilder(b.snapshot()).transform(matrix).detach();
        SkSpan<const SkPoint> pts1 = p1.points();
        REPORTER_ASSERT(reporter, kPtCount == pts1.size());
        for (size_t i = 0; i < pts1.size(); ++i) {
            SkPoint newPt = SkPoint::Make(pts[i].fX * 2, pts[i].fY * 3);
            REPORTER_ASSERT(reporter, newPt == pts1[i]);
        }
    }

    {
        SkMatrix matrix;
        matrix.reset();
        matrix.setPerspX(4);

        SkPathBuilder b1 = SkPathBuilder(b.snapshot())
            .moveTo(SkPoint::Make(0, 0))
            .transform(matrix);
        REPORTER_ASSERT(reporter, matrix.invert(&matrix));
        b1.transform(matrix);
        SkRect pBounds = b.snapshot().getBounds();
        SkRect p1Bounds = b1.detach().getBounds();
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fLeft, p1Bounds.fLeft));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fTop, p1Bounds.fTop));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fRight, p1Bounds.fRight));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(pBounds.fBottom, p1Bounds.fBottom));
    }

    b.reset();
    b.addCircle(0, 0, 1, SkPathDirection::kCW);

    {
        SkMatrix matrix;
        matrix.reset();
        SkPathBuilder b1(b.snapshot());
        b1.moveTo(SkPoint::Make(0, 0));
        b1.transform(matrix);
        REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(b1.detach()) == SkPathFirstDirection::kCW);
    }


    {
        SkMatrix matrix;
        matrix.reset();
        matrix.setScaleX(-1);
        SkPathBuilder b1(b.snapshot());
        b1.moveTo(SkPoint::Make(0, 0)); // Make b1 unique (i.e., not empty path)

        b1.transform(matrix);
        REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(b1.detach()) == SkPathFirstDirection::kCCW);
    }

    {
        SkMatrix matrix;
        matrix.setAll(1, 1, 0, 1, 1, 0, 0, 0, 1);
        SkPathBuilder b1(b.snapshot());
        b1.moveTo(SkPoint::Make(0, 0)); // Make p1 unique (i.e., not empty path)

        b1.transform(matrix);
        REPORTER_ASSERT(reporter, SkPathPriv::ComputeFirstDirection(b1.snapshot()) == SkPathFirstDirection::kUnknown);
    }
}

DEF_TEST(SkPathBuilder_Path_arcTo, reporter) {
#ifndef SK_HIDE_PATH_EDIT_METHODS
    auto check_both_methods = [reporter](const SkRect& r, float start, float sweep) {
        SkPath path;
        path.arcTo(r, start, sweep, true);

        SkPathBuilder builder;
        builder.arcTo(r, start, sweep, true);

        auto bupath = builder.snapshot();
        REPORTER_ASSERT(reporter, bupath == path);
    };

    // this specific case was known to fail before
    const SkRect r = {-18, -18, 18, 18};
    float start = 375, sweep = 320;

    check_both_methods(r, start, sweep);

    // so now try a lot of other variants
    SkRandom rand;
    for (int i = 0; i < 1000; ++i) {
        start = rand.nextSScalar1() * 1000;
        sweep = rand.nextSScalar1() * 1000;
        check_both_methods(r, start, sweep);
    }
#endif
}

DEF_TEST(SkPathBuilder_cleaning, reporter) {
    // Test that we safely handle meaningless verbs, like repeated kClose
    SkPathBuilder b;
    b.moveTo(1, 2);
    b.close();
    b.close();  // this call should be silently ignored

    auto verbs = b.verbs();
    REPORTER_ASSERT(reporter, verbs.size() == 2);
    REPORTER_ASSERT(reporter, verbs[0] == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, verbs[1] == SkPathVerb::kClose);

    auto pts = b.points();
    REPORTER_ASSERT(reporter, pts.size() == 1);
    REPORTER_ASSERT(reporter, (pts[0] == SkPoint{1, 2}));
}

DEF_TEST(SkPathBuilder_path_roundtrip, reporter) {
    auto check_roundtrip = [&reporter](const SkPath& path) {
        const SkPath rpath = SkPathBuilder(path).detach();

        REPORTER_ASSERT(reporter, path == rpath);
        REPORTER_ASSERT(reporter, path.isConvex() == rpath.isConvex());

        // convexity is tricky after a (complex) transform ...
        {
            SkMatrix mx = SkMatrix::RotateDeg(30);
            SkPathBuilder bu(path);
            bu.transform(mx);
            auto bupath = bu.detach();
            SkPath copy = path.makeTransform(mx);

            SkRect r;
            bool ovals[4] = {
                path.isOval(&r),
                rpath.isOval(&r),

                copy.isOval(&r),
                bupath.isOval(&r),
            };

            REPORTER_ASSERT(reporter, ovals[0] == ovals[1]);
            REPORTER_ASSERT(reporter, ovals[2] == false);
            REPORTER_ASSERT(reporter, ovals[3] == false);

            REPORTER_ASSERT(reporter, bupath.isConvex() == copy.isConvex());
        }


        const std::optional<SkPathOvalInfo> is_oval[] = {
            SkPathPriv::IsOval(path),
            SkPathPriv::IsOval(rpath)
        };
        REPORTER_ASSERT(reporter, is_oval[0].has_value() == is_oval[1].has_value());
        if (is_oval[0] && is_oval[1]) {
            REPORTER_ASSERT(reporter, is_oval[0]->fBounds     == is_oval[1]->fBounds);
            REPORTER_ASSERT(reporter, is_oval[0]->fDirection  == is_oval[1]->fDirection);
            REPORTER_ASSERT(reporter, is_oval[0]->fStartIndex == is_oval[1]->fStartIndex);
        }

        const std::optional<SkPathRRectInfo> is_rrect[] = {
            SkPathPriv::IsRRect(path),
            SkPathPriv::IsRRect(rpath)
        };
        REPORTER_ASSERT(reporter, is_rrect[0].has_value() == is_rrect[1].has_value());
        if (is_rrect[0] && is_rrect[1]) {
            REPORTER_ASSERT(reporter, is_rrect[0]->fRRect      == is_rrect[1]->fRRect);
            REPORTER_ASSERT(reporter, is_rrect[0]->fDirection  == is_rrect[1]->fDirection);
            REPORTER_ASSERT(reporter, is_rrect[0]->fStartIndex == is_rrect[1]->fStartIndex);
        }
    };

    check_roundtrip(SkPath());
    check_roundtrip(SkPath::Circle(10, 20, 30, SkPathDirection::kCCW));
    check_roundtrip(SkPath::Oval({10, 20, 30, 40}, SkPathDirection::kCCW, 2));
    check_roundtrip(SkPath::Rect({10, 20, 30, 40}, SkPathDirection::kCCW, 2));
    check_roundtrip(SkPath::RRect({10, 20, 30, 40}, 1, 2, SkPathDirection::kCCW));
    check_roundtrip(SkPathBuilder()
                      .lineTo(100, 0)
                      .quadTo({0, 0}, {0, 100})
                      .close()
                      .detach());
}

static void check_move(skiatest::Reporter* reporter, SkPathIter* iter,
                       SkScalar x0, SkScalar y0) {
    auto rec = iter->next().value();
    REPORTER_ASSERT(reporter, rec.fVerb == SkPathVerb::kMove);
    REPORTER_ASSERT(reporter, rec.fPoints[0].fX == x0);
    REPORTER_ASSERT(reporter, rec.fPoints[0].fY == y0);
}

static void check_line(skiatest::Reporter* reporter, SkPathIter* iter,
                       SkScalar x1, SkScalar y1) {
    auto rec = iter->next().value();
    REPORTER_ASSERT(reporter, rec.fVerb == SkPathVerb::kLine);
    REPORTER_ASSERT(reporter, rec.fPoints[1].fX == x1);
    REPORTER_ASSERT(reporter, rec.fPoints[1].fY == y1);
}

static void check_close(skiatest::Reporter* reporter, SkPathIter* iter) {
    auto rec = iter->next().value();
    REPORTER_ASSERT(reporter, rec.fVerb == SkPathVerb::kClose);
}

static void check_done(skiatest::Reporter* reporter, SkPathBuilder* p, SkPathIter* iter) {
    REPORTER_ASSERT(reporter, !iter->next().has_value());
}

static void check_done_and_reset(skiatest::Reporter* reporter, SkPathBuilder* p,
                                 SkPathIter* iter) {
    check_done(reporter, p, iter);
    p->reset();
}

DEF_TEST(SkPathBuilder_rMoveTo, reporter) {
    SkPathBuilder p;
    p.moveTo(10, 11);
    p.lineTo(20, 21);
    p.close();
    p.rMoveTo({30, 31});
    p.lineTo(30, 40);
    SkPathIter iter(p.points(), p.verbs(), {} /* no conics */);
    check_move(reporter, &iter, 10, 11);
    check_line(reporter, &iter, 20, 21);
    check_close(reporter, &iter);
    check_move(reporter, &iter, 10 + 30, 11 + 31);
    check_line(reporter, &iter, 30, 40);
    check_done_and_reset(reporter, &p, &iter);

    p.moveTo(10, 11);
    p.lineTo(20, 21);
    p.rMoveTo(30, 31);
    p.lineTo(30, 40);
    iter = p.iter();    //(p.points(), p.verbs(), {} /* no conics */);
    check_move(reporter, &iter, 10, 11);
    check_line(reporter, &iter, 20, 21);
    check_move(reporter, &iter, 20 + 30, 21 + 31);
    check_line(reporter, &iter, 30, 40);
    check_done_and_reset(reporter, &p, &iter);

    p.rMoveTo({30, 31});
    iter = p.iter();//SkPathRaw::Iter(p.points(), p.verbs(), {} /* no conics */);
    //  PathIter, for compat, is snuffing out trailing moves
    check_done_and_reset(reporter, &p, &iter);
}

const SkPathFillType gFillTypes[] = {
    SkPathFillType::kWinding,
    SkPathFillType::kEvenOdd,
    SkPathFillType::kInverseWinding,
    SkPathFillType::kInverseEvenOdd,
};

DEF_TEST(SkPathBuilder_equality, reporter) {
    auto check_filltype_eq = [reporter](const SkPathBuilder& a) {
        SkPathBuilder copy = a;
        REPORTER_ASSERT(reporter, a == copy);

        for (auto ft : gFillTypes) {
            if (ft != a.fillType()) {
                copy.setFillType(ft);
                REPORTER_ASSERT(reporter, a != copy);
            }
        }
    };


    SkPathBuilder a, b;

    REPORTER_ASSERT(reporter, a == b);
    check_filltype_eq(a);

    a.moveTo(0, 0);
    REPORTER_ASSERT(reporter, a != b);
    b.moveTo(0, 0);
    REPORTER_ASSERT(reporter, a == b);
    check_filltype_eq(a);

    b.close();
    REPORTER_ASSERT(reporter, a != b);
    a.close();
    REPORTER_ASSERT(reporter, a == b);
    check_filltype_eq(a);

    auto set_segments = [](SkPathBuilder& bu) {
        bu.reset()
          .moveTo(1, 2)
          .lineTo(3, 4)
          .quadTo(5, 6, 7, 8)
          .conicTo(9, 10, 11, 12, 0.5f)
          .cubicTo(13, 14, 15, 16, 17, 18)
          .close();
    };
    set_segments(a);
    set_segments(b);
    REPORTER_ASSERT(reporter, a == b);
    check_filltype_eq(a);

    // mutate point value, but not verb sequence
    a.setLastPt(-1, -2);
    REPORTER_ASSERT(reporter, a != b);
    check_filltype_eq(a);
}

DEF_TEST(SkPathBuilder_dump, reporter) {
    SkPathBuilder builder;
    builder.moveTo(1, 2)
            .lineTo(3, 4)
            .quadTo(5, 6, 7, 8)
            .conicTo(9, 10, 11, 12, 0.5f)
            .cubicTo(13, 14, 15, 16, 17, 18)
            .close()
            .moveTo(1, 2)
            .lineTo(3, 4);

    SkString str = builder.dumpToString();

    const char expected[] =
        "SkPathBuilder(SkPathFillType::kWinding)\n"
        ".moveTo(1, 2)\n"
        ".lineTo(3, 4)\n"
        ".quadTo(5, 6, 7, 8)\n"
        ".conicTo(9, 10, 11, 12, 0.5f)\n"
        ".cubicTo(13, 14, 15, 16, 17, 18)\n"
        ".close()\n"
        ".moveTo(1, 2)\n"
        ".lineTo(3, 4)\n";

    REPORTER_ASSERT(reporter, str.equals(expected));
}
