/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMipMap.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"

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
