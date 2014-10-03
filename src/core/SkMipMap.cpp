/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMipMap.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"

static void downsampleby2_proc32(SkBitmap* dst, int x, int y,
                                 const SkBitmap& src) {
    x <<= 1;
    y <<= 1;
    const SkPMColor* p = src.getAddr32(x, y);
    const SkPMColor* baseP = p;
    SkPMColor c, ag, rb;

    c = *p; ag = (c >> 8) & 0xFF00FF; rb = c & 0xFF00FF;
    if (x < src.width() - 1) {
        p += 1;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    p = baseP;
    if (y < src.height() - 1) {
        p += src.rowBytes() >> 2;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;
    if (x < src.width() - 1) {
        p += 1;
    }
    c = *p; ag += (c >> 8) & 0xFF00FF; rb += c & 0xFF00FF;

    *dst->getAddr32(x >> 1, y >> 1) =
    ((rb >> 2) & 0xFF00FF) | ((ag << 6) & 0xFF00FF00);
}

static inline uint32_t expand16(U16CPU c) {
    return (c & ~SK_G16_MASK_IN_PLACE) | ((c & SK_G16_MASK_IN_PLACE) << 16);
}

// returns dirt in the top 16bits, but we don't care, since we only
// store the low 16bits.
static inline U16CPU pack16(uint32_t c) {
    return (c & ~SK_G16_MASK_IN_PLACE) | ((c >> 16) & SK_G16_MASK_IN_PLACE);
}

static void downsampleby2_proc16(SkBitmap* dst, int x, int y,
                                 const SkBitmap& src) {
    x <<= 1;
    y <<= 1;
    const uint16_t* p = src.getAddr16(x, y);
    const uint16_t* baseP = p;
    SkPMColor       c;

    c = expand16(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand16(*p);

    p = baseP;
    if (y < src.height() - 1) {
        p += src.rowBytes() >> 1;
    }
    c += expand16(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand16(*p);

    *dst->getAddr16(x >> 1, y >> 1) = (uint16_t)pack16(c >> 2);
}

static uint32_t expand4444(U16CPU c) {
    return (c & 0xF0F) | ((c & ~0xF0F) << 12);
}

static U16CPU collaps4444(uint32_t c) {
    return (c & 0xF0F) | ((c >> 12) & ~0xF0F);
}

static void downsampleby2_proc4444(SkBitmap* dst, int x, int y,
                                   const SkBitmap& src) {
    x <<= 1;
    y <<= 1;
    const uint16_t* p = src.getAddr16(x, y);
    const uint16_t* baseP = p;
    uint32_t        c;

    c = expand4444(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);

    p = baseP;
    if (y < src.height() - 1) {
        p += src.rowBytes() >> 1;
    }
    c += expand4444(*p);
    if (x < src.width() - 1) {
        p += 1;
    }
    c += expand4444(*p);

    *dst->getAddr16(x >> 1, y >> 1) = (uint16_t)collaps4444(c >> 2);
}

SkMipMap::Level* SkMipMap::AllocLevels(int levelCount, size_t pixelSize) {
    if (levelCount < 0) {
        return NULL;
    }
    int64_t size = sk_64_mul(levelCount + 1, sizeof(Level)) + pixelSize;
    if (!sk_64_isS32(size)) {
        return NULL;
    }
    return (Level*)sk_malloc_throw(sk_64_asS32(size));
}

SkMipMap* SkMipMap::Build(const SkBitmap& src) {
    void (*proc)(SkBitmap* dst, int x, int y, const SkBitmap& src);

    const SkColorType ct = src.colorType();
    const SkAlphaType at = src.alphaType();
    switch (ct) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            proc = downsampleby2_proc32;
            break;
        case kRGB_565_SkColorType:
            proc = downsampleby2_proc16;
            break;
        case kARGB_4444_SkColorType:
            proc = downsampleby2_proc4444;
            break;
        default:
            return NULL; // don't build mipmaps for any other colortypes (yet)
    }

    SkAutoLockPixels alp(src);
    if (!src.readyToDraw()) {
        return NULL;
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
        return NULL;
    }

    Level* levels = SkMipMap::AllocLevels(countLevels, size);
    if (NULL == levels) {
        return NULL;
    }

    uint8_t*    baseAddr = (uint8_t*)&levels[countLevels];
    uint8_t*    addr = baseAddr;
    int         width = src.width();
    int         height = src.height();
    uint32_t    rowBytes;
    SkBitmap    srcBM(src);

    for (int i = 0; i < countLevels; ++i) {
        width >>= 1;
        height >>= 1;
        rowBytes = SkToU32(SkColorTypeMinRowBytes(ct, width));

        levels[i].fPixels   = addr;
        levels[i].fWidth    = width;
        levels[i].fHeight   = height;
        levels[i].fRowBytes = rowBytes;
        levels[i].fScale    = (float)width / src.width();

        SkBitmap dstBM;
        dstBM.installPixels(SkImageInfo::Make(width, height, ct, at), addr, rowBytes);

        srcBM.lockPixels();
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                proc(&dstBM, x, y, srcBM);
            }
        }
        srcBM.unlockPixels();

        srcBM = dstBM;
        addr += height * rowBytes;
    }
    SkASSERT(addr == baseAddr + size);

    return SkNEW_ARGS(SkMipMap, (levels, countLevels, size));
}

///////////////////////////////////////////////////////////////////////////////

//static int gCounter;

SkMipMap::SkMipMap(Level* levels, int count, size_t size)
    : fSize(size), fLevels(levels), fCount(count) {
    SkASSERT(levels);
    SkASSERT(count > 0);
//    SkDebugf("mips %d\n", ++gCounter);
}

SkMipMap::~SkMipMap() {
    sk_free(fLevels);
//    SkDebugf("mips %d\n", --gCounter);
}

static SkFixed compute_level(SkScalar scale) {
    SkFixed s = SkAbs32(SkScalarToFixed(SkScalarInvert(scale)));

    if (s < SK_Fixed1) {
        return 0;
    }
    int clz = SkCLZ(s);
    SkASSERT(clz >= 1 && clz <= 15);
    return SkIntToFixed(15 - clz) + ((unsigned)(s << (clz + 1)) >> 16);
}

bool SkMipMap::extractLevel(SkScalar scale, Level* levelPtr) const {
    if (scale >= SK_Scalar1) {
        return false;
    }

    int level = compute_level(scale) >> 16;
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
