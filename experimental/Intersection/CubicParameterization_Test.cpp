#include "CubicIntersection.h"
#include "CubicIntersection_Tests.h"
#include "TestUtilities.h"

const Quadratic quadratics[] = {
    {{0, 0}, {1, 0}, {1, 1}},
};

const size_t quadratics_count = sizeof(quadratics) / sizeof(quadratics[0]);

int firstCubicCoincidenceTest = 0;

void CubicCoincidence_Test() {
    // split large quadratic
    // upscale quadratics to cubics
    // compare original, parts, to see if the are coincident
    for (size_t index = firstCubicCoincidenceTest; index < quadratics_count; ++index) {
        const Quadratic& test = quadratics[index];
        QuadraticPair split;
        chop_at(test, split, 0.5);
        Quadratic midThird;
        sub_divide(test, 1.0/3, 2.0/3, midThird);
        Cubic whole, first, second, mid;
        quad_to_cubic(test, whole);
        quad_to_cubic(split.first(), first);
        quad_to_cubic(split.second(), second);
        quad_to_cubic(midThird, mid);
        if (!implicit_matches(whole, first)) {
            printf("%s-1 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(whole, second)) {
            printf("%s-2 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(mid, first)) {
            printf("%s-3 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(mid, second)) {
            printf("%s-4 %d\n", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(first, second)) {
            printf("%s-5 %d\n", __FUNCTION__, (int)index);
        }
    }
}
