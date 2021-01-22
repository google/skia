/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "tests/Test.h"

static bool eq(const SkM44& a, const SkM44& b, float tol) {
    float fa[16], fb[16];
    a.getColMajor(fa);
    b.getColMajor(fb);
    for (int i = 0; i < 16; ++i) {
        if (!SkScalarNearlyEqual(fa[i], fb[i], tol)) {
            return false;
        }
    }
    return true;
}

DEF_TEST(M44, reporter) {
    SkM44 m, im;

    REPORTER_ASSERT(reporter, SkM44(1, 0, 0, 0,
                                    0, 1, 0, 0,
                                    0, 0, 1, 0,
                                    0, 0, 0, 1) == m);
    REPORTER_ASSERT(reporter, SkM44() == m);
    REPORTER_ASSERT(reporter, m.invert(&im));
    REPORTER_ASSERT(reporter, SkM44() == im);

    m.setTranslate(3, 4, 2);
    REPORTER_ASSERT(reporter, SkM44(1, 0, 0, 3,
                                    0, 1, 0, 4,
                                    0, 0, 1, 2,
                                    0, 0, 0, 1) == m);

    const float f[] = { 1, 0, 0, 2, 3, 1, 2, 5, 0, 5, 3, 0, 0, 1, 0, 2 };
    m = SkM44::ColMajor(f);
    REPORTER_ASSERT(reporter, SkM44(f[0], f[4], f[ 8], f[12],
                                    f[1], f[5], f[ 9], f[13],
                                    f[2], f[6], f[10], f[14],
                                    f[3], f[7], f[11], f[15]) == m);

    {
        SkM44 t = m.transpose();
        REPORTER_ASSERT(reporter, t != m);
        REPORTER_ASSERT(reporter, t.rc(1,0) == m.rc(0,1));
        SkM44 tt = t.transpose();
        REPORTER_ASSERT(reporter, tt == m);
    }

    m = SkM44::RowMajor(f);
    REPORTER_ASSERT(reporter, SkM44(f[ 0], f[ 1], f[ 2], f[ 3],
                                    f[ 4], f[ 5], f[ 6], f[ 7],
                                    f[ 8], f[ 9], f[10], f[14],
                                    f[12], f[13], f[14], f[15]) == m);

    REPORTER_ASSERT(reporter, m.invert(&im));

    m = m * im;
    // m should be identity now, but our calc is not perfect...
    REPORTER_ASSERT(reporter, eq(SkM44(), m, 0.0000005f));
    REPORTER_ASSERT(reporter, SkM44() != m);
}

DEF_TEST(M44_v3, reporter) {
    SkV3 a = {1, 2, 3},
         b = {1, 2, 2};

    REPORTER_ASSERT(reporter, a.lengthSquared() == 1 + 4 + 9);
    REPORTER_ASSERT(reporter, b.length() == 3);
    REPORTER_ASSERT(reporter, a.dot(b) == 1 + 4 + 6);
    REPORTER_ASSERT(reporter, b.dot(a) == 1 + 4 + 6);
    REPORTER_ASSERT(reporter, (a.cross(b) == SkV3{-2,  1, 0}));
    REPORTER_ASSERT(reporter, (b.cross(a) == SkV3{ 2, -1, 0}));

    SkM44 m = {
        2, 0, 0, 3,
        0, 1, 0, 5,
        0, 0, 3, 1,
        0, 0, 0, 1
    };

    SkV3 c = m * a;
    REPORTER_ASSERT(reporter, (c == SkV3{2, 2, 9}));
    SkV4 d = m.map(4, 3, 2, 1);
    REPORTER_ASSERT(reporter, (d == SkV4{11, 8, 7, 1}));
}

DEF_TEST(M44_v4, reporter) {
    SkM44 m( 1,  2,  3,  4,
             5,  6,  7,  8,
             9, 10, 11, 12,
            13, 14, 15, 16);

    SkV4 r0 = m.row(0),
         r1 = m.row(1),
         r2 = m.row(2),
         r3 = m.row(3);

    REPORTER_ASSERT(reporter, (r0 == SkV4{ 1,  2,  3,  4}));
    REPORTER_ASSERT(reporter, (r1 == SkV4{ 5,  6,  7,  8}));
    REPORTER_ASSERT(reporter, (r2 == SkV4{ 9, 10, 11, 12}));
    REPORTER_ASSERT(reporter, (r3 == SkV4{13, 14, 15, 16}));

    REPORTER_ASSERT(reporter, SkM44::Rows(r0, r1, r2, r3) == m);

    SkV4 c0 = m.col(0),
         c1 = m.col(1),
         c2 = m.col(2),
         c3 = m.col(3);

    REPORTER_ASSERT(reporter, (c0 == SkV4{1, 5,  9, 13}));
    REPORTER_ASSERT(reporter, (c1 == SkV4{2, 6, 10, 14}));
    REPORTER_ASSERT(reporter, (c2 == SkV4{3, 7, 11, 15}));
    REPORTER_ASSERT(reporter, (c3 == SkV4{4, 8, 12, 16}));

    REPORTER_ASSERT(reporter, SkM44::Cols(c0, c1, c2, c3) == m);

    // implement matrix * vector using column vectors
    SkV4 v = {1, 2, 3, 4};
    SkV4 v1 = m * v;
    SkV4 v2 = c0 * v.x + c1 * v.y + c2 * v.z + c3 * v.w;
    REPORTER_ASSERT(reporter, v1 == v2);

    REPORTER_ASSERT(reporter, (c0 + r0 == SkV4{c0.x+r0.x, c0.y+r0.y, c0.z+r0.z, c0.w+r0.w}));
    REPORTER_ASSERT(reporter, (c0 - r0 == SkV4{c0.x-r0.x, c0.y-r0.y, c0.z-r0.z, c0.w-r0.w}));
    REPORTER_ASSERT(reporter, (c0 * r0 == SkV4{c0.x*r0.x, c0.y*r0.y, c0.z*r0.z, c0.w*r0.w}));
}

DEF_TEST(M44_rotate, reporter) {
    const SkV3 x = {1, 0, 0},
               y = {0, 1, 0},
               z = {0, 0, 1};

    // We have radians version of setRotateAbout methods, but even with our best approx
    // for PI, sin(SK_ScalarPI) != 0, so to make the comparisons in the unittest clear,
    // I'm using the variants that explicitly take the sin,cos values.

    struct {
        SkScalar sinAngle, cosAngle;
        SkV3 aboutAxis;
        SkV3 expectedX, expectedY, expectedZ;
    } recs[] = {
        { 0, 1,    x,   x, y, z},    // angle = 0
        { 0, 1,    y,   x, y, z},    // angle = 0
        { 0, 1,    z,   x, y, z},    // angle = 0

        { 0,-1,    x,   x,-y,-z},    // angle = 180
        { 0,-1,    y,  -x, y,-z},    // angle = 180
        { 0,-1,    z,  -x,-y, z},    // angle = 180

        // Skia coordinate system is right-handed

        { 1, 0,    x,   x, z,-y},    // angle = 90
        { 1, 0,    y,  -z, y, x},    // angle = 90
        { 1, 0,    z,   y,-x, z},    // angle = 90

        {-1, 0,    x,   x,-z, y},    // angle = -90
        {-1, 0,    y,   z, y,-x},    // angle = -90
        {-1, 0,    z,  -y, x, z},    // angle = -90
    };

    for (const auto& r : recs) {
        SkM44 m(SkM44::kNaN_Constructor);
        m.setRotateUnitSinCos(r.aboutAxis, r.sinAngle, r.cosAngle);

        auto mx = m * x;
        auto my = m * y;
        auto mz = m * z;
        REPORTER_ASSERT(reporter, mx == r.expectedX);
        REPORTER_ASSERT(reporter, my == r.expectedY);
        REPORTER_ASSERT(reporter, mz == r.expectedZ);

        // flipping the axis-of-rotation should flip the results
        mx = m * -x;
        my = m * -y;
        mz = m * -z;
        REPORTER_ASSERT(reporter, mx == -r.expectedX);
        REPORTER_ASSERT(reporter, my == -r.expectedY);
        REPORTER_ASSERT(reporter, mz == -r.expectedZ);
    }
}
