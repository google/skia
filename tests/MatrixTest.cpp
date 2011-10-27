
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkMath.h"
#include "SkMatrix.h"
#include "SkRandom.h"

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

static bool nearly_equal(const SkMatrix& a, const SkMatrix& b) {
    for (int i = 0; i < 9; i++) {
        if (!nearly_equal_scalar(a[i], b[i])) {
            printf("not equal %g %g\n", (float)a[i], (float)b[i]);
            return false;
        }
    }
    return true;
}

static bool is_identity(const SkMatrix& m) {
    SkMatrix identity;
    identity.reset();
    return nearly_equal(m, identity);
}

static void test_flatten(skiatest::Reporter* reporter, const SkMatrix& m) {
    // add 100 in case we have a bug, I don't want to kill my stack in the test
    char buffer[SkMatrix::kMaxFlattenSize + 100];
    uint32_t size1 = m.flatten(NULL);
    uint32_t size2 = m.flatten(buffer);
    REPORTER_ASSERT(reporter, size1 == size2);
    REPORTER_ASSERT(reporter, size1 <= SkMatrix::kMaxFlattenSize);
    
    SkMatrix m2;
    uint32_t size3 = m2.unflatten(buffer);
    REPORTER_ASSERT(reporter, size1 == size2);
    REPORTER_ASSERT(reporter, m == m2);
    
    char buffer2[SkMatrix::kMaxFlattenSize + 100];
    size3 = m2.flatten(buffer2);
    REPORTER_ASSERT(reporter, size1 == size2);
    REPORTER_ASSERT(reporter, memcmp(buffer, buffer2, size1) == 0);
}

void test_matrix_max_stretch(skiatest::Reporter* reporter) {
    SkMatrix identity;
    identity.reset();
    REPORTER_ASSERT(reporter, SK_Scalar1 == identity.getMaxStretch());

    SkMatrix scale;
    scale.setScale(SK_Scalar1 * 2, SK_Scalar1 * 4);
    REPORTER_ASSERT(reporter, SK_Scalar1 * 4 == scale.getMaxStretch());

    SkMatrix rot90Scale;
    rot90Scale.setRotate(90 * SK_Scalar1);
    rot90Scale.postScale(SK_Scalar1 / 4, SK_Scalar1 / 2);
    REPORTER_ASSERT(reporter, SK_Scalar1 / 2 == rot90Scale.getMaxStretch());

    SkMatrix rotate;
    rotate.setRotate(128 * SK_Scalar1);
    REPORTER_ASSERT(reporter, SkScalarAbs(SK_Scalar1 - rotate.getMaxStretch()) <= SK_ScalarNearlyZero);

    SkMatrix translate;
    translate.setTranslate(10 * SK_Scalar1, -5 * SK_Scalar1);
    REPORTER_ASSERT(reporter, SK_Scalar1 == translate.getMaxStretch());

    SkMatrix perspX;
    perspX.reset();
    perspX.setPerspX(SkScalarToPersp(SK_Scalar1 / 1000));
    REPORTER_ASSERT(reporter, -SK_Scalar1 == perspX.getMaxStretch());

    SkMatrix perspY;
    perspY.reset();
    perspY.setPerspX(SkScalarToPersp(-SK_Scalar1 / 500));
    REPORTER_ASSERT(reporter, -SK_Scalar1 == perspY.getMaxStretch());

    SkMatrix baseMats[] = {scale, rot90Scale, rotate,
                           translate, perspX, perspY};
    SkMatrix mats[2*SK_ARRAY_COUNT(baseMats)];
    for (size_t i = 0; i < SK_ARRAY_COUNT(baseMats); ++i) {
        mats[i] = baseMats[i];
        bool invertable = mats[i].invert(&mats[i + SK_ARRAY_COUNT(baseMats)]);
        REPORTER_ASSERT(reporter, invertable);
    }
    SkRandom rand;
    for (int m = 0; m < 1000; ++m) {
        SkMatrix mat;
        mat.reset();
        for (int i = 0; i < 4; ++i) {
            int x = rand.nextU() % SK_ARRAY_COUNT(mats);
            mat.postConcat(mats[x]);
        }
        SkScalar stretch = mat.getMaxStretch();
        
        if ((stretch < 0) != mat.hasPerspective()) {
            stretch = mat.getMaxStretch();
        }

        REPORTER_ASSERT(reporter, (stretch < 0) == mat.hasPerspective());

        if (mat.hasPerspective()) {
            m -= 1; // try another non-persp matrix
            continue;
        }

        // test a bunch of vectors. None should be scaled by more than stretch
        // (modulo some error) and we should find a vector that is scaled by
        // almost stretch.
        static const SkScalar gStretchTol = (105 * SK_Scalar1) / 100;
        static const SkScalar gMaxStretchTol = (97 * SK_Scalar1) / 100;
        SkScalar max = 0;
        SkVector vectors[1000];
        for (size_t i = 0; i < SK_ARRAY_COUNT(vectors); ++i) {
            vectors[i].fX = rand.nextSScalar1();
            vectors[i].fY = rand.nextSScalar1();
            if (!vectors[i].normalize()) {
                i -= 1;
                continue;
            }
        }
        mat.mapVectors(vectors, SK_ARRAY_COUNT(vectors));
        for (size_t i = 0; i < SK_ARRAY_COUNT(vectors); ++i) {
            SkScalar d = vectors[i].length();
            REPORTER_ASSERT(reporter, SkScalarDiv(d, stretch) < gStretchTol);
            if (max < d) {
                max = d;
            }
        }
        REPORTER_ASSERT(reporter, SkScalarDiv(max, stretch) >= gMaxStretchTol);
    }
}

void TestMatrix(skiatest::Reporter* reporter) {
    SkMatrix    mat, inverse, iden1, iden2;

    mat.reset();
    mat.setTranslate(SK_Scalar1, SK_Scalar1);
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(SkIntToScalar(2), SkIntToScalar(2));
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    test_flatten(reporter, mat);

    mat.setScale(SK_Scalar1/2, SK_Scalar1/2);
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    test_flatten(reporter, mat);

    mat.setScale(SkIntToScalar(3), SkIntToScalar(5), SkIntToScalar(20), 0);
    mat.postRotate(SkIntToScalar(25));
    REPORTER_ASSERT(reporter, mat.invert(NULL));
    mat.invert(&inverse);
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    iden2.setConcat(inverse, mat);
    REPORTER_ASSERT(reporter, is_identity(iden2));
    test_flatten(reporter, mat);
    test_flatten(reporter, iden2);

    // rectStaysRect test
    {
        static const struct {
            SkScalar    m00, m01, m10, m11;
            bool        mStaysRect;
        }
        gRectStaysRectSamples[] = {
            {          0,          0,          0,           0, false },
            {          0,          0,          0,  SK_Scalar1, false },
            {          0,          0, SK_Scalar1,           0, false },
            {          0,          0, SK_Scalar1,  SK_Scalar1, false },
            {          0, SK_Scalar1,          0,           0, false },
            {          0, SK_Scalar1,          0,  SK_Scalar1, false },
            {          0, SK_Scalar1, SK_Scalar1,           0, true },
            {          0, SK_Scalar1, SK_Scalar1,  SK_Scalar1, false },
            { SK_Scalar1,          0,          0,           0, false },
            { SK_Scalar1,          0,          0,  SK_Scalar1, true },
            { SK_Scalar1,          0, SK_Scalar1,           0, false },
            { SK_Scalar1,          0, SK_Scalar1,  SK_Scalar1, false },
            { SK_Scalar1, SK_Scalar1,          0,           0, false },
            { SK_Scalar1, SK_Scalar1,          0,  SK_Scalar1, false },
            { SK_Scalar1, SK_Scalar1, SK_Scalar1,           0, false },
            { SK_Scalar1, SK_Scalar1, SK_Scalar1,  SK_Scalar1, false }
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(gRectStaysRectSamples); i++) {
            SkMatrix    m;

            m.reset();
            m.set(SkMatrix::kMScaleX, gRectStaysRectSamples[i].m00);
            m.set(SkMatrix::kMSkewX,  gRectStaysRectSamples[i].m01);
            m.set(SkMatrix::kMSkewY,  gRectStaysRectSamples[i].m10);
            m.set(SkMatrix::kMScaleY, gRectStaysRectSamples[i].m11);
            REPORTER_ASSERT(reporter,
                    m.rectStaysRect() == gRectStaysRectSamples[i].mStaysRect);
        }
    }

    mat.reset();
    mat.set(SkMatrix::kMScaleX, SkIntToScalar(1));
    mat.set(SkMatrix::kMSkewX,  SkIntToScalar(2));
    mat.set(SkMatrix::kMTransX, SkIntToScalar(3));
    mat.set(SkMatrix::kMSkewY,  SkIntToScalar(4));
    mat.set(SkMatrix::kMScaleY, SkIntToScalar(5));
    mat.set(SkMatrix::kMTransY, SkIntToScalar(6));
    SkScalar affine[6];
    REPORTER_ASSERT(reporter, mat.asAffine(affine));

    #define affineEqual(e) affine[SkMatrix::kA##e] == mat.get(SkMatrix::kM##e)
    REPORTER_ASSERT(reporter, affineEqual(ScaleX));
    REPORTER_ASSERT(reporter, affineEqual(SkewY));
    REPORTER_ASSERT(reporter, affineEqual(SkewX));
    REPORTER_ASSERT(reporter, affineEqual(ScaleY));
    REPORTER_ASSERT(reporter, affineEqual(TransX));
    REPORTER_ASSERT(reporter, affineEqual(TransY));
    #undef affineEqual

    mat.set(SkMatrix::kMPersp1, SkScalarToPersp(SK_Scalar1 / 2));
    REPORTER_ASSERT(reporter, !mat.asAffine(affine));

    test_matrix_max_stretch(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Matrix", MatrixTestClass, TestMatrix)
