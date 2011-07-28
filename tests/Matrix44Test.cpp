
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkMatrix44.h"

static bool nearly_equal_scalar(SkMScalar a, SkMScalar b) {
    // Note that we get more compounded error for multiple operations when
    // SK_SCALAR_IS_FIXED.
#ifdef SK_SCALAR_IS_FLOAT
    const SkScalar tolerance = SK_Scalar1 / 200000;
#else
    const SkScalar tolerance = SK_Scalar1 / 1024;
#endif

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

static void test_common_angles(skiatest::Reporter* reporter) {
    SkMatrix44 rot;
    // Test precision of rotation in common cases
    int common_angles[] = { 0, 90, -90, 180, -180, 270, -270, 360, -360 };
    for (int i = 0; i < 9; ++i) {
        rot.setRotateDegreesAbout(0, 0, -1, common_angles[i]);

        SkMatrix rot3x3 = rot;
        REPORTER_ASSERT(reporter, rot3x3.rectStaysRect());
    }
}

void TestMatrix44(skiatest::Reporter* reporter) {
#ifdef SK_SCALAR_IS_FLOAT
    SkMatrix44 mat, inverse, iden1, iden2, rot;

    mat.reset();
    mat.setTranslate(SK_Scalar1, SK_Scalar1, SK_Scalar1);
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(SkIntToScalar(2), SkIntToScalar(2), SkIntToScalar(2));
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(SK_Scalar1/2, SK_Scalar1/2, SK_Scalar1/2);
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(SkIntToScalar(3), SkIntToScalar(5), SkIntToScalar(20));
    rot.setRotateDegreesAbout(
        SkIntToScalar(0),
        SkIntToScalar(0),
        SkIntToScalar(-1),
        SkIntToScalar(90));
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

#if 0   // working on making this pass
    test_common_angles(reporter);
#endif
#endif
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Matrix44", Matrix44TestClass, TestMatrix44)
