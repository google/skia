
/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Sk64.h"
#include "SkMask.h"

/** returns the product if it is positive and fits in 31 bits. Otherwise this
    returns 0.
 */
static int32_t safeMul32(int32_t a, int32_t b) {
    Sk64 size;
    size.setMul(a, b);
    if (size.is32() && size.isPos()) {
        return size.get32();
    }
    return 0;
}

size_t SkMask::computeImageSize() const {
    return safeMul32(fBounds.height(), fRowBytes);
}

size_t SkMask::computeTotalImageSize() const {
    size_t size = this->computeImageSize();
    if (fFormat == SkMask::k3D_Format) {
        size = safeMul32(size, 3);
    }
    return size;
}

/** We explicitly use this allocator for SkBimap pixels, so that we can
    freely assign memory allocated by one class to the other.
*/
uint8_t* SkMask::AllocImage(size_t size) {
    return (uint8_t*)sk_malloc_throw(SkAlign4(size));
}

/** We explicitly use this allocator for SkBimap pixels, so that we can
    freely assign memory allocated by one class to the other.
*/
void SkMask::FreeImage(void* image) {
    sk_free(image);
}

