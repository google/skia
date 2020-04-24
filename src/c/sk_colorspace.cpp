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

void sk_colorspace_ref(sk_colorspace_t* colorspace) {
    SkSafeRef(AsColorSpace(colorspace));
}

void sk_colorspace_unref(sk_colorspace_t* colorspace) {
    SkSafeUnref(AsColorSpace(colorspace));
}

sk_colorspace_t* sk_colorspace_new_srgb(void) {
    return ToColorSpace(SkColorSpace::MakeSRGB().release());
}

sk_colorspace_t* sk_colorspace_new_srgb_linear(void) {
    return ToColorSpace(SkColorSpace::MakeSRGBLinear().release());
}

sk_colorspace_t* sk_colorspace_new_rgb(const sk_colorspace_transfer_fn_t* transferFn, const sk_colorspace_xyz_t* toXYZD50) {
    return ToColorSpace(SkColorSpace::MakeRGB(*AsColorSpaceTransferFn(transferFn), *AsColorSpaceXyz(toXYZD50)).release());
}

sk_colorspace_t* sk_colorspace_new_icc(const sk_colorspace_icc_profile_t* profile) {
    return ToColorSpace(SkColorSpace::Make(*AsColorSpaceIccProfile(profile)).release());
}

void sk_colorspace_to_profile(const sk_colorspace_t* colorspace, sk_colorspace_icc_profile_t* profile) {
    AsColorSpace(colorspace)->toProfile(AsColorSpaceIccProfile(profile));
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

bool sk_colorspace_to_xyzd50(const sk_colorspace_t* colorspace, sk_colorspace_xyz_t* toXYZD50) {
    return AsColorSpace(colorspace)->toXYZD50(AsColorSpaceXyz(toXYZD50));
}

sk_colorspace_t* sk_colorspace_make_linear_gamma(const sk_colorspace_t* colorspace) {
    return ToColorSpace(AsColorSpace(colorspace)->makeLinearGamma().release());
}

sk_colorspace_t* sk_colorspace_make_srgb_gamma(const sk_colorspace_t* colorspace) {
    return ToColorSpace(AsColorSpace(colorspace)->makeSRGBGamma().release());
}

bool sk_colorspace_is_srgb(const sk_colorspace_t* colorspace) {
    return AsColorSpace(colorspace)->isSRGB();
}

bool sk_colorspace_equals(const sk_colorspace_t* src, const sk_colorspace_t* dst) {
    return SkColorSpace::Equals(AsColorSpace(src), AsColorSpace(dst));
}


// sk_colorspace_transfer_fn_t

void sk_colorspace_transfer_fn_named_srgb(sk_colorspace_transfer_fn_t* transferFn) {
    *transferFn = ToColorSpaceTransferFn(SkNamedTransferFn::kSRGB);
}

void sk_colorspace_transfer_fn_named_2dot2(sk_colorspace_transfer_fn_t* transferFn) {
    *transferFn = ToColorSpaceTransferFn(SkNamedTransferFn::k2Dot2);
}

void sk_colorspace_transfer_fn_named_linear(sk_colorspace_transfer_fn_t* transferFn) {
    *transferFn = ToColorSpaceTransferFn(SkNamedTransferFn::kLinear);
}

void sk_colorspace_transfer_fn_named_rec2020(sk_colorspace_transfer_fn_t* transferFn) {
    *transferFn = ToColorSpaceTransferFn(SkNamedTransferFn::kRec2020);
}

void sk_colorspace_transfer_fn_named_pq(sk_colorspace_transfer_fn_t* transferFn) {
    *transferFn = ToColorSpaceTransferFn(SkNamedTransferFn::kPQ);
}

void sk_colorspace_transfer_fn_named_hlg(sk_colorspace_transfer_fn_t* transferFn) {
    *transferFn = ToColorSpaceTransferFn(SkNamedTransferFn::kHLG);
}

float sk_colorspace_transfer_fn_eval(const sk_colorspace_transfer_fn_t* transferFn, float x) {
    return skcms_TransferFunction_eval(AsColorSpaceTransferFn(transferFn), x);
}

bool sk_colorspace_transfer_fn_invert(const sk_colorspace_transfer_fn_t* src, sk_colorspace_transfer_fn_t* dst) {
    return skcms_TransferFunction_invert(AsColorSpaceTransferFn(src), AsColorSpaceTransferFn(dst));
}


// sk_colorspace_primaries_t

bool sk_colorspace_primaries_to_xyzd50(const sk_colorspace_primaries_t* primaries, sk_colorspace_xyz_t* toXYZD50) {
    return AsColorSpacePrimaries(primaries)->toXYZD50(AsColorSpaceXyz(toXYZD50));
}


// sk_colorspace_xyz_t

void sk_colorspace_xyz_named_srgb(sk_colorspace_xyz_t* xyz) {
    *xyz = ToColorSpaceXyz(SkNamedGamut::kSRGB);
}

void sk_colorspace_xyz_named_adobe_rgb(sk_colorspace_xyz_t* xyz) {
    *xyz = ToColorSpaceXyz(SkNamedGamut::kAdobeRGB);
}

void sk_colorspace_xyz_named_dcip3(sk_colorspace_xyz_t* xyz) {
    *xyz = ToColorSpaceXyz(SkNamedGamut::kDCIP3);
}

void sk_colorspace_xyz_named_rec2020(sk_colorspace_xyz_t* xyz) {
    *xyz = ToColorSpaceXyz(SkNamedGamut::kRec2020);
}

void sk_colorspace_xyz_named_xyz(sk_colorspace_xyz_t* xyz) {
    *xyz = ToColorSpaceXyz(SkNamedGamut::kXYZ);
}

bool sk_colorspace_xyz_invert(const sk_colorspace_xyz_t* src, sk_colorspace_xyz_t* dst) {
    return skcms_Matrix3x3_invert(AsColorSpaceXyz(src), AsColorSpaceXyz(dst));
}

void sk_colorspace_xyz_concat(const sk_colorspace_xyz_t* a, const sk_colorspace_xyz_t* b, sk_colorspace_xyz_t* result) {
    *result = ToColorSpaceXyz(skcms_Matrix3x3_concat(AsColorSpaceXyz(a), AsColorSpaceXyz(b)));
}


// sk_colorspace_icc_profile_t

void sk_colorspace_icc_profile_delete(sk_colorspace_icc_profile_t* profile) {
    delete AsColorSpaceIccProfile(profile);
}

sk_colorspace_icc_profile_t* sk_colorspace_icc_profile_new(void) {
    skcms_ICCProfile* profile = new skcms_ICCProfile();
    return ToColorSpaceIccProfile(profile);
}

bool sk_colorspace_icc_profile_parse(const void* buffer, size_t length, sk_colorspace_icc_profile_t* profile) {
    return skcms_Parse(buffer, length, AsColorSpaceIccProfile(profile));
}

const uint8_t* sk_colorspace_icc_profile_get_buffer(const sk_colorspace_icc_profile_t* profile, uint32_t* size) {
    const skcms_ICCProfile* p = AsColorSpaceIccProfile(profile);
    if (size)
        *size = p->size;
    return p->buffer;
}

bool sk_colorspace_icc_profile_get_to_xyzd50(const sk_colorspace_icc_profile_t* profile, sk_colorspace_xyz_t* toXYZD50) {
    const skcms_ICCProfile* p = AsColorSpaceIccProfile(profile);
    if (toXYZD50)
        *toXYZD50 = ToColorSpaceXyz(p->toXYZD50);
    return p->has_toXYZD50;
}


// sk_color4f_t

sk_color_t sk_color4f_to_color(const sk_color4f_t* color4f) {
    return AsColor4f(color4f)->toSkColor();
}

void sk_color4f_from_color(sk_color_t color, sk_color4f_t* color4f) {
    *color4f = ToColor4f(SkColor4f::FromColor(color));
}
