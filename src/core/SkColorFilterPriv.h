/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterPriv_DEFINED
#define SkColorFilterPriv_DEFINED

#include "include/core/SkColorFilter.h"

class SkColorFilterPriv {
public:
    static sk_sp<SkColorFilter> MakeGaussian();

    // Runs the child filter in a different working color format than usual (premul in
    // destination surface's color space), with all inputs and outputs expressed in this format.
    // Each non-null {tf,gamut,at} parameter overrides that particular aspect of the color format.
    static sk_sp<SkColorFilter> WithWorkingFormat(sk_sp<SkColorFilter>          child,
                                                  const skcms_TransferFunction* tf,
                                                  const skcms_Matrix3x3*        gamut,
                                                  const SkAlphaType*            at);

};

#endif
