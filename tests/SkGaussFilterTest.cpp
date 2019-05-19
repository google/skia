/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkGaussFilter.h"

#include <cmath>
#include <tuple>
#include <vector>
#include "tests/Test.h"

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
    using Test = std::tuple<double, std::vector<double>>;

    auto golden_check = [&](const Test& test) {
        double sigma; std::vector<double> golden;
        std::tie(sigma, golden) = test;
        SkGaussFilter filter{sigma};
        double result[SkGaussFilter::kGaussArrayMax];
        int n = 0;
        for (auto d : filter) {
            result[n++] = d;
        }
        REPORTER_ASSERT(r, static_cast<size_t>(n) == golden.size());
        double sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);
        for (size_t i = 0; i < golden.size(); i++) {
            REPORTER_ASSERT(r, std::abs(golden[i] - result[i]) < kEpsilon);
        }
    };

    // The following two sigmas account for about 85% of all sigmas used for masks.
    // Golden values generated using Mathematica.
    auto tests = {
        // GaussianMatrix[{{Automatic}, {.788675}}]
        Test{0.788675,   {0.593605, 0.176225, 0.0269721}},

        // GaussianMatrix[{{4}, {1.07735}}, Method -> "Bessel"]
        Test{1.07735,  {0.429537, 0.214955, 0.059143, 0.0111337}},
    };

    for (auto& test : tests) {
        golden_check(test);
    }
}

DEF_TEST(SkGaussFilterSweep, r) {
    // The double just before 2.0.
    const double maxSigma = nextafter(2.0, 0.0);
    auto check = [&](double sigma) {
        SkGaussFilter filter{sigma};
        double result[SkGaussFilter::kGaussArrayMax];
        int n = 0;
        for (auto d : filter) {
            result[n++] = d;
        }
        REPORTER_ASSERT(r, n <= SkGaussFilter::kGaussArrayMax);
        double sum = careful_add(n, result);
        REPORTER_ASSERT(r, sum == 1.0);
    };

    for (double sigma = 0.0; sigma < 2.0; sigma += 0.1) {
        check(sigma);
    }
    check(maxSigma);
}
