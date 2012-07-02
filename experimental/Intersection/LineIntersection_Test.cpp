#include "CurveUtilities.h"
#include "Intersection_Tests.h"
#include "LineIntersection.h"

// FIXME: add tests for intersecting, non-intersecting, degenerate, coincident
const _Line tests[][2] = {
    {{{0, 0}, {1, 0}}, {{1, 0}, {0, 0}}},
    {{{0, 0}, {0, 0}}, {{0, 0}, {1, 0}}},
    {{{0, 1}, {0, 1}}, {{0, 0}, {0, 2}}},
    {{{0, 0}, {1, 0}}, {{0, 0}, {2, 0}}},
    {{{1, 1}, {2, 2}}, {{0, 0}, {3, 3}}},
    {{{166.86950047022856, 112.69654129527828}, {166.86948801592692, 112.69655741235339}}, 
     {{166.86960700313026, 112.6965477747386},  {166.86925794355412, 112.69656471103423}}}
};

const size_t tests_count = sizeof(tests) / sizeof(tests[0]);

const _Line noIntersect[][2] = {
    {{{0, 0}, {1, 0}}, {{3, 0}, {2, 0}}},
    {{{0, 0}, {0, 0}}, {{1, 0}, {2, 0}}},
    {{{0, 1}, {0, 1}}, {{0, 3}, {0, 2}}},
    {{{0, 0}, {1, 0}}, {{2, 0}, {3, 0}}},
    {{{1, 1}, {2, 2}}, {{4, 4}, {3, 3}}},
};

const size_t noIntersect_count = sizeof(noIntersect) / sizeof(noIntersect[0]);

static size_t firstLineIntersectionTest = 0;
static size_t firstNoIntersectionTest = 0;

void LineIntersection_Test() {
    size_t index;
    for (index = firstLineIntersectionTest; index < tests_count; ++index) {
        const _Line& line1 = tests[index][0];
        const _Line& line2 = tests[index][1];
        double t1[2], t2[2];
        int pts = intersect(line1, line2, t1, t2);
        if (!pts) {
            printf("%s [%zu] no intersection found\n", __FUNCTION__, index);
        }
        for (int i = 0; i < pts; ++i) {
            _Point result1, result2;
            xy_at_t(line1, t1[i], result1.x, result1.y);
            xy_at_t(line2, t2[i], result2.x, result2.y);
            if (!result1.approximatelyEqual(result2)) {
                if (pts == 1) {
                    printf("%s [%zu] not equal\n", __FUNCTION__, index);
                } else {
                    xy_at_t(line2, t2[i ^ 1], result2.x, result2.y);
                    if (!result1.approximatelyEqual(result2)) {
                        printf("%s [%zu] not equal\n", __FUNCTION__, index);
                    }
                }
            }
        }
    }
    for (index = firstNoIntersectionTest; index < noIntersect_count; ++index) {
        const _Line& line1 = noIntersect[index][0];
        const _Line& line2 = noIntersect[index][1];
        double t1[2], t2[2];
        int pts = intersect(line1, line2, t1, t2);
        if (pts) {
            printf("%s [%zu] no intersection expected\n", __FUNCTION__, index);
        }
    }
}
