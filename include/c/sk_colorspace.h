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

SK_C_API void sk_colorspace_unref(sk_colorspace_t* cColorSpace);
SK_C_API sk_colorspace_t* sk_colorspace_new_srgb(void);
SK_C_API sk_colorspace_t* sk_colorspace_new_srgb_linear(void);
SK_C_API sk_colorspace_t* sk_colorspace_new_icc(const void* input, size_t len);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_gamma(sk_colorspace_render_target_gamma_t gamma, const sk_matrix44_t* toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_gamma_and_gamut(sk_colorspace_render_target_gamma_t gamma, sk_colorspace_gamut_t gamut);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_coeffs(const sk_colorspace_transfer_fn_t* coeffs, const sk_matrix44_t* toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_coeffs_and_gamut(const sk_colorspace_transfer_fn_t* coeffs, sk_colorspace_gamut_t gamut);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_gamma_named(sk_gamma_named_t gamma, const sk_matrix44_t* toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_gamma_named_and_gamut(sk_gamma_named_t gamma, sk_colorspace_gamut_t gamut);
SK_C_API sk_colorspace_type_t sk_colorspace_gamma_get_type(const sk_colorspace_t* cColorSpace);
SK_C_API sk_gamma_named_t sk_colorspace_gamma_get_gamma_named(const sk_colorspace_t* cColorSpace);
SK_C_API bool sk_colorspace_gamma_close_to_srgb(const sk_colorspace_t* cColorSpace);
SK_C_API bool sk_colorspace_gamma_is_linear(const sk_colorspace_t* cColorSpace);
SK_C_API bool sk_colorspace_is_srgb(const sk_colorspace_t* cColorSpace);
SK_C_API bool sk_colorspace_equals(const sk_colorspace_t* src, const sk_colorspace_t* dst);
SK_C_API bool sk_colorspace_to_xyzd50(const sk_colorspace_t* cColorSpace, sk_matrix44_t* toXYZD50);
SK_C_API const sk_matrix44_t* sk_colorspace_as_to_xyzd50(const sk_colorspace_t* cColorSpace);
SK_C_API const sk_matrix44_t* sk_colorspace_as_from_xyzd50(const sk_colorspace_t* cColorSpace);
SK_C_API bool sk_colorspace_is_numerical_transfer_fn(const sk_colorspace_t* cColorSpace, sk_colorspace_transfer_fn_t* fn);
SK_C_API bool sk_colorspaceprimaries_to_xyzd50(const sk_colorspaceprimaries_t* primaries, sk_matrix44_t* toXYZD50);
SK_C_API void sk_colorspace_transfer_fn_invert(const sk_colorspace_transfer_fn_t* transfer, sk_colorspace_transfer_fn_t* inverted);
SK_C_API float sk_colorspace_transfer_fn_transform(const sk_colorspace_transfer_fn_t* transfer, float x);

SK_C_PLUS_PLUS_END_GUARD

#endif
