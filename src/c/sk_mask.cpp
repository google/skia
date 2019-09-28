/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Bluebeam Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMask.h"

#include "include/c/sk_mask.h"

#include "src/c/sk_types_priv.h"

uint8_t* sk_mask_alloc_image(size_t bytes) {
    return SkMask::AllocImage(bytes);
}

void sk_mask_free_image(void* image) {
    SkMask::FreeImage(image);
}

bool sk_mask_is_empty(sk_mask_t* cmask) {
    return AsMask(cmask)->isEmpty();
}

size_t sk_mask_compute_image_size(sk_mask_t* cmask) {
    return AsMask(cmask)->computeImageSize();
}

size_t sk_mask_compute_total_image_size(sk_mask_t* cmask) {
    return AsMask(cmask)->computeTotalImageSize();
}

uint8_t sk_mask_get_addr_1(sk_mask_t* cmask, int x, int y) {
    return *(AsMask(cmask)->getAddr1(x, y));
}

uint8_t sk_mask_get_addr_8(sk_mask_t* cmask, int x, int y) {
    return *(AsMask(cmask)->getAddr8(x, y));
}

uint16_t sk_mask_get_addr_lcd_16(sk_mask_t* cmask, int x, int y) {
    return *(AsMask(cmask)->getAddrLCD16(x, y));
}

uint32_t sk_mask_get_addr_32(sk_mask_t* cmask, int x, int y) {
    return *(AsMask(cmask)->getAddr32(x, y));
}

void* sk_mask_get_addr(sk_mask_t* cmask, int x, int y) {
    return AsMask(cmask)->getAddr(x, y);
}
