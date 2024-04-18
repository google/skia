/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "src/base/SkRandom.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMatrixUtils.h"
#include "src/core/SkPointPriv.h"
#include "tests/Test.h"

#include <cstring>
#include <initializer_list>
#include <string>

static bool nearly_equal_scalar(SkScalar a, SkScalar b) {
    const SkScalar tolerance = SK_Scalar1 / 200000;
    return SkScalarAbs(a - b) <= tolerance;
}

static bool nearly_equal(const SkMatrix& a, const SkMatrix& b) {
    for (int i = 0; i < 9; i++) {
        if (!nearly_equal_scalar(a[i], b[i])) {
            SkDebugf("matrices not equal [%d] %g %g\n", i, (float)a[i], (float)b[i]);
            return false;
        }
    }
    return true;
}

static int float_bits(float f) {
    int result;
    memcpy(&result, &f, 4);
    return result;
}

static bool are_equal(skiatest::Reporter* reporter,
                      const SkMatrix& a,
                      const SkMatrix& b) {
    bool equal = a == b;
    bool cheapEqual = SkMatrixPriv::CheapEqual(a, b);
    if (equal != cheapEqual) {
        if (equal) {
            bool foundZeroSignDiff = false;
            for (int i = 0; i < 9; ++i) {
                float aVal = a.get(i);
                float bVal = b.get(i);
                int aValI = float_bits(aVal);
                int bValI = float_bits(bVal);
                if (0 == aVal && 0 == bVal && aValI != bValI) {
                    foundZeroSignDiff = true;
                } else {
                    REPORTER_ASSERT(reporter, aVal == bVal && aValI == bValI);
                }
            }
            REPORTER_ASSERT(reporter, foundZeroSignDiff);
        } else {
            bool foundNaN = false;
            for (int i = 0; i < 9; ++i) {
                float aVal = a.get(i);
                float bVal = b.get(i);
                int aValI = float_bits(aVal);
                int bValI = float_bits(bVal);
                if (std::isnan(aVal) && aValI == bValI) {
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

    REPORTER_ASSERT(reporter, m.rc(0, 0) == a);
    REPORTER_ASSERT(reporter, m.rc(0, 1) == b);
    REPORTER_ASSERT(reporter, m.rc(0, 2) == c);
    REPORTER_ASSERT(reporter, m.rc(1, 0) == d);
    REPORTER_ASSERT(reporter, m.rc(1, 1) == e);
    REPORTER_ASSERT(reporter, m.rc(1, 2) == f);
    REPORTER_ASSERT(reporter, m.rc(2, 0) == g);
    REPORTER_ASSERT(reporter, m.rc(2, 1) == h);
    REPORTER_ASSERT(reporter, m.rc(2, 2) == i);
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

    src.setLTRB(0, 0, 10, 10);
    dst = src;
    matrix = SkMatrix::RectToRect(src, dst);
    REPORTER_ASSERT(reporter, SkMatrix::kIdentity_Mask == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());

    dst.offset(1, 1);
    matrix = SkMatrix::RectToRect(src, dst);
    REPORTER_ASSERT(reporter, SkMatrix::kTranslate_Mask == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());

    dst.fRight += 1;
    matrix = SkMatrix::RectToRect(src, dst);
    REPORTER_ASSERT(reporter,
                    (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask) == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());

    dst = src;
    dst.fRight = src.fRight * 2;
    matrix = SkMatrix::RectToRect(src, dst);
    REPORTER_ASSERT(reporter, SkMatrix::kScale_Mask == matrix.getType());
    REPORTER_ASSERT(reporter, matrix.rectStaysRect());
}

static void test_flatten(skiatest::Reporter* reporter, const SkMatrix& m) {
    // add 100 in case we have a bug, I don't want to kill my stack in the test
    static const size_t kBufferSize = SkMatrixPriv::kMaxFlattenSize + 100;
    char buffer[kBufferSize];
    size_t size1 = SkMatrixPriv::WriteToMemory(m, nullptr);
    size_t size2 = SkMatrixPriv::WriteToMemory(m, buffer);
    REPORTER_ASSERT(reporter, size1 == size2);
    REPORTER_ASSERT(reporter, size1 <= SkMatrixPriv::kMaxFlattenSize);

    SkMatrix m2;
    size_t size3 = SkMatrixPriv::ReadFromMemory(&m2, buffer, kBufferSize);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, are_equal(reporter, m, m2));

    char buffer2[kBufferSize];
    size3 = SkMatrixPriv::WriteToMemory(m2, buffer2);
    REPORTER_ASSERT(reporter, size1 == size3);
    REPORTER_ASSERT(reporter, memcmp(buffer, buffer2, size1) == 0);
}

static void test_matrix_min_max_scale(skiatest::Reporter* reporter) {
    SkScalar scales[2];
    bool success;

    SkMatrix identity;
    identity.reset();
    REPORTER_ASSERT(reporter, 1 == identity.getMinScale());
    REPORTER_ASSERT(reporter, 1 == identity.getMaxScale());
    success = identity.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && 1 == scales[0] && 1 == scales[1]);

    SkMatrix scale;
    scale.setScale(2, 4);
    REPORTER_ASSERT(reporter, 2 == scale.getMinScale());
    REPORTER_ASSERT(reporter, 4 == scale.getMaxScale());
    success = scale.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && 2 == scales[0] && 4 == scales[1]);

    SkMatrix rot90Scale;
    rot90Scale.setRotate(90).postScale(SK_Scalar1 / 4, SK_Scalar1 / 2);
    REPORTER_ASSERT(reporter, SK_Scalar1 / 4 == rot90Scale.getMinScale());
    REPORTER_ASSERT(reporter, SK_Scalar1 / 2 == rot90Scale.getMaxScale());
    success = rot90Scale.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && SK_Scalar1 / 4  == scales[0] && SK_Scalar1 / 2 == scales[1]);

    SkMatrix rotate;
    rotate.setRotate(128);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(1, rotate.getMinScale(), SK_ScalarNearlyZero));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(1, rotate.getMaxScale(), SK_ScalarNearlyZero));
    success = rotate.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(1, scales[0], SK_ScalarNearlyZero));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(1, scales[1], SK_ScalarNearlyZero));

    SkMatrix translate;
    translate.setTranslate(10, -5);
    REPORTER_ASSERT(reporter, 1 == translate.getMinScale());
    REPORTER_ASSERT(reporter, 1 == translate.getMaxScale());
    success = translate.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && 1 == scales[0] && 1 == scales[1]);

    SkMatrix perspX;
    perspX.reset().setPerspX(SK_Scalar1 / 1000);
    REPORTER_ASSERT(reporter, -1 == perspX.getMinScale());
    REPORTER_ASSERT(reporter, -1 == perspX.getMaxScale());
    success = perspX.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, !success);

    // skbug.com/4718
    SkMatrix big;
    big.setAll(2.39394089e+36f, 8.85347779e+36f, 9.26526204e+36f,
               3.9159619e+36f, 1.44823453e+37f, 1.51559342e+37f,
               0.f, 0.f, 1.f);
    success = big.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, !success);

    // skbug.com/4718
    SkMatrix givingNegativeNearlyZeros;
    givingNegativeNearlyZeros.setAll(0.00436534f, 0.114138f, 0.37141f,
                                     0.00358857f, 0.0936228f, -0.0174198f,
                                     0.f, 0.f, 1.f);
    success = givingNegativeNearlyZeros.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, success && 0 == scales[0]);

    SkMatrix perspY;
    perspY.reset().setPerspY(-SK_Scalar1 / 500);
    REPORTER_ASSERT(reporter, -1 == perspY.getMinScale());
    REPORTER_ASSERT(reporter, -1 == perspY.getMaxScale());
    scales[0] = -5;
    scales[1] = -5;
    success = perspY.getMinMaxScales(scales);
    REPORTER_ASSERT(reporter, !success && -5 == scales[0] && -5  == scales[1]);

    SkMatrix baseMats[] = {scale, rot90Scale, rotate,
                           translate, perspX, perspY};
    SkMatrix mats[2*std::size(baseMats)];
    for (size_t i = 0; i < std::size(baseMats); ++i) {
        mats[i] = baseMats[i];
        bool invertible = mats[i].invert(&mats[i + std::size(baseMats)]);
        REPORTER_ASSERT(reporter, invertible);
    }
    SkRandom rand;
    for (int m = 0; m < 1000; ++m) {
        SkMatrix mat;
        mat.reset();
        for (int i = 0; i < 4; ++i) {
            int x = rand.nextU() % std::size(mats);
            mat.postConcat(mats[x]);
        }

        SkScalar minScale = mat.getMinScale();
        SkScalar maxScale = mat.getMaxScale();
        REPORTER_ASSERT(reporter, (minScale < 0) == (maxScale < 0));
        REPORTER_ASSERT(reporter, (maxScale < 0) == mat.hasPerspective());

        success = mat.getMinMaxScales(scales);
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
        for (size_t i = 0; i < std::size(vectors); ++i) {
            vectors[i].fX = rand.nextSScalar1();
            vectors[i].fY = rand.nextSScalar1();
            if (!vectors[i].normalize()) {
                i -= 1;
                continue;
            }
        }
        mat.mapVectors(vectors, std::size(vectors));
        for (size_t i = 0; i < std::size(vectors); ++i) {
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
    mat.setTranslate(100, 100);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with same size
    mat.setScale(15, 15);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with one negative
    mat.setScale(-15, 15);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with different size
    mat.setScale(15, 20);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with same size at a pivot point
    mat.setScale(15, 15, 2, 2);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scale with different size at a pivot point
    mat.setScale(15, 20, 2, 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // skew with same size
    mat.setSkew(15, 15);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // skew with different size
    mat.setSkew(15, 20);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // skew with same size at a pivot point
    mat.setSkew(15, 15, 2, 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // skew with different size at a pivot point
    mat.setSkew(15, 20, 2, 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // perspective x
    mat.reset().setPerspX(SK_Scalar1 / 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // perspective y
    mat.reset().setPerspY(SK_Scalar1 / 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // rotate
    for (int angle = 0; angle < 360; ++angle) {
        mat.setRotate(SkIntToScalar(angle));
        REPORTER_ASSERT(reporter, mat.isSimilarity());
        REPORTER_ASSERT(reporter, mat.preservesRightAngles());
    }

    // see if there are any accumulated precision issues
    mat.reset();
    for (int i = 1; i < 360; i++) {
        mat.postRotate(1);
    }
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // rotate + translate
    mat.setRotate(30).postTranslate(10, 20);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // rotate + uniform scale
    mat.setRotate(30).postScale(2, 2);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // rotate + non-uniform scale
    mat.setRotate(30).postScale(3, 2);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // non-uniform scale + rotate
    mat.setScale(3, 2).postRotate(30);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // all zero
    mat.setAll(0, 0, 0, 0, 0, 0, 0, 0, 0);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // all zero except perspective
    mat.setAll(0, 0, 0, 0, 0, 0, 0, 0, 1);
    REPORTER_ASSERT(reporter, !mat.isSimilarity());
    REPORTER_ASSERT(reporter, !mat.preservesRightAngles());

    // scales zero, only skews (rotation)
    mat.setAll(0, 1, 0,
               -1, 0, 0,
               0, 0, 1);
    REPORTER_ASSERT(reporter, mat.isSimilarity());
    REPORTER_ASSERT(reporter, mat.preservesRightAngles());

    // scales zero, only skews (reflection)
    mat.setAll(0, 1, 0,
               1, 0, 0,
               0, 0, 1);
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
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, nullptr, nullptr, nullptr));

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
    mat.setRotate(kRotation1).postScale(kScale0, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // uniform scale then rotation
    mat.setScale(kScale0, kScale0).postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation then uniform scale+reflection
    mat.setRotate(kRotation0).postScale(kScale1, -kScale1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // uniform scale+reflection, then rotate
    mat.setScale(kScale0, -kScale0).postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation then anisotropic scale
    mat.setRotate(kRotation1).postScale(kScale1, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation then anisotropic scale
    mat.setRotate(90).postScale(kScale1, kScale0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // anisotropic scale then rotation
    mat.setScale(kScale1, kScale0).postRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // anisotropic scale then rotation
    mat.setScale(kScale1, kScale0).postRotate(90);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation, uniform scale, then different rotation
    mat.setRotate(kRotation1).postScale(kScale0, kScale0).postRotate(kRotation0);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation, anisotropic scale, then different rotation
    mat.setRotate(kRotation0).postScale(kScale1, kScale0).postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // rotation, anisotropic scale + reflection, then different rotation
    mat.setRotate(kRotation0).postScale(-kScale1, kScale0).postRotate(kRotation1);
    REPORTER_ASSERT(reporter, SkDecomposeUpper2x2(mat, &rotation1, &scale, &rotation2));
    REPORTER_ASSERT(reporter, check_matrix_recomposition(mat, rotation1, scale, rotation2));

    // try some random matrices
    SkRandom rand;
    for (int m = 0; m < 1000; ++m) {
        SkScalar rot0 = rand.nextRangeF(-180, 180);
        SkScalar sx = rand.nextRangeF(-3000.f, 3000.f);
        SkScalar sy = rand.nextRangeF(-3000.f, 3000.f);
        SkScalar rot1 = rand.nextRangeF(-180, 180);
        mat.setRotate(rot0).postScale(sx, sy).postRotate(rot1);

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
static bool point3_array_nearly_equal_relative(const SkPoint3 a[], const SkPoint3 b[], int count) {
    for (int i = 0; i < count; ++i) {
        if (!scalar_nearly_equal_relative(a[i].fX, b[i].fX)) {
            return false;
        }
        if (!scalar_nearly_equal_relative(a[i].fY, b[i].fY)) {
            return false;
        }
        if (!scalar_nearly_equal_relative(a[i].fZ, b[i].fZ)) {
            return false;
        }
    }
    return true;
}

// For test_matrix_homogeneous, below.
// Maps a single triple in src using m and compares results to those in dst
static bool naive_homogeneous_mapping(const SkMatrix& m, const SkPoint3& src,
                                      const SkPoint3& dst) {
    SkPoint3 res;
    SkScalar ms[9] = {m[0], m[1], m[2],
                      m[3], m[4], m[5],
                      m[6], m[7], m[8]};
    res.fX = src.fX * ms[0] + src.fY * ms[1] + src.fZ * ms[2];
    res.fY = src.fX * ms[3] + src.fY * ms[4] + src.fZ * ms[5];
    res.fZ = src.fX * ms[6] + src.fY * ms[7] + src.fZ * ms[8];
    return point3_array_nearly_equal_relative(&res, &dst, 1);
}

static void test_matrix_homogeneous(skiatest::Reporter* reporter) {
    SkMatrix mat;

    const float kRotation0 = 15.5f;
    const float kRotation1 = -50.f;
    const float kScale0 = 5000.f;

#if defined(SK_BUILD_FOR_GOOGLE3)
    // Stack frame size is limited in SK_BUILD_FOR_GOOGLE3.
    const int kTripleCount = 100;
    const int kMatrixCount = 100;
#else
    const int kTripleCount = 1000;
    const int kMatrixCount = 1000;
#endif
    SkRandom rand;

    SkPoint3 randTriples[kTripleCount];
    for (int i = 0; i < kTripleCount; ++i) {
        randTriples[i].fX = rand.nextRangeF(-3000.f, 3000.f);
        randTriples[i].fY = rand.nextRangeF(-3000.f, 3000.f);
        randTriples[i].fZ = rand.nextRangeF(-3000.f, 3000.f);
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
    SkPoint3 dst[kTripleCount];
    mat.mapHomogeneousPoints(dst, randTriples, kTripleCount);
    REPORTER_ASSERT(reporter, point3_array_nearly_equal_relative(randTriples, dst, kTripleCount));
    }

    const SkPoint3 zeros = {0.f, 0.f, 0.f};
    // zero matrix
    {
    mat.setAll(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
    SkPoint3 dst[kTripleCount];
    mat.mapHomogeneousPoints(dst, randTriples, kTripleCount);
    for (int i = 0; i < kTripleCount; ++i) {
        REPORTER_ASSERT(reporter, point3_array_nearly_equal_relative(&dst[i], &zeros, 1));
    }
    }

    // zero point
    {
    for (int i = 0; i < kMatrixCount; ++i) {
        SkPoint3 dst;
        mats[i].mapHomogeneousPoints(&dst, &zeros, 1);
        REPORTER_ASSERT(reporter, point3_array_nearly_equal_relative(&dst, &zeros, 1));
    }
    }

    // doesn't crash with null dst, src, count == 0
    {
    mats[0].mapHomogeneousPoints(nullptr, (const SkPoint3*)nullptr, 0);
    }

    // uniform scale of point
    {
    mat.setScale(kScale0, kScale0);
    SkPoint3 dst;
    SkPoint3 src = {randTriples[0].fX, randTriples[0].fY, 1.f};
    SkPoint pnt;
    pnt.set(src.fX, src.fY);
    mat.mapHomogeneousPoints(&dst, &src, 1);
    mat.mapPoints(&pnt, &pnt, 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fX, pnt.fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fY, pnt.fY));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fZ, 1));
    }

    // rotation of point
    {
    mat.setRotate(kRotation0);
    SkPoint3 dst;
    SkPoint3 src = {randTriples[0].fX, randTriples[0].fY, 1.f};
    SkPoint pnt;
    pnt.set(src.fX, src.fY);
    mat.mapHomogeneousPoints(&dst, &src, 1);
    mat.mapPoints(&pnt, &pnt, 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fX, pnt.fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fY, pnt.fY));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fZ, 1));
    }

    // rotation, scale, rotation of point
    {
    mat.setRotate(kRotation1);
    mat.postScale(kScale0, kScale0);
    mat.postRotate(kRotation0);
    SkPoint3 dst;
    SkPoint3 src = {randTriples[0].fX, randTriples[0].fY, 1.f};
    SkPoint pnt;
    pnt.set(src.fX, src.fY);
    mat.mapHomogeneousPoints(&dst, &src, 1);
    mat.mapPoints(&pnt, &pnt, 1);
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fX, pnt.fX));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fY, pnt.fY));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.fZ, 1));
    }

    // compare with naive approach
    {
    for (int i = 0; i < kMatrixCount; ++i) {
        for (int j = 0; j < kTripleCount; ++j) {
            SkPoint3 dst;
            mats[i].mapHomogeneousPoints(&dst, &randTriples[j], 1);
            REPORTER_ASSERT(reporter, naive_homogeneous_mapping(mats[i], randTriples[j], dst));
        }
    }
    }

}

static bool check_decompScale(const SkMatrix& original) {
    SkSize scale;
    SkMatrix remaining;

    if (!original.decomposeScale(&scale, &remaining)) {
        return false;
    }
    if (scale.width() <= 0 || scale.height() <= 0) {
        return false;
    }

    // First ensure that the decomposition reconstitutes back to the original
    {
        SkMatrix reconstituted = remaining;

        reconstituted.preScale(scale.width(), scale.height());
        if (!nearly_equal(original, reconstituted)) {
            return false;
        }
    }

    // Then push some points through both paths and make sure they are the same.
    static const int kNumPoints = 5;
    const SkPoint testPts[kNumPoints] = {
        {  0.0f,  0.0f },
        {  1.0f,  1.0f },
        {  1.0f,  0.5f },
        { -1.0f, -0.5f },
        { -1.0f,  2.0f }
    };

    SkPoint v1[kNumPoints];
    original.mapPoints(v1, testPts, kNumPoints);

    SkPoint v2[kNumPoints];
    SkMatrix scaleMat = SkMatrix::Scale(scale.width(), scale.height());

    // Note, we intend the decomposition to be applied in the order scale and then remainder but,
    // due to skbug.com/7211, the order is reversed!
    scaleMat.mapPoints(v2, testPts, kNumPoints);
    remaining.mapPoints(v2, kNumPoints);

    for (int i = 0; i < kNumPoints; ++i) {
        if (!SkPointPriv::EqualsWithinTolerance(v1[i], v2[i], 0.00001f)) {
            return false;
        }
    }

    return true;
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

    m.setRotate(35, 0, 0).preScale(2, 3);
    REPORTER_ASSERT(reporter, check_decompScale(m));

    m.setRotate(35, 0, 0).postScale(2, 3);
    REPORTER_ASSERT(reporter, check_decompScale(m));
}

DEF_TEST(Matrix, reporter) {
    SkMatrix    mat, inverse, iden1, iden2;

    mat.reset();
    mat.setTranslate(1, 1);
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));

    mat.setScale(2, 4);
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    test_flatten(reporter, mat);

    mat.setScale(SK_Scalar1/2, 2);
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    test_flatten(reporter, mat);

    mat.setScale(3, 5, 20, 0).postRotate(25);
    REPORTER_ASSERT(reporter, mat.invert(nullptr));
    REPORTER_ASSERT(reporter, mat.invert(&inverse));
    iden1.setConcat(mat, inverse);
    REPORTER_ASSERT(reporter, is_identity(iden1));
    iden2.setConcat(inverse, mat);
    REPORTER_ASSERT(reporter, is_identity(iden2));
    test_flatten(reporter, mat);
    test_flatten(reporter, iden2);

    mat.setScale(0, 1);
    REPORTER_ASSERT(reporter, !mat.invert(nullptr));
    REPORTER_ASSERT(reporter, !mat.invert(&inverse));
    mat.setScale(1, 0);
    REPORTER_ASSERT(reporter, !mat.invert(nullptr));
    REPORTER_ASSERT(reporter, !mat.invert(&inverse));

    // Inverting this matrix results in a non-finite matrix
    mat.setAll(0.0f, 1.0f, 2.0f,
               0.0f, 1.0f, -3.40277175e+38f,
               1.00003040f, 1.0f, 0.0f);
    REPORTER_ASSERT(reporter, !mat.invert(nullptr));
    REPORTER_ASSERT(reporter, !mat.invert(&inverse));

    // rectStaysRect test
    {
        static const struct {
            SkScalar    m00, m01, m10, m11;
            bool        mStaysRect;
        }
        gRectStaysRectSamples[] = {
            { 0, 0, 0, 0, false },
            { 0, 0, 0, 1, false },
            { 0, 0, 1, 0, false },
            { 0, 0, 1, 1, false },
            { 0, 1, 0, 0, false },
            { 0, 1, 0, 1, false },
            { 0, 1, 1, 0, true },
            { 0, 1, 1, 1, false },
            { 1, 0, 0, 0, false },
            { 1, 0, 0, 1, true },
            { 1, 0, 1, 0, false },
            { 1, 0, 1, 1, false },
            { 1, 1, 0, 0, false },
            { 1, 1, 0, 1, false },
            { 1, 1, 1, 0, false },
            { 1, 1, 1, 1, false }
        };

        for (size_t i = 0; i < std::size(gRectStaysRectSamples); i++) {
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
    mat.set(SkMatrix::kMScaleX, 1)
       .set(SkMatrix::kMSkewX,  2)
       .set(SkMatrix::kMTransX, 3)
       .set(SkMatrix::kMSkewY,  4)
       .set(SkMatrix::kMScaleY, 5)
       .set(SkMatrix::kMTransY, 6);
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

    mat.setScaleTranslate(2, 3, 1, 4);
    mat2.setScale(2, 3).postTranslate(1, 4);
    REPORTER_ASSERT(reporter, mat == mat2);
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

// Test that all variants of maprect are correct.
DEF_TEST(Matrix_maprects, r) {
    const SkScalar scale = 1000;

    SkMatrix mat;
    mat.setScale(2, 3).postTranslate(1, 4);

    SkRandom rand;
    for (int i = 0; i < 10000; ++i) {
        SkRect src = SkRect::MakeLTRB(rand.nextSScalar1() * scale,
                                      rand.nextSScalar1() * scale,
                                      rand.nextSScalar1() * scale,
                                      rand.nextSScalar1() * scale);
        SkRect dst[4];

        mat.mapPoints((SkPoint*)&dst[0].fLeft, (SkPoint*)&src.fLeft, 2);
        dst[0].sort();
        mat.mapRect(&dst[1], src);
        mat.mapRectScaleTranslate(&dst[2], src);
        dst[3] = mat.mapRect(src);

        REPORTER_ASSERT(r, dst[0] == dst[1]);
        REPORTER_ASSERT(r, dst[0] == dst[2]);
        REPORTER_ASSERT(r, dst[0] == dst[3]);
    }

    // We should report nonfinite-ness after a mapping
    {
        // We have special-cases in mapRect for different matrix types
        SkMatrix m0 = SkMatrix::Scale(1e20f, 1e20f);
        SkMatrix m1; m1.setRotate(30); m1.postScale(1e20f, 1e20f);

        for (const auto& m : { m0, m1 }) {
            SkRect rect = { 0, 0, 1e20f, 1e20f };
            REPORTER_ASSERT(r, rect.isFinite());
            rect = m.mapRect(rect);
            REPORTER_ASSERT(r, !rect.isFinite());
        }
    }
}

DEF_TEST(Matrix_mapRect_skbug12335, r) {
    // Stripped down test case from skbug.com/12335. Essentially, the corners of this rect would
    // map to homogoneous coords with very small w's (below the old value of kW0PlaneDistance) and
    // so they would be clipped "behind" the plane, resulting in an empty mapped rect. Coordinates
    // with positive that wouldn't overflow when divided by w should still be included in the mapped
    // rectangle.
    SkRect rect = SkRect::MakeLTRB(0, 0, 319, 620);
    SkMatrix m = SkMatrix::MakeAll( 0.000152695269f, 0.00000000f,     -6.53848401e-05f,
                                   -1.75697533e-05f, 0.000157153074f, -1.10847975e-06f,
                                   -6.00415362e-08f, 0.00000000f,      0.000169880834f);
    SkRect out = m.mapRect(rect);
    REPORTER_ASSERT(r, !out.isEmpty());
}

DEF_TEST(Matrix_Ctor, r) {
    REPORTER_ASSERT(r, SkMatrix{} == SkMatrix::I());
}

DEF_TEST(Matrix_LookAt, r) {
    // Degenerate inputs should not trigger *SAN errors.
    const auto m = SkM44::LookAt({0,0,0}, {0,0,0}, {0,0,0});
    REPORTER_ASSERT(r, m == SkM44());
}

DEF_TEST(Matrix_SetRotateSnap, r) {
    SkMatrix m;

    // We need to snap sin & cos when we call setRotate, or rotations by multiples of 90 degrees
    // will end up with slight drift (and we won't consider them to satisfy rectStaysRect, which
    // is an important performance constraint). We test up to +-1080 degrees.
    for (float deg = 90.0f; deg <= 1080.0f; deg += 90.0f) {
        m.setRotate(deg);
        REPORTER_ASSERT(r, m.rectStaysRect());
        m.setRotate(-deg);
        REPORTER_ASSERT(r, m.rectStaysRect());
    }

    // But: we don't want to be too lenient with snapping. That prevents small rotations from being
    // registered at all. Ensure that .01 degrees produces an actual rotation. (crbug.com/1345038)
    m.setRotate(0.01f);
    REPORTER_ASSERT(r, !m.rectStaysRect());
}

DEF_TEST(Matrix_rectStaysRect_zeroScale, r) {
    // rectStaysRect() returns true if the scale factors are non-zero, so preScale(0,0),
    // setScale(0,0), setScaleTranslate(0,0,...), ::Scale(), should not have the flag set.
    REPORTER_ASSERT(r, !SkMatrix::Scale(0.f, 0.f).rectStaysRect());
    REPORTER_ASSERT(r, !SkMatrix::Scale(0.f, 2.f).rectStaysRect());
    REPORTER_ASSERT(r, !SkMatrix::Scale(2.f, 0.f).rectStaysRect());

    // RectToRect() is like scaling. It fails if the source rect is empty, but if the dst rect is
    // empty it's as if it had a zero scale factor, so it's type mask should reflect that.
    const SkRect src = {0.f,0.f,10.f,10.f};
    REPORTER_ASSERT(r, !SkMatrix::RectToRect(src, {0.f,0.f,0.f,0.f}).rectStaysRect());
    REPORTER_ASSERT(r, !SkMatrix::RectToRect(src, {0.f,0.f,0.f,20.f}).rectStaysRect());
    REPORTER_ASSERT(r, !SkMatrix::RectToRect(src, {0.f,0.f,20.f,0.f}).rectStaysRect());

    {
        SkMatrix rectMatrix = SkMatrix::I(); // trivially
        REPORTER_ASSERT(r, rectMatrix.rectStaysRect());

        SkMatrix nonRectMatrix = rectMatrix;
        nonRectMatrix.preScale(0.f, 0.f);
        REPORTER_ASSERT(r, !nonRectMatrix.rectStaysRect());

        nonRectMatrix = rectMatrix;
        nonRectMatrix.preScale(0.f, 2.f);
        REPORTER_ASSERT(r, !nonRectMatrix.rectStaysRect());

        nonRectMatrix = rectMatrix;
        nonRectMatrix.preScale(2.f, 0.f);
        REPORTER_ASSERT(r, !nonRectMatrix.rectStaysRect());
    }

    {
        SkMatrix m;
        m.setScale(0.f, 0.f);
        REPORTER_ASSERT(r, !m.rectStaysRect());
    }

    {
        SkMatrix m;
        m.setScale(0.f, 2.f);
        REPORTER_ASSERT(r, !m.rectStaysRect());
    }

    {
        SkMatrix m;
        m.setScale(2.f, 0.f);
        REPORTER_ASSERT(r, !m.rectStaysRect());
    }

    {
        SkMatrix m;
        m.setScaleTranslate(0.f, 0.f, 10.f, 10.f);
        REPORTER_ASSERT(r, !m.rectStaysRect());
    }

    {
        SkMatrix m;
        m.setScaleTranslate(0.f, 2.f, 10.f, 10.f);
        REPORTER_ASSERT(r, !m.rectStaysRect());
    }

    {
        SkMatrix m;
        m.setScaleTranslate(2.f, 0.f, 10.f, 10.f);
        REPORTER_ASSERT(r, !m.rectStaysRect());
    }

    }
