/*
 * Copyright 2017 Bluebeam Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMask.h"

#include "sk_mask.h"

sk_mask_t* sk_mask_new() {
    return ToMask(new SkMask());
}

sk_mask_t* sk_mask_new_allocated(uint8_t* image, sk_irect_t bounds, uint32_t rowBytes, sk_mask_format_t format) {
    SkMask* mask = new SkMask();
    mask->fImage = image;
    mask->fBounds = bounds;
    mask->fRowBytes = rowBytes;
    mask->fFormat = (SkMask::Format)format;
    return ToMask(mask);
}

void sk_mask_destructor(sk_mask_t* cmask) {
    SkMask* mask = AsMask(cmask);
    SkMask::FreeImage(mask->fImage);
    delete mask;
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
    return AsMask(cmask)->getAddr1(x, y);
}

uint8_t sk_mask_get_addr_8(sk_mask_t* cmask, int x, int y) {
    return AsMask(cmask)->getAddr8(x, y);
}

uint16_t sk_mask_get_addr_lcd_16(sk_mask_t* cmask, int x, int y) {
    return AsMask(cmask)->getAddr16(x, y);
}

uint32_t sk_mask_get_addr_32(sk_mask_t* cmask, int x, int y) {
    return AsMask(cmask)->getAddr32(x, y);
}

void* sk_mask_get_addr(sk_mask_t* cmask, int x, int y) {
    return AsMask(cmask)->getAddr(x, y);
}

uint8_t* sk_mask_get_image(sk_mask_t* cmask) {
    return AsMask(cmask)->fImage();
}

sk_irect_t sk_mask_get_bounds(sk_mask_t* cmask) {
    return ToIRect(AsMask(cmask)->fBounds);
}

uint32_t sk_mask_get_row_bytes(sk_mask_t* cmask) {
    return AsMask(cmask)->fRowBytes;
}

sk_mask_format_t sk_mask_get_format(sk_mask_t* cmask) {
    return (sk_mask_format_t)(AsMask(cmask)->fFormat);
}