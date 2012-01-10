#include "CubicIntersection.h"
#include "CubicIntersection_Tests.h"
#include "IntersectionUtilities.h"

const Cubic convex[] = {
    {{0, 0}, {2, 0}, {2, 1}, {0, 1}},
    {{1, 0}, {1, 1}, {0, 1}, {0, 0}},
    {{1, 1}, {0, 1}, {0, 0}, {1, 0}},
    {{0, 1}, {0, 0}, {1, 0}, {1, 1}},
    {{0, 0}, {10, 0}, {10, 10}, {5, 6}},
};

size_t convex_count = sizeof(convex) / sizeof(convex[0]);

const Cubic bowtie[] = {
    {{0, 0}, {1, 1}, {1, 0}, {0, 1}},
    {{1, 0}, {0, 1}, {1, 1}, {0, 0}},
    {{1, 1}, {0, 0}, {0, 1}, {1, 0}},
    {{0, 1}, {1, 0}, {0, 0}, {1, 1}},
};

size_t bowtie_count = sizeof(bowtie) / sizeof(bowtie[0]);

const Cubic arrow[] = {
    {{0, 0}, {10, 0}, {10, 10}, {5, 4}},
    {{10, 0}, {10, 10}, {5, 4}, {0, 0}},
    {{10, 10}, {5, 4}, {0, 0}, {10, 0}},
    {{5, 4}, {0, 0}, {10, 0}, {10, 10}},
};

size_t arrow_count = sizeof(arrow) / sizeof(arrow[0]);

const Cubic three[] = {
    {{1, 0}, {1, 0}, {1, 1}, {0, 1}}, // 0 == 1
    {{0, 0}, {1, 1}, {1, 1}, {0, 1}}, // 1 == 2
    {{0, 0}, {1, 0}, {0, 1}, {0, 1}}, // 2 == 3
    {{1, 0}, {1, 1}, {1, 0}, {0, 1}}, // 0 == 2
    {{1, 0}, {1, 1}, {0, 1}, {1, 0}}, // 0 == 3
    {{0, 0}, {1, 0}, {1, 1}, {1, 0}}, // 1 == 3
};

size_t three_count = sizeof(three) / sizeof(three[0]);

const Cubic triangle[] = {
    {{0, 0}, {1, 0}, {2, 0}, {0, 1}}, // extra point on horz
    {{1, 0}, {2, 0}, {0, 1}, {0, 0}},
    {{2, 0}, {0, 1}, {0, 0}, {1, 0}},
    {{0, 1}, {0, 0}, {1, 0}, {2, 0}},
    
    {{0, 0}, {0, 1}, {0, 2}, {1, 1}}, // extra point on vert
    {{0, 1}, {0, 2}, {1, 1}, {0, 0}},
    {{0, 2}, {1, 1}, {0, 0}, {0, 1}},
    {{1, 1}, {0, 0}, {0, 1}, {0, 2}},
    
    {{0, 0}, {1, 1}, {2, 2}, {2, 0}}, // extra point on diag
    {{1, 1}, {2, 2}, {2, 0}, {0, 0}},
    {{2, 2}, {2, 0}, {0, 0}, {1, 1}},
    {{2, 0}, {0, 0}, {1, 1}, {2, 2}},
    
    {{0, 0}, {2, 0}, {2, 2}, {1, 1}}, // extra point on diag
    {{2, 0}, {2, 2}, {1, 1}, {0, 0}},
    {{2, 2}, {1, 1}, {0, 0}, {2, 0}},
    {{1, 1}, {0, 0}, {2, 0}, {2, 2}},
};

size_t triangle_count = sizeof(triangle) / sizeof(triangle[0]);

const struct CubicDataSet {
    const Cubic* data;
    size_t size;
} cubicDataSet[] = {
    { three, three_count },
    { convex, convex_count },
    { bowtie, bowtie_count },
    { arrow, arrow_count },
    { triangle, triangle_count },
};

size_t cubicDataSet_count = sizeof(cubicDataSet) / sizeof(cubicDataSet[0]);

typedef double Matrix3x2[3][2];

static bool rotateToAxis(const _Point& a, const _Point& b, Matrix3x2& matrix) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;
    double length = sqrt(dx * dx + dy * dy);
    if (length == 0) {
        return false;
    }
    double invLength = 1 / length;
    matrix[0][0] = dx * invLength;
    matrix[1][0] = dy * invLength;
    matrix[2][0] = 0;
    matrix[0][1] = -dy * invLength;
    matrix[1][1] = dx * invLength;
    matrix[2][1] = 0;
    return true;
}

static void transform(const Cubic& cubic, const Matrix3x2& matrix, Cubic& rotPath) {
    for (int index = 0; index < 4; ++index) {
        rotPath[index].x = cubic[index].x * matrix[0][0] 
                + cubic[index].y * matrix[1][0] + matrix[2][0];
        rotPath[index].y = cubic[index].x * matrix[0][1] 
                + cubic[index].y * matrix[1][1] + matrix[2][1];
    }
}

// brute force way to find convex hull:
// pick two points
// rotate all four until the two points are horizontal
// are the remaining two points both above or below the horizontal line?
// if so, the two points must be an edge of the convex hull
static int rotate_to_hull(const Cubic& cubic, char order[4], size_t idx, size_t inr) {
    bool debug_rotate_to_hull = false;
    int outsidePtSet[4];
    memset(outsidePtSet, -1, sizeof(outsidePtSet));
    for (int outer = 0; outer < 3; ++outer) {
        for (int priorOuter = 0; priorOuter < outer; ++priorOuter) {
            if (cubic[outer].approximatelyEqual(cubic[priorOuter])) {
                goto skip;
            }
        }
        for (int inner = outer + 1; inner < 4; ++inner) {
            for (int priorInner = outer + 1; priorInner < inner; ++priorInner) {
                if (cubic[inner].approximatelyEqual(cubic[priorInner])) {
                    goto skipInner;
                }
            }
            if (cubic[outer].approximatelyEqual(cubic[inner])) {
                continue;
            }
            Matrix3x2 matrix;
            if (!rotateToAxis(cubic[outer], cubic[inner], matrix)) {
                continue;
            }
            Cubic rotPath;
            transform(cubic, matrix, rotPath);
            int sides[3];
            int zeroes;
            zeroes = -1;
            bzero(sides, sizeof(sides));
            if (debug_rotate_to_hull) printf("%s [%d,%d] [o=%d,i=%d] src=(%g,%g) rot=", __FUNCTION__,
                    (int)idx, (int)inr, (int)outer, (int)inner,
                    cubic[inner].x, cubic[inner].y);
            for (int index = 0; index < 4; ++index) {
                if (debug_rotate_to_hull) printf("(%g,%g) ", rotPath[index].x, rotPath[index].y);
                sides[side(rotPath[index].y - rotPath[inner].y)]++;
                if (index != outer && index != inner 
                        && side(rotPath[index].y - rotPath[inner].y) == 1)
                    zeroes = index;
            }
            if (debug_rotate_to_hull) printf("sides=(%d,%d,%d)\n", sides[0], sides[1], sides[2]);
            if (sides[0] && sides[2]) {
                continue;
            }
            if (sides[1] == 3 && zeroes >= 0) {
                // verify that third point is between outer, inner
                // if either of remaining two equals outer or equal, pick lower
                if (rotPath[zeroes].approximatelyEqual(rotPath[inner])
                        && zeroes < inner) {
                    if (debug_rotate_to_hull) printf("%s [%d,%d] [o=%d,i=%d] zeroes < inner\n",
                        __FUNCTION__, (int)idx, (int)inr, (int)outer, (int)inner);
                    continue;
                }
                 if (rotPath[zeroes].approximatelyEqual(rotPath[outer])
                        && zeroes < outer) {
                    if (debug_rotate_to_hull) printf("%s [%d,%d] [o=%d,i=%d] zeroes < outer\n",
                        __FUNCTION__, (int)idx, (int)inr, (int)outer, (int)inner);
                    continue;
                }
                if (rotPath[zeroes].x < rotPath[inner].x 
                        && rotPath[zeroes].x < rotPath[outer].x) {
                    if (debug_rotate_to_hull) printf("%s [%d,%d] [o=%d,i=%d] zeroes < inner && outer\n",
                        __FUNCTION__, (int)idx, (int)inr, (int)outer, (int)inner);
                    continue;
                }
                if (rotPath[zeroes].x > rotPath[inner].x 
                        && rotPath[zeroes].x > rotPath[outer].x) {
                    if (debug_rotate_to_hull) printf("%s [%d,%d] [o=%d,i=%d] zeroes > inner && outer\n",
                        __FUNCTION__, (int)idx, (int)inr, (int)outer, (int)inner);
                    continue;
                }
            }
            if (outsidePtSet[outer] < 0) {
                outsidePtSet[outer] = inner;
            } else {
                if (outsidePtSet[inner] > 0) {
                    if (debug_rotate_to_hull) printf("%s [%d,%d] [o=%d,i=%d] too many rays from one point\n",
                        __FUNCTION__, (int)idx, (int)inr, (int)outer, (int)inner);
                }
                outsidePtSet[inner] = outer;
            }
skipInner:
            ;
        }
skip:
        ;
    }
    int totalSides = 0;
    int first = 0;
    for (; first < 4; ++first) {
        if (outsidePtSet[first] >= 0) {
            break;
        }
    }
    if (first > 3) {
        order[0] = 0;
        return 1;
    }
    int next = first;
    do {
        order[totalSides++] = next;
        next = outsidePtSet[next];
    } while (next != -1 && next != first);
    return totalSides;
}

int firstIndex = 0;
int firstInner = 0;

void ConvexHull_Test() {
    for (size_t index = firstIndex; index < cubicDataSet_count; ++index) {
        const CubicDataSet& set = cubicDataSet[index];
        for (size_t inner = firstInner; inner < set.size; ++inner) {
            const Cubic& cubic = set.data[inner];
            char order[4], cmpOrder[4];
            int cmp = rotate_to_hull(cubic, cmpOrder, index, inner);
            if (cmp < 3) {
                continue;
            }
            int result = convex_hull(cubic, order);
            if (cmp != result) {
                printf("%s [%d,%d] result=%d cmp=%d\n", __FUNCTION__,
                    (int)index, (int)inner, result, cmp);
                continue;
            }
            // check for same indices
            char pts = 0;
            char cmpPts = 0;
            int pt, bit;
            for (pt = 0; pt < cmp; ++pt) {
                if (pts & 1 << order[pt]) {
                    printf("%s [%d,%d] duplicate index in order: %d,%d,%d",
                            __FUNCTION__, (int)index, (int)inner, 
                            order[0], order[1], order[2]);
                    if (cmp == 4) {
                        printf(",%d", order[3]);
                    }
                    printf("\n");
                    goto next;
                }
                if (cmpPts & 1 << cmpOrder[pt]) {
                    printf("%s [%d,%d] duplicate index in order: %d,%d,%d",
                            __FUNCTION__, (int)index, (int)inner, 
                            cmpOrder[0], cmpOrder[1], cmpOrder[2]);
                    if (cmp == 4) {
                        printf(",%d", cmpOrder[3]);
                    }
                    printf("\n");
                    goto next;
                }
                pts |= 1 << order[pt];
                cmpPts |= 1 << cmpOrder[pt];
            }
            for (bit = 0; bit < 4; ++bit) {
                if (pts & 1 << bit) {
                    continue;
                }
                for (pt = 0; pt < cmp; ++pt) {
                    if (order[pt] == bit) {
                        continue;
                    }
                    if (cubic[order[pt]] == cubic[bit]) {
                        pts |= 1 << bit;
                    }
                }
            }
            for (bit = 0; bit < 4; ++bit) {
                if (cmpPts & 1 << bit) {
                    continue;
                }
                for (pt = 0; pt < cmp; ++pt) {
                    if (cmpOrder[pt] == bit) {
                        continue;
                    }
                    if (cubic[cmpOrder[pt]] == cubic[bit]) {
                        cmpPts |= 1 << bit;
                    }
                }
            }
            if (pts != cmpPts) {
                printf("%s [%d,%d] mismatch indices: order=%d,%d,%d",
                        __FUNCTION__, (int)index, (int)inner, 
                        order[0], order[1], order[2]);
                if (cmp == 4) {
                    printf(",%d", order[3]);
                }
                printf(" cmpOrder=%d,%d,%d", cmpOrder[0], cmpOrder[1], cmpOrder[2]);
                if (cmp == 4) {
                    printf(",%d", cmpOrder[3]);
                }
                printf("\n");
                continue;
            }
            if (cmp == 4) { // check for bow ties
                int match = 0;
                while (cmpOrder[match] != order[0]) {
                    ++match;
                }
                if (cmpOrder[match ^ 2] != order[2]) {
                    printf("%s [%d,%d] bowtie mismatch: order=%d,%d,%d,%d"
                            " cmpOrder=%d,%d,%d,%d\n",
                            __FUNCTION__, (int)index, (int)inner, 
                            order[0], order[1], order[2], order[3],
                            cmpOrder[0], cmpOrder[1], cmpOrder[2], cmpOrder[3]);
                }
            }
    next:
            ;
        }
    }
}

const double a = 1.0/3;
const double b = 2.0/3;

const Cubic x_cubic[] = {
    {{0, 0}, {a, 0}, {b, 0}, {1, 0}}, // 0
    {{0, 0}, {a, 0}, {b, 0}, {1, 1}}, // 1
    {{0, 0}, {a, 0}, {b, 1}, {1, 0}}, // 2
    {{0, 0}, {a, 0}, {b, 1}, {1, 1}}, // 3
    {{0, 0}, {a, 1}, {b, 0}, {1, 0}}, // 4
    {{0, 0}, {a, 1}, {b, 0}, {1, 1}}, // 5
    {{0, 0}, {a, 1}, {b, 1}, {1, 0}}, // 6
    {{0, 0}, {a, 1}, {b, 1}, {1, 1}}, // 7
    {{0, 1}, {a, 0}, {b, 0}, {1, 0}}, // 8
    {{0, 1}, {a, 0}, {b, 0}, {1, 1}}, // 9
    {{0, 1}, {a, 0}, {b, 1}, {1, 0}}, // 10
    {{0, 1}, {a, 0}, {b, 1}, {1, 1}}, // 11
    {{0, 1}, {a, 1}, {b, 0}, {1, 0}}, // 12
    {{0, 1}, {a, 1}, {b, 0}, {1, 1}}, // 13
    {{0, 1}, {a, 1}, {b, 1}, {1, 0}}, // 14
    {{0, 1}, {a, 1}, {b, 1}, {1, 1}}, // 15
};

size_t x_cubic_count = sizeof(x_cubic) / sizeof(x_cubic[0]);

static int first_x_test = 0;

void ConvexHull_X_Test() {
    for (size_t index = first_x_test; index < x_cubic_count; ++index) {
        const Cubic& cubic = x_cubic[index];
        char connectTo0[2] = {-1, -1};
        char connectTo3[2] = {-1, -1};
        convex_x_hull(cubic, connectTo0, connectTo3);
        int idx, cmp;
        for (idx = 0; idx < 2; ++idx) {
            if (connectTo0[idx] >= 1 && connectTo0[idx] < 4) {
                continue;
            } else {
                printf("%s connectTo0[idx]=%d", __FUNCTION__, connectTo0[idx]);
            }
            if (connectTo3[idx] >= 0 && connectTo3[idx] < 3) {
                continue;
            } else {
                printf("%s connectTo3[idx]=%d", __FUNCTION__, connectTo3[idx]);
            }
            goto nextTest;
        }
        char rOrder[4];
        char cmpOrder[4];
        cmp = rotate_to_hull(cubic, cmpOrder, index, 0);
        if (index == 0 || index == 15) {
            // FIXME: make rotate_to_hull work for degenerate 2 edge hull cases
            cmpOrder[0] = 0;
            cmpOrder[1] = 3;
            cmp = 2;
        }
        if (cmp < 3) {
            // FIXME: make rotate_to_hull work for index == 3 etc
            continue;
        }
        for (idx = 0; idx < cmp; ++idx) {
            if (cmpOrder[idx] == 0) {
                rOrder[0] = cmpOrder[(idx + 1) % cmp]; 
                rOrder[1] = cmpOrder[(idx + cmp - 1) % cmp];
            } else if (cmpOrder[idx] == 3) {
                rOrder[2] = cmpOrder[(idx + 1) % cmp]; 
                rOrder[3] = cmpOrder[(idx + cmp - 1) % cmp];
            }
        }
        if (connectTo0[0] != connectTo0[1]) {
            if (rOrder[0] == rOrder[1]) {
                printf("%s [%d] (1) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
            int unused = 6 - connectTo0[0] - connectTo0[1];
            int rUnused = 6 - rOrder[0] - rOrder[1];
            if (unused != rUnused) {
                printf("%s [%d] (2) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
        } else {
            if (rOrder[0] != rOrder[1]) {
                printf("%s [%d] (3) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
            if (connectTo0[0] != rOrder[0]) {
                printf("%s [%d] (4) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
        }
        if (connectTo3[0] != connectTo3[1]) {
             if (rOrder[2] == rOrder[3]) {
                printf("%s [%d] (5) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
           int unused = 6 - connectTo3[0] - connectTo3[1];
           int rUnused = 6 - rOrder[2] - rOrder[3];
            if (unused != rUnused) {
                printf("%s [%d] (6) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
        } else {
            if (rOrder[2] != rOrder[3]) {
                printf("%s [%d] (7) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
            if (connectTo3[1] != rOrder[3]) {
                printf("%s [%d] (8) order=(%d,%d,%d,%d) r_order=(%d,%d,%d,%d)\n",
                    __FUNCTION__, (int)index, connectTo0[0], connectTo0[1],
                    connectTo3[0], connectTo3[1],
                    rOrder[0], rOrder[1], rOrder[2], rOrder[3]);
                continue;
            }
        }
nextTest:
        ;
    }
}



