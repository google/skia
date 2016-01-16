/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMipMap.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"

#ifdef SK_SUPPORT_LEGACY_MIPLEVEL_BUILDER

static void downsample32_nocheck(void* dst, int, int, const void* srcPtr, const SkPixmap& srcPM) {
    const uint32_t* p = static_cast<const uint32_t*>(srcPtr);
    const uint32_t* baseP = p;
    uint32_t c, ag, rb;

    c = *p; ag = (c >> 8) & 0xFF00FF; rb = c & 0xFF00FF;
    p += 1;

    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    p = baseP;
    p += srcPM.rowBytes() >> 2;

    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;
    p += 1;

    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    *(uint32_t*)dst = ((rb >> 2) & 0xFF00FF) | ((ag << 6) & 0xFF00FF00);
}

static void downsample32_check(void* dst, int x, int y, const void* srcPtr, const SkPixmap& srcPM) {
    const uint32_t* p = static_cast<const uint32_t*>(srcPtr);
    const uint32_t* baseP = p;

    x <<= 1;
    y <<= 1;
    SkASSERT(srcPM.addr32(x, y) == p);

    SkPMColor c, ag, rb;

    c = *p; ag = (c >> 8) & 0xFF00FF; rb = c & 0xFF00FF;
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    p = baseP;
    if (y < srcPM.height() - 1) {
        p += srcPM.rowBytes() >> 2;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    *((uint32_t*)dst) = ((rb >> 2) & 0xFF00FF) | ((ag << 6) & 0xFF00FF00);
}

static inline uint32_t expand16(U16CPU c) {
    return (c & ~SK_G16_MASK_IN_PLACE) | ((c & SK_G16_MASK_IN_PLACE) << 16);
}

// returns dirt in the top 16bits, but we don't care, since we only
// store the low 16bits.
static inline U16CPU pack16(uint32_t c) {
    return (c & ~SK_G16_MASK_IN_PLACE) | ((c >> 16) & SK_G16_MASK_IN_PLACE);
}

static void downsample16(void* dst, int x, int y, const void* srcPtr, const SkPixmap& srcPM) {
    const uint16_t* p = static_cast<const uint16_t*>(srcPtr);
    const uint16_t* baseP = p;

    x <<= 1;
    y <<= 1;
    SkASSERT(srcPM.addr16(x, y) == p);

    SkPMColor c;

    c = expand16(*p);
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c += expand16(*p);

    p = baseP;
    if (y < srcPM.height() - 1) {
        p += srcPM.rowBytes() >> 1;
    }
    c += expand16(*p);
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c += expand16(*p);

    *((uint16_t*)dst) = (uint16_t)pack16(c >> 2);
}

static uint32_t expand4444(U16CPU c) {
    return (c & 0xF0F) | ((c & ~0xF0F) << 12);
}

static U16CPU collaps4444(uint32_t c) {
    return (c & 0xF0F) | ((c >> 12) & ~0xF0F);
}

static void downsample4444(void* dst, int x, int y, const void* srcPtr, const SkPixmap& srcPM) {
    const uint16_t* p = static_cast<const uint16_t*>(srcPtr);
    const uint16_t* baseP = p;

    x <<= 1;
    y <<= 1;
    SkASSERT(srcPM.addr16(x, y) == p);

    uint32_t c;

    c = expand4444(*p);
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);

    p = baseP;
    if (y < srcPM.height() - 1) {
        p += srcPM.rowBytes() >> 1;
    }
    c += expand4444(*p);
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);

    *((uint16_t*)dst) = (uint16_t)collaps4444(c >> 2);
}

static void downsample8_nocheck(void* dst, int, int, const void* srcPtr, const SkPixmap& srcPM) {
    const size_t rb = srcPM.rowBytes();
    const uint8_t* p = static_cast<const uint8_t*>(srcPtr);
    *(uint8_t*)dst = (p[0] + p[1] + p[rb] + p[rb + 1]) >> 2;
}

static void downsample8_check(void* dst, int x, int y, const void* srcPtr, const SkPixmap& srcPM) {
    const uint8_t* p = static_cast<const uint8_t*>(srcPtr);
    const uint8_t* baseP = p;

    x <<= 1;
    y <<= 1;
    SkASSERT(srcPM.addr8(x, y) == p);

    unsigned c = *p;
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c += *p;

    p = baseP;
    if (y < srcPM.height() - 1) {
        p += srcPM.rowBytes();
    }
    c += *p;
    if (x < srcPM.width() - 1) {
        p += 1;
    }
    c += *p;

    *(uint8_t*)dst = c >> 2;
}

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

typedef void SkDownSampleProc(void*, int x, int y, const void* srcPtr, const SkPixmap& srcPM);

SkMipMap* SkMipMap::Build(const SkBitmap& src, SkDiscardableFactoryProc fact) {
    SkDownSampleProc* proc_nocheck, *proc_check;

    const SkColorType ct = src.colorType();
    const SkAlphaType at = src.alphaType();
    switch (ct) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            proc_check = downsample32_check;
            proc_nocheck = downsample32_nocheck;
            break;
        case kRGB_565_SkColorType:
            proc_check = downsample16;
            proc_nocheck = proc_check;
            break;
        case kARGB_4444_SkColorType:
            proc_check = downsample4444;
            proc_nocheck = proc_check;
            break;
        case kAlpha_8_SkColorType:
        case kGray_8_SkColorType:
            proc_check = downsample8_check;
            proc_nocheck = downsample8_nocheck;
            break;
        default:
            return nullptr; // don't build mipmaps for any other colortypes (yet)
    }

    // whip through our loop to compute the exact size needed
    size_t  size = 0;
    int     countLevels = 0;
    {
        int width = src.width();
        int height = src.height();
        for (;;) {
            width >>= 1;
            height >>= 1;
            if (0 == width || 0 == height) {
                break;
            }
            size += SkColorTypeMinRowBytes(ct, width) * height;
            countLevels += 1;
        }
    }
    if (0 == countLevels) {
        return nullptr;
    }

    size_t storageSize = SkMipMap::AllocLevelsSize(countLevels, size);
    if (0 == storageSize) {
        return nullptr;
    }

    SkAutoPixmapUnlock srcUnlocker;
    if (!src.requestLock(&srcUnlocker)) {
        return nullptr;
    }
    const SkPixmap& srcPixmap = srcUnlocker.pixmap();
    // Try to catch where we might have returned nullptr for src crbug.com/492818
    if (nullptr == srcPixmap.addr()) {
        sk_throw();
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
    SkPixmap    srcPM(srcPixmap);

    for (int i = 0; i < countLevels; ++i) {
        width >>= 1;
        height >>= 1;
        rowBytes = SkToU32(SkColorTypeMinRowBytes(ct, width));

        levels[i].fPixels   = addr;
        levels[i].fWidth    = width;
        levels[i].fHeight   = height;
        levels[i].fRowBytes = rowBytes;
        levels[i].fScale    = (float)width / src.width();

        SkPixmap dstPM(SkImageInfo::Make(width, height, ct, at), addr, rowBytes);

        const int widthEven = width & ~1;
        const int heightEven = height & ~1;
        const size_t pixelSize = srcPM.info().bytesPerPixel();

        const void* srcBasePtr = srcPM.addr();
        void* dstBasePtr = dstPM.writable_addr();
        for (int y = 0; y < heightEven; y++) {
            const void* srcPtr = srcBasePtr;
            void* dstPtr = dstBasePtr;
            for (int x = 0; x < widthEven; x++) {
                proc_nocheck(dstPtr, x, y, srcPtr, srcPM);
                srcPtr = (char*)srcPtr + pixelSize * 2;
                dstPtr = (char*)dstPtr + pixelSize;
            }
            if (width & 1) {
                proc_check(dstPtr, widthEven, y, srcPtr, srcPM);
            }

            srcBasePtr = (char*)srcBasePtr + srcPM.rowBytes() * 2;
            dstBasePtr = (char*)dstBasePtr + dstPM.rowBytes();
        }
        if (height & 1) {
            const void* srcPtr = srcBasePtr;
            void* dstPtr = dstBasePtr;
            for (int x = 0; x < width; x++) {
                proc_check(dstPtr, x, heightEven, srcPtr, srcPM);
                srcPtr = (char*)srcPtr + pixelSize * 2;
                dstPtr = (char*)dstPtr + pixelSize;
            }
        }
        srcPM = dstPM;
        addr += height * rowBytes;
    }
    SkASSERT(addr == baseAddr + size);
    
    return mipmap;
}

#else   // new technique that handles odd dimensions better

//
// ColorTypeFilter is the "Type" we pass to some downsample template functions.
// It controls how we expand a pixel into a large type, with space between each component,
// so we can then perform our simple filter (either box or triangle) and store the intermediates
// in the expanded type.
//

struct ColorTypeFilter_8888 {
    typedef uint32_t Type;
    static uint64_t Expand(uint32_t x) {
        return (x & 0xFF00FF) | ((uint64_t)(x & 0xFF00FF00) << 24);
    }
    static uint32_t Compact(uint64_t x) {
        return (uint32_t)((x & 0xFF00FF) | ((x >> 24) & 0xFF00FF00));
    }
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

template <typename T> T add_121(T a, T b, T c) {
    return a + b + b + c;
}

//
//  To produce each mip level, we need to filter down by 1/2 (e.g. 100x100 -> 50,50)
//  If the starting dimension is odd, we floor the size of the lower level (e.g. 101 -> 50)
//  In those (odd) cases, we use a triangle filter, with 1-pixel overlap between samplings,
//  else for even cases, we just use a 2x box filter.
//
//  This produces 4 possible filters: 2x2 2x3 3x2 3x3 where WxH indicates the number of src pixels
//  we need to sample in each dimension to produce 1 dst pixel.
//

template <typename F> void downsample_2_2(void* dst, const void* src, size_t srcRB, int count) {
    auto p0 = static_cast<const typename F::Type*>(src);
    auto p1 = (const typename F::Type*)((const char*)p0 + srcRB);
    auto d = static_cast<typename F::Type*>(dst);

    for (int i = 0; i < count; ++i) {
        auto c00 = F::Expand(p0[0]);
        auto c01 = F::Expand(p0[1]);
        auto c10 = F::Expand(p1[0]);
        auto c11 = F::Expand(p1[1]);

        auto c = c00 + c10 + c01 + c11;
        d[i] = F::Compact(c >> 2);
        p0 += 2;
        p1 += 2;
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
        d[i] = F::Compact(c >> 3);
        p0 += 2;
        p1 += 2;
    }
}

template <typename F> void downsample_2_3(void* dst, const void* src, size_t srcRB, int count) {
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
        d[i] = F::Compact(c >> 3);
        p0 += 2;
        p1 += 2;
        p2 += 2;
    }
}

template <typename F> void downsample_3_3(void* dst, const void* src, size_t srcRB, int count) {
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

        auto c = add_121(c00, c01, c02) + (add_121(c10, c11, c12) << 1) + add_121(c20, c21, c22);
        d[i] = F::Compact(c >> 4);
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

SkMipMap* SkMipMap::Build(const SkBitmap& src, SkDiscardableFactoryProc fact) {
    typedef void FilterProc(void*, const void* srcPtr, size_t srcRB, int count);

    FilterProc* proc_2_2 = nullptr;
    FilterProc* proc_2_3 = nullptr;
    FilterProc* proc_3_2 = nullptr;
    FilterProc* proc_3_3 = nullptr;

    const SkColorType ct = src.colorType();
    const SkAlphaType at = src.alphaType();
    switch (ct) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            proc_2_2 = downsample_2_2<ColorTypeFilter_8888>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_8888>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_8888>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_8888>;
            break;
        case kRGB_565_SkColorType:
            proc_2_2 = downsample_2_2<ColorTypeFilter_565>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_565>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_565>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_565>;
            break;
        case kARGB_4444_SkColorType:
            proc_2_2 = downsample_2_2<ColorTypeFilter_4444>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_4444>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_4444>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_4444>;
            break;
        case kAlpha_8_SkColorType:
        case kGray_8_SkColorType:
            proc_2_2 = downsample_2_2<ColorTypeFilter_8>;
            proc_2_3 = downsample_2_3<ColorTypeFilter_8>;
            proc_3_2 = downsample_3_2<ColorTypeFilter_8>;
            proc_3_3 = downsample_3_3<ColorTypeFilter_8>;
            break;
        default:
            // TODO: We could build miplevels for kIndex8 if the levels were in 8888.
            //       Means using more ram, but the quality would be fine.
            return nullptr;
    }

    // whip through our loop to compute the exact size needed
    size_t  size = 0;
    int     countLevels = 0;
    {
        int width = src.width();
        int height = src.height();
        for (;;) {
            width >>= 1;
            height >>= 1;
            if (0 == width || 0 == height) {
                break;
            }
            size += SkColorTypeMinRowBytes(ct, width) * height;
            countLevels += 1;
        }
    }
    if (0 == countLevels) {
        return nullptr;
    }

    size_t storageSize = SkMipMap::AllocLevelsSize(countLevels, size);
    if (0 == storageSize) {
        return nullptr;
    }

    SkAutoPixmapUnlock srcUnlocker;
    if (!src.requestLock(&srcUnlocker)) {
        return nullptr;
    }
    const SkPixmap& srcPixmap = srcUnlocker.pixmap();
    // Try to catch where we might have returned nullptr for src crbug.com/492818
    if (nullptr == srcPixmap.addr()) {
        sk_throw();
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
    SkPixmap    srcPM(srcPixmap);

    for (int i = 0; i < countLevels; ++i) {
        FilterProc* proc;
        if (height & 1) {        // src-height is 3
            if (width & 1) {    // src-width is 3
                proc = proc_3_3;
            } else {            // src-width is 2
                proc = proc_2_3;
            }
        } else {                // src-height is 2
            if (width & 1) {    // src-width is 3
                proc = proc_3_2;
            } else {            // src-width is 2
                proc = proc_2_2;
            }
        }
        width >>= 1;
        height >>= 1;
        rowBytes = SkToU32(SkColorTypeMinRowBytes(ct, width));

        levels[i].fPixels   = addr;
        levels[i].fWidth    = width;
        levels[i].fHeight   = height;
        levels[i].fRowBytes = rowBytes;
        levels[i].fScale    = (float)width / src.width();

        SkPixmap dstPM(SkImageInfo::Make(width, height, ct, at), addr, rowBytes);

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
#endif

///////////////////////////////////////////////////////////////////////////////

bool SkMipMap::extractLevel(SkScalar scale, Level* levelPtr) const {
    if (nullptr == fLevels) {
        return false;
    }

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
