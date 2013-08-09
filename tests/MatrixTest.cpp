
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Test.h"
#include "SkMath.h"
#include "SkMatrix.h"
#include "SkMatrixUtils.h"
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

static bool are_equal(skiatest::Reporter* reporter,
                      const SkMatrix& a,
                      const SkMatrix& b) {
    bool equal = a == b;
    bool cheapEqual = a.cheapEqualTo(b);
    if (equal != cheapEqual) {
#ifdef SK_SCALAR_IS_FLOAT
        if (equal) {
            bool foundZeroSignDiff = false;
            for (int i = 0; i < 9; ++i) {
                float aVal = a.get(i);
                float bVal = b.get(i);
                int aValI = *SkTCast<int*>(&aVal);
                int bValI = *SkTCast<int*>(&bVal);
                if (0 == aVal && 0 == bVal && aValI != bValI) {
                    foundZeroSignDiff = true;
                } else {
                    REPORTER_ASSERT(reporter, aVal == bVal && aValI == aValI);
                }
            }
            REPORTER_ASSERT(reporter, foundZeroSignDiff);
        } else {
            bool foundNaN = false;
            for (int i = 0; i < 9; ++i) {
                float aVal = a.get(i);
                float bVal = b.get(i);
                int aValI = *SkTCast<int*>(&aVal);
                int bValI = *SkTCast<int*>(&bVal);
                if (sk_float_isnan(aVal) && aValI == bValI) {
                    foundNaN = true;
                } else {
                    REPORTER_ASSERT(reporter, aVal == bVal && aValI == bValI);
                }
            }
            REPORTER_ASSERT(reporter, foundNaN);
        }
#else
        REPORTER_ASSERT(reporter, false);
#endif
    }
    return equal;
}

static bool is_identity(const SkMatrix& m) {
    SkMatrix identity;
    identity.reset();
    return nearly_equal(m, identity);
}

static void test_matrix_recttorect(skiatest::Reporter* reporter) {
    SkRect src, dst;
    SkMatrix matrix;

    src.set(0, 0, SK_Scalar1*10, SK_Scalar1*10);
    dst = src;
    matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    REPORTER_ASSERT(reporter, SkMatrix::kIdentity_Mask == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());

    dst.offset(SK_Scalar1, SK_Scalar1);
    matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    REPORTER_ASSERT(reporter, SkMatrix::kTranslate_Mask == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());

    dst.fRight += SK_Scalar1;
    matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    REPORTER_ASSERT(reporter,
                    (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask) == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());

    dst = src;
    dst.fRight = src.fRight * 2;
    matrix.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    REPORTER_ASSERT(reporter, SkMatrix::kScale_Mask == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());
}

static void test_flatten(skiatest::Reporter* reporter, const SkMatrix& m) {
    // add 100 in case we have a bug, I don't want to kill my stack in the test
    char buffer[SkMatrix::kMaxFlattenSize + 100];
    uint32_t size1 = m.writeToMemory(NULL);
    uint32_t size2 = m.writeToMemory(buffer);
    REPORTER_ASSERT(reporter, size1 == size2);
    REPORTER_ASSERT(reporter, size1 <= SkMatrix::kMaxFlattenSize);

    SkMatrix m2;
    uint32_t size3 = m2.readFromMemory(buffer);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, are_equal(reporter, m, m2));

    char buffer2[SkMatrix::kMaxFlattenSize + 100];
    size3 = m2.writeToMemory(buffer2);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, memcmp(buffer, buffer2, size1) == 0);
}

static void test_matrix_max_stretch(skiatest::Reporter* reporter) {
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
    SkMWCRandom rand;
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

static void test_matrix_is_similarity(skiatest::Reporter* reporter) {
    SkMatrix mat;

    // identity
    mat.setIdentity();
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // translation only
    mat.reset();
    mat.setTranslate(SkIntToScalar(100), SkIntToScalar(100));
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // scale with same size
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(15));
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // scale with one negative
    mat.reset();
    mat.setScale(SkIntToScalar(-15), SkIntToScalar(15));
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // scale with different size
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(20));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // scale with same size at a pivot point
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(15),
                 SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // scale with different size at a pivot point
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(20),
                 SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // skew with same size
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(15));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // skew with different size
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(20));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // skew with same size at a pivot point
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(15),
                SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // skew with different size at a pivot point
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(20),
                SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // perspective x
    mat.reset();
    mat.setPerspX(SkScalarToPersp(SK_Scalar1 / 2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // perspective y
    mat.reset();
    mat.setPerspY(SkScalarToPersp(SK_Scalar1 / 2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

#ifdef SK_SCALAR_IS_FLOAT
    /* We bypass the following tests for SK_SCALAR_IS_FIXED build.
     * The long discussion can be found in this issue:
     *     http://codereview.appspot.com/5999050/
     * In short, we haven't found a perfect way to fix the precision
     * issue, i.e. the way we use tolerance in isSimilarityTransformation
     * is incorrect. The situation becomes worse in fixed build, so
     * we disabled rotation related tests for fixed build.
     */

    // rotate
    for (int angle = 0; angle < 360; ++angle) {
        mat.reset();
        mat.setRotate(SkIntToScalar(angle));
        REPORTER_ASSERT(reporter, mat.isSimilarity());
    }

    // see if there are any accumulated precision issues
    mat.reset();
    for (int i = 1; i < 360; i++) {
        mat.postRotate(SkIntToScalar(1));
    }
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // rotate + translate
    mat.reset();
    mat.setRotate(SkIntToScalar(30));
    mat.postTranslate(SkIntToScalar(10), SkIntToScalar(20));
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // rotate + uniform scale
    mat.reset();
    mat.setRotate(SkIntToScalar(30));
    mat.postScale(SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, mat.isSimilarity());

    // rotate + non-uniform scale
    mat.reset();
    mat.setRotate(SkIntToScalar(30));
    mat.postScale(SkIntToScalar(3), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
#endif

    // all zero
    mat.setAll(0, 0, 0, 0, 0, 0, 0, 0, 0);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // all zero except perspective
    mat.setAll(0, 0, 0, 0, 0, 0, 0, 0, SK_Scalar1);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());

    // scales zero, only skews
    mat.setAll(0, SK_Scalar1, 0,
               SK_Scalar1, 0, 0,
               0, 0, SkMatrix::I()[8]);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
}

// For test_matrix_decomposition, below.
static bool scalar_nearly_equal_relative(SkScalar a, SkScalar b,
                                         SkScalar tolerance = SK_ScalarNearlyZero) {
    // from Bruce Dawson
    SkScalar diff = SkScalarAbs(a - b);
    if (diff < tolerance) {
        return true;
    }

    a = SkScalarAbs(a);
    b = SkScalarAbs(b);
    SkScalar largest = (b > a) ? b : a;

    if (diff <= largest*tolerance) {
        return true;
    }

    return false;
}

static void test_matrix_decomposition(skiatest::Reporter* reporter) {
    SkMatrix mat;
    SkScalar rotation0, scaleX, scaleY, rotation1;

    const float kRotation0 = 15.5f;
    const float kRotation1 = -50.f;
    const float kScale0 = 5000.f;
    const float kScale1 = 0.001f;

    // identity
    mat.reset();
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, SK_Scalar1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, SK_Scalar1));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));
    // make sure it doesn't crash if we pass in NULLs
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, NULL, NULL, NULL, NULL));

    // rotation only
    mat.setRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation0, SkDegreesToRadians(kRotation0)));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, SK_Scalar1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, SK_Scalar1));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // uniform scale only
    mat.setScale(kScale0, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // anisotropic scale only
    mat.setScale(kScale1, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // rotation then uniform scale
    mat.setRotate(kRotation1);
    mat.postScale(kScale0, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation0, SkDegreesToRadians(kRotation1)));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // uniform scale then rotation
    mat.setScale(kScale0, kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation0, SkDegreesToRadians(kRotation1)));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // rotation then uniform scale+reflection
    mat.setRotate(kRotation0);
    mat.postScale(kScale1, -kScale1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation0, SkDegreesToRadians(kRotation0)));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, -kScale1));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // uniform scale+reflection, then rotate
    mat.setScale(kScale0, -kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation0, SkDegreesToRadians(-kRotation1)));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, -kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // rotation then anisotropic scale
    mat.setRotate(kRotation1);
    mat.postScale(kScale1, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation0, SkDegreesToRadians(kRotation1)));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // anisotropic scale then rotation
    mat.setScale(kScale1, kScale0);
    mat.postRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation1, SkDegreesToRadians(kRotation0)));

    // rotation, uniform scale, then different rotation
    mat.setRotate(kRotation1);
    mat.postScale(kScale0, kScale0);
    mat.postRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(rotation0,
                                                  SkDegreesToRadians(kRotation0 + kRotation1)));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleX, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(scaleY, kScale0));
    REPORTER_ASSERT(reporter, SkScalarNearlyZero(rotation1));

    // rotation, anisotropic scale, then different rotation
    mat.setRotate(kRotation0);
    mat.postScale(kScale1, kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    // Because of the shear/skew we won't get the same results, so we need to multiply it out.
    // Generating the matrices requires doing a radian-to-degree calculation, then degree-to-radian
    // calculation (in setRotate()), which adds error, so this just computes the matrix elements
    // directly.
    SkScalar c0;
    SkScalar s0 = SkScalarSinCos(rotation0, &c0);
    SkScalar c1;
    SkScalar s1 = SkScalarSinCos(rotation1, &c1);
    // We do a relative check here because large scale factors cause problems with an absolute check
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleX],
                                                           scaleX*c0*c1 - scaleY*s0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewX],
                                                           -scaleX*s0*c1 - scaleY*c0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewY],
                                                           scaleX*c0*s1 + scaleY*s0*c1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleY],
                                                           -scaleX*s0*s1 + scaleY*c0*c1));

    // try some random matrices
    SkMWCRandom rand;
    for (int m = 0; m < 1000; ++m) {
        SkScalar rot0 = rand.nextRangeF(-SK_ScalarPI, SK_ScalarPI);
        SkScalar sx = rand.nextRangeF(-3000.f, 3000.f);
        SkScalar sy = rand.nextRangeF(-3000.f, 3000.f);
        SkScalar rot1 = rand.nextRangeF(-SK_ScalarPI, SK_ScalarPI);
        mat.setRotate(rot0);
        mat.postScale(sx, sy);
        mat.postRotate(rot1);

        if (SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1)) {
            SkScalar c0;
            SkScalar s0 = SkScalarSinCos(rotation0, &c0);
            SkScalar c1;
            SkScalar s1 = SkScalarSinCos(rotation1, &c1);
            REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleX],
                                                                   scaleX*c0*c1 - scaleY*s0*s1));
            REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewX],
                                                                   -scaleX*s0*c1 - scaleY*c0*s1));
            REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewY],
                                                                   scaleX*c0*s1 + scaleY*s0*c1));
            REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleY],
                                                                   -scaleX*s0*s1 + scaleY*c0*c1));
        } else {
            // if the matrix is degenerate, the basis vectors should be near-parallel or near-zero
            SkScalar perpdot = mat[SkMatrix::kMScaleX]*mat[SkMatrix::kMScaleY] -
                               mat[SkMatrix::kMSkewX]*mat[SkMatrix::kMSkewY];
            REPORTER_ASSERT(reporter, SkScalarNearlyZero(perpdot));
        }
    }

    // translation shouldn't affect this
    mat.postTranslate(-1000.f, 1000.f);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    s0 = SkScalarSinCos(rotation0, &c0);
    s1 = SkScalarSinCos(rotation1, &c1);
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleX],
                                                           scaleX*c0*c1 - scaleY*s0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewX],
                                                           -scaleX*s0*c1 - scaleY*c0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewY],
                                                           scaleX*c0*s1 + scaleY*s0*c1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleY],
                                                           -scaleX*s0*s1 + scaleY*c0*c1));

    // perspective shouldn't affect this
    mat[SkMatrix::kMPersp0] = 12.f;
    mat[SkMatrix::kMPersp1] = 4.f;
    mat[SkMatrix::kMPersp2] = 1872.f;
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    s0 = SkScalarSinCos(rotation0, &c0);
    s1 = SkScalarSinCos(rotation1, &c1);
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleX],
                                                           scaleX*c0*c1 - scaleY*s0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewX],
                                                           -scaleX*s0*c1 - scaleY*c0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewY],
                                                           scaleX*c0*s1 + scaleY*s0*c1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleY],
                                                           -scaleX*s0*s1 + scaleY*c0*c1));

    // rotation, anisotropic scale + reflection, then different rotation
    mat.setRotate(kRotation0);
    mat.postScale(-kScale1, kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    s0 = SkScalarSinCos(rotation0, &c0);
    s1 = SkScalarSinCos(rotation1, &c1);
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleX],
                                                           scaleX*c0*c1 - scaleY*s0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewX],
                                                           -scaleX*s0*c1 - scaleY*c0*s1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMSkewY],
                                                           scaleX*c0*s1 + scaleY*s0*c1));
    REPORTER_ASSERT(reporter, scalar_nearly_equal_relative(mat[SkMatrix::kMScaleY],
                                                           -scaleX*s0*s1 + scaleY*c0*c1));

    // degenerate matrices
    // mostly zero entries
    mat.reset();
    mat[SkMatrix::kMScaleX] = 0.f;
    REPORTER_ASSERT(reporter, !SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    mat.reset();
    mat[SkMatrix::kMScaleY] = 0.f;
    REPORTER_ASSERT(reporter, !SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
    mat.reset();
    // linearly dependent entries
    mat[SkMatrix::kMScaleX] = 1.f;
    mat[SkMatrix::kMSkewX] = 2.f;
    mat[SkMatrix::kMSkewY] = 4.f;
    mat[SkMatrix::kMScaleY] = 8.f;
    REPORTER_ASSERT(reporter, !SkDecomposeUpper2x2(mat, &rotation0, &scaleX, &scaleY, &rotation1));
}

static void TestMatrix(skiatest::Reporter* reporter) {
    SkMatrix    mat, inverse, iden1, iden2;

    mat.reset();
    mat.setTranslate(SK_Scalar1, SK_Scalar1);
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(SkIntToScalar(2), SkIntToScalar(4));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    test_flatten(reporter, mat);

    mat.setScale(SK_Scalar1/2, SkIntToScalar(2));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    test_flatten(reporter, mat);

    mat.setScale(SkIntToScalar(3), SkIntToScalar(5), SkIntToScalar(20), 0);
    mat.postRotate(SkIntToScalar(25));
    REPORTER_ASSERT(reporter, mat.invert(NULL));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    iden2.setConcat(inverse, mat);
    REPORTER_ASSERT(reporter, is_identity(iden2));
    test_flatten(reporter, mat);
    test_flatten(reporter, iden2);

    mat.setScale(0, SK_Scalar1);
    REPORTER_ASSERT(reporter, !mat.invert(NULL));
    REPORTER_ASSERT(reporter, !mat.invert(&inverse));
    mat.setScale(SK_Scalar1, 0);
    REPORTER_ASSERT(reporter, !mat.invert(NULL));
    REPORTER_ASSERT(reporter, !mat.invert(&inverse));

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

    SkMatrix mat2;
    mat2.reset();
    mat.reset();
    SkScalar zero = 0;
    mat.set(SkMatrix::kMSkewX, -zero);
    REPORTER_ASSERT(reporter, are_equal(reporter, mat, mat2));

    mat2.reset();
    mat.reset();
    mat.set(SkMatrix::kMSkewX, SK_ScalarNaN);
    mat2.set(SkMatrix::kMSkewX, SK_ScalarNaN);
    // fixed pt doesn't have the property that NaN does not equal itself.
#ifdef SK_SCALAR_IS_FIXED
    REPORTER_ASSERT(reporter, are_equal(reporter, mat, mat2));
#else
    REPORTER_ASSERT(reporter, !are_equal(reporter, mat, mat2));
#endif

    test_matrix_max_stretch(reporter);
    test_matrix_is_similarity(reporter);
    test_matrix_recttorect(reporter);
    test_matrix_decomposition(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Matrix", MatrixTestClass, TestMatrix)
