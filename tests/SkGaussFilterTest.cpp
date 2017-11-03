/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGaussFilter.h"

#include <cmath>
#include "Test.h"

// one part in a million
static constexpr double kEpsilon = 0.000001;

static double careful_add(int n, double* gauss) {
    // Sum smallest to largest to retain precision.
    double sum = 0;
    for (int i = n - 1; i >= 1; i--) {
        sum += 2.0 * gauss[i];
    }
    sum += gauss[0];
    return sum;
}

DEF_TEST(SkGaussFilterCommon, r) {
    {
        // 0.788675 - most common mask sigma.
        static constexpr double testSigma = 0.788675;
        // GaussianMatrix[{{Automatic}, {.788675}}, Method -> "Gaussian"]
        double gaussGolden[] = {0.506205, 0.226579, 0.0203189};

        // GaussianMatrix[{{Automatic}, {.788675}}]
        double besselGolden[] = {0.593605, 0.176225, 0.0269721};

        SkGaussFilter gauss{testSigma, SkGaussFilter::Type::Gaussian};
        SkGaussFilter bessel{testSigma, SkGaussFilter::Type::Bessel};

        double result[5];
        int n = gauss.filterDouble(result);
        REPORTER_ASSERT(r, n == 3);
        double sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);
        for (int i = 0; i < 3; i++) {
            REPORTER_ASSERT(r, std::abs(gaussGolden[i] - result[i]) < kEpsilon);
        }

        n = bessel.filterDouble(result);
        REPORTER_ASSERT(r, n == 3);
        sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);
        for (int i = 0; i < 3; i++) {
            REPORTER_ASSERT(r, std::abs(besselGolden[i] - result[i]) < kEpsilon);
        }
    }

    {
        // 1.07735 - second most common mask sigma.
        static constexpr double testSigma = 1.07735;
        // GaussianMatrix[{{Automatic}, {1.07735}}, Method -> "Gaussian"]
        double gaussGolden[] = {0.376362, 0.244636, 0.0671835};

        // GaussianMatrix[{{4}, {1.07735}}, Method -> "Bessel"]
        double besselGolden[] = {0.429537, 0.214955, 0.059143, 0.0111337};

        SkGaussFilter gauss{testSigma, SkGaussFilter::Type::Gaussian};
        SkGaussFilter bessel{testSigma, SkGaussFilter::Type::Bessel};

        double result[5];
        int n = gauss.filterDouble(result);
        REPORTER_ASSERT(r, n == 3);
        double sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);
        for (int i = 0; i < n; i++) {
            REPORTER_ASSERT(r, std::abs(gaussGolden[i] - result[i]) < kEpsilon);
        }

        n = bessel.filterDouble(result);
        REPORTER_ASSERT(r, n == 4);
        sum = careful_add(n, result);

        REPORTER_ASSERT(r, sum == 1.0);
        for (int i = 0; i < n; i++) {
            REPORTER_ASSERT(r, std::abs(besselGolden[i] - result[i]) < kEpsilon);
        }
    }
}

DEF_TEST(SkGaussFilterSweep, r) {
    for (double sigma = 0.0; sigma < 2.0; sigma += 0.1) {

        SkGaussFilter gauss{sigma, SkGaussFilter::Type::Gaussian};
        SkGaussFilter bessel{sigma, SkGaussFilter::Type::Bessel};

        double result[5];
        int n = gauss.filterDouble(result);
        REPORTER_ASSERT(r, n <= 5);
        double sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);

        n = bessel.filterDouble(result);
        REPORTER_ASSERT(r, n <= 5);
        sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);
    }

    {
        // The double just before 2.0.
        double sigma = std::nextafter(2.0, 0.0);
        SkGaussFilter gauss{sigma, SkGaussFilter::Type::Gaussian};
        SkGaussFilter bessel{sigma, SkGaussFilter::Type::Bessel};

        double result[5];
        int n = gauss.filterDouble(result);
        REPORTER_ASSERT(r, n <= 5);
        double sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);

        n = bessel.filterDouble(result);
        REPORTER_ASSERT(r, n <= 5);
        sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);
    }
}
