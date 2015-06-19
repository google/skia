/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMath.h"
#include "SkMatrix.h"
#include "SkMatrixUtils.h"
#include "SkRandom.h"
#include "Test.h"

static bool nearly_equal_scalar(SkScalar a, SkScalar b) {
    const SkScalar tolerance = SK_Scalar1 / 200000;
    return SkScalarAbs(a - b) <= tolerance;
}

static bool nearly_equal(const SkMatrix& a, const SkMatrix& b) {
    for (int i = 0; i < 9; i++) {
        if (!nearly_equal_scalar(a[i], b[i])) {
            SkDebugf("not equal %g %g\n", (float)a[i], (float)b[i]);
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
    }
    return equal;
}

static bool is_identity(const SkMatrix& m) {
    SkMatrix identity;
    identity.reset();
    return nearly_equal(m, identity);
}

static void assert9(skiatest::Reporter* reporter, const SkMatrix& m,
                    SkScalar a, SkScalar b, SkScalar c,
                    SkScalar d, SkScalar e, SkScalar f,
                    SkScalar g, SkScalar h, SkScalar i) {
    SkScalar buffer[9];
    m.get9(buffer);
    REPORTER_ASSERT(reporter, buffer[0] == a);
    REPORTER_ASSERT(reporter, buffer[1] == b);
    REPORTER_ASSERT(reporter, buffer[2] == c);
    REPORTER_ASSERT(reporter, buffer[3] == d);
    REPORTER_ASSERT(reporter, buffer[4] == e);
    REPORTER_ASSERT(reporter, buffer[5] == f);
    REPORTER_ASSERT(reporter, buffer[6] == g);
    REPORTER_ASSERT(reporter, buffer[7] == h);
    REPORTER_ASSERT(reporter, buffer[8] == i);
}

static void test_set9(skiatest::Reporter* reporter) {

    SkMatrix m;
    m.reset();
    assert9(reporter, m, 1, 0, 0, 0, 1, 0, 0, 0, 1);
    
    m.setScale(2, 3);
    assert9(reporter, m, 2, 0, 0, 0, 3, 0, 0, 0, 1);
    
    m.postTranslate(4, 5);
    assert9(reporter, m, 2, 0, 4, 0, 3, 5, 0, 0, 1);

    SkScalar buffer[9];
    sk_bzero(buffer, sizeof(buffer));
    buffer[SkMatrix::kMScaleX] = 1;
    buffer[SkMatrix::kMScaleY] = 1;
    buffer[SkMatrix::kMPersp2] = 1;
    REPORTER_ASSERT(reporter, !m.isIdentity());
    m.set9(buffer);
    REPORTER_ASSERT(reporter, m.isIdentity());
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
    static const size_t kBufferSize = SkMatrix::kMaxFlattenSize + 100;
    char buffer[kBufferSize];
    size_t size1 = m.writeToMemory(NULL);
    size_t size2 = m.writeToMemory(buffer);
    REPORTER_ASSERT(reporter, size1 == size2);
    REPORTER_ASSERT(reporter, size1 <= SkMatrix::kMaxFlattenSize);

    SkMatrix m2;
    size_t size3 = m2.readFromMemory(buffer, kBufferSize);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, are_equal(reporter, m, m2));

    char buffer2[kBufferSize];
    size3 = m2.writeToMemory(buffer2);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, memcmp(buffer, buffer2, size1) == 0);
}

static void test_matrix_min_max_scale(skiatest::Reporter* reporter) {
    SkScalar scales[2];
    bool success;

    SkMatrix identity;
    identity.reset();
    REPORTER_ASSERT(reporter, SK_Scalar1 == identity.getMinScale());
    REPORTER_ASSERT(reporter, SK_Scalar1 == identity.getMaxScale());
    success = identity.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && SK_Scalar1 == scales[0] && SK_Scalar1 == scales[1]);

    SkMatrix scale;
    scale.setScale(SK_Scalar1 * 2, SK_Scalar1 * 4);
    REPORTER_ASSERT(reporter, SK_Scalar1 * 2 == scale.getMinScale());
    REPORTER_ASSERT(reporter, SK_Scalar1 * 4 == scale.getMaxScale());
    success = scale.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && SK_Scalar1 * 2 == scales[0] && SK_Scalar1 * 4 == scales[1]);

    SkMatrix rot90Scale;
    rot90Scale.setRotate(90 * SK_Scalar1);
    rot90Scale.postScale(SK_Scalar1 / 4, SK_Scalar1 / 2);
    REPORTER_ASSERT(reporter, SK_Scalar1 / 4 == rot90Scale.getMinScale());
    REPORTER_ASSERT(reporter, SK_Scalar1 / 2 == rot90Scale.getMaxScale());
    success = rot90Scale.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && SK_Scalar1 / 4  == scales[0] && SK_Scalar1 / 2 == scales[1]);

    SkMatrix rotate;
    rotate.setRotate(128 * SK_Scalar1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SK_Scalar1, rotate.getMinScale(), SK_ScalarNearlyZero));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SK_Scalar1, rotate.getMaxScale(), SK_ScalarNearlyZero));
    success = rotate.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SK_Scalar1, scales[0], SK_ScalarNearlyZero));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(SK_Scalar1, scales[1], SK_ScalarNearlyZero));

    SkMatrix translate;
    translate.setTranslate(10 * SK_Scalar1, -5 * SK_Scalar1);
    REPORTER_ASSERT(reporter, SK_Scalar1 == translate.getMinScale());
    REPORTER_ASSERT(reporter, SK_Scalar1 == translate.getMaxScale());
    success = translate.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && SK_Scalar1 == scales[0] && SK_Scalar1 == scales[1]);

    SkMatrix perspX;
    perspX.reset();
    perspX.setPerspX(SK_Scalar1 / 1000);
    REPORTER_ASSERT(reporter, -SK_Scalar1 == perspX.getMinScale());
    REPORTER_ASSERT(reporter, -SK_Scalar1 == perspX.getMaxScale());
    // Verify that getMinMaxScales() doesn't update the scales array on failure.
    scales[0] = -5;
    scales[1] = -5;
    success = perspX.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, !success && -5 * SK_Scalar1 == scales[0] && -5 * SK_Scalar1  == scales[1]);

    SkMatrix perspY;
    perspY.reset();
    perspY.setPerspY(-SK_Scalar1 / 500);
    REPORTER_ASSERT(reporter, -SK_Scalar1 == perspY.getMinScale());
    REPORTER_ASSERT(reporter, -SK_Scalar1 == perspY.getMaxScale());
    scales[0] = -5;
    scales[1] = -5;
    success = perspY.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, !success && -5 * SK_Scalar1 == scales[0] && -5 * SK_Scalar1  == scales[1]);

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

        SkScalar minScale = mat.getMinScale();
        SkScalar maxScale = mat.getMaxScale();
        REPORTER_ASSERT(reporter, (minScale < 0) == (maxScale < 0));
        REPORTER_ASSERT(reporter, (maxScale < 0) == mat.hasPerspective());

        SkScalar scales[2];
        bool success = mat.getMinMaxScales(scales);
        REPORTER_ASSERT(reporter, success == !mat.hasPerspective());
        REPORTER_ASSERT(reporter, !success || (scales[0] == minScale && scales[1] == maxScale));

        if (mat.hasPerspective()) {
            m -= 1; // try another non-persp matrix
            continue;
        }

        // test a bunch of vectors. All should be scaled by between minScale and maxScale
        // (modulo some error) and we should find a vector that is scaled by almost each.
        static const SkScalar gVectorScaleTol = (105 * SK_Scalar1) / 100;
        static const SkScalar gCloseScaleTol = (97 * SK_Scalar1) / 100;
        SkScalar max = 0, min = SK_ScalarMax;
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
            REPORTER_ASSERT(reporter, d / maxScale < gVectorScaleTol);
            REPORTER_ASSERT(reporter, minScale / d < gVectorScaleTol);
            if (max < d) {
                max = d;
            }
            if (min > d) {
                min = d;
            }
        }
        REPORTER_ASSERT(reporter, max / maxScale >= gCloseScaleTol);
        REPORTER_ASSERT(reporter, minScale / min >= gCloseScaleTol);
    }
}

static void test_matrix_preserve_shape(skiatest::Reporter* reporter) {
    SkMatrix mat;

    // identity
    mat.setIdentity();
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // translation only
    mat.reset();
    mat.setTranslate(SkIntToScalar(100), SkIntToScalar(100));
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with same size
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(15));
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with one negative
    mat.reset();
    mat.setScale(SkIntToScalar(-15), SkIntToScalar(15));
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with different size
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(20));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with same size at a pivot point
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(15),
                 SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with different size at a pivot point
    mat.reset();
    mat.setScale(SkIntToScalar(15), SkIntToScalar(20),
                 SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // skew with same size
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(15));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // skew with different size
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(20));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // skew with same size at a pivot point
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(15),
                SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // skew with different size at a pivot point
    mat.reset();
    mat.setSkew(SkIntToScalar(15), SkIntToScalar(20),
                SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // perspective x
    mat.reset();
    mat.setPerspX(SK_Scalar1 / 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // perspective y
    mat.reset();
    mat.setPerspY(SK_Scalar1 / 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // rotate
    for (int angle = 0; angle < 360; ++angle) {
        mat.reset();
        mat.setRotate(SkIntToScalar(angle));
        REPORTER_ASSERT(reporter, mat.isSimilarity());
        REPORTER_ASSERT(reporter, mat.preservesRightAngles());
    }

    // see if there are any accumulated precision issues
    mat.reset();
    for (int i = 1; i < 360; i++) {
        mat.postRotate(SkIntToScalar(1));
    }
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // rotate + translate
    mat.reset();
    mat.setRotate(SkIntToScalar(30));
    mat.postTranslate(SkIntToScalar(10), SkIntToScalar(20));
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // rotate + uniform scale
    mat.reset();
    mat.setRotate(SkIntToScalar(30));
    mat.postScale(SkIntToScalar(2), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // rotate + non-uniform scale
    mat.reset();
    mat.setRotate(SkIntToScalar(30));
    mat.postScale(SkIntToScalar(3), SkIntToScalar(2));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // non-uniform scale + rotate
    mat.reset();
    mat.setScale(SkIntToScalar(3), SkIntToScalar(2));
    mat.postRotate(SkIntToScalar(30));
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // all zero
    mat.setAll(0, 0, 0, 0, 0, 0, 0, 0, 0);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // all zero except perspective
    mat.reset();
    mat.setAll(0, 0, 0, 0, 0, 0, 0, 0, SK_Scalar1);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // scales zero, only skews (rotation)
    mat.setAll(0, SK_Scalar1, 0,
               -SK_Scalar1, 0, 0,
               0, 0, SkMatrix::I()[8]);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scales zero, only skews (reflection)
    mat.setAll(0, SK_Scalar1, 0,
               SK_Scalar1, 0, 0,
               0, 0, SkMatrix::I()[8]);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());
}

// For test_matrix_decomposition, below.
static bool scalar_nearly_equal_relative(SkScalar a, SkScalar b,
                                         SkScalar tolerance = SK_ScalarNearlyZero) {
    // from Bruce Dawson
    // absolute check
    SkScalar diff = SkScalarAbs(a - b);
    if (diff < tolerance) {
        return true;
    }

    // relative check
    a = SkScalarAbs(a);
    b = SkScalarAbs(b);
    SkScalar largest = (b > a) ? b : a;

    if (diff <= largest*tolerance) {
        return true;
    }

    return false;
}

static bool check_matrix_recomposition(const SkMatrix& mat,
                                       const SkPoint& rotation1,
                                       const SkPoint& scale,
                                       const SkPoint& rotation2) {
    SkScalar c1 = rotation1.fX;
    SkScalar s1 = rotation1.fY;
    SkScalar scaleX = scale.fX;
    SkScalar scaleY = scale.fY;
    SkScalar c2 = rotation2.fX;
    SkScalar s2 = rotation2.fY;

    // We do a relative check here because large scale factors cause problems with an absolute check
    bool result = scalar_nearly_equal_relative(mat[SkMatrix::kMScaleX],
                                               scaleX*c1*c2 - scaleY*s1*s2) &&
                  scalar_nearly_equal_relative(mat[SkMatrix::kMSkewX],
                                               -scaleX*s1*c2 - scaleY*c1*s2) &&
                  scalar_nearly_equal_relative(mat[SkMatrix::kMSkewY],
                                               scaleX*c1*s2 + scaleY*s1*c2) &&
                  scalar_nearly_equal_relative(mat[SkMatrix::kMScaleY],
                                               -scaleX*s1*s2 + scaleY*c1*c2);
    return result;
}

static void test_matrix_decomposition(skiatest::Reporter* reporter) {
    SkMatrix mat;
    SkPoint rotation1, scale, rotation2;

    const float kRotation0 = 15.5f;
    const float kRotation1 = -50.f;
    const float kScale0 = 5000.f;
    const float kScale1 = 0.001f;

    // identity
    mat.reset();
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));
    // make sure it doesn't crash if we pass in NULLs
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, NULL, NULL, NULL));

    // rotation only
    mat.setRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // uniform scale only
    mat.setScale(kScale0, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // anisotropic scale only
    mat.setScale(kScale1, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation then uniform scale
    mat.setRotate(kRotation1);
    mat.postScale(kScale0, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // uniform scale then rotation
    mat.setScale(kScale0, kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation then uniform scale+reflection
    mat.setRotate(kRotation0);
    mat.postScale(kScale1, -kScale1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // uniform scale+reflection, then rotate
    mat.setScale(kScale0, -kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation then anisotropic scale
    mat.setRotate(kRotation1);
    mat.postScale(kScale1, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation then anisotropic scale
    mat.setRotate(90);
    mat.postScale(kScale1, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // anisotropic scale then rotation
    mat.setScale(kScale1, kScale0);
    mat.postRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // anisotropic scale then rotation
    mat.setScale(kScale1, kScale0);
    mat.postRotate(90);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation, uniform scale, then different rotation
    mat.setRotate(kRotation1);
    mat.postScale(kScale0, kScale0);
    mat.postRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation, anisotropic scale, then different rotation
    mat.setRotate(kRotation0);
    mat.postScale(kScale1, kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation, anisotropic scale + reflection, then different rotation
    mat.setRotate(kRotation0);
    mat.postScale(-kScale1, kScale0);
    mat.postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // try some random matrices
    SkRandom rand;
    for (int m = 0; m < 1000; ++m) {
        SkScalar rot0 = rand.nextRangeF(-180, 180);
        SkScalar sx = rand.nextRangeF(-3000.f, 3000.f);
        SkScalar sy = rand.nextRangeF(-3000.f, 3000.f);
        SkScalar rot1 = rand.nextRangeF(-180, 180);
        mat.setRotate(rot0);
        mat.postScale(sx, sy);
        mat.postRotate(rot1);

        if (SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2)) {
            REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));
        } else {
            // if the matrix is degenerate, the basis vectors should be near-parallel or near-zero
            SkScalar perpdot = mat[SkMatrix::kMScaleX]*mat[SkMatrix::kMScaleY] -
                               mat[SkMatrix::kMSkewX]*mat[SkMatrix::kMSkewY];
            REPORTER_ASSERT(reporter, SkScalarNearlyZero(perpdot));
        }
    }

    // translation shouldn't affect this
    mat.postTranslate(-1000.f, 1000.f);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // perspective shouldn't affect this
    mat[SkMatrix::kMPersp0] = 12.f;
    mat[SkMatrix::kMPersp1] = 4.f;
    mat[SkMatrix::kMPersp2] = 1872.f;
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // degenerate matrices
    // mostly zero entries
    mat.reset();
    mat[SkMatrix::kMScaleX] = 0.f;
    REPORTER_ASSERT(reporter, !SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    mat.reset();
    mat[SkMatrix::kMScaleY] = 0.f;
    REPORTER_ASSERT(reporter, !SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    mat.reset();
    // linearly dependent entries
    mat[SkMatrix::kMScaleX] = 1.f;
    mat[SkMatrix::kMSkewX] = 2.f;
    mat[SkMatrix::kMSkewY] = 4.f;
    mat[SkMatrix::kMScaleY] = 8.f;
    REPORTER_ASSERT(reporter, !SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
}

// For test_matrix_homogeneous, below.
static bool scalar_array_nearly_equal_relative(const SkScalar a[], const SkScalar b[], int count) {
    for (int i = 0; i < count; ++i) {
        if (!scalar_nearly_equal_relative(a[i], b[i])) {
            return false;
        }
    }
    return true;
}

// For test_matrix_homogeneous, below.
// Maps a single triple in src using m and compares results to those in dst
static bool naive_homogeneous_mapping(const SkMatrix& m, const SkScalar src[3],
                                      const SkScalar dst[3]) {
    SkScalar res[3];
    SkScalar ms[9] = {m[0], m[1], m[2],
                      m[3], m[4], m[5],
                      m[6], m[7], m[8]};
    res[0] = src[0] * ms[0] + src[1] * ms[1] + src[2] * ms[2];
    res[1] = src[0] * ms[3] + src[1] * ms[4] + src[2] * ms[5];
    res[2] = src[0] * ms[6] + src[1] * ms[7] + src[2] * ms[8];
    return scalar_array_nearly_equal_relative(res, dst, 3);
}

static void test_matrix_homogeneous(skiatest::Reporter* reporter) {
    SkMatrix mat;

    const float kRotation0 = 15.5f;
    const float kRotation1 = -50.f;
    const float kScale0 = 5000.f;

    const int kTripleCount = 1000;
    const int kMatrixCount = 1000;
    SkRandom rand;

    SkScalar randTriples[3*kTripleCount];
    for (int i = 0; i < 3*kTripleCount; ++i) {
        randTriples[i] = rand.nextRangeF(-3000.f, 3000.f);
    }

    SkMatrix mats[kMatrixCount];
    for (int i = 0; i < kMatrixCount; ++i) {
        for (int j = 0; j < 9; ++j) {
            mats[i].set(j, rand.nextRangeF(-3000.f, 3000.f));
        }
    }

    // identity
    {
    mat.reset();
    SkScalar dst[3*kTripleCount];
    mat.mapHomogeneousPoints(dst, randTriples, kTripleCount);
    REPORTER_ASSERT(reporter, scalar_array_nearly_equal_relative(randTriples, dst, kTripleCount*3));
    }

    // zero matrix
    {
    mat.setAll(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
    SkScalar dst[3*kTripleCount];
    mat.mapHomogeneousPoints(dst, randTriples, kTripleCount);
    SkScalar zeros[3] = {0.f, 0.f, 0.f};
    for (int i = 0; i < kTripleCount; ++i) {
        REPORTER_ASSERT(reporter, scalar_array_nearly_equal_relative(&dst[i*3], zeros, 3));
    }
    }

    // zero point
    {
    SkScalar zeros[3] = {0.f, 0.f, 0.f};
    for (int i = 0; i < kMatrixCount; ++i) {
        SkScalar dst[3];
        mats[i].mapHomogeneousPoints(dst, zeros, 1);
        REPORTER_ASSERT(reporter, scalar_array_nearly_equal_relative(dst, zeros, 3));
    }
    }

    // doesn't crash with null dst, src, count == 0
    {
    mats[0].mapHomogeneousPoints(NULL, NULL, 0);
    }

    // uniform scale of point
    {
    mat.setScale(kScale0, kScale0);
    SkScalar dst[3];
    SkScalar src[3] = {randTriples[0], randTriples[1], 1.f};
    SkPoint pnt;
    pnt.set(src[0], src[1]);
    mat.mapHomogeneousPoints(dst, src, 1);
    mat.mapPoints(&pnt, &pnt, 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[0], pnt.fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[1], pnt.fY));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[2], SK_Scalar1));
    }

    // rotation of point
    {
    mat.setRotate(kRotation0);
    SkScalar dst[3];
    SkScalar src[3] = {randTriples[0], randTriples[1], 1.f};
    SkPoint pnt;
    pnt.set(src[0], src[1]);
    mat.mapHomogeneousPoints(dst, src, 1);
    mat.mapPoints(&pnt, &pnt, 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[0], pnt.fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[1], pnt.fY));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[2], SK_Scalar1));
    }

    // rotation, scale, rotation of point
    {
    mat.setRotate(kRotation1);
    mat.postScale(kScale0, kScale0);
    mat.postRotate(kRotation0);
    SkScalar dst[3];
    SkScalar src[3] = {randTriples[0], randTriples[1], 1.f};
    SkPoint pnt;
    pnt.set(src[0], src[1]);
    mat.mapHomogeneousPoints(dst, src, 1);
    mat.mapPoints(&pnt, &pnt, 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[0], pnt.fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[1], pnt.fY));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst[2], SK_Scalar1));
    }

    // compare with naive approach
    {
    for (int i = 0; i < kMatrixCount; ++i) {
        for (int j = 0; j < kTripleCount; ++j) {
            SkScalar dst[3];
            mats[i].mapHomogeneousPoints(dst, &randTriples[j*3], 1);
            REPORTER_ASSERT(reporter, naive_homogeneous_mapping(mats[i], &randTriples[j*3], dst));
        }
    }
    }

}

static bool check_decompScale(const SkMatrix& matrix) {
    SkSize scale;
    SkMatrix remaining;

    if (!matrix.decomposeScale(&scale, &remaining)) {
        return false;
    }
    if (scale.width() <= 0 || scale.height() <= 0) {
        return false;
    }
    remaining.preScale(scale.width(), scale.height());
    return nearly_equal(matrix, remaining);
}

static void test_decompScale(skiatest::Reporter* reporter) {
    SkMatrix m;

    m.reset();
    REPORTER_ASSERT(reporter, check_decompScale(m));
    m.setScale(2, 3);
    REPORTER_ASSERT(reporter, check_decompScale(m));
    m.setRotate(35, 0, 0);
    REPORTER_ASSERT(reporter, check_decompScale(m));

    m.setScale(1, 0);
    REPORTER_ASSERT(reporter, !check_decompScale(m));
}

DEF_TEST(Matrix, reporter) {
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

    // Inverting this matrix results in a non-finite matrix
    mat.setAll(0.0f, 1.0f, 2.0f,
               0.0f, 1.0f, -3.40277175e+38f,
               1.00003040f, 1.0f, 0.0f);
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

    mat.set(SkMatrix::kMPersp1, SK_Scalar1 / 2);
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
    REPORTER_ASSERT(reporter, !are_equal(reporter, mat, mat2));

    test_matrix_min_max_scale(reporter);
    test_matrix_preserve_shape(reporter);
    test_matrix_recttorect(reporter);
    test_matrix_decomposition(reporter);
    test_matrix_homogeneous(reporter);
    test_set9(reporter);

    test_decompScale(reporter);
}

DEF_TEST(Matrix_Concat, r) {
    SkMatrix a;
    a.setTranslate(10, 20);

    SkMatrix b;
    b.setScale(3, 5);

    SkMatrix expected;
    expected.setConcat(a,b);

    REPORTER_ASSERT(r, expected == SkMatrix::Concat(a, b));
}
