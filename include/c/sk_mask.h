/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Bluebeam Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_mask_DEFINED
#define sk_mask_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API uint8_t* sk_mask_alloc_image(size_t bytes);
SK_C_API void sk_mask_free_image(void* image);
SK_C_API bool sk_mask_is_empty(sk_mask_t* cmask);
SK_C_API size_t sk_mask_compute_image_size(sk_mask_t* cmask);
SK_C_API size_t sk_mask_compute_total_image_size(sk_mask_t* cmask);
SK_C_API uint8_t sk_mask_get_addr_1(sk_mask_t* cmask, int x, int y);
SK_C_API uint8_t sk_mask_get_addr_8(sk_mask_t* cmask, int x, int y);
SK_C_API uint16_t sk_mask_get_addr_lcd_16(sk_mask_t* cmask, int x, int y);
SK_C_API uint32_t sk_mask_get_addr_32(sk_mask_t* cmask, int x, int y);
SK_C_API void* sk_mask_get_addr(sk_mask_t* cmask, int x, int y);

SK_C_PLUS_PLUS_END_GUARD

#endif
