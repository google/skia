#include "CubicIntersection.h"
#include "Intersection_Tests.h"

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
        const Quadratic* quads[] = {
            &test, &midThird, &split.first(), &split.second()
        };
        size_t quadsCount = sizeof(quads) / sizeof(quads[0]);
        for (size_t one = 0; one < quadsCount; ++one) {
            for (size_t two = 0; two < quadsCount; ++two) {
                for (size_t inner = 0; inner < 3; inner += 2) {
                    if (!point_on_parameterized_curve(*quads[one], (*quads[two])[inner])) {
                            printf("%s %zu [%zu,%zu] %zu parameterization failed\n", 
                                __FUNCTION__, index, one, two, inner);
                    }
                }
                if (!implicit_matches(*quads[one], *quads[two])) {
                    printf("%s %zu [%zu,%zu] coincidence failed\n", __FUNCTION__,
                            index, one, two);
                }
            }
        }
    }
}
