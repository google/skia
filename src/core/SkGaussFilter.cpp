/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGaussFilter.h"

#include <cmath>
#include "SkTypes.h"

static constexpr double kPi = 3.14159265358979323846264338327950288;

int calculate_space_scale_factors(double sigma, double* gauss) {
    SkASSERT(sigma < 2);
    auto var = sigma * sigma;

    // The two function below come from the equations in "Handbook of Mathematical Functions"
    // by Abramowitz and Stegun. Specifically, equation 9.6.10 on page 375. Bessel0 is given
    // explicitly as 9.6.12
    // BesselI_0 for 0 <= sigma < 2.
    // NB the k = 0 factor is just sum.
    auto besselI_0 = [](double t) -> double {
        auto tSquaredOver4 = t * t / 4.0;
        auto sum = 1.0;
        auto factor = 1.0;
        for (int k = 1; k < 9; k++) {
            factor *= tSquaredOver4 / (k * k);
            sum += factor;
        }
        return sum;
    };
    // BesselI_1 for 0 <= sigma < 2.
    auto besselI_1 = [](double t) -> double {
        auto tSquaredOver4 = t * t / 4.0;
        auto sum = t / 2.0;
        auto factor = sum;
        for (int k = 1; k < 8; k++) {
            factor *= tSquaredOver4 / (k * (k + 1));
            sum += factor;
        }
        return sum;
    };

    // The following formula for calculating the Gaussian kernel is from
    // "Scale-Space for Discrete Signals" by Tony Lindeberg.
    // gauss(n; var) = besselI_n(var) / (e^var)
    auto d = std::exp(var);
    double b[6] = {besselI_0(var), besselI_1(var)};
    gauss[0] = b[0]/d;
    gauss[1] = b[1]/d;

    int n = 1;
    auto sum = gauss[0];
    // The recurrence relation below is from "Numerical Recipes" 3rd Edition.
    // Equation 6.5.16 p.282
    while (gauss[n] > 1.0/100.0) {
        b[n+1] = -(2*n/var) * b[n] + b[n-1];
        gauss[n+1] = b[n+1] / d;
        sum += 2 * gauss[n];
        n += 1;
    }

    // NB n is one beyond the last, and is too small to produce anything other than zero.
    for (int i = 0; i < n; i++) {
        gauss[i] /= sum;
    }

    // Return the radius.
    return n - 1;
}

int calculate_gauss_factors(double sigma, double* guass) {
    SkASSERT(sigma < 2);

    // From the SVG blur spec: 8.13 Filter primitive <feGaussianBlur>.
    // H(x) = exp(-x^2/ (2s^2)) / sqrt(2π * s^2)
    auto expGaussDenom = -2 * sigma * sigma;
    auto normalizeDenom = std::sqrt(2 * kPi * sigma * sigma);
    auto specGauss = [&](double x) {
        return std::exp((x*x) / expGaussDenom) / normalizeDenom;
    };

    double gauss[6] = {1.0 / normalizeDenom, specGauss(1)};

    int n = 1;
    auto sum = gauss[0];
    while (gauss[n] > 1.0/100.0) {
        gauss[n+1] = specGauss(n+1);
        sum += 2 * gauss[n];
        n += 1;
    }

    // NB n is one beyond the last, and is too small to produce anything other than zero.
    for (int i = 0; i < n; i++) {
        gauss[i] /= sum;
    }

    // Return the radius.
    return n - 1;
}

SkGaussFilter::SkGaussFilter(double sigma, Type type) {
    if (type == Type::Bessel) {
        fRadius = calculate_space_scale_factors(sigma, fBasis);
    } else {
        fRadius = calculate_gauss_factors(sigma, fBasis);
    }
}

int SkGaussFilter::filterDouble(double* values) const {
    for (int i = 0; i < fRadius; i++) {
        values[i] = fBasis[i];
    }
    return fRadius;
}

int SkGaussFilter::filterUint16(uint16_t* values) const {
    // NB n is one beyond the last, and is too small to produce anything other than zero.
    for (int i = 0; i < fRadius; i++) {
        values[i] = static_cast<uint16_t>(std::round(fBasis[i] * (1 << 16)));
    }

    // Return the radius.
    return fRadius;
}
