/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaussFilter_DEFINED
#define SkGaussFilter_DEFINED

#include <cstddef>

// Define gaussian filters for values of sigma < 2. Produce values good to 1 part in 1,000,000.
// Gaussian produces values as defined in the SVG 1.1 spec:
// https://www.w3.org/TR/SVG/filters.html#feGaussianBlurElement
// Bessel produces values as defined in "Scale-Space for Discrete Signals" by Tony Lindeberg
class SkGaussFilter {
public:
    static constexpr int kGaussArrayMax = 6;
    enum class Type : bool {
        Gaussian,
        Bessel
    };

    // Type selects which method is used to calculate the gaussian factors.
    SkGaussFilter(double sigma, Type type);

    size_t size()   const { return fN; }
    int radius() const { return fN - 1; }
    int width()  const { return 2 * this->radius() + 1; }

    // Allow a filter to be used in a C++ ranged-for loop.
    const double* begin() const { return &fBasis[0];  }
    const double* end()   const { return &fBasis[fN]; }

private:
    double fBasis[kGaussArrayMax];
    int    fN;
};

#endif  // SkGaussFilter_DEFINED
