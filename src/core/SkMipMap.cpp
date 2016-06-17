/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMipMap.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkHalf.h"
#include "SkMath.h"
#include "SkNx.h"
#include "SkTypes.h"

//
// ColorTypeFilter is the "Type" we pass to some downsample template functions.
// It controls how we expand a pixel into a large type, with space between each component,
// so we can then perform our simple filter (either box or triangle) and store the intermediates
// in the expanded type.
//

struct ColorTypeFilter_8888 {
    typedef uint32_t Type;
#if defined(SKNX_IS_FAST)
    static Sk4h Expand(uint32_t x) {
        return SkNx_cast<uint16_t>(Sk4b::Load(&x));
    }
    static uint32_t Compact(const Sk4h& x) {
        uint32_t r;
        SkNx_cast<uint8_t>(x).store(&r);
        return r;
    }
#else
    static uint64_t Expand(uint32_t x) {
        return (x & 0xFF00FF) | ((uint64_t)(x & 0xFF00FF00) << 24);
    }
    static uint32_t Compact(uint64_t x) {
        return (uint32_t)((x & 0xFF00FF) | ((x >> 24) & 0xFF00FF00));
    }
#endif
};

struct ColorTypeFilter_565 {
    typedef uint16_t Type;
    static uint32_t Expand(uint16_t x) {
        return (x & ~SK_G16_MASK_IN_PLACE) | ((x & SK_G16_MASK_IN_PLACE) << 16);
    }
    static uint16_t Compact(uint32_t x) {
        return (x & ~SK_G16_MASK_IN_PLACE) | ((x >> 16) & SK_G16_MASK_IN_PLACE);
    }
};

struct ColorTypeFilter_4444 {
    typedef uint16_t Type;
    static uint32_t Expand(uint16_t x) {
        return (x & 0xF0F) | ((x & ~0xF0F) << 12);
    }
    static uint16_t Compact(uint32_t x) {
        return (x & 0xF0F) | ((x >> 12) & ~0xF0F);
    }
};

struct ColorTypeFilter_8 {
    typedef uint8_t Type;
    static unsigned Expand(unsigned x) {
        return x;
    }
    static uint8_t Compact(unsigned x) {
        return (uint8_t)x;
    }
};

struct ColorTypeFilter_F16 {
    typedef uint64_t Type; // SkHalf x4
    static Sk4f Expand(uint64_t x) {
        return SkHalfToFloat_01(x);
    }
    static uint64_t Compact(const Sk4f& x) {
        return SkFloatToHalf_01(x);
    }
};

template <typename T> T add_121(const T& a, const T& b, const T& c) {
    return a + b + b + c;
}

template <typename T> T shift_right(const T& x, int bits) {
    return x >> bits;
}

Sk4f shift_right(const Sk4f& x, int bits) {
    return x * (1.0f / (1 << bits));
}

template <typename T> T shift_left(const T& x, int bits) {
    return x << bits;
}

Sk4f shift_left(const Sk4f& x, int bits) {
    return x * (1 << bits);
}

//
//  To produce each mip level, we need to filter down by 1/2 (e.g. 100x100 -> 50,50)
//  If the starting dimension is odd, we floor the size of the lower level (e.g. 101 -> 50)
//  In those (odd) cases, we use a triangle filter, with 1-pixel overlap between samplings,
//  else for even cases, we just use a 2x box filter.
//
//  This produces 4 possible isotropic filters: 2x2 2x3 3x2 3x3 where WxH indicates the number of
//  src pixels we need to sample in each dimension to produce 1 dst pixel.
//
//  OpenGL expects a full mipmap stack to contain anisotropic space as well.
//  This means a 100x1 image would continue down to a 50x1 image, 25x1 image...
//  Because of this, we need 4 more anisotropic filters: 1x2, 1x3, 2x1, 3x1.

template <typename F> void downsample_1_2(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c10 = F::Expand(p1[0]);

        auto c = c00 + c10;
        d[i] = F::Compact(shift_right(c, 1));
        p0 += 2;
        p1 += 2;
    }
}

template <typename F> void downsample_1_3(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto p2 = (const typename F::Type*)((const char*)p1 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c10 = F::Expand(p1[0]);
        auto c20 = F::Expand(p2[0]);

        auto c = add_121(c00, c10, c20);
        d[i] = F::Compact(shift_right(c, 2));
        p0 += 2;
        p1 += 2;
        p2 += 2;
    }
}

template <typename F> void downsample_2_1(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c01 = F::Expand(p0[1]);

        auto c = c00 + c01;
        d[i] = F::Compact(shift_right(c, 1));
        p0 += 2;
    }
}

template <typename F> void downsample_2_2(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c01 = F::Expand(p0[1]);
        auto c10 = F::Expand(p1[0]);
        auto c11 = F::Expand(p1[1]);

        auto c = c00 + c10 + c01 + c11;
        d[i] = F::Compact(shift_right(c, 2));
        p0 += 2;
        p1 += 2;
    }
}

template <typename F> void downsample_2_3(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto p2 = (const typename F::Type*)((const char*)p1 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c01 = F::Expand(p0[1]);
        auto c10 = F::Expand(p1[0]);
        auto c11 = F::Expand(p1[1]);
        auto c20 = F::Expand(p2[0]);
        auto c21 = F::Expand(p2[1]);

        auto c = add_121(c00, c10, c20) + add_121(c01, c11, c21);
        d[i] = F::Compact(shift_right(c, 3));
        p0 += 2;
        p1 += 2;
        p2 += 2;
    }
}

template <typename F> void downsample_3_1(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto d = static_cast<typename F::Type*>(dst);

    auto c02 = F::Expand(p0[0]);
    for (int i = 0; i < count; ++i) {
        auto c00 = c02;
        auto c01 = F::Expand(p0[1]);
             c02 = F::Expand(p0[2]);

        auto c = add_121(c00, c01, c02);
        d[i] = F::Compact(shift_right(c, 2));
        p0 += 2;
    }
}

template <typename F> void downsample_3_2(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    auto c02 = F::Expand(p0[0]);
    auto c12 = F::Expand(p1[0]);
    for (int i = 0; i < count; ++i) {
        auto c00 = c02;
        auto c01 = F::Expand(p0[1]);
             c02 = F::Expand(p0[2]);
        auto c10 = c12;
        auto c11 = F::Expand(p1[1]);
             c12 = F::Expand(p1[2]);

        auto c = add_121(c00, c01, c02) + add_121(c10, c11, c12);
        d[i] = F::Compact(shift_right(c, 3));
        p0 += 2;
        p1 += 2;
    }
}

template <typename F> void downsample_3_3(void* dst, const void* src, size_t srcRB, int count) {
    SkASSERT(count > 0);
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto p2 = (const typename F::Type*)((const char*)p1 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    auto c02 = F::Expand(p0[0]);
    auto c12 = F::Expand(p1[0]);
    auto c22 = F::Expand(p2[0]);
    for (int i = 0; i < count; ++i) {
        auto c00 = c02;
        auto c01 = F::Expand(p0[1]);
             c02 = F::Expand(p0[2]);
        auto c10 = c12;
        auto c11 = F::Expand(p1[1]);
             c12 = F::Expand(p1[2]);
        auto c20 = c22;
        auto c21 = F::Expand(p2[1]);
             c22 = F::Expand(p2[2]);

        auto c =
            add_121(c00, c01, c02) +
            shift_left(add_121(c10, c11, c12), 1) +
            add_121(c20, c21, c22);
        d[i] = F::Compact(shift_right(c, 4));
        p0 += 2;
        p1 += 2;
        p2 += 2;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t SkMipMap::AllocLevelsSize(int levelCount, size_t pixelSize) {
    if (levelCount < 0) {
        return 0;
    }
    int64_t size = sk_64_mul(levelCount + 1, sizeof(Level)) + pixelSize;
    if (!sk_64_isS32(size)) {
        return 0;
    }
    return sk_64_asS32(size);
}

SkMipMap* SkMipMap::Build(const SkPixmap& src, SkDiscardableFactoryProc fact) {
    typedef void FilterProc(void*, const void* srcPtr, size_t srcRB, int count);

    FilterProc* proc_1_2 = nullptr;
    FilterProc* proc_1_3 = nullptr;
    FilterProc* proc_2_1 = nullptr;
    FilterProc* proc_2_2 = nullptr;
    FilterProc* proc_2_3 = nullptr;
    FilterProc* proc_3_1 = nullptr;
    FilterProc* proc_3_2 = nullptr;
    FilterProc* proc_3_3 = nullptr;

    const SkColorType ct = src.colorType();
    const SkAlphaType at = src.alphaType();
    switch (ct) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_8888>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_8888>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_8888>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_8888>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_8888>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_8888>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_8888>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_8888>;
            break;
        case kRGB_565_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_565>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_565>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_565>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_565>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_565>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_565>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_565>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_565>;
            break;
        case kARGB_4444_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_4444>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_4444>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_4444>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_4444>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_4444>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_4444>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_4444>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_4444>;
            break;
        case kAlpha_8_SkColorType:
        case kGray_8_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_8>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_8>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_8>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_8>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_8>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_8>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_8>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_8>;
            break;
        case kRGBA_F16_SkColorType:
            proc_1_2 = downsample_1_2<ColorTypeFilter_F16>;
            proc_1_3 = downsample_1_3<ColorTypeFilter_F16>;
            proc_2_1 = downsample_2_1<ColorTypeFilter_F16>;
            proc_2_2 = downsample_2_2<ColorTypeFilter_F16>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_F16>;
            proc_3_1 = downsample_3_1<ColorTypeFilter_F16>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_F16>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_F16>;
            break;
        default:
            // TODO: We could build miplevels for kIndex8 if the levels were in 8888.
            //       Means using more ram, but the quality would be fine.
            return nullptr;
    }

    if (src.width() <= 1 && src.height() <= 1) {
        return nullptr;
    }
    // whip through our loop to compute the exact size needed
    size_t  size = 0;
    int countLevels = 0;
    {
        int width = src.width();
        int height = src.height();
        for (;;) {
            width = SkTMax(1, width >> 1);
            height = SkTMax(1, height >> 1);
            size += SkColorTypeMinRowBytes(ct, width) * height;
            countLevels += 1;
            if (1 == width && 1 == height) {
                break;
            }
        }
    }

    SkASSERT(countLevels == SkMipMap::ComputeLevelCount(src.width(), src.height()));

    size_t storageSize = SkMipMap::AllocLevelsSize(countLevels, size);
    if (0 == storageSize) {
        return nullptr;
    }

    SkMipMap* mipmap;
    if (fact) {
        SkDiscardableMemory* dm = fact(storageSize);
        if (nullptr == dm) {
            return nullptr;
        }
        mipmap = new SkMipMap(storageSize, dm);
    } else {
        mipmap = new SkMipMap(sk_malloc_throw(storageSize), storageSize);
    }

    // init
    mipmap->fCount = countLevels;
    mipmap->fLevels = (Level*)mipmap->writable_data();

    Level* levels = mipmap->fLevels;
    uint8_t*    baseAddr = (uint8_t*)&levels[countLevels];
    uint8_t*    addr = baseAddr;
    int         width = src.width();
    int         height = src.height();
    uint32_t    rowBytes;
    SkPixmap    srcPM(src);

    for (int i = 0; i < countLevels; ++i) {
        FilterProc* proc;
        if (height & 1) {
            if (height == 1) {        // src-height is 1
                if (width & 1) {      // src-width is 3
                    proc = proc_3_1;
                } else {              // src-width is 2
                    proc = proc_2_1;
                }
            } else {                  // src-height is 3
                if (width & 1) {
                    if (width == 1) { // src-width is 1
                        proc = proc_1_3;
                    } else {          // src-width is 3
                        proc = proc_3_3;
                    }
                } else {              // src-width is 2
                    proc = proc_2_3;
                }
            }
        } else {                      // src-height is 2
            if (width & 1) {
                if (width == 1) {     // src-width is 1
                    proc = proc_1_2;
                } else {              // src-width is 3
                    proc = proc_3_2;
                }
            } else {                  // src-width is 2
                proc = proc_2_2;
            }
        }
        width = SkTMax(1, width >> 1);
        height = SkTMax(1, height >> 1);
        rowBytes = SkToU32(SkColorTypeMinRowBytes(ct, width));

        levels[i].fPixmap = SkPixmap(SkImageInfo::Make(width, height, ct, at), addr, rowBytes);
        levels[i].fScale  = SkSize::Make(SkIntToScalar(width)  / src.width(),
                                         SkIntToScalar(height) / src.height());

        const SkPixmap& dstPM = levels[i].fPixmap;
        const void* srcBasePtr = srcPM.addr();
        void* dstBasePtr = dstPM.writable_addr();

        const size_t srcRB = srcPM.rowBytes();
        for (int y = 0; y < height; y++) {
            proc(dstBasePtr, srcBasePtr, srcRB, width);
            srcBasePtr = (char*)srcBasePtr + srcRB * 2; // jump two rows
            dstBasePtr = (char*)dstBasePtr + dstPM.rowBytes();
        }
        srcPM = dstPM;
        addr += height * rowBytes;
    }
    SkASSERT(addr == baseAddr + size);

    return mipmap;
}

int SkMipMap::ComputeLevelCount(int baseWidth, int baseHeight) {
    if (baseWidth < 1 || baseHeight < 1) {
        return 0;
    }

    // OpenGL's spec requires that each mipmap level have height/width equal to
    // max(1, floor(original_height / 2^i)
    // (or original_width) where i is the mipmap level.
    // Continue scaling down until both axes are size 1.

    const int largestAxis = SkTMax(baseWidth, baseHeight);
    if (largestAxis < 2) {
        // SkMipMap::Build requires a minimum size of 2.
        return 0;
    }
    const int leadingZeros = SkCLZ(static_cast<uint32_t>(largestAxis));
    // If the value 00011010 has 3 leading 0s then it has 5 significant bits
    // (the bits which are not leading zeros)
    const int significantBits = (sizeof(uint32_t) * 8) - leadingZeros;
    // This is making the assumption that the size of a byte is 8 bits
    // and that sizeof(uint32_t)'s implementation-defined behavior is 4.
    int mipLevelCount = significantBits;

    // SkMipMap does not include the base mip level.
    // For example, it contains levels 1-x instead of 0-x.
    // This is because the image used to create SkMipMap is the base level.
    // So subtract 1 from the mip level count.
    if (mipLevelCount > 0) {
        --mipLevelCount;
    }

    return mipLevelCount;
}

///////////////////////////////////////////////////////////////////////////////

bool SkMipMap::extractLevel(const SkSize& scaleSize, Level* levelPtr) const {
    if (nullptr == fLevels) {
        return false;
    }

    SkASSERT(scaleSize.width() >= 0 && scaleSize.height() >= 0);

#ifndef SK_SUPPORT_LEGACY_ANISOTROPIC_MIPMAP_SCALE
    // Use the smallest scale to match the GPU impl.
    const SkScalar scale = SkTMin(scaleSize.width(), scaleSize.height());
#else
    // Ideally we'd pick the smaller scale, to match Ganesh.  But ignoring one of the
    // scales can produce some atrocious results, so for now we use the geometric mean.
    // (https://bugs.chromium.org/p/skia/issues/detail?id=4863)
    const SkScalar scale = SkScalarSqrt(scaleSize.width() * scaleSize.height());
#endif

    if (scale >= SK_Scalar1 || scale <= 0 || !SkScalarIsFinite(scale)) {
        return false;
    }

    SkScalar L = -SkScalarLog2(scale);
    if (!SkScalarIsFinite(L)) {
        return false;
    }
    SkASSERT(L >= 0);
//    int rndLevel = SkScalarRoundToInt(L);
    int level = SkScalarFloorToInt(L);
//    SkDebugf("mipmap scale=%g L=%g level=%d rndLevel=%d\n", scale, L, level, rndLevel);

    SkASSERT(level >= 0);
    if (level <= 0) {
        return false;
    }

    if (level > fCount) {
        level = fCount;
    }
    if (levelPtr) {
        *levelPtr = fLevels[level - 1];
    }
    return true;
}

// Helper which extracts a pixmap from the src bitmap
//
SkMipMap* SkMipMap::Build(const SkBitmap& src, SkDiscardableFactoryProc fact) {
    SkAutoPixmapUnlock srcUnlocker;
    if (!src.requestLock(&srcUnlocker)) {
        return nullptr;
    }
    const SkPixmap& srcPixmap = srcUnlocker.pixmap();
    // Try to catch where we might have returned nullptr for src crbug.com/492818
    if (nullptr == srcPixmap.addr()) {
        sk_throw();
    }
    return Build(srcPixmap, fact);
}

int SkMipMap::countLevels() const {
    return fCount;
}

bool SkMipMap::getLevel(int index, Level* levelPtr) const {
    if (NULL == fLevels) {
        return false;
    }
    if (index < 0) {
        return false;
    }
    if (index > fCount - 1) {
        return false;
    }
    if (levelPtr) {
        *levelPtr = fLevels[index];
    }
    return true;
}
