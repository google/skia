/*
 * Copyright 2017 Bluebeam Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_mask_DEFINED
#define sk_mask_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_API sk_mask_t* sk_mask_new();
SK_API sk_mask_t* sk_mask_new_allocated(uint8_t* image, sk_irect_t bounds, uint32_t rowBytes, sk_mask_format_t format);
SK_API void sk_mask_destructor(sk_mask_t* cmask);
SK_API bool sk_mask_is_empty(sk_mask_t* cmask);
SK_API size_t sk_mask_compute_image_size(sk_mask_t* cmask);
SK_API size_t sk_mask_compute_total_image_size(sk_mask_t* cmask);
SK_API uint8_t sk_mask_get_addr_1(sk_mask_t* cmask, int x, int y);
SK_API uint8_t sk_mask_get_addr_8(sk_mask_t* cmask, int x, int y);
SK_API uint16_t sk_mask_get_addr_lcd_16(sk_mask_t* cmask, int x, int y);
SK_API uint32_t sk_mask_get_addr_32(sk_mask_t* cmask, int x, int y);
SK_API void* sk_mask_get_addr(sk_mask_t* cmask, int x, int y);
SK_API uint8_t* sk_mask_get_image(sk_mask_t* cmask);
SK_API sk_irect_t sk_mask_get_bounds(sk_mask_t* cmask);
SK_API uint32_t sk_mask_get_row_bytes(sk_mask_t* cmask);
SK_API sk_mask_format_t sk_mask_get_format(sk_mask_t* cmask);

SK_C_PLUS_PLUS_END_GUARD

#endif
