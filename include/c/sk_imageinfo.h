/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_imageinfo_DEFINED
#define sk_imageinfo_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_imageinfo_t* sk_imageinfo_new(int width, int height, sk_colortype_t ct, sk_alphatype_t at, sk_colorspace_t* cs);
SK_C_API void sk_imageinfo_delete(sk_imageinfo_t*);

SK_C_API int32_t sk_imageinfo_get_width(const sk_imageinfo_t*);
SK_C_API int32_t sk_imageinfo_get_height(const sk_imageinfo_t*);
SK_C_API sk_colortype_t sk_imageinfo_get_colortype(const sk_imageinfo_t*);
SK_C_API sk_alphatype_t sk_imageinfo_get_alphatype(const sk_imageinfo_t*);

SK_C_API sk_colorspace_t* sk_imageinfo_get_colorspace(const sk_imageinfo_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
