/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaussFilter_DEFINED
#define SkGaussFilter_DEFINED

#include <cstdint>

// Define gaussian filters for values of sigma < 2.
class SkGaussFilter {
public:
    enum class Type : bool {
        Gaussian,
        Bessel
    };

    SkGaussFilter(double sigma, Type type);

    int radius() const { return fRadius; }

    int filterDouble(double* values) const;
    int filterUint16(uint16_t* values) const;

private:
    double fBasis[5];
    int    fRadius;
};

#endif  //SkGaussFilter_DEFINED


