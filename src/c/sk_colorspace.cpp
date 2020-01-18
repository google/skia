/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"

#include "include/c/sk_colorspace.h"

#include "src/c/sk_types_priv.h"

// sk_colorspace_t

void sk_colorspace_unref(sk_colorspace_t* colorspace) {
    SkSafeUnref(AsColorSpace(colorspace));
}

sk_colorspace_t* sk_colorspace_new_srgb(void) {
    return ToColorSpace(SkColorSpace::MakeSRGB().release());
}

sk_colorspace_t* sk_colorspace_new_srgb_linear(void) {
    return ToColorSpace(SkColorSpace::MakeSRGBLinear().release());
}

sk_colorspace_t* sk_colorspace_new_rgb_matrix44(const sk_colorspace_transfer_fn_t* transferFn, const sk_matrix44_t* toXYZD50) {
    const SkMatrix44* m44 = AsMatrix44(toXYZD50);
    skcms_Matrix3x3 m33;
    // TODO: copy the values out of toXYZD50 and into m33
    return ToColorSpace(SkColorSpace::MakeRGB(*AsColorSpaceTransferFn(transferFn), m33).release());
}

sk_colorspace_t* sk_colorspace_new_rgb_named(sk_named_transfer_fn_t transferFn, sk_named_gamut_t toXYZD50) {
    return ToColorSpace(SkColorSpace::MakeRGB(AsNamedTransferFn(transferFn), AsNamedGamut(toXYZD50)).release());
}

sk_colorspace_t* sk_colorspace_new_icc(const void* input, size_t len) {
    skcms_ICCProfile profile;
    if (skcms_Parse(input, len, &profile)) {
        return ToColorSpace(SkColorSpace::Make(profile).release());
    }
    return nullptr;
}

bool sk_colorspace_gamma_close_to_srgb(const sk_colorspace_t* colorspace) {
    return AsColorSpace(colorspace)->gammaCloseToSRGB();
}

bool sk_colorspace_gamma_is_linear(const sk_colorspace_t* colorspace) {
    return AsColorSpace(colorspace)->gammaIsLinear();
}

bool sk_colorspace_is_numerical_transfer_fn(const sk_colorspace_t* colorspace, sk_colorspace_transfer_fn_t* transferFn) {
    return AsColorSpace(colorspace)->isNumericalTransferFn(AsColorSpaceTransferFn(transferFn));
}

bool sk_colorspace_to_xyzd50(const sk_colorspace_t* colorspace, sk_matrix44_t* toXYZD50) {
    return AsColorSpace(colorspace)->toXYZD50(AsMatrix44(toXYZD50));
}

bool sk_colorspace_is_srgb(const sk_colorspace_t* colorspace) {
    return AsColorSpace(colorspace)->isSRGB();
}

bool sk_colorspace_equals(const sk_colorspace_t* src, const sk_colorspace_t* dst) {
    return SkColorSpace::Equals(AsColorSpace(src), AsColorSpace(dst));
}

// sk_colorspace_transfer_fn_t

float sk_colorspace_transfer_fn_eval(const sk_colorspace_transfer_fn_t* transferFn, float x) {
    return skcms_TransferFunction_eval(AsColorSpaceTransferFn(transferFn), x);
}

bool sk_colorspace_transfer_fn_invert(const sk_colorspace_transfer_fn_t* src, sk_colorspace_transfer_fn_t* dst) {
    return skcms_TransferFunction_invert(AsColorSpaceTransferFn(src), AsColorSpaceTransferFn(dst));
}

// sk_colorspace_primaries_t

bool sk_colorspace_primaries_to_xyzd50(const sk_colorspace_primaries_t* primaries, sk_matrix44_t* toXYZD50) {
    SkMatrix44* m44 = AsMatrix44(toXYZD50);
    skcms_Matrix3x3 m33;
    bool result = AsColorSpacePrimaries(primaries)->toXYZD50(&m33);
    m44->set3x3RowMajorf(&m33.vals[0][0]);
    return result;
}
