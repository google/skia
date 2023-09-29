/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkMask.h"

#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkSafeMath.h"

#include <array>
#include <climits>

/** returns the product if it is positive and fits in 31 bits. Otherwise this
    returns 0.
 */
static int32_t safeMul32(int32_t a, int32_t b) {
    int64_t size = sk_64_mul(a, b);
    if (size > 0 && SkTFitsIn<int32_t>(size)) {
        return size;
    }
    return 0;
}

size_t SkMask::computeImageSize() const {
    return safeMul32(fBounds.height(), fRowBytes);
}

size_t SkMask::computeTotalImageSize() const {
    size_t size = this->computeImageSize();
    if (fFormat == SkMask::k3D_Format) {
        size = safeMul32(SkToS32(size), 3);
    }
    return size;
}

/** We explicitly use this allocator for SkBimap pixels, so that we can
    freely assign memory allocated by one class to the other.
*/
uint8_t* SkMaskBuilder::AllocImage(size_t size, AllocType at) {
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
void SkMaskBuilder::FreeImage(void* image) {
    sk_free(image);
}

SkMaskBuilder SkMaskBuilder::PrepareDestination(int radiusX, int radiusY, const SkMask& src) {
    SkSafeMath safe;

    SkMaskBuilder dst;
    dst.image() = nullptr;
    dst.format() = SkMask::kA8_Format;

    // dstW = srcW + 2 * radiusX;
    size_t dstW = safe.add(src.fBounds.width(), safe.add(radiusX, radiusX));
    // dstH = srcH + 2 * radiusY;
    size_t dstH = safe.add(src.fBounds.height(), safe.add(radiusY, radiusY));

    size_t toAlloc = safe.mul(dstW, dstH);

    // We can only deal with masks that fit in INT_MAX and sides that fit in int.
    if (!SkTFitsIn<int>(dstW) || !SkTFitsIn<int>(dstH) || toAlloc > INT_MAX || !safe) {
        dst.bounds().setEmpty();
        dst.rowBytes() = 0;
        return dst;
    }

    dst.bounds().setWH(SkTo<int>(dstW), SkTo<int>(dstH));
    dst.bounds().offset(src.fBounds.x(), src.fBounds.y());
    dst.bounds().offset(-radiusX, -radiusY);
    dst.rowBytes() = SkTo<uint32_t>(dstW);

    if (src.fImage != nullptr) {
        dst.image() = SkMaskBuilder::AllocImage(toAlloc);
    }

    return dst;
}


///////////////////////////////////////////////////////////////////////////////

static const int gMaskFormatToShift[] = {
    ~0, // BW -- not supported
    0,  // A8
    0,  // 3D
    2,  // ARGB32
    1,  // LCD16
    0,  // SDF
};

static int maskFormatToShift(SkMask::Format format) {
    SkASSERT((unsigned)format < std::size(gMaskFormatToShift));
    SkASSERT(SkMask::kBW_Format != format);
    return gMaskFormatToShift[format];
}

const void* SkMask::getAddr(int x, int y) const {
    SkASSERT(kBW_Format != fFormat);
    SkASSERT(fBounds.contains(x, y));
    SkASSERT(fImage);

    char* addr = (char*)fImage;
    addr += (y - fBounds.fTop) * fRowBytes;
    addr += (x - fBounds.fLeft) << maskFormatToShift(fFormat);
    return addr;
}
