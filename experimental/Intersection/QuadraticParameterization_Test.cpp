#include "CubicIntersection.h"
#include "CubicIntersection_Tests.h"

const Quadratic quadratics[] = {
    {{0, 0}, {1, 0}, {1, 1}},
};

const size_t quadratics_count = sizeof(quadratics) / sizeof(quadratics[0]);

int firstQuadraticCoincidenceTest = 0;

void QuadraticCoincidence_Test() {
    // split large quadratic
    // compare original, parts, to see if the are coincident
    for (size_t index = firstQuadraticCoincidenceTest; index < quadratics_count; ++index) {
        const Quadratic& test = quadratics[index];
        QuadraticPair split;
        chop_at(test, split, 0.5);
        Quadratic midThird;
        sub_divide(test, 1.0/3, 2.0/3, midThird);
        if (!implicit_matches(test, split.first())) {
            printf("%s-1 %d", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(test, split.second())) {
            printf("%s-2 %d", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(midThird, split.first())) {
            printf("%s-3 %d", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(midThird, split.second())) {
            printf("%s-4 %d", __FUNCTION__, (int)index);
        }
        if (!implicit_matches(split.first(), split.second())) {
            printf("%s-5 %d", __FUNCTION__, (int)index);
        }
    }
}
