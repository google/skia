/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix44.h"
#include "include/core/SkPoint3.h"
#include "tests/Test.h"

static bool nearly_equal_double(double a, double b) {
    const double tolerance = 1e-7;
    double diff = a - b;
    if (diff < 0)
        diff = -diff;
    return diff <= tolerance;
}

static bool nearly_equal_mscalar(SkMScalar a, SkMScalar b) {
    const SkMScalar tolerance = SK_MScalar1 / 200000;

    return SkTAbs<SkMScalar>(a - b) <= tolerance;
}

static bool nearly_equal_scalar(SkScalar a, SkScalar b) {
    const SkScalar tolerance = SK_Scalar1 / 200000;
    return SkScalarAbs(a - b) <= tolerance;
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
            if (!nearly_equal_mscalar(a.get(i, j), b.get(i, j))) {
                SkDebugf("not equal %g %g\n", a.get(i, j), b.get(i, j));
                return false;
            }
        }
    }
    return true;
}

static bool is_identity(const SkMatrix44& m) {
    SkMatrix44 identity(SkMatrix44::kIdentity_Constructor);
    return nearly_equal(m, identity);
}

///////////////////////////////////////////////////////////////////////////////
static bool bits_isonly(int value, int mask) {
    return 0 == (value & ~mask);
}

static void test_constructor(skiatest::Reporter* reporter) {
    // Allocate a matrix on the heap
    SkMatrix44* placeholderMatrix = new SkMatrix44;
    std::unique_ptr<SkMatrix44> deleteMe(placeholderMatrix);

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

    // Verify that that constructing from an SkMatrix initializes everything.
    SkMatrix44 scaleMatrix;
    scaleMatrix.setScale(3, 4, 5);
    REPORTER_ASSERT(reporter, scaleMatrix.isScale());
    testMatrix = new(&scaleMatrix) SkMatrix44(SkMatrix::I());
    REPORTER_ASSERT(reporter, testMatrix->isIdentity());
    REPORTER_ASSERT(reporter, *testMatrix == SkMatrix44::I());
}

static void test_translate(skiatest::Reporter* reporter) {
    SkMatrix44 mat;
    SkMatrix44 inverse;

    mat.setTranslate(0, 0, 0);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kIdentity_Mask));
    mat.setTranslate(1, 2, 3);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kTranslate_Mask));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    REPORTER_ASSERT(reporter, bits_isonly(inverse.getType(), SkMatrix44::kTranslate_Mask));

    SkMatrix44 a,b,c;
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
    SkMatrix44 mat;
    SkMatrix44 inverse;

    mat.setScale(1, 1, 1);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kIdentity_Mask));
    mat.setScale(1, 2, 3);
    REPORTER_ASSERT(reporter, bits_isonly(mat.getType(), SkMatrix44::kScale_Mask));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    REPORTER_ASSERT(reporter, bits_isonly(inverse.getType(), SkMatrix44::kScale_Mask));

    SkMatrix44 a,b,c;
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
        dstA[i] = SkDoubleToMScalar(123456789);
        dstB[i] = SkDoubleToMScalar(987654321);
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
    SkMatrix44 matrix(SkMatrix44::kIdentity_Constructor);

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
    SkMatrix44 a,b,c,d;

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
    SkMatrix44 a(SkMatrix44::kIdentity_Constructor);
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

static void test_invert(skiatest::Reporter* reporter) {
    SkMatrix44 inverse;
    double inverseData[16];

    SkMatrix44 identity(SkMatrix44::kIdentity_Constructor);
    identity.invert(&inverse);
    inverse.asRowMajord(inverseData);
    assert16<double>(reporter, inverseData,
                     1, 0, 0, 0,
                     0, 1, 0, 0,
                     0, 0, 1, 0,
                     0, 0, 0, 1);

    SkMatrix44 translation;
    translation.setTranslate(2, 3, 4);
    translation.invert(&inverse);
    inverse.asRowMajord(inverseData);
    assert16<double>(reporter, inverseData,
                     1, 0, 0, -2,
                     0, 1, 0, -3,
                     0, 0, 1, -4,
                     0, 0, 0, 1);

    SkMatrix44 scale;
    scale.setScale(2, 4, 8);
    scale.invert(&inverse);
    inverse.asRowMajord(inverseData);
    assert16<double>(reporter, inverseData,
                     0.5, 0,    0,     0,
                     0,   0.25, 0,     0,
                     0,   0,    0.125, 0,
                     0,   0,    0,     1);

    SkMatrix44 scaleTranslation;
    scaleTranslation.setScale(32, 128, 1024);
    scaleTranslation.preTranslate(2, 3, 4);
    scaleTranslation.invert(&inverse);
    inverse.asRowMajord(inverseData);
    assert16<double>(reporter, inverseData,
                     0.03125,  0,          0,            -2,
                     0,        0.0078125,  0,            -3,
                     0,        0,          0.0009765625, -4,
                     0,        0,          0,             1);

    SkMatrix44 rotation;
    rotation.setRotateDegreesAbout(0, 0, 1, 90);
    rotation.invert(&inverse);
    SkMatrix44 expected;
    double expectedInverseRotation[16] =
            {0,  1, 0, 0,
             -1, 0, 0, 0,
             0,  0, 1, 0,
             0,  0, 0, 1};
    expected.setRowMajord(expectedInverseRotation);
    REPORTER_ASSERT(reporter, nearly_equal(expected, inverse));

    SkMatrix44 affine;
    affine.setRotateDegreesAbout(0, 0, 1, 90);
    affine.preScale(10, 20, 100);
    affine.preTranslate(2, 3, 4);
    affine.invert(&inverse);
    double expectedInverseAffine[16] =
            {0,    0.1,  0,   -2,
             -0.05, 0,   0,   -3,
             0,     0,  0.01, -4,
             0,     0,   0,   1};
    expected.setRowMajord(expectedInverseAffine);
    REPORTER_ASSERT(reporter, nearly_equal(expected, inverse));

    SkMatrix44 perspective(SkMatrix44::kIdentity_Constructor);
    perspective.setDouble(3, 2, 1.0);
    perspective.invert(&inverse);
    double expectedInversePerspective[16] =
            {1, 0,  0, 0,
             0, 1,  0, 0,
             0, 0,  1, 0,
             0, 0, -1, 1};
    expected.setRowMajord(expectedInversePerspective);
    REPORTER_ASSERT(reporter, nearly_equal(expected, inverse));

    SkMatrix44 affineAndPerspective(SkMatrix44::kIdentity_Constructor);
    affineAndPerspective.setDouble(3, 2, 1.0);
    affineAndPerspective.preScale(10, 20, 100);
    affineAndPerspective.preTranslate(2, 3, 4);
    affineAndPerspective.invert(&inverse);
    double expectedInverseAffineAndPerspective[16] =
            {0.1, 0,    2,   -2,
             0,  0.05,  3,   -3,
             0,   0,   4.01, -4,
             0,   0,   -1,    1};
    expected.setRowMajord(expectedInverseAffineAndPerspective);
    REPORTER_ASSERT(reporter, nearly_equal(expected, inverse));

    SkMatrix44 tinyScale(SkMatrix44::kIdentity_Constructor);
    tinyScale.setDouble(0, 0, 1e-39);
    REPORTER_ASSERT(reporter, tinyScale.getType() == SkMatrix44::kScale_Mask);
    REPORTER_ASSERT(reporter, !tinyScale.invert(nullptr));
    REPORTER_ASSERT(reporter, !tinyScale.invert(&inverse));

    SkMatrix44 tinyScaleTranslate(SkMatrix44::kIdentity_Constructor);
    tinyScaleTranslate.setDouble(0, 0, 1e-38);
    REPORTER_ASSERT(reporter, tinyScaleTranslate.invert(nullptr));
    tinyScaleTranslate.setDouble(0, 3, 10);
    REPORTER_ASSERT(
        reporter, tinyScaleTranslate.getType() ==
                      (SkMatrix44::kScale_Mask | SkMatrix44::kTranslate_Mask));
    REPORTER_ASSERT(reporter, !tinyScaleTranslate.invert(nullptr));
    REPORTER_ASSERT(reporter, !tinyScaleTranslate.invert(&inverse));

    SkMatrix44 tinyScalePerspective(SkMatrix44::kIdentity_Constructor);
    tinyScalePerspective.setDouble(0, 0, 1e-39);
    tinyScalePerspective.setDouble(3, 2, -1);
    REPORTER_ASSERT(reporter, (tinyScalePerspective.getType() &
                               SkMatrix44::kPerspective_Mask) ==
                                  SkMatrix44::kPerspective_Mask);
    REPORTER_ASSERT(reporter, !tinyScalePerspective.invert(nullptr));
    REPORTER_ASSERT(reporter, !tinyScalePerspective.invert(&inverse));
}

static void test_transpose(skiatest::Reporter* reporter) {
    SkMatrix44 a,b;

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

static void test_set_3x3(skiatest::Reporter* r) {
    static float vals[9] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, };

    SkMatrix44 mat;
    mat.set3x3RowMajorf(vals);

    REPORTER_ASSERT(r, 1.0f == mat.getFloat(0, 0));
    REPORTER_ASSERT(r, 2.0f == mat.getFloat(0, 1));
    REPORTER_ASSERT(r, 3.0f == mat.getFloat(0, 2));
    REPORTER_ASSERT(r, 4.0f == mat.getFloat(1, 0));
    REPORTER_ASSERT(r, 5.0f == mat.getFloat(1, 1));
    REPORTER_ASSERT(r, 6.0f == mat.getFloat(1, 2));
    REPORTER_ASSERT(r, 7.0f == mat.getFloat(2, 0));
    REPORTER_ASSERT(r, 8.0f == mat.getFloat(2, 1));
    REPORTER_ASSERT(r, 9.0f == mat.getFloat(2, 2));
}

static void test_set_row_col_major(skiatest::Reporter* reporter) {
    SkMatrix44 a,b;

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

static void test_3x3_conversion(skiatest::Reporter* reporter) {
    SkMScalar values4x4[16] = { 1, 2, 3, 4,
                                5, 6, 7, 8,
                                9, 10, 11, 12,
                                13, 14, 15, 16 };
    SkScalar values3x3[9] = { 1, 2, 4,
                              5, 6, 8,
                              13, 14, 16 };
    SkMScalar values4x4flattened[16] = { 1, 2, 0, 4,
                                         5, 6, 0, 8,
                                         0, 0, 1, 0,
                                         13, 14, 0, 16 };
    SkMatrix44 a44;
    a44.setRowMajor(values4x4);

    SkMatrix a33 = a44;
    SkMatrix expected33;
    for (int i = 0; i < 9; i++) expected33[i] = values3x3[i];
    REPORTER_ASSERT(reporter, expected33 == a33);

    SkMatrix44 a44flattened = a33;
    SkMatrix44 expected44flattened;
    expected44flattened.setRowMajor(values4x4flattened);
    REPORTER_ASSERT(reporter, nearly_equal(a44flattened, expected44flattened));

    // Test that a point with a Z value of 0 is transformed the same way.
    SkScalar vec4[4] = { 2, 4, 0, 8 };
    SkPoint3 vec3 = { 2, 4, 8 };

    SkScalar vec4transformed[4];
    SkPoint3 vec3transformed;
    SkScalar vec4transformed2[4];
    a44.mapScalars(vec4, vec4transformed);
    a33.mapHomogeneousPoints(&vec3transformed, &vec3, 1);
    a44flattened.mapScalars(vec4, vec4transformed2);
    REPORTER_ASSERT(reporter, nearly_equal_scalar(vec4transformed[0], vec3transformed.fX));
    REPORTER_ASSERT(reporter, nearly_equal_scalar(vec4transformed[1], vec3transformed.fY));
    REPORTER_ASSERT(reporter, nearly_equal_scalar(vec4transformed[3], vec3transformed.fZ));
    REPORTER_ASSERT(reporter, nearly_equal_scalar(vec4transformed[0], vec4transformed2[0]));
    REPORTER_ASSERT(reporter, nearly_equal_scalar(vec4transformed[1], vec4transformed2[1]));
    REPORTER_ASSERT(reporter, !nearly_equal_scalar(vec4transformed[2], vec4transformed2[2]));
    REPORTER_ASSERT(reporter, nearly_equal_scalar(vec4transformed[3], vec4transformed2[3]));
}

static void test_has_perspective(skiatest::Reporter* reporter) {
    SkMatrix44 transform(SkMatrix44::kIdentity_Constructor);

    transform.setDouble(3, 2, -0.1);
    REPORTER_ASSERT(reporter, transform.hasPerspective());

    transform.reset();
    REPORTER_ASSERT(reporter, !transform.hasPerspective());

    transform.setDouble(3, 0, -1.0);
    REPORTER_ASSERT(reporter, transform.hasPerspective());

    transform.reset();
    transform.setDouble(3, 1, -1.0);
    REPORTER_ASSERT(reporter, transform.hasPerspective());

    transform.reset();
    transform.setDouble(3, 2, -0.3);
    REPORTER_ASSERT(reporter, transform.hasPerspective());

    transform.reset();
    transform.setDouble(3, 3, 0.5);
    REPORTER_ASSERT(reporter, transform.hasPerspective());

    transform.reset();
    transform.setDouble(3, 3, 0.0);
    REPORTER_ASSERT(reporter, transform.hasPerspective());
}

static bool is_rectilinear (SkVector4& p1, SkVector4& p2, SkVector4& p3, SkVector4& p4) {
    return (SkScalarNearlyEqual(p1.fData[0], p2.fData[0]) &&
            SkScalarNearlyEqual(p2.fData[1], p3.fData[1]) &&
            SkScalarNearlyEqual(p3.fData[0], p4.fData[0]) &&
            SkScalarNearlyEqual(p4.fData[1], p1.fData[1])) ||
           (SkScalarNearlyEqual(p1.fData[1], p2.fData[1]) &&
            SkScalarNearlyEqual(p2.fData[0], p3.fData[0]) &&
            SkScalarNearlyEqual(p3.fData[1], p4.fData[1]) &&
            SkScalarNearlyEqual(p4.fData[0], p1.fData[0]));
}

static SkVector4 mul_with_persp_divide(const SkMatrix44& transform, const SkVector4& target) {
    SkVector4 result = transform * target;
    if (result.fData[3] != 0.0f && result.fData[3] != SK_Scalar1) {
        float wInverse = SK_Scalar1 / result.fData[3];
        result.set(result.fData[0] * wInverse,
                   result.fData[1] * wInverse,
                   result.fData[2] * wInverse,
                   SK_Scalar1);
    }
    return result;
}

static bool empirically_preserves_2d_axis_alignment(skiatest::Reporter* reporter,
                                                    const SkMatrix44& transform) {
  SkVector4 p1(5.0f, 5.0f, 0.0f);
  SkVector4 p2(10.0f, 5.0f, 0.0f);
  SkVector4 p3(10.0f, 20.0f, 0.0f);
  SkVector4 p4(5.0f, 20.0f, 0.0f);

  REPORTER_ASSERT(reporter, is_rectilinear(p1, p2, p3, p4));

  p1 = mul_with_persp_divide(transform, p1);
  p2 = mul_with_persp_divide(transform, p2);
  p3 = mul_with_persp_divide(transform, p3);
  p4 = mul_with_persp_divide(transform, p4);

  return is_rectilinear(p1, p2, p3, p4);
}

static void test(bool expected, skiatest::Reporter* reporter, const SkMatrix44& transform) {
    if (expected) {
        REPORTER_ASSERT(reporter, empirically_preserves_2d_axis_alignment(reporter, transform));
        REPORTER_ASSERT(reporter, transform.preserves2dAxisAlignment());
    } else {
        REPORTER_ASSERT(reporter, !empirically_preserves_2d_axis_alignment(reporter, transform));
        REPORTER_ASSERT(reporter, !transform.preserves2dAxisAlignment());
    }
}

static void test_preserves_2d_axis_alignment(skiatest::Reporter* reporter) {
  SkMatrix44 transform;
  SkMatrix44 transform2;

  static const struct TestCase {
    SkMScalar a; // row 1, column 1
    SkMScalar b; // row 1, column 2
    SkMScalar c; // row 2, column 1
    SkMScalar d; // row 2, column 2
    bool expected;
  } test_cases[] = {
    { 3.f, 0.f,
      0.f, 4.f, true }, // basic case
    { 0.f, 4.f,
      3.f, 0.f, true }, // rotate by 90
    { 0.f, 0.f,
      0.f, 4.f, true }, // degenerate x
    { 3.f, 0.f,
      0.f, 0.f, true }, // degenerate y
    { 0.f, 0.f,
      3.f, 0.f, true }, // degenerate x + rotate by 90
    { 0.f, 4.f,
      0.f, 0.f, true }, // degenerate y + rotate by 90
    { 3.f, 4.f,
      0.f, 0.f, false },
    { 0.f, 0.f,
      3.f, 4.f, false },
    { 0.f, 3.f,
      0.f, 4.f, false },
    { 3.f, 0.f,
      4.f, 0.f, false },
    { 3.f, 4.f,
      5.f, 0.f, false },
    { 3.f, 4.f,
      0.f, 5.f, false },
    { 3.f, 0.f,
      4.f, 5.f, false },
    { 0.f, 3.f,
      4.f, 5.f, false },
    { 2.f, 3.f,
      4.f, 5.f, false },
  };

  for (size_t i = 0; i < sizeof(test_cases)/sizeof(TestCase); ++i) {
    const TestCase& value = test_cases[i];
    transform.setIdentity();
    transform.set(0, 0, value.a);
    transform.set(0, 1, value.b);
    transform.set(1, 0, value.c);
    transform.set(1, 1, value.d);

    test(value.expected, reporter, transform);
  }

  // Try the same test cases again, but this time make sure that other matrix
  // elements (except perspective) have entries, to test that they are ignored.
  for (size_t i = 0; i < sizeof(test_cases)/sizeof(TestCase); ++i) {
    const TestCase& value = test_cases[i];
    transform.setIdentity();
    transform.set(0, 0, value.a);
    transform.set(0, 1, value.b);
    transform.set(1, 0, value.c);
    transform.set(1, 1, value.d);

    transform.set(0, 2, 1.f);
    transform.set(0, 3, 2.f);
    transform.set(1, 2, 3.f);
    transform.set(1, 3, 4.f);
    transform.set(2, 0, 5.f);
    transform.set(2, 1, 6.f);
    transform.set(2, 2, 7.f);
    transform.set(2, 3, 8.f);

    test(value.expected, reporter, transform);
  }

  // Try the same test cases again, but this time add perspective which is
  // always assumed to not-preserve axis alignment.
  for (size_t i = 0; i < sizeof(test_cases)/sizeof(TestCase); ++i) {
    const TestCase& value = test_cases[i];
    transform.setIdentity();
    transform.set(0, 0, value.a);
    transform.set(0, 1, value.b);
    transform.set(1, 0, value.c);
    transform.set(1, 1, value.d);

    transform.set(0, 2, 1.f);
    transform.set(0, 3, 2.f);
    transform.set(1, 2, 3.f);
    transform.set(1, 3, 4.f);
    transform.set(2, 0, 5.f);
    transform.set(2, 1, 6.f);
    transform.set(2, 2, 7.f);
    transform.set(2, 3, 8.f);
    transform.set(3, 0, 9.f);
    transform.set(3, 1, 10.f);
    transform.set(3, 2, 11.f);
    transform.set(3, 3, 12.f);

    test(false, reporter, transform);
  }

  // Try a few more practical situations to check precision
  // Reuse TestCase (a, b, c, d) as (x, y, z, degrees) axis to rotate about.
  TestCase rotation_tests[] = {
    { 0.0, 0.0, 1.0, 90.0, true },
    { 0.0, 0.0, 1.0, 180.0, true },
    { 0.0, 0.0, 1.0, 270.0, true },
    { 0.0, 1.0, 0.0, 90.0, true },
    { 1.0, 0.0, 0.0, 90.0, true },
    { 0.0, 0.0, 1.0, 45.0, false },
    // In 3d these next two are non-preserving, but we're testing in 2d after
    // orthographic projection, where they are.
    { 0.0, 1.0, 0.0, 45.0, true },
    { 1.0, 0.0, 0.0, 45.0, true },
  };

  for (size_t i = 0; i < sizeof(rotation_tests)/sizeof(TestCase); ++i) {
    const TestCase& value = rotation_tests[i];
    transform.setRotateDegreesAbout(value.a, value.b, value.c, value.d);
    test(value.expected, reporter, transform);
  }

  static const struct DoubleRotationCase {
    SkMScalar x1;
    SkMScalar y1;
    SkMScalar z1;
    SkMScalar degrees1;
    SkMScalar x2;
    SkMScalar y2;
    SkMScalar z2;
    SkMScalar degrees2;
    bool expected;
  } double_rotation_tests[] = {
    { 0.0, 0.0, 1.0, 90.0, 0.0, 1.0, 0.0, 90.0, true },
    { 0.0, 0.0, 1.0, 90.0, 1.0, 0.0, 0.0, 90.0, true },
    { 0.0, 1.0, 0.0, 90.0, 0.0, 0.0, 1.0, 90.0, true },
  };

  for (size_t i = 0; i < sizeof(double_rotation_tests)/sizeof(DoubleRotationCase); ++i) {
    const DoubleRotationCase& value = double_rotation_tests[i];
    transform.setRotateDegreesAbout(value.x1, value.y1, value.z1, value.degrees1);
    transform2.setRotateDegreesAbout(value.x2, value.y2, value.z2, value.degrees2);
    transform.postConcat(transform2);
    test(value.expected, reporter, transform);
  }

  // Perspective cases.
  transform.setIdentity();
  transform.setDouble(3, 2, -0.1); // Perspective depth 10
  transform2.setRotateDegreesAbout(0.0, 1.0, 0.0, 45.0);
  transform.preConcat(transform2);
  test(false, reporter, transform);

  transform.setIdentity();
  transform.setDouble(3, 2, -0.1); // Perspective depth 10
  transform2.setRotateDegreesAbout(0.0, 0.0, 1.0, 90.0);
  transform.preConcat(transform2);
  test(true, reporter, transform);
}

// just want to exercise the various converters for MScalar
static void test_toint(skiatest::Reporter* reporter) {
    SkMatrix44 mat;
    mat.setScale(3, 3, 3);

    SkMScalar sum = SkMScalarFloor(mat.get(0, 0)) +
                    SkMScalarRound(mat.get(1, 0)) +
                    SkMScalarCeil(mat.get(2, 0));
    int isum =      SkMScalarFloorToInt(mat.get(0, 1)) +
                    SkMScalarRoundToInt(mat.get(1, 2)) +
                    SkMScalarCeilToInt(mat.get(2, 3));
    REPORTER_ASSERT(reporter, sum >= 0);
    REPORTER_ASSERT(reporter, isum >= 0);
    REPORTER_ASSERT(reporter, static_cast<SkMScalar>(isum) == SkIntToMScalar(isum));
}

DEF_TEST(Matrix44, reporter) {
    SkMatrix44 mat;
    SkMatrix44 inverse;
    SkMatrix44 iden1;
    SkMatrix44 iden2;
    SkMatrix44 rot;

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
    REPORTER_ASSERT(reporter, mat.invert(nullptr));
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    iden2.setConcat(inverse, mat);
    REPORTER_ASSERT(reporter, is_identity(iden2));

    // test tiny-valued matrix inverse
    mat.reset();
    auto v = SkDoubleToMScalar(1.0e-12);
    mat.setScale(v,v,v);
    rot.setRotateDegreesAbout(0, 0, -1, 90);
    mat.postConcat(rot);
    mat.postTranslate(v,v,v);
    REPORTER_ASSERT(reporter, mat.invert(nullptr));
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    // test mixed-valued matrix inverse
    mat.reset();
    mat.setScale(SkDoubleToMScalar(1.0e-2),
                 SkDoubleToMScalar(3.0),
                 SkDoubleToMScalar(1.0e+2));
    rot.setRotateDegreesAbout(0, 0, -1, 90);
    mat.postConcat(rot);
    mat.postTranslate(SkDoubleToMScalar(1.0e+2),
                      SkDoubleToMScalar(3.0),
                      SkDoubleToMScalar(1.0e-2));
    REPORTER_ASSERT(reporter, mat.invert(nullptr));
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    // test degenerate matrix
    mat.reset();
    mat.set3x3(1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    REPORTER_ASSERT(reporter, !mat.invert(nullptr));

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
    test_invert(reporter);
    test_transpose(reporter);
    test_get_set_double(reporter);
    test_set_row_col_major(reporter);
    test_set_3x3(reporter);
    test_translate(reporter);
    test_scale(reporter);
    test_map2(reporter);
    test_3x3_conversion(reporter);
    test_has_perspective(reporter);
    test_preserves_2d_axis_alignment(reporter);
    test_toint(reporter);
}
