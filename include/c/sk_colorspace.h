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

SK_C_API void sk_colorspace_unref(sk_colorspace_t* colorspace);
SK_C_API sk_colorspace_t* sk_colorspace_new_srgb(void);
SK_C_API sk_colorspace_t* sk_colorspace_new_srgb_linear(void);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_matrix44(const sk_colorspace_transfer_fn_t* transferFn, const sk_matrix44_t* toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_new_rgb_named(sk_named_transfer_fn_t transferFn, sk_named_gamut_t toXYZD50);
SK_C_API sk_colorspace_t* sk_colorspace_new_icc(const void* input, size_t len);
SK_C_API bool sk_colorspace_gamma_close_to_srgb(const sk_colorspace_t* colorspace);
SK_C_API bool sk_colorspace_gamma_is_linear(const sk_colorspace_t* colorspace);
SK_C_API bool sk_colorspace_is_numerical_transfer_fn(const sk_colorspace_t* colorspace, sk_colorspace_transfer_fn_t* transferFn);
SK_C_API bool sk_colorspace_to_xyzd50(const sk_colorspace_t* colorspace, sk_matrix44_t* toXYZD50);
SK_C_API bool sk_colorspace_is_srgb(const sk_colorspace_t* colorspace);
SK_C_API bool sk_colorspace_equals(const sk_colorspace_t* src, const sk_colorspace_t* dst);

// sk_colorspace_transfer_fn_t

SK_C_API float sk_colorspace_transfer_fn_eval(const sk_colorspace_transfer_fn_t* transferFn, float x);
SK_C_API bool sk_colorspace_transfer_fn_invert(const sk_colorspace_transfer_fn_t* src, sk_colorspace_transfer_fn_t* dst);

// sk_colorspace_primaries_t

SK_C_API bool sk_colorspace_primaries_to_xyzd50(const sk_colorspace_primaries_t* primaries, sk_matrix44_t* toXYZD50);

SK_C_PLUS_PLUS_END_GUARD

#endif
