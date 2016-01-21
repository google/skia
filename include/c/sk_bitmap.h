/*
 * Copyright 2015 Xamarin Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_bitmap_DEFINED
#define sk_bitmap_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API void sk_bitmap_destructor(sk_bitmap_t* cbitmap);
SK_API sk_bitmap_t* sk_bitmap_new();
SK_API sk_imageinfo_t sk_bitmap_get_info(sk_bitmap_t* cbitmap);
SK_API void* sk_bitmap_get_pixels(sk_bitmap_t* cbitmap, size_t* length);
SK_API size_t sk_bitmap_get_row_bytes(sk_bitmap_t* cbitmap);
SK_API size_t sk_bitmap_get_byte_count(sk_bitmap_t* cbitmap);
SK_API void sk_bitmap_reset(sk_bitmap_t* cbitmap);
SK_API bool sk_bitmap_is_null(sk_bitmap_t* cbitmap);
SK_API bool sk_bitmap_is_immutable(sk_bitmap_t* cbitmap);
SK_API void sk_bitmap_set_immutable(sk_bitmap_t* cbitmap, bool value);
SK_API bool sk_bitmap_is_volatile(sk_bitmap_t* cbitmap);
SK_API void sk_bitmap_set_volatile(sk_bitmap_t* cbitmap, bool value);
SK_API void sk_bitmap_erase(sk_bitmap_t* cbitmap, sk_color_t color);
SK_API void sk_bitmap_erase_rect(sk_bitmap_t* cbitmap, sk_color_t color, sk_irect_t* rect);
SK_API sk_color_t sk_bitmap_get_pixel_color(sk_bitmap_t* cbitmap, int x, int y);
SK_API void sk_bitmap_set_pixel_color(sk_bitmap_t* cbitmap, int x, int y, sk_color_t color);
SK_API bool sk_bitmap_copy(sk_bitmap_t* cbitmap, sk_bitmap_t* dst, sk_colortype_t ct);
SK_API bool sk_bitmap_can_copy_to(sk_bitmap_t* cbitmap, sk_colortype_t ct);
SK_API void sk_bitmap_lock_pixels(sk_bitmap_t* cbitmap);
SK_API void sk_bitmap_unlock_pixels(sk_bitmap_t* cbitmap);
SK_API void sk_bitmap_get_pixel_colors(sk_bitmap_t* cbitmap, sk_color_t* colors);
SK_API void sk_bitmap_set_pixel_colors(sk_bitmap_t* cbitmap, const sk_color_t* colors);
        
SK_API bool sk_imagedecoder_decode_stream (sk_stream_streamrewindable_t* cstream, sk_bitmap_t* bitmap, sk_colortype_t pref, sk_imagedecoder_mode_t mode, sk_imagedecoder_format_t* format);

SK_C_PLUS_PLUS_END_GUARD

#endif
