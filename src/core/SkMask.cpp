/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMask.h"
#include "SkMalloc.h"
#include "SkSafeMath.h"

/** returns the product if it is positive and fits in 31 bits. Otherwise this
    returns 0.
 */
static size_t safe_mul(size_t a, size_t b) {
    SkSafeMath safe;
    size_t r = safe.mul(a, b);
    if (safe) {
        return r;
    }

    return 0;
}

static size_t safe_align4(size_t v) {
    uint64_t adjusted = v + 3;
    uint64_t possible = adjusted > v ? (adjusted & ~3) : 0;
    return SkTFitsIn<size_t>(possible) ? possible : 0;
}

size_t SkMask::computeImageSize() const {
    auto height = fBounds.height();
    auto safeHeight = SkTo<uint32_t>(height >= 0 ? height : 0);
    return safe_mul(safeHeight, fRowBytes);
}

size_t SkMask::computeTotalImageSize() const {
    return ComputeImageSize(fFormat, fBounds.width(), fBounds.height());
}

static size_t bits_to_bytes(size_t bits) {
    return (bits + 7) >> 3;
}

size_t SkMask::AlignmentForMask(SkMask::Format format) {
    switch (format) {
        case SkMask::kBW_Format:
            return alignof(uint8_t);
        case SkMask::kA8_Format:
            return alignof(uint8_t);
        case SkMask::k3D_Format:
            return alignof(uint8_t);
        case SkMask::kARGB32_Format:
            return alignof(uint32_t);
        case SkMask::kLCD16_Format:
            return alignof(uint16_t);
        default:
            SK_ABORT("Unknown mask format.");
            break;
    }
    // Should not reach here.
    return alignof(uint64_t);
}

size_t SkMask::ComputeRowBytes(SkMask::Format format, int32_t width) {
    auto safeWidth = SkTo<uint32_t>(width >= 0 ? width : 0);
    switch (format) {
        case SkMask::kBW_Format:
            // Always safe.
            return SkTo<size_t>(bits_to_bytes(safeWidth));
        case SkMask::kA8_Format:
            return safe_align4(static_cast<size_t>(safeWidth));
        case SkMask::k3D_Format:
            return safe_align4(static_cast<size_t>(safeWidth));
        case SkMask::kARGB32_Format:
            return safe_mul(safeWidth, sizeof(uint32_t));
        case SkMask::kLCD16_Format:
            return safe_align4(safe_mul(safeWidth, sizeof(uint16_t)));
        default:
            SK_ABORT("Unknown mask format.");
            break;
    }
    // Should not reach here.
    return 0;
}

size_t SkMask::ComputeImageSize(SkMask::Format format, int32_t width, int32_t height) {
    size_t size = ComputeRowBytes(format, width);
    if (format == SkMask::k3D_Format) {
        size = safe_mul(size, 3);
    }
    auto safeHeight = SkTo<uint32_t>(height >= 0 ? height : 0);
    auto result = safe_mul(size, safeHeight);
    return result;
}

/** We explicitly use this allocator for SkBimap pixels, so that we can
    freely assign memory allocated by one class to the other.
*/
uint8_t* SkMask::AllocImage(size_t size, AllocType at) {
    size_t aligned_size = SkSafeMath::Align4(size);
    unsigned flags = SK_MALLOC_THROW;
    if (at == kZeroInit_Alloc) {
        flags |= SK_MALLOC_ZERO_INITIALIZE;
    }
    return static_cast<uint8_t*>(sk_malloc_flags(aligned_size, flags));
}

/** We explicitly use this allocator for SkBimap pixels, so that we can
    freely assign memory allocated by one class to the other.
*/
void SkMask::FreeImage(void* image) {
    sk_free(image);
}

///////////////////////////////////////////////////////////////////////////////

static const int gMaskFormatToShift[] = {
    ~0, // BW -- not supported
    0,  // A8
    0,  // 3D
    2,  // ARGB32
    1,  // LCD16
};

static int maskFormatToShift(SkMask::Format format) {
    SkASSERT((unsigned)format < SK_ARRAY_COUNT(gMaskFormatToShift));
    SkASSERT(SkMask::kBW_Format != format);
    return gMaskFormatToShift[format];
}

void* SkMask::getAddr(int x, int y) const {
    SkASSERT(kBW_Format != fFormat);
    SkASSERT(fBounds.contains(x, y));
    SkASSERT(fImage);

    char* addr = (char*)fImage;
    addr += (y - fBounds.fTop) * fRowBytes;
    addr += (x - fBounds.fLeft) << maskFormatToShift(fFormat);
    return addr;
}
