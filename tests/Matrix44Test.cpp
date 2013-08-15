/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkMatrix44.h"

static bool nearly_equal_double(double a, double b) {
    const double tolerance = 1e-7;
    double diff = a - b;
    if (diff < 0)
        diff = -diff;
    return diff <= tolerance;
}

static bool nearly_equal_scalar(SkMScalar a, SkMScalar b) {
    // Note that we get more compounded error for multiple operations when
    // SK_SCALAR_IS_FIXED.
#ifdef SK_SCALAR_IS_FLOAT
    const SkScalar tolerance = SK_Scalar1 / 200000;
#else
    const SkScalar tolerance = SK_Scalar1 / 1024;
#endif

    return SkTAbs<SkMScalar>(a - b) <= tolerance;
}

template <typename T> void assert16(skiatest::Reporter* reporter, const T data[],
                                    T m0,  T m1,  T m2,  T m3,
                                    T m4,  T m5,  T m6,  T m7,
                                    T m8,  T m9,  T m10, T m11,
                                    T m12, T m13, T m14, T m15) {
    REPORTER_ASSERT(reporter, data[0] == m0);
    REPORTER_ASSERT(reporter, data[1] == m1);
    REPORTER_ASSERT(reporter, data[2] == m2);
    REPORTER_ASSERT(reporter, data[3] == m3);

    REPORTER_ASSERT(reporter, data[4] == m4);
    REPORTER_ASSERT(reporter, data[5] == m5);
    REPORTER_ASSERT(reporter, data[6] == m6);
    REPORTER_ASSERT(reporter, data[7] == m7);

    REPORTER_ASSERT(reporter, data[8] == m8);
    REPORTER_ASSERT(reporter, data[9] == m9);
    REPORTER_ASSERT(reporter, data[10] == m10);
    REPORTER_ASSERT(reporter, data[11] == m11);

    REPORTER_ASSERT(reporter, data[12] == m12);
    REPORTER_ASSERT(reporter, data[13] == m13);
    REPORTER_ASSERT(reporter, data[14] == m14);
    REPORTER_ASSERT(reporter, data[15] == m15);
}

static bool nearly_equal(const SkMatrix44& a, const SkMatrix44& b) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (!nearly_equal_scalar(a.get(i, j), b.get(i, j))) {
                printf("not equal %g %g\n", a.get(i, j), b.get(i, j));
                return false;
            }
        }
    }
    return true;
}

static bool is_identity(const SkMatrix44& m) {
    SkMatrix44 identity;
    identity.reset();
    return nearly_equal(m, identity);
}

///////////////////////////////////////////////////////////////////////////////
static bool bits_isonly(int value, int mask) {
    return 0 == (value & ~mask);
}

static void test_constructor(skiatest::Reporter* reporter) {
    // Allocate a matrix on the heap
    SkMatrix44* placeholderMatrix = new SkMatrix44();
    SkAutoTDelete<SkMatrix44> deleteMe(placeholderMatrix);

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            placeholderMatrix->setDouble(row, col, row * col);
        }
    }

    // Use placement-new syntax to trigger the constructor on top of the heap
    // address we already initialized. This allows us to check that the
    // constructor did avoid initializing the matrix contents.
    SkMatrix44* testMatrix = new(placeholderMatrix) SkMatrix44(SkMatrix44::kUninitialized_Constructor);
    REPORTER_ASSERT(reporter, testMatrix == placeholderMatrix);
    REPORTER_ASSERT(reporter, !testMatrix->isIdentity());
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            REPORTER_ASSERT(reporter, nearly_equal_double(row * col, testMatrix->getDouble(row, col)));
        }
    }

    // Verify that kIdentity_Constructor really does initialize to an identity matrix.
    testMatrix = 0;
    testMatrix = new(placeholderMatrix) SkMatrix44(SkMatrix44::kIdentity_Constructor);
    REPORTER_ASSERT(reporter, testMatrix == placeholderMatrix);
    REPORTER_ASSERT(reporter, testMatrix->isIdentity());
    REPORTER_ASSERT(reporter, *testMatrix == SkMatrix44::I());
}

static void test_translate(skiatest::Reporter* reporter) {
    SkMatrix44 mat, inverse;

    mat.setTranslate(0, 0, 0);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kIdentity_Mask));
    mat.setTranslate(1, 2, 3);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kTranslate_Mask));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    REPORTER_ASSERT(reporter, bits_isonly(inverse.getType(), SkMatrix44::kTranslate_Mask));

    SkMatrix44 a, b, c;
    a.set3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    b.setTranslate(10, 11, 12);

    c.setConcat(a, b);
    mat = a;
    mat.preTranslate(10, 11, 12);
    REPORTER_ASSERT(reporter, mat == c);

    c.setConcat(b, a);
    mat = a;
    mat.postTranslate(10, 11, 12);
    REPORTER_ASSERT(reporter, mat == c);
}

static void test_scale(skiatest::Reporter* reporter) {
    SkMatrix44 mat, inverse;

    mat.setScale(1, 1, 1);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kIdentity_Mask));
    mat.setScale(1, 2, 3);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kScale_Mask));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    REPORTER_ASSERT(reporter, bits_isonly(inverse.getType(), SkMatrix44::kScale_Mask));

    SkMatrix44 a, b, c;
    a.set3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    b.setScale(10, 11, 12);

    c.setConcat(a, b);
    mat = a;
    mat.preScale(10, 11, 12);
    REPORTER_ASSERT(reporter, mat == c);

    c.setConcat(b, a);
    mat = a;
    mat.postScale(10, 11, 12);
    REPORTER_ASSERT(reporter, mat == c);
}

static void make_i(SkMatrix44* mat) { mat->setIdentity(); }
static void make_t(SkMatrix44* mat) { mat->setTranslate(1, 2, 3); }
static void make_s(SkMatrix44* mat) { mat->setScale(1, 2, 3); }
static void make_st(SkMatrix44* mat) {
    mat->setScale(1, 2, 3);
    mat->postTranslate(1, 2, 3);
}
static void make_a(SkMatrix44* mat) {
    mat->setRotateDegreesAbout(1, 2, 3, 45);
}
static void make_p(SkMatrix44* mat) {
    SkMScalar data[] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        1, 2, 3, 4, 5, 6, 7, 8,
    };
    mat->setRowMajor(data);
}

typedef void (*Make44Proc)(SkMatrix44*);

static const Make44Proc gMakeProcs[] = {
    make_i, make_t, make_s, make_st, make_a, make_p
};

static void test_map2(skiatest::Reporter* reporter, const SkMatrix44& mat) {
    SkMScalar src2[] = { 1, 2 };
    SkMScalar src4[] = { src2[0], src2[1], 0, 1 };
    SkMScalar dstA[4], dstB[4];

    for (int i = 0; i < 4; ++i) {
        dstA[i] = 123456789;
        dstB[i] = 987654321;
    }

    mat.map2(src2, 1, dstA);
    mat.mapMScalars(src4, dstB);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, dstA[i] == dstB[i]);
    }
}

static void test_map2(skiatest::Reporter* reporter) {
    SkMatrix44 mat;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gMakeProcs); ++i) {
        gMakeProcs[i](&mat);
        test_map2(reporter, mat);
    }
}

static void test_gettype(skiatest::Reporter* reporter) {
    SkMatrix44 matrix;

    REPORTER_ASSERT(reporter, matrix.isIdentity());
    REPORTER_ASSERT(reporter, SkMatrix44::kIdentity_Mask == matrix.getType());

    int expectedMask;

    matrix.set(1, 1, 0);
    expectedMask = SkMatrix44::kScale_Mask;
    REPORTER_ASSERT(reporter, matrix.getType() == expectedMask);

    matrix.set(0, 3, 1);    // translate-x
    expectedMask |= SkMatrix44::kTranslate_Mask;
    REPORTER_ASSERT(reporter, matrix.getType() == expectedMask);

    matrix.set(2, 0, 1);
    expectedMask |= SkMatrix44::kAffine_Mask;
    REPORTER_ASSERT(reporter, matrix.getType() == expectedMask);

    matrix.set(3, 2, 1);
    REPORTER_ASSERT(reporter, matrix.getType() & SkMatrix44::kPerspective_Mask);

    // ensure that negative zero is treated as zero
    SkMScalar dx = 0;
    SkMScalar dy = 0;
    SkMScalar dz = 0;
    matrix.setTranslate(-dx, -dy, -dz);
    REPORTER_ASSERT(reporter, matrix.isIdentity());
    matrix.preTranslate(-dx, -dy, -dz);
    REPORTER_ASSERT(reporter, matrix.isIdentity());
    matrix.postTranslate(-dx, -dy, -dz);
    REPORTER_ASSERT(reporter, matrix.isIdentity());
}

static void test_common_angles(skiatest::Reporter* reporter) {
    SkMatrix44 rot;
    // Test precision of rotation in common cases
    int common_angles[] = { 0, 90, -90, 180, -180, 270, -270, 360, -360 };
    for (int i = 0; i < 9; ++i) {
        rot.setRotateDegreesAbout(0, 0, -1, SkIntToScalar(common_angles[i]));

        SkMatrix rot3x3 = rot;
        REPORTER_ASSERT(reporter, rot3x3.rectStaysRect());
    }
}

static void test_concat(skiatest::Reporter* reporter) {
    int i;
    SkMatrix44 a, b, c, d;

    a.setTranslate(10, 10, 10);
    b.setScale(2, 2, 2);

    SkScalar src[8] = {
        0, 0, 0, 1,
        1, 1, 1, 1
    };
    SkScalar dst[8];

    c.setConcat(a, b);

    d = a;
    d.preConcat(b);
    REPORTER_ASSERT(reporter, d == c);

    c.mapScalars(src, dst); c.mapScalars(src + 4, dst + 4);
    for (i = 0; i < 3; ++i) {
        REPORTER_ASSERT(reporter, 10 == dst[i]);
        REPORTER_ASSERT(reporter, 12 == dst[i + 4]);
    }

    c.setConcat(b, a);

    d = a;
    d.postConcat(b);
    REPORTER_ASSERT(reporter, d == c);

    c.mapScalars(src, dst); c.mapScalars(src + 4, dst + 4);
    for (i = 0; i < 3; ++i) {
        REPORTER_ASSERT(reporter, 20 == dst[i]);
        REPORTER_ASSERT(reporter, 22 == dst[i + 4]);
    }
}

static void test_determinant(skiatest::Reporter* reporter) {
    SkMatrix44 a;
    REPORTER_ASSERT(reporter, nearly_equal_double(1, a.determinant()));
    a.set(1, 1, 2);
    REPORTER_ASSERT(reporter, nearly_equal_double(2, a.determinant()));
    SkMatrix44 b;
    REPORTER_ASSERT(reporter, a.invert(&b));
    REPORTER_ASSERT(reporter, nearly_equal_double(0.5, b.determinant()));
    SkMatrix44 c = b = a;
    c.set(0, 1, 4);
    b.set(1, 0, 4);
    REPORTER_ASSERT(reporter,
                    nearly_equal_double(a.determinant(),
                                        b.determinant()));
    SkMatrix44 d = a;
    d.set(0, 0, 8);
    REPORTER_ASSERT(reporter, nearly_equal_double(16, d.determinant()));

    SkMatrix44 e = a;
    e.postConcat(d);
    REPORTER_ASSERT(reporter, nearly_equal_double(32, e.determinant()));
    e.set(0, 0, 0);
    REPORTER_ASSERT(reporter, nearly_equal_double(0, e.determinant()));
}

static void test_transpose(skiatest::Reporter* reporter) {
    SkMatrix44 a;
    SkMatrix44 b;

    int i = 0;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            a.setDouble(row, col, i);
            b.setDouble(col, row, i++);
        }
    }

    a.transpose();
    REPORTER_ASSERT(reporter, nearly_equal(a, b));
}

static void test_get_set_double(skiatest::Reporter* reporter) {
    SkMatrix44 a;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            a.setDouble(row, col, 3.141592653589793);
            REPORTER_ASSERT(reporter,
                            nearly_equal_double(3.141592653589793,
                                                a.getDouble(row, col)));
            a.setDouble(row, col, 0);
            REPORTER_ASSERT(reporter,
                            nearly_equal_double(0, a.getDouble(row, col)));
        }
    }
}

static void test_set_row_col_major(skiatest::Reporter* reporter) {
    SkMatrix44 a, b, c, d;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            a.setDouble(row, col, row * 4 + col);
        }
    }

    double bufferd[16];
    float bufferf[16];
    a.asColMajord(bufferd);
    b.setColMajord(bufferd);
    REPORTER_ASSERT(reporter, nearly_equal(a, b));
    b.setRowMajord(bufferd);
    b.transpose();
    REPORTER_ASSERT(reporter, nearly_equal(a, b));
    a.asColMajorf(bufferf);
    b.setColMajorf(bufferf);
    REPORTER_ASSERT(reporter, nearly_equal(a, b));
    b.setRowMajorf(bufferf);
    b.transpose();
    REPORTER_ASSERT(reporter, nearly_equal(a, b));
}

static void TestMatrix44(skiatest::Reporter* reporter) {
    SkMatrix44 mat, inverse, iden1, iden2, rot;

    mat.reset();
    mat.setTranslate(1, 1, 1);
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(2, 2, 2);
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(SK_MScalar1/2, SK_MScalar1/2, SK_MScalar1/2);
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(3, 3, 3);
    rot.setRotateDegreesAbout(0, 0, -1, 90);
    mat.postConcat(rot);
    REPORTER_ASSERT(reporter, mat.invert(NULL));
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    iden2.setConcat(inverse, mat);
    REPORTER_ASSERT(reporter, is_identity(iden2));

    // test rol/col Major getters
    {
        mat.setTranslate(2, 3, 4);
        float dataf[16];
        double datad[16];

        mat.asColMajorf(dataf);
        assert16<float>(reporter, dataf,
                 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 2, 3, 4, 1);
        mat.asColMajord(datad);
        assert16<double>(reporter, datad, 1, 0, 0, 0,
                        0, 1, 0, 0,
                        0, 0, 1, 0,
                        2, 3, 4, 1);
        mat.asRowMajorf(dataf);
        assert16<float>(reporter, dataf, 1, 0, 0, 2,
                        0, 1, 0, 3,
                        0, 0, 1, 4,
                        0, 0, 0, 1);
        mat.asRowMajord(datad);
        assert16<double>(reporter, datad, 1, 0, 0, 2,
                        0, 1, 0, 3,
                        0, 0, 1, 4,
                        0, 0, 0, 1);
    }

    test_concat(reporter);

    if (false) { // avoid bit rot, suppress warning (working on making this pass)
        test_common_angles(reporter);
    }

    test_constructor(reporter);
    test_gettype(reporter);
    test_determinant(reporter);
    test_transpose(reporter);
    test_get_set_double(reporter);
    test_set_row_col_major(reporter);
    test_translate(reporter);
    test_scale(reporter);
    test_map2(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Matrix44", Matrix44TestClass, TestMatrix44)
