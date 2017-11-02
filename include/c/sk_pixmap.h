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

SK_C_API void sk_pixmap_destructor(sk_pixmap_t* cpixmap);
SK_C_API sk_pixmap_t* sk_pixmap_new(void);
SK_C_API sk_pixmap_t* sk_pixmap_new_with_params(const sk_imageinfo_t* cinfo, const void* addr, size_t rowBytes, sk_colortable_t* ctable);
SK_C_API void sk_pixmap_reset(sk_pixmap_t* cpixmap);
SK_C_API void sk_pixmap_reset_with_params(sk_pixmap_t* cpixmap, const sk_imageinfo_t* cinfo, const void* addr, size_t rowBytes, sk_colortable_t* ctable);
SK_C_API void sk_pixmap_get_info(sk_pixmap_t* cpixmap, sk_imageinfo_t* cinfo);
SK_C_API size_t sk_pixmap_get_row_bytes(sk_pixmap_t* cpixmap);
SK_C_API const void* sk_pixmap_get_pixels(sk_pixmap_t* cpixmap);
SK_C_API sk_colortable_t* sk_pixmap_get_colortable(sk_pixmap_t* cpixmap);

SK_C_API bool sk_bitmapscaler_resize(const sk_pixmap_t* dst, const sk_pixmap_t* src, sk_bitmapscaler_resizemethod_t method);

SK_C_API sk_color_t sk_color_unpremultiply(const sk_pmcolor_t pmcolor);
SK_C_API sk_pmcolor_t sk_color_premultiply(const sk_color_t color);
SK_C_API void sk_color_unpremultiply_array(const sk_pmcolor_t* pmcolors, int size, sk_color_t* colors);
SK_C_API void sk_color_premultiply_array(const sk_color_t* colors, int size, sk_pmcolor_t* pmcolors);
SK_C_API void sk_color_get_bit_shift(int* a, int* r, int* g, int* b);

SK_C_API bool sk_pixmap_encode_image(sk_wstream_t* dst, const sk_pixmap_t* src, sk_encoded_image_format_t encoder, int quality);

SK_C_API bool sk_pixmap_read_pixels(const sk_pixmap_t* cpixmap, const sk_imageinfo_t* dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY);

SK_C_API void sk_swizzle_swap_rb(uint32_t* dest, const uint32_t* src, int count);

SK_C_PLUS_PLUS_END_GUARD

#endif
