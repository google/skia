/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_bitmap_DEFINED
#define sk_bitmap_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API void sk_bitmap_destructor(sk_bitmap_t* cbitmap);
SK_C_API sk_bitmap_t* sk_bitmap_new(void);
SK_C_API void sk_bitmap_get_info(sk_bitmap_t* cbitmap, sk_imageinfo_t* info);
SK_C_API void* sk_bitmap_get_pixels(sk_bitmap_t* cbitmap, size_t* length);
SK_C_API size_t sk_bitmap_get_row_bytes(sk_bitmap_t* cbitmap);
SK_C_API size_t sk_bitmap_get_byte_count(sk_bitmap_t* cbitmap);
SK_C_API void sk_bitmap_reset(sk_bitmap_t* cbitmap);
SK_C_API bool sk_bitmap_is_null(sk_bitmap_t* cbitmap);
SK_C_API bool sk_bitmap_is_immutable(sk_bitmap_t* cbitmap);
SK_C_API void sk_bitmap_set_immutable(sk_bitmap_t* cbitmap);
SK_C_API bool sk_bitmap_is_volatile(sk_bitmap_t* cbitmap);
SK_C_API void sk_bitmap_set_volatile(sk_bitmap_t* cbitmap, bool value);
SK_C_API void sk_bitmap_erase(sk_bitmap_t* cbitmap, sk_color_t color);
SK_C_API void sk_bitmap_erase_rect(sk_bitmap_t* cbitmap, sk_color_t color, sk_irect_t* rect);
SK_C_API uint8_t sk_bitmap_get_addr_8(sk_bitmap_t* cbitmap, int x, int y);
SK_C_API uint16_t sk_bitmap_get_addr_16(sk_bitmap_t* cbitmap, int x, int y);
SK_C_API uint32_t sk_bitmap_get_addr_32(sk_bitmap_t* cbitmap, int x, int y);
SK_C_API void* sk_bitmap_get_addr(sk_bitmap_t* cbitmap, int x, int y);
SK_C_API sk_color_t sk_bitmap_get_pixel_color(sk_bitmap_t* cbitmap, int x, int y);
SK_C_API sk_pmcolor_t sk_bitmap_get_index8_color(sk_bitmap_t* cbitmap, int x, int y);
SK_C_API void sk_bitmap_set_pixel_color(sk_bitmap_t* cbitmap, int x, int y, sk_color_t color);
SK_C_API bool sk_bitmap_ready_to_draw(sk_bitmap_t* cbitmap);
SK_C_API void sk_bitmap_get_pixel_colors(sk_bitmap_t* cbitmap, sk_color_t* colors);
SK_C_API void sk_bitmap_set_pixel_colors(sk_bitmap_t* cbitmap, const sk_color_t* colors);
SK_C_API bool sk_bitmap_install_pixels(sk_bitmap_t* cbitmap, const sk_imageinfo_t* cinfo, void* pixels, size_t rowBytes, const sk_bitmap_release_proc releaseProc, void* context);
SK_C_API bool sk_bitmap_install_pixels_with_pixmap(sk_bitmap_t* cbitmap, const sk_pixmap_t* cpixmap);
SK_C_API bool sk_bitmap_install_mask_pixels(sk_bitmap_t* cbitmap, const sk_mask_t* cmask);
SK_C_API bool sk_bitmap_try_alloc_pixels(sk_bitmap_t* cbitmap, const sk_imageinfo_t* requestedInfo, size_t rowBytes);
SK_C_API bool sk_bitmap_try_alloc_pixels_with_flags(sk_bitmap_t* cbitmap, const sk_imageinfo_t* requestedInfo, uint32_t flags);
SK_C_API void sk_bitmap_set_pixels(sk_bitmap_t* cbitmap, void* pixels);
SK_C_API bool sk_bitmap_peek_pixels(sk_bitmap_t* cbitmap, sk_pixmap_t* cpixmap);
SK_C_API bool sk_bitmap_extract_subset(sk_bitmap_t* cbitmap, sk_bitmap_t* dst, sk_irect_t* subset);
SK_C_API bool sk_bitmap_extract_alpha(sk_bitmap_t* cbitmap, sk_bitmap_t* dst, const sk_paint_t* paint, sk_ipoint_t* offset);
SK_C_API void sk_bitmap_notify_pixels_changed(sk_bitmap_t* cbitmap);
SK_C_API void sk_bitmap_swap(sk_bitmap_t* cbitmap, sk_bitmap_t* cother);

SK_C_PLUS_PLUS_END_GUARD

#endif
