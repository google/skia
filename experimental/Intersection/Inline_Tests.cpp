#include "CubicIntersection.h"
#include "CubicIntersection_Tests.h"
#include "IntersectionUtilities.h"

static void assert_that(int x, int y, const char* s) {
    if (x == y) {
        return;
    }
    printf("result=%d expected=%d %s\n", x, y, s);
}

static void side_test() {
    assert_that(side(-1), 0, "side(-1) != 0");
    assert_that(side(0), 1, "side(0) != 1");
    assert_that(side(1), 2, "side(1) != 2");
}

static void sideBit_test() {
    assert_that(sideBit(-1), 1, "sideBit(-1) != 1");
    assert_that(sideBit(0), 2, "sideBit(0) != 2");
    assert_that(sideBit(1), 4, "sideBit(1) != 4");
}

static void other_two_test() {
    for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 4; ++y) {
            if (x == y) {
                continue;
            }
            int mask = other_two(x, y);
            int all = 1 << x;
            all |= 1 << y;
            all |= 1 << (x ^ mask);
            all |= 1 << (y ^ mask);
            if (all == 0x0F) {
                continue;
            }
            printf("[%d,%d] other_two failed mask=%d [%d,%d]\n",
                x, y, mask, x ^ mask, y ^ mask);
        }
    }
}

void Inline_Tests() {
    side_test();
    sideBit_test();
    other_two_test();
}
