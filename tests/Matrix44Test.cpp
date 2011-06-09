#include "Test.h"
#include "SkMatrix44.h"

static bool nearly_equal_scalar(SkScalar a, SkScalar b) {
    // Note that we get more compounded error for multiple operations when
    // SK_SCALAR_IS_FIXED.
#ifdef SK_SCALAR_IS_FLOAT
    const SkScalar tolerance = SK_Scalar1 / 200000;
#else
    const SkScalar tolerance = SK_Scalar1 / 1024;
#endif

    return SkScalarAbs(a - b) <= tolerance;
}

static bool nearly_equal(const SkMatrix44& a, const SkMatrix44& b) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (!nearly_equal_scalar(a.get(i, j), b.get(i, j))) {
                printf("not equal %g %g\n", (float)a.get(i, j), (float)b.get(i, j));
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


void TestMatrix44(skiatest::Reporter* reporter) {
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
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Matrix44", Matrix44TestClass, TestMatrix44)
