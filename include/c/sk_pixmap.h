/*
 * Copyright 2017 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_pixmap_DEFINED
#define sk_pixmap_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API void sk_pixmap_destructor(sk_pixmap_t* cpixmap);
SK_API sk_pixmap_t* sk_pixmap_new();
SK_API sk_pixmap_t* sk_pixmap_new_with_params(const sk_imageinfo_t* cinfo, const void* addr, size_t rowBytes, sk_colortable_t* ctable);
SK_API void sk_pixmap_reset(sk_pixmap_t* cpixmap);
SK_API void sk_pixmap_reset_with_params(sk_pixmap_t* cpixmap, const sk_imageinfo_t* cinfo, const void* addr, size_t rowBytes, sk_colortable_t* ctable);
SK_API void sk_pixmap_get_info(sk_pixmap_t* cpixmap, sk_imageinfo_t* cinfo);
SK_API size_t sk_pixmap_get_row_bytes(sk_pixmap_t* cpixmap);
SK_API const void* sk_pixmap_get_pixels(sk_pixmap_t* cpixmap);
SK_API sk_colortable_t* sk_pixmap_get_colortable(sk_pixmap_t* cpixmap);

SK_C_PLUS_PLUS_END_GUARD

#endif
