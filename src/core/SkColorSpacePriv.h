/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#define SkColorSpacePrintf(...)

inline bool color_space_almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.01f;
}

inline void set_gamma_value(SkGammaCurve* gamma, float value) {
    if (color_space_almost_equal(2.2f, value)) {
        gamma->fNamed = SkColorSpace::k2Dot2Curve_GammaNamed;
    } else if (color_space_almost_equal(1.0f, value)) {
        gamma->fNamed = SkColorSpace::kLinear_GammaNamed;
    } else if (color_space_almost_equal(0.0f, value)) {
        SkColorSpacePrintf("Treating invalid zero gamma as linear.");
        gamma->fNamed = SkColorSpace::kLinear_GammaNamed;
    } else {
        gamma->fValue = value;
    }
}
