/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpaceXformSteps.h"

// Our logical flow modulo any optimization is:
//   For source colors:
//    1) get colors into linear, unpremul format
//    2) transform to dst gamut
//    3) encode with dst transfer function if dst has non-linear blending
//    4) premul
//
//   For destination colors:
//    a) Linearize if dst has linear blending
//    b) (Presumably all destinations are already premul, but logically, premul here)
//
//   Now the source and destination pipelines unify:
//    I)  blend the src and dst colors, which are encoded the same way in the same gamut
//    II) if dst has linear blending, encode using dst transfer function
//
//  Step 1) is represented by three bits:
//     - early_unpremul, converting f(s)*a,a to f(s),a if src has non-linear blending
//     - linearize_src,  converting f(s),a to s,a   _or_  f(s*a),a to s*a,a
//     - late_unpremul,  converting s*a,a to s,a if src has linear blending
//  So we'll either early_unpremul and linearize_src, or linearize_src and late_unpremul.
//

SkColorSpaceXformSteps::SkColorSpaceXformSteps(SkColorSpace* src, SkAlphaType srcAT,
                                               SkColorSpace* dst) {
    // Set all bools to false, all floats to 0.0f.
    memset(this, 0, sizeof(*this));

    // We have some options about what to do with null src or dst here.
#if 1
    SkASSERT(src && dst);
#else
    // If either the source or destination color space is unspecified,
    // treat it as non-linearly-blended sRGB ("legacy").
    sk_sp<SkColorSpace> sRGB_NL;
    if (!src || !dst) {
        sRGB_NL = SkColorSpace::MakeSRGB()->makeNonlinearBlending();
        if (!src) { src = sRGB_NL.get(); }
        if (!dst) { dst = sRGB_NL.get(); }
    }
#endif

    bool srcNL = src->nonlinearBlending(),
         dstNL = dst->nonlinearBlending();

    // Step 1)
    this->early_unpremul =  srcNL;
    this->linearize_src  =   true;
    this->late_unpremul  = !srcNL;

    // Step 2)
    this->gamut_transform = true;

    // Step 3)
    this->early_encode = dstNL;

    // Step 4)
    this->premul = true;

    // Step a)
    this->linearize_dst = !dstNL;

    // Step II)
    this->late_encode = !dstNL;
}
