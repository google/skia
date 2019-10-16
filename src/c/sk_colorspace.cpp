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

void sk_colorspace_unref(sk_colorspace_t* cColorSpace) {
    SkSafeUnref(AsColorSpace(cColorSpace));
}

sk_colorspace_t* sk_colorspace_new_srgb() {
    return ToColorSpace(SkColorSpace::MakeSRGB().release());
}

sk_colorspace_t* sk_colorspace_new_srgb_linear() {
    return ToColorSpace(SkColorSpace::MakeSRGBLinear().release());
}

sk_colorspace_t* sk_colorspace_new_rgb(sk_named_transfer_fn_t transfer, sk_named_gamut_t gamut) {
    return ToColorSpace(SkColorSpace::MakeRGB(AsNamedTransferFn(transfer), AsNamedGamut(gamut)).release());
}

sk_colorspace_t* sk_colorspace_new_icc(const void* input, size_t len) {
    skcms_ICCProfile profile;
    if (skcms_Parse(input, len, &profile)) {
        return ToColorSpace(SkColorSpace::MakeICC(input, len).release());
    }
}

sk_gamma_named_t sk_colorspace_gamma_get_gamma_named(const sk_colorspace_t* cColorSpace) {
    return (sk_gamma_named_t)AsColorSpace(cColorSpace)->gammaNamed();
}

bool sk_colorspace_gamma_close_to_srgb(const sk_colorspace_t* cColorSpace) {
    return AsColorSpace(cColorSpace)->gammaCloseToSRGB();
}

bool sk_colorspace_gamma_is_linear(const sk_colorspace_t* cColorSpace) {
    return AsColorSpace(cColorSpace)->gammaIsLinear();
}

bool sk_colorspace_is_srgb(const sk_colorspace_t* cColorSpace) {
    return AsColorSpace(cColorSpace)->isSRGB();
}

bool sk_colorspace_equals(const sk_colorspace_t* src, const sk_colorspace_t* dst) {
    return SkColorSpace::Equals(AsColorSpace(src), AsColorSpace(dst));
}

bool sk_colorspace_to_xyzd50(const sk_colorspace_t* cColorSpace, sk_matrix44_t* toXYZD50) {
    return AsColorSpace(cColorSpace)->toXYZD50(AsMatrix44(toXYZD50));
}

const sk_matrix44_t* sk_colorspace_as_to_xyzd50(const sk_colorspace_t* cColorSpace) {
    return ToMatrix44(AsColorSpace(cColorSpace)->toXYZD50());
}

const sk_matrix44_t* sk_colorspace_as_from_xyzd50(const sk_colorspace_t* cColorSpace) {
    return ToMatrix44(AsColorSpace(cColorSpace)->fromXYZD50());
}

bool sk_colorspace_is_numerical_transfer_fn(const sk_colorspace_t* cColorSpace, sk_colorspace_transfer_fn_t* fn) {
    return AsColorSpace(cColorSpace)->isNumericalTransferFn(AsColorSpaceTransferFn(fn));
}

bool sk_colorspace_primaries_to_xyzd50(const sk_colorspace_primaries_t* primaries, sk_matrix44_t* toXYZD50) {
    return AsColorSpacePrimaries(primaries)->toXYZD50(AsMatrix44(toXYZD50));
}

void sk_colorspace_transfer_fn_invert(const sk_colorspace_transfer_fn_t* transfer, sk_colorspace_transfer_fn_t* inverted) {
    *inverted = ToColorSpaceTransferFn(AsColorSpaceTransferFn(transfer)->invert());
}

float sk_colorspace_transfer_fn_transform(const sk_colorspace_transfer_fn_t* transfer, float x) {
    SkColorSpaceTransferFn fn = *AsColorSpaceTransferFn(transfer);
    return fn(x);
}
