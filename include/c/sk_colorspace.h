/*
 * Copyright 2017 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_colorspace_DEFINED
#define sk_colorspace_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_colorspace_unref(sk_colorspace_t* cColorSpace);
SK_C_API sk_colorspace_t* sk_colorspace_new_named(sk_colorspace_named_t named);
SK_C_API sk_colorspace_t* sk_colorspace_new_icc(const void* input, size_t len);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_gamma(sk_colorspace_render_target_gamma_t gamma, const sk_matrix44_t* toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_with_coeffs(const sk_colorspace_transfer_fn_t* coeffs, const sk_matrix44_t* toXYZD50);
SK_C_API bool sk_colorspace_gamma_close_to_srgb(const sk_colorspace_t* cColorSpace);
SK_C_API bool sk_colorspace_gamma_is_linear(const sk_colorspace_t* cColorSpace);
SK_C_API bool sk_colorspace_equals(const sk_colorspace_t* src, const sk_colorspace_t* dst);
SK_C_API bool sk_colorspaceprimaries_to_xyzd50(const sk_colorspaceprimaries_t* primaries, sk_matrix44_t* toXYZD50);

SK_C_PLUS_PLUS_END_GUARD

#endif
