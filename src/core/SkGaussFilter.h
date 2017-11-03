/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaussFilter_DEFINED
#define SkGaussFilter_DEFINED

// Define gaussian filters for values of sigma < 2.

class SkGaussFilter {
public:
    enum class : bool Type {
        Gaussian,
        Bessel
    };

    SkGaussFilter(double sigma, Type type) {

    }

private:
    double fBasis[5];
};

#endif  //SkGaussFilter_DEFINED
