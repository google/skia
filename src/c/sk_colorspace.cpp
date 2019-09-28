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

sk_colorspace_t* sk_colorspace_new_icc(const void* input, size_t len) {
    return ToColorSpace(SkColorSpace::MakeICC(input, len).release());
}

sk_colorspace_t* sk_colorspace_new_rgb_with_gamma(sk_colorspace_render_target_gamma_t gamma, const sk_matrix44_t* toXYZD50) {
    return ToColorSpace(SkColorSpace::MakeRGB((SkColorSpace::RenderTargetGamma)gamma, AsMatrix44(*toXYZD50)).release());
}

sk_colorspace_t* sk_colorspace_new_rgb_with_gamma_and_gamut(sk_colorspace_render_target_gamma_t gamma, sk_colorspace_gamut_t gamut) {
    return ToColorSpace(SkColorSpace::MakeRGB((SkColorSpace::RenderTargetGamma)gamma, (SkColorSpace::Gamut)gamut).release());
}

sk_colorspace_t* sk_colorspace_new_rgb_with_coeffs(const sk_colorspace_transfer_fn_t* coeffs, const sk_matrix44_t* toXYZD50) {
    return ToColorSpace(SkColorSpace::MakeRGB(*AsColorSpaceTransferFn(coeffs), AsMatrix44(*toXYZD50)).release());
}

sk_colorspace_t* sk_colorspace_new_rgb_with_coeffs_and_gamut(const sk_colorspace_transfer_fn_t* coeffs, sk_colorspace_gamut_t gamut) {
    return ToColorSpace(SkColorSpace::MakeRGB(*AsColorSpaceTransferFn(coeffs), (SkColorSpace::Gamut)gamut).release());
}

sk_colorspace_t* sk_colorspace_new_rgb_with_gamma_named(sk_gamma_named_t gamma, const sk_matrix44_t* toXYZD50) {
    return ToColorSpace(SkColorSpace::MakeRGB((SkGammaNamed)gamma, AsMatrix44(*toXYZD50)).release());
}

sk_colorspace_t* sk_colorspace_new_rgb_with_gamma_named_and_gamut(sk_gamma_named_t gamma, sk_colorspace_gamut_t gamut) {
    return ToColorSpace(SkColorSpace::MakeRGB((SkGammaNamed)gamma, (SkColorSpace::Gamut)gamut).release());
}

sk_colorspace_type_t sk_colorspace_gamma_get_type(const sk_colorspace_t* cColorSpace) {
    return (sk_colorspace_type_t)AsColorSpace(cColorSpace)->type();
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

bool sk_colorspaceprimaries_to_xyzd50(const sk_colorspaceprimaries_t* primaries, sk_matrix44_t* toXYZD50) {
    return AsColorSpacePrimaries(primaries)->toXYZD50(AsMatrix44(toXYZD50));
}

void sk_colorspace_transfer_fn_invert(const sk_colorspace_transfer_fn_t* transfer, sk_colorspace_transfer_fn_t* inverted) {
    *inverted = ToColorSpaceTransferFn(AsColorSpaceTransferFn(transfer)->invert());
}

float sk_colorspace_transfer_fn_transform(const sk_colorspace_transfer_fn_t* transfer, float x) {
    SkColorSpaceTransferFn fn = *AsColorSpaceTransferFn(transfer);
    return fn(x);
}
