/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBicubicFilterWeights_DEFINED
#define SkBicubicFilterWeights_DEFINED

#include "include/core/SkM44.h"

/**
 *  Example bicubic coefficients, suitable for passing to SkImage when making a bicubic shader.
 */
namespace SkBicubicFilterCoefficients {

constexpr SkM44 gCentripetalCatmulRom
    (0.0f/2, -1.0f/2,  2.0f/2, -1.0f/2,
     2.0f/2,  0.0f/2, -5.0f/2,  3.0f/2,
     0.0f/2,  1.0f/2,  4.0f/2, -3.0f/2,
     0.0f/2,  0.0f/2, -1.0f/2,  1.0f/2);

constexpr SkM44 gMitchellNetravali
    ( 1.0f/18, -9.0f/18,  15.0f/18,  -7.0f/18,
     16.0f/18,  0.0f/18, -36.0f/18,  21.0f/18,
      1.0f/18,  9.0f/18,  27.0f/18, -21.0f/18,
      0.0f/18,  0.0f/18,  -6.0f/18,   7.0f/18);

}

#endif
