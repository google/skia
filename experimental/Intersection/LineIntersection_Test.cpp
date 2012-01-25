#include "CubicIntersection_Tests.h"
#include "LineIntersection.h"

// FIXME: add tests for intersecting, non-intersecting, degenerate, coincident
const _Line tests[][2] = {
    {{{166.86950047022856, 112.69654129527828}, {166.86948801592692, 112.69655741235339}}, 
     {{166.86960700313026, 112.6965477747386},  {166.86925794355412, 112.69656471103423}}},
};

const size_t tests_count = sizeof(tests) / sizeof(tests[0]);

static size_t firstLineIntersectionTest = 0;

void LineIntersection_Test() {
    for (size_t index = firstLineIntersectionTest; index < tests_count; ++index) {
        const _Line& line1 = tests[index][0];
        const _Line& line2 = tests[index][1];
        _Point result;
        lineIntersect(line1, line2, &result);
        // FIXME: validate results
        // see if result is between start and end of both lines
        // see if result is on both lines 
        // printf("%s (%g,%g)\n", __FUNCTION__, result.x, result.y);
    }
}
