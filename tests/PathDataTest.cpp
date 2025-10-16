/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"
#include "src/core/SkPathData.h"
#include "src/core/SkPathPriv.h"

#include "tests/Test.h"

#include <limits>

namespace {
template <typename T> bool spaneq(SkSpan<T> a, SkSpan<T> b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}
}

DEF_TEST(pathdata_empty, reporter) {
    auto pdata = SkPathData::Empty();

    REPORTER_ASSERT(reporter, pdata->empty());
    REPORTER_ASSERT(reporter, pdata->points().empty());
    REPORTER_ASSERT(reporter, pdata->conics().empty());
    REPORTER_ASSERT(reporter, pdata->verbs().empty());

    REPORTER_ASSERT(reporter, pdata->bounds() == SkRect::MakeEmpty());
    REPORTER_ASSERT(reporter, pdata->segmentMask() == 0);

    REPORTER_ASSERT(reporter, !pdata->asLine());
    REPORTER_ASSERT(reporter, !pdata->asOval());
    REPORTER_ASSERT(reporter, !pdata->asRect());
    REPORTER_ASSERT(reporter, !pdata->asRRect());

    auto xformed = pdata->makeTransform(SkMatrix::Scale(2, 3));
    REPORTER_ASSERT(reporter, *pdata == *xformed);
}

using IsAPredicate = bool(const SkPathData&);

static void check_asA_transforms(skiatest::Reporter* reporter, sk_sp<SkPathData> orig,
                                 IsAPredicate returns_asA) {
    SkASSERT(returns_asA(*orig));

    const struct {
        SkMatrix mx;
        bool     expectedAsA;
    } gPairs[] = {
        { SkMatrix::I(),             true },
        { SkMatrix::Translate(1, 2), true },
        { SkMatrix::Scale(2, 3),     true },
        { SkMatrix::RotateDeg(30),  false },
    };

    for (const auto& pair : gPairs) {
        auto pdata = orig->makeTransform(pair.mx);
        REPORTER_ASSERT(reporter, returns_asA(*pdata) == pair.expectedAsA);
    }
}

/*
 *  Differe ways to "make" a rectangular PathData
 */
using RectMaker = sk_sp<SkPathData>(const SkRect&, SkPathDirection);

static sk_sp<SkPathData> factory_rect(const SkRect& r, SkPathDirection d) {
    return SkPathData::Rect(r, d);
}
static sk_sp<SkPathData> poly4_rect(const SkRect& r, SkPathDirection d) {
    std::array<SkPoint, 4> pts = r.toQuad(d);
    return SkPathData::Polygon(pts, true);
}
static sk_sp<SkPathData> poly5_rect(const SkRect& r, SkPathDirection d) {
    std::array<SkPoint, 5> pts;
    r.copyToQuad(pts, d);
    pts[4] = pts[0];    // explicly add the closing line
    return SkPathData::Polygon(pts, true);
}
static sk_sp<SkPathData> builder_rect_rect(const SkRect& r, SkPathDirection d) {
    SkPathBuilder bu;
    bu.addRect(r, d);
    return bu.detachData();
}
static sk_sp<SkPathData> builder_poly4_rect(const SkRect& r, SkPathDirection d) {
    std::array<SkPoint, 4> pts = r.toQuad(d);
    SkPathBuilder bu;
    bu.addPolygon(pts, true);
    return bu.detachData();
}
static sk_sp<SkPathData> builder_poly5_rect(const SkRect& r, SkPathDirection d) {
    std::array<SkPoint, 5> pts;
    r.copyToQuad(pts, d);
    pts[4] = pts[0];    // explicly add the closing line
    SkPathBuilder bu;
    bu.addPolygon(pts, true);
    return bu.detachData();
}

RectMaker* const gRectMakers[] = {
    factory_rect,
    poly4_rect,
    poly5_rect,
    builder_rect_rect,
    builder_poly4_rect,
    builder_poly5_rect,
};

DEF_TEST(pathdata_rect, reporter) {
    const SkRect r = {1, 2, 3, 4};

    auto sign = [](float x) -> float {
        if (x == 0) {
            return 0;
        } else {
            return x > 0 ? 1 : -1;
        }
    };

    for (auto maker : gRectMakers) {
        for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            auto pdata = maker(r, dir);
            REPORTER_ASSERT(reporter, r == pdata->bounds());

            // 1. manually determine if *we* think it is a rect

            const SkSpan<const SkPoint> pts = pdata->points();
            REPORTER_ASSERT(reporter, r == SkRect::Bounds(pts).value());

            const float crossSign = (dir == SkPathDirection::kCW) ? 1 : -1;
            for (int i = 1; i < 3; ++i) {
                SkVector u = pts[i]   - pts[i-1],
                         v = pts[i+1] - pts[i];
                const float cross = u.cross(v),
                            dot   = u.dot(v);
                REPORTER_ASSERT(reporter, dot == 0);
                REPORTER_ASSERT(reporter, crossSign == sign(cross));
            }

            // 2. now ask the pathdata

            auto isa = pdata->asRect();
            REPORTER_ASSERT(reporter, isa.has_value());
            REPORTER_ASSERT(reporter, isa->fRect == r);
            REPORTER_ASSERT(reporter, isa->fDirection == dir);
            REPORTER_ASSERT(reporter, isa->fStartIndex == 0);

            check_asA_transforms(reporter, pdata, [](const SkPathData& pd) {
                return pd.asRect().has_value();
            });
        }
    }
}

static sk_sp<SkPathData> factory_poly(SkSpan<const SkPoint> pts, bool isClosed) {
    return SkPathData::Polygon(pts, isClosed);
}
static sk_sp<SkPathData> builder_poly(SkSpan<const SkPoint> pts, bool isClosed) {
    SkPathBuilder bu;
    bu.addPolygon(pts, isClosed);
    return bu.detachData();
}

DEF_TEST(pathdata_polygon, reporter) {
    const SkPoint points[] = {
        {0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9},
    };

    for (auto maker : {factory_poly, builder_poly}) {
        for (auto isClosed : {false, true}) {
            for (size_t n = 0; n <= std::size(points); ++n) {
                const SkSpan<const SkPoint> pts = {points, n};
                auto pdata = maker(pts, isClosed);

                const bool shouldBeEmpty = isClosed ? n == 0 : n <= 1;
                if (shouldBeEmpty) {
                    REPORTER_ASSERT(reporter, pdata->empty());
                    continue;
                }

                REPORTER_ASSERT(reporter, spaneq(pdata->points(), pts));
                REPORTER_ASSERT(reporter, pdata->conics().empty());

                auto line = pdata->asLine();
                if (n == 2 && !isClosed) {
                    REPORTER_ASSERT(reporter, line.has_value());
                    REPORTER_ASSERT(reporter, line.value()[0] == points[0]);
                    REPORTER_ASSERT(reporter, line.value()[1] == points[1]);

                    auto pline = SkPathData::Line(points[0], points[1]);
                    REPORTER_ASSERT(reporter, *pline == *pdata);
                } else {
                    REPORTER_ASSERT(reporter, !line.has_value());
                }

                const size_t expectedVerbs = pts.size() + isClosed;
                auto vbs = pdata->verbs();
                REPORTER_ASSERT(reporter, vbs.size() == expectedVerbs);

                REPORTER_ASSERT(reporter, vbs[0] == SkPathVerb::kMove);
                for (size_t i = 1; i < pts.size(); ++i) {
                    REPORTER_ASSERT(reporter, vbs[i] == SkPathVerb::kLine);
                }
                if (isClosed) {
                    REPORTER_ASSERT(reporter, vbs.back() == SkPathVerb::kClose);
                }
            }
        }
    }
}

static sk_sp<SkPathData> factory_oval(const SkRect& r, SkPathDirection dir, unsigned start) {
    return SkPathData::Oval(r, dir, start);
}
static sk_sp<SkPathData> builder_oval(const SkRect& r, SkPathDirection dir, unsigned start) {
    SkPathBuilder bu;
    bu.addOval(r, dir, start);
    return bu.detachData();
}

DEF_TEST(pathdata_oval, reporter) {
    const SkRect bounds = {1, 2, 3, 4};
    const unsigned kStartIndexCount = 4;

    for (auto maker : {factory_oval, builder_oval}) {
        for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            for (unsigned start = 0; start < kStartIndexCount; ++start) {
                auto pdata = maker(bounds, dir, start);

                REPORTER_ASSERT(reporter, pdata->bounds() == bounds);

                auto oval = pdata->asOval();
                REPORTER_ASSERT(reporter, oval.has_value());
                REPORTER_ASSERT(reporter, oval->fBounds == bounds);
                REPORTER_ASSERT(reporter, oval->fDirection == dir);
                REPORTER_ASSERT(reporter, oval->fStartIndex == start);

                check_asA_transforms(reporter, pdata, [](const SkPathData& pd) {
                    return pd.asOval().has_value();
                });
            }
        }
    }
}

static sk_sp<SkPathData> factory_rrect(const SkRRect& r, SkPathDirection dir, unsigned start) {
    return SkPathData::RRect(r, dir, start);
}
static sk_sp<SkPathData> builder_rrect(const SkRRect& r, SkPathDirection dir, unsigned start) {
    SkPathBuilder bu;
    bu.addRRect(r, dir, start);
    return bu.detachData();
}

DEF_TEST(pathdata_rrect, reporter) {
    const SkRect bounds = {0, 0, 20, 30};
    const SkRRect rrect = SkRRect::MakeRectXY(bounds, 2, 3);
    const unsigned kStartIndexCount = 8;

    for (auto maker : {factory_rrect, builder_rrect}) {
        for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            for (unsigned start = 0; start < kStartIndexCount; ++start) {
                auto pdata = maker(rrect, dir, start);

                REPORTER_ASSERT(reporter, pdata->bounds() == bounds);

                auto rr = pdata->asRRect();
                REPORTER_ASSERT(reporter, rr.has_value());
                REPORTER_ASSERT(reporter, rr->fRRect == rrect);
                REPORTER_ASSERT(reporter, rr->fDirection == dir);
                REPORTER_ASSERT(reporter, rr->fStartIndex == start);

                check_asA_transforms(reporter, pdata, [](const SkPathData& pd) {
                    return pd.asRRect().has_value();
                });
            }
        }
    }
}

DEF_TEST(pathdata_make_edgecases, reporter) {

    // just create some points for our tests
    SkPoint pts[20];
    for (size_t i = 0; i < std::size(pts); ++i) {
        pts[i] = {i * 1.0f, i * 1.0f };
    }
    const float conicWeights[] = {1.5f, 2, 3};

    constexpr SkPathVerb M = SkPathVerb::kMove,
                         L = SkPathVerb::kLine,
                         Q = SkPathVerb::kQuad,
                         K = SkPathVerb::kConic,
                         C = SkPathVerb::kCubic,
                         X = SkPathVerb::kClose;

    // only these two sequence will result in an "empty" PathData

    const SkPathVerb empty[] = { M };  // the M will be trimmed

    REPORTER_ASSERT(reporter, SkPathData::Make({}, {empty, 0}, {})->empty());
    REPORTER_ASSERT(reporter, SkPathData::Make({pts, 1}, empty, {})->empty());

    // these sequenes are all illegal (bad verb sequencing)

    const SkPathVerb bad0[] = { L };            // didn't start with M
    const SkPathVerb bad1[] = { M, M, L };      // consecutive Ms
    const SkPathVerb bad2[] = { M, L, X, X};    // consecutive Xs
    const SkPathVerb bad3[] = { M, L, M, M};    // consecutive Ms

    REPORTER_ASSERT(reporter, SkPathData::Make({pts, 1}, bad0, {}) == nullptr);
    REPORTER_ASSERT(reporter, SkPathData::Make({pts, 3}, bad1, {}) == nullptr);
    REPORTER_ASSERT(reporter, SkPathData::Make({pts, 2}, bad2, {}) == nullptr);
    REPORTER_ASSERT(reporter, SkPathData::Make({pts, 4}, bad3, {}) == nullptr);

    // Odd but legal, the trailing M will be removed

    const SkPathVerb trimmed[] = { M, L, M }; //legal, but will trim the last M
    auto pdata = SkPathData::Make({pts, 3}, trimmed, {});

    REPORTER_ASSERT(reporter, pdata->points().size() == 2);
    REPORTER_ASSERT(reporter, pdata->verbs().size() == 2);

    // Now check on # of points and conic weights

    const SkPathVerb verbs[] = { M, L, Q, K, C, X };    // 1+1+2+2+3 = 9 + 1 conic weight

    const struct {
        size_t nPts, nConics;
        bool success;
    } combos[] = {
        {  9, 1, true },    // just right
        {  8, 1, false },   // not enough points
        { 10, 1, false },   // too many points
        {  9, 0, false },   // not enough conics
        {  9, 2, false },   // too many conics
    };
    for (auto c : combos) {
        pdata = SkPathData::Make({pts, c.nPts}, verbs, {conicWeights, c.nConics});
        if (c.success) {
            REPORTER_ASSERT(reporter, pdata != nullptr);
        } else {
            REPORTER_ASSERT(reporter, pdata == nullptr);
        }
    }
}

static inline std::optional<SkRRect> make_bad_rrect() {
    constexpr float big = std::numeric_limits<float>::max();
    SkRRect rr = SkRRect::MakeRectXY({0, 0, big, big}, 4, 4);
    rr.offset(big, big);
    if (!rr.rect().isFinite()) {
        return rr;
    }
    return {};  // failed to make a non-finite rrect
}

/*
 *  Test that we cannot make a non-finite PathData
 */
DEF_TEST(pathdata_make_nonfinite, reporter) {
    const float inf = SK_FloatInfinity;

    SkPoint pts[] = {
        {0, 0}, {inf, 1}, {2, 4},
    };
    SkPathVerb vbs[] = {
        SkPathVerb::kMove, SkPathVerb::kConic,
    };
    float weights[] = { 2 };

    sk_sp<SkPathData> pdata = SkPathData::Make(pts, vbs, weights);
    REPORTER_ASSERT(reporter, pdata == nullptr);

    pts[1].fX = 3;  // remove non-finite from pts
    const float badWValues[] = { -1, inf, -inf, inf * 0 /* nan */ };
    for (auto bad : badWValues) {
        weights[0] = bad;
        REPORTER_ASSERT(reporter, SkPathData::Make(pts, vbs, weights) == nullptr);
    }

    SkRect r = {1, 2, inf, 4};
    REPORTER_ASSERT(reporter, SkPathData::Rect(r) == nullptr);
    REPORTER_ASSERT(reporter, SkPathData::Oval(r) == nullptr);

    // Most RRect methods 'sanitize' the values before returning the RRect, so it hard to
    // actually make one for testing. If our attempt suceeds, we will test with it.
    if (auto rr = make_bad_rrect()) {
        REPORTER_ASSERT(reporter, SkPathData::RRect(*rr) == nullptr);
    }

    pts[1].fX = inf;  // restore non-finite value
    REPORTER_ASSERT(reporter, SkPathData::Polygon(pts, false) == nullptr);
}

DEF_TEST(pathdata_transform, reporter) {
    SkMatrix mx;
    const SkRect r = {10, 20, 30, 40};
    auto data = SkPathData::Oval(r);

    mx = SkMatrix::I();
    auto newd = data->makeTransform(mx);
    REPORTER_ASSERT(reporter, *newd == *data);

    mx = SkMatrix::Translate(5, 6);
    newd = data->makeTransform(mx);
    REPORTER_ASSERT(reporter, newd->bounds() == r.makeOffset(5, 6));

    mx = SkMatrix::Scale(0.5f, 2);
    newd = data->makeTransform(mx);
    SkRect r2 = {
        r.fLeft * 0.5f,
        r.fTop * 2,
        r.fRight * 0.5f,
        r.fBottom * 2,
    };
    REPORTER_ASSERT(reporter, newd->bounds() == r2);

    mx = SkMatrix::Scale(SK_FloatInfinity, 2);
    newd = data->makeTransform(mx);
    REPORTER_ASSERT(reporter, newd == nullptr);

    mx = SkMatrix::Scale(SK_ScalarNaN, 2);
    newd = data->makeTransform(mx);
    REPORTER_ASSERT(reporter, newd == nullptr);
}

/*
 *  This tests how convexity is tracked under transformation
 *  1. unknown stays unknown (we don't actively compute convexity)
 *  2. concave stays concave
 *  3. convex ... may stay convex -- it depends if we feel it is (numerically) safe.
 *     See SkPathPriv::TransformConvexity() for the current heuristics.
 *  4. The (above) helper is shared with SkPath::transform(), so it and SkPathData
 *     should handle transforms + convexity the same.
 */
DEF_TEST(pathdata_transform_convexity, reporter) {
    const SkPoint pts[] = {
        {0, 0}, {100, 0}, {200, 0}, {200, 200},
    };
    // needed late for our assumpts about convexity preservation
    REPORTER_ASSERT(reporter, SkPathPriv::IsAxisAligned(pts));

    auto src = SkPathData::Polygon(pts, true);
    auto convexity = SkPathPriv::GetConvexityOrUnknown(*src);

    // don't do any work we didn't ask for
    REPORTER_ASSERT(reporter, convexity == SkPathConvexity::kUnknown);
    auto raw = src->raw(SkPathFillType::kDefault, SkResolveConvexity::kNo);
    REPORTER_ASSERT(reporter, raw.fConvexity == SkPathConvexity::kUnknown);
    // now ask for it
    raw = src->raw(SkPathFillType::kDefault, SkResolveConvexity::kYes);
    REPORTER_ASSERT(reporter, raw.isKnownToBeConvex());

    // For these matrices, given that our points are axis-aligned, we should be able
    // to preserve whatever convexity our src has.

    const SkMatrix safeMatrices[] = {
        SkMatrix(), SkMatrix::Translate(1, 2), SkMatrix::Scale(2, 3),
    };
    const SkPathConvexity convexities[] = {
        SkPathConvexity::kUnknown,
        SkPathConvexity::kConvex_CW,    // matches our test data
        SkPathConvexity::kConcave,
    };
    for (const auto& mx : safeMatrices) {
        for (auto conv : convexities) {
            raw.fConvexity = conv;
            auto dst = SkPathData::MakeTransform(raw, mx);
            convexity = SkPathPriv::GetConvexityOrUnknown(*dst);
            REPORTER_ASSERT(reporter, convexity == conv);
        }
    }

    // for this matrix, we do not expect to preserve convexity
    // (since we don't choose to actually compute convexity at this stage)
    SkMatrix mx = SkMatrix::RotateDeg(30);
    for (auto conv : convexities) {
        raw.fConvexity = conv;
        auto dst = SkPathData::MakeTransform(raw, mx);
        convexity = SkPathPriv::GetConvexityOrUnknown(*dst);
        auto expected = SkPathConvexity_IsConvex(conv) ? SkPathConvexity::kUnknown
                                                       : conv;
        REPORTER_ASSERT(reporter, convexity == expected);
    }
}
