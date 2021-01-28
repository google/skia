/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_colorspace_DEFINED
#define sk_colorspace_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

// TODO: skcms.h has things that may be useful

// sk_colorspace_t

SK_C_API void sk_colorspace_ref(sk_colorspace_t* colorspace);
SK_C_API void sk_colorspace_unref(sk_colorspace_t* colorspace);
SK_C_API sk_colorspace_t* sk_colorspace_new_srgb(void);
SK_C_API sk_colorspace_t* sk_colorspace_new_srgb_linear(void);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb(const sk_colorspace_transfer_fn_t* transferFn, const sk_colorspace_xyz_t* toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_new_icc(const sk_colorspace_icc_profile_t* profile);
SK_C_API void sk_colorspace_to_profile(const sk_colorspace_t* colorspace, sk_colorspace_icc_profile_t* profile);
SK_C_API bool sk_colorspace_gamma_close_to_srgb(const sk_colorspace_t* colorspace);
SK_C_API bool sk_colorspace_gamma_is_linear(const sk_colorspace_t* colorspace);
SK_C_API bool sk_colorspace_is_numerical_transfer_fn(const sk_colorspace_t* colorspace, sk_colorspace_transfer_fn_t* transferFn);
SK_C_API bool sk_colorspace_to_xyzd50(const sk_colorspace_t* colorspace, sk_colorspace_xyz_t* toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_make_linear_gamma(const sk_colorspace_t* colorspace);
SK_C_API sk_colorspace_t* sk_colorspace_make_srgb_gamma(const sk_colorspace_t* colorspace);
SK_C_API bool sk_colorspace_is_srgb(const sk_colorspace_t* colorspace);
SK_C_API bool sk_colorspace_equals(const sk_colorspace_t* src, const sk_colorspace_t* dst);

// sk_colorspace_transfer_fn_t

SK_C_API void sk_colorspace_transfer_fn_named_srgb(sk_colorspace_transfer_fn_t* transferFn);
SK_C_API void sk_colorspace_transfer_fn_named_2dot2(sk_colorspace_transfer_fn_t* transferFn);
SK_C_API void sk_colorspace_transfer_fn_named_linear(sk_colorspace_transfer_fn_t* transferFn);
SK_C_API void sk_colorspace_transfer_fn_named_rec2020(sk_colorspace_transfer_fn_t* transferFn);
SK_C_API void sk_colorspace_transfer_fn_named_pq(sk_colorspace_transfer_fn_t* transferFn);
SK_C_API void sk_colorspace_transfer_fn_named_hlg(sk_colorspace_transfer_fn_t* transferFn);
SK_C_API float sk_colorspace_transfer_fn_eval(const sk_colorspace_transfer_fn_t* transferFn, float x);
SK_C_API bool sk_colorspace_transfer_fn_invert(const sk_colorspace_transfer_fn_t* src, sk_colorspace_transfer_fn_t* dst);

// sk_colorspace_primaries_t

SK_C_API bool sk_colorspace_primaries_to_xyzd50(const sk_colorspace_primaries_t* primaries, sk_colorspace_xyz_t* toXYZD50);

// sk_colorspace_xyz_t

SK_C_API void sk_colorspace_xyz_named_srgb(sk_colorspace_xyz_t* xyz);
SK_C_API void sk_colorspace_xyz_named_adobe_rgb(sk_colorspace_xyz_t* xyz);
SK_C_API void sk_colorspace_xyz_named_display_p3(sk_colorspace_xyz_t* xyz);
SK_C_API void sk_colorspace_xyz_named_rec2020(sk_colorspace_xyz_t* xyz);
SK_C_API void sk_colorspace_xyz_named_xyz(sk_colorspace_xyz_t* xyz);
SK_C_API bool sk_colorspace_xyz_invert(const sk_colorspace_xyz_t* src, sk_colorspace_xyz_t* dst);
SK_C_API void sk_colorspace_xyz_concat(const sk_colorspace_xyz_t* a, const sk_colorspace_xyz_t* b, sk_colorspace_xyz_t* result);

// sk_colorspace_icc_profile_t

SK_C_API void sk_colorspace_icc_profile_delete(sk_colorspace_icc_profile_t* profile);
SK_C_API sk_colorspace_icc_profile_t* sk_colorspace_icc_profile_new(void);
SK_C_API bool sk_colorspace_icc_profile_parse(const void* buffer, size_t length, sk_colorspace_icc_profile_t* profile);
SK_C_API const uint8_t* sk_colorspace_icc_profile_get_buffer(const sk_colorspace_icc_profile_t* profile, uint32_t* size);
SK_C_API bool sk_colorspace_icc_profile_get_to_xyzd50(const sk_colorspace_icc_profile_t* profile, sk_colorspace_xyz_t* toXYZD50);

// sk_color4f_t

SK_C_API sk_color_t sk_color4f_to_color(const sk_color4f_t* color4f);
SK_C_API void sk_color4f_from_color(sk_color_t color, sk_color4f_t* color4f);

SK_C_PLUS_PLUS_END_GUARD

#endif
