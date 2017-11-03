/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGaussFilter.h"

#include <cmath>
#include "Test.h"

static constexpr double kEpsilon = 0.000001;

DEF_TEST(SkGaussFilterBasic, r) {
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
    REPORTER_ASSERT(r, n == 2);
    double sum = result[0];
    for (int i = 1; i < 3; i++) {
        sum += 2.0 * result[i];
    }
    REPORTER_ASSERT(r, sum == 1.0);
    for (int i = 0; i < 3; i++) {
        REPORTER_ASSERT(r, std::abs(gaussGolden[i] - result[i]) < kEpsilon);
    }

    n = bessel.filterDouble(result);
    REPORTER_ASSERT(r, n == 2);
    sum = result[0];
    for (int i = 1; i < 3; i++) {
        sum += 2.0 * result[i];
    }
    REPORTER_ASSERT(r, sum == 1.0);
    for (int i = 0; i < 3; i++) {
        REPORTER_ASSERT(r, std::abs(besselGolden[i] - result[i]) < kEpsilon);
    }
}

DEF_TEST(SkGaussFilterSweep, r) {
    for (double sigma = 0.1; sigma < 2.0; sigma += 0.1) {

        SkGaussFilter gauss{sigma, SkGaussFilter::Type::Gaussian};
        SkGaussFilter bessel{sigma, SkGaussFilter::Type::Bessel};

        double result[5];
        int n = gauss.filterDouble(result);
        REPORTER_ASSERT(r, n <= 4);
        double sum = 0;
        for (int i = n; i >= 1; i--) {
            sum += 2.0 * result[i];
        }
        sum += result[0];
        REPORTER_ASSERT(r, sum == 1.0);

        n = bessel.filterDouble(result);
        REPORTER_ASSERT(r, n <= 4);
        sum = result[0];
        for (int i = 1; i < n + 1; i++) {
            sum += 2.0 * result[i];
        }
        REPORTER_ASSERT(r, sum == 1.0);
    }
}
