/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaussFilter_DEFINED
#define SkGaussFilter_DEFINED

#include <cstdint>

// Define gaussian filters for values of sigma < 2. Produce values good to 1 part in 1,000,000.
// Gaussian produces values as defined in the SVG 1.1 spec:
// https://www.w3.org/TR/SVG/filters.html#feGaussianBlurElement
// Bessel produces values as defined in "Scale-Space for Discrete Signals" by Tony Lindeberg
class SkGaussFilter {
public:
    enum class Type : bool {
        Gaussian,
        Bessel
    };

    // Type selects which method is used to calculate the gaussian factors.
    SkGaussFilter(double sigma, Type type);

    int radius() const { return fN - 1; }
    int width() const { return 2 * this->radius() + 1; }

    // Take an array of values where the gaussian factors will be placed. Return the number of
    // values filled.
    int filterDouble(double values[5]) const;

    // Fill in the array values with gaussian factors scaled by 2^16 resulting in a 0.16 format.
    // Because all the prescaled values sum to one, then all of the values will be less than 2^16.
    // This code assumes that the degenerate case where value[0] == 1 and count == 1 are handled by
    // other code.
    int filterUint16(uint16_t values[5]) const;

private:
    double fBasis[5];
    int    fN;
};

#endif  // SkGaussFilter_DEFINED


