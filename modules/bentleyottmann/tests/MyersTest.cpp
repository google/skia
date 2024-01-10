// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Myers.h"

#include "src/base/SkRandom.h"
#include "tests/Test.h"

#include <chrono>
#include <cinttypes>
#include <cstdint>

namespace myers {
bool slope_s0_less_than_slope_s1(const Segment& s0, const Segment& s1);
bool segment_less_than_upper_to_insert(const Segment& segment, const Segment& to_insert);
bool s0_less_than_s1_at_y(const Segment& s0, const Segment& s1, int32_t y);
bool s0_intersects_s1(const Segment& s0, const Segment& s1);
}  // namespace myers

using namespace myers;

static bool operator==(std::pair<const Point &, const Point &> l, std::tuple<Point, Point> r) {
    return std::get<0>(l) == std::get<0>(r) && std::get<1>(l) == std::get<1>(r);
}

DEF_TEST(MFC_order_segment_points, r) {
    {
        Point p0 = {0, 0},
              p1 = {1, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
    {
        Point p0 = {0, 0},
              p1 = {-1, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
    {
        Point p0 = {0, 0},
              p1 = {0, 1};
        REPORTER_ASSERT(r, std::minmax(p0, p1) == std::make_tuple(p0, p1));
        REPORTER_ASSERT(r, std::minmax(p1, p0) == std::make_tuple(p0, p1));
    }
}

DEF_TEST(MFC_segment_ctor, r) {
    {
        Point p0 = {0, 0},
              p1 = {1, 1};
        Segment s = {p1, p0};
        const auto [u, l] = s;
        REPORTER_ASSERT(r, u == s.upper() && u == p0);
        REPORTER_ASSERT(r, l == s.lower() && l == p1);
    }

    {
        Point p0 = {0, 0},
              p1 = {0, 1};
        Segment s = {p1, p0};
        const auto [u, l] = s;
        REPORTER_ASSERT(r, u == s.upper() && u == p0);
        REPORTER_ASSERT(r, l == s.lower() && l == p1);
    }
}

DEF_TEST(MFC_slope_less_than, r) {
    {
        Segment s0 = {{0, 0}, {1, 1}},
                s1 = {{0, 0}, {-1, 1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s1));
        REPORTER_ASSERT(r, slope_s0_less_than_slope_s1(s1, s0));
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s0));
    }
    {
        Segment s = {{0, 0}, {0,1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s, s));
    }
    {  // Check slopes for horizontals.
        Segment s0 = {{-2, 0}, {1, 0}},
                s1 = {{-1, 0}, {2, 0}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s1));
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s1, s0));
    }
    {  // Check slopes for horizontals.
        Segment s0 = {{-2, 0}, {1, 0}},
                s1 = {{0, 0}, {1, 1}};
        REPORTER_ASSERT(r, !slope_s0_less_than_slope_s1(s0, s1));
        REPORTER_ASSERT(r, slope_s0_less_than_slope_s1(s1, s0));
    }
}

DEF_TEST(MFC_segment_less_than_upper_to_insert, r) {
    Segment s0 = {{-10, -10}, {10, 10}},
            s1 = {{10, -10}, {-10, 10}},
            to_insert = {{0, 0}, {0, 3}};

    // Above y = 0, the sweepLine is {s0, s1}, but at y=0 s1 and s0 swap because of their slopes.
    std::vector<Segment> sweepLine = {s1, s0};

    auto insertionPoint = std::lower_bound(sweepLine.begin(), sweepLine.end(), to_insert,
                                           segment_less_than_upper_to_insert);

    // The insertion point is between s1 and s0.
    REPORTER_ASSERT(r, *insertionPoint == s0);
    REPORTER_ASSERT(r, *(insertionPoint-1) == s1);
}

DEF_TEST(MFC_less_than_at_y, r) {
    {
        Segment s0 = {{0, 0}, {2, 2}},
                s1 = {{0, 0}, {-2, 2}};
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 1));
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s1, s0, 1));
    }
    {  // cross at 0 use slope to break tie.
        Segment s0 = {{-2, -2}, {2, 2}},
                s1 = {{2, -2}, {-2, 2}};
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s0, s1, -1));
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s1, s0, -1));
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 0));
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s1, s0, 0));
        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 1));
        REPORTER_ASSERT(r, s0_less_than_s1_at_y(s1, s0, 1));
    }
    {
        Segment s0 = {{-2, -100}, {-2, 89}},
                s1 = {{6, -70}, {-2, 72}};

        REPORTER_ASSERT(r, !s0_less_than_s1_at_y(s0, s1, 72));
    }
}

static Segment swap_ends(const Segment& s) {
    return {s.lower(), s.upper()};
}

DEF_TEST(MFC_has_inner_intersection, r) {
    auto checkIntersection = [&](Segment s0, Segment s1) {
        REPORTER_ASSERT(r, s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, s0_intersects_s1(s1, s0));
        REPORTER_ASSERT(r, s0_intersects_s1(swap_ends(s0), swap_ends(s1)));
        REPORTER_ASSERT(r, s0_intersects_s1(swap_ends(s1), swap_ends(s0)));
    };

    {
        Segment s0 = {{-1, 0}, {1,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        checkIntersection(s0, s1);
    }
    {
        Segment s0 = {{-1, 0}, {5,  0}},
                s1 = {{ 0, 1}, {0, -1}};

        checkIntersection(s0, s1);
    }

    {
        Segment s0 = {{5, 0}, {-1,  0}},
                s1 = {{ 0, -1}, {0, 1}};

        checkIntersection(s0, s1);
    }

    {
        Segment s0 = {{-5, -5}, {5, 5}},
                s1 = {{-5, 5}, {5, -5}};

        checkIntersection(s0, s1);
    }

    // Test very close segments (x0, 0) -> (x1, 1) & (x2, 0) -> (x3, 1)
    for (int32_t x0 = -10; x0 <= 10; x0++) {
        for (int32_t x1 = -10; x1 <= 10; x1++) {
            for (int32_t x2 = -10; x2 <= 10; x2++) {
                for (int32_t x3 = -10; x3 <= 10; x3++) {
                    Point P0 = {x0, 0},
                          P1 = {x1, 1},
                          P2 = {x2, 0},
                          P3 = {x3, 1};
                    bool actual = s0_intersects_s1({P0, P1}, {P2, P3});
                    bool expected = (x0 <= x2 && x3 <= x1) || (x2 <= x0 && x1 <= x3);
                    if (actual != expected) {
                        s0_intersects_s1({P0, P1}, {P2, P3});
                        REPORTER_ASSERT(r, actual == expected);
                    }
                }
            }
        }
    }

    {
        Segment s0 = {{0, -100}, {0, -50}},
                s1 = {{100, -100}, {-100, 100}};  // goes through (0,0)
        REPORTER_ASSERT(r, !s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, !s0_intersects_s1(s1, s0));
    }
    {
        Segment s0 = {{0, 100}, {0, 50}},
                s1 = {{100, -100}, {-100, 100}};  // goes through (0,0)
        REPORTER_ASSERT(r, !s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, !s0_intersects_s1(s1, s0));
    }
    {
        Segment s0 = {{0, -101}, {0, -50}},
                s1 = {{100, -100}, {-100, 100}};  // goes through (0,0)
        REPORTER_ASSERT(r, !s0_intersects_s1(s0, s1));
        REPORTER_ASSERT(r, !s0_intersects_s1(s1, s0));
    }
}

DEF_TEST(MFC_myers_brute_force_comparison, r) {
    const std::vector<Segment> tests[] = {
            {{{-58, -100}, {75, 105}}, {{149, -58}, {-156, 49}}, {{-34, -55}, {37, 49}}, {{-58, -100}, {75, 105}}, {{-147, -229}, {143, 220}}},
            {{{-57, -138}, {56, 178}}, {{14, -146}, {-22, 132}}},
            {{{-4, -23}, {-11, 11}}, {{6, -2}, {-11, 11}}, {{159, -244}, {-159, 233}}},
            {{{-7, -22}, {10, 14}}, {{-7, -71}, {-7, 80}}, {{-7, -22}, {-4, 5}}},
            {{{91, -22}, {-93, 24}}, {{31, -18}, {-25, 7}}, {{-25, 7}, {33, 12}}, {{-26, -24}, {18, 20}}},
            {{{2, -21}, {-16, 7}}, {{-45, -28}, {51, 35}}, {{39, -48}, {-53, 44}}, {{-16, 7}, {26, 7}}},
            {{{142, -82}, {-128, 64}}, {{208, -16}, {-217, -3}}, {{91, -22}, {-93, 24}}, {{31, -18}, {-25, 7}}, {{-25, 7}, {33, 12}}},
            {{{-159, -101}, {167, 91}}, {{-96, -117}, {99, 117}}, {{-16, -21}, {12, 35}}, {{-48, -55}, {33, 63}}, {{-16, -21}, {26, 41}}},
            {{{-51, -18}, {34, 1}}, {{189, -169}, {-171, 150}}, {{24, -8}, {-5, 7}}, {{24, -8}, {-26, 16}}, {{54, -22}, {-36, 20}}},
            {{{-29, -3}, {15, -3}}, {{-28, -7}, {15, -3}}},
            {{{20, -149}, {-32, 130}}, {{-29, -3}, {15, -3}}, {{-28, -7}, {15, -3}}},
            {{{-32, -8}, {16, -8}}, {{-28, -104}, {23, 88}}, {{-17, -11}, {16, -8}}},
            {{{-59, -9}, {48, 11}}, {{-59, -9}, {75, -9}}, {{173, -20}, {-178, 13}}},
            {{{-11, 1}, {12, 1}}, {{-42, -35}, {54, 29}}},
            {{{14, -11}, {-15, -2}}, {{-9, -2}, {13, -2}}}, // both end same s0 horz s1 < s0
            {{{-38, 7}, {47, 7}}, {{-148, 6}, {166, 7}}},  // just sort of s0 along s1
            {{{-26, -22}, {9, 21}}, {{-32, -28}, {13, 17}}}, // s1 endpoint on s0
            {{{23, -2}, {-12, 3}}, {{22, -13}, {-5, 2}}},   // s1 endpoint on s0
            {{{-2, -100}, {-2, 89}}, {{6, -70}, {-2, 72}}},
            {{{8, -1}, {-8, 19}}, {{-130, -93}, {137, 85}}},  // Endpoint of s0 lies on s1
            {{{-39, -111}, {25, 119}}, {{-26, -112}, {25, 119}}}, // Same end points
            {{{-9, -5}, {16, -5}}, {{90, -134}, {-71, 144}}},  // Diag crossing horizontal
            {{{-1, -1}, {1, 1}}, {{1, -1}, {-1, 1}}},  // Crossing
            {{{-1, -1}, {-1, 1}}, {{1, -1}, {1, 1}}},  // Two vertical lines
            {{{-1, -1}, {1, -1}}, {{-1, 1}, {1, 1}}},  // Two horizontal lines
            {{{-2, 1}, {1, 1}}, {{-1, 1}, {2, 1}}},  // Overlapping horizontal lines
            {{{0, -100}, {0, -50}}, {{100, -100}, {-100, 100}}},
            {{{0, 100}, {0, 50}}, {{100, -100}, {-100, 100}}},
            {{{0, -101}, {0, -50}}, {{100, -100}, {-100, 100}}},
            {{{0, 0}, {0, 50}}, {{100, -100}, {-100, 100}}},
            {{{-10, -10}, {10, 10}}, {{-10, -10}, {11, 11}}, {{10, -10}, {-10, 10}}},
            {{{10, -10}, {-10, 10}}, {{10, -10}, {-11, 11}}, {{-10, -10}, {10, 10}}},
            {{{-11, -11}, {10, 10}}, {{-10, -10}, {11, 11}}, {{10, -10}, {-10, 10}}},
    };

    for (const auto& segments : tests) {
        std::vector<Segment> myersSegments = segments;
        std::vector<Segment> bruteSegments = segments;
        auto myersResponse = myers_find_crossings(myersSegments);
        auto bruteResponse = brute_force_crossings(bruteSegments);

        std::sort(myersResponse.begin(), myersResponse.end());
        std::sort(bruteResponse.begin(), bruteResponse.end());

        REPORTER_ASSERT(r, myersResponse.size() == bruteResponse.size());
#if 0
        if (myersResponse.size() != bruteResponse.size()) {
            SkASSERT(false);
        }
#endif
        // There should be no duplicate crossings.
        REPORTER_ASSERT(r,
                        std::unique(myersResponse.begin(), myersResponse.end()) ==
                            myersResponse.end());
        REPORTER_ASSERT(r,
                        std::unique(bruteResponse.begin(), bruteResponse.end()) ==
                            bruteResponse.end());

        // Both should be equal.
        REPORTER_ASSERT(r, std::equal(myersResponse.begin(), myersResponse.end(),
                                      bruteResponse.begin(), bruteResponse.end()));
    }
}

class StopWatch {
public:
    void start() {
        fStart = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        std::chrono::high_resolution_clock::time_point stop =
                std::chrono::high_resolution_clock::now();

        fAccumulatedTime += std::chrono::duration_cast<std::chrono::microseconds>(stop - fStart);
        fCount += 1;
    }

    void print() {
        int64_t average = fAccumulatedTime.count() / fCount;
        SkDebugf("average time: %" PRId64 " µs\n", average);
    }

private:
    int fCount = 0;
    std::chrono::high_resolution_clock::time_point fStart;
    std::chrono::microseconds fAccumulatedTime = std::chrono::microseconds::zero();
};

constexpr bool kRunRandomTest = false;
DEF_TEST(MFC_myers_brute_force_random_comparison, r) {
    if constexpr (!kRunRandomTest) {
        return;
    }
    const int n = 200;
    const int boxSize = 20000;
    SkRandom random{n + boxSize};
    std::vector<Segment> segments;

    StopWatch myersStopWatch;
    StopWatch bruteStopWatch;

    for (int trials = 0; trials < 100'000; trials++) {
    for (int i = 0; i < n; ++i) {
        float x = random.nextRangeF(-boxSize, boxSize),
              y = random.nextRangeF(-boxSize, boxSize);

        float angle = random.nextF() * SK_FloatPI;
        float distance = random.nextRangeF(10, 300);

        Point p0 = {sk_float_round2int(x + cos(angle) * distance),
                    sk_float_round2int(y + sin(angle) * distance)};
        Point p1 = {sk_float_round2int(x - cos(angle) * distance),
                    sk_float_round2int(y - sin(angle) * distance)};

        segments.emplace_back(p0, p1);
    }

    std::vector<Segment> myersSegments = segments;
    std::vector<Segment> bruteSegments = segments;
    myersStopWatch.start();
    auto myersResponse = myers_find_crossings(myersSegments);
    myersStopWatch.stop();
    bruteStopWatch.start();
    auto bruteResponse = brute_force_crossings(bruteSegments);
    bruteStopWatch.stop();

    std::sort(myersResponse.begin(), myersResponse.end());
    std::sort(bruteResponse.begin(), bruteResponse.end());

    //SkDebugf("myers size: %zu brute size: %zu\n", myersResponse.size(), bruteResponse.size());

    REPORTER_ASSERT(r, myersResponse.size() == bruteResponse.size());
    if (myersResponse.size() != bruteResponse.size()) {
        SkDebugf("myers size: %zu brute size: %zu\n", myersResponse.size(), bruteResponse.size());
        SkDebugf("{");
        for (const Segment& s : segments) {
            const auto [u, l] = s;
            SkDebugf("{{%d, %d}, {%d, %d}}, ", u.x, u.y, l.x, l.y);
        }
        SkDebugf("},\n");
    }

    // There should be no duplicate crossings.
    REPORTER_ASSERT(r,
                    std::unique(myersResponse.begin(), myersResponse.end()) ==
                    myersResponse.end());
    REPORTER_ASSERT(r,
                    std::unique(bruteResponse.begin(), bruteResponse.end()) ==
                    bruteResponse.end());

    // Both should be equal.
    REPORTER_ASSERT(r, std::equal(myersResponse.begin(), myersResponse.end(),
                                  bruteResponse.begin(), bruteResponse.end()));
    segments.clear();
    }
    SkDebugf("myers ");
    myersStopWatch.print();
    SkDebugf("brute ");
    bruteStopWatch.print();
}
