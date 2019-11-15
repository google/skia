/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkSpriteBlitter.h"
#include "src/core/SkXfermodePriv.h"

///////////////////////////////////////////////////////////////////////////////

static void S32_src(uint16_t dst[], const SkPMColor src[], int count) {
    for (int i = 0; i < count; ++i) {
        dst[i] = SkPixel32ToPixel16(src[i]);
    }
}

static void S32_srcover(uint16_t dst[], const SkPMColor src[], int count) {
    for (int i = 0; i < count; ++i) {
        dst[i] = SkSrcOver32To16(src[i], dst[i]);
    }
}

class Sprite_D16_S32 : public SkSpriteBlitter {
public:
    Sprite_D16_S32(const SkPixmap& src, SkBlendMode mode)  : INHERITED(src) {
        SkASSERT(src.colorType() == kN32_SkColorType);
        SkASSERT(mode == SkBlendMode::kSrc || mode == SkBlendMode::kSrcOver);

        fUseSrcOver = (mode == SkBlendMode::kSrcOver) && !src.isOpaque();
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint16_t* SK_RESTRICT dst = fDst.writable_addr16(x, y);
        const uint32_t* SK_RESTRICT src = fSource.addr32(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();

        do {
            if (fUseSrcOver) {
                S32_srcover(dst, src, width);
            } else {
                S32_src(dst, src, width);
            }

            dst = (uint16_t* SK_RESTRICT)((char*)dst + dstRB);
            src = (const uint32_t* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    bool fUseSrcOver;

    typedef SkSpriteBlitter INHERITED;
};

SkSpriteBlitter* SkSpriteBlitter::ChooseL565(const SkPixmap& source, const SkPaint& paint,
                                             SkArenaAlloc* allocator) {
    SkASSERT(allocator != nullptr);

    if (paint.getColorFilter() != nullptr) {
        return nullptr;
    }
    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }

    U8CPU alpha = paint.getAlpha();
    if (alpha != 0xFF) {
        return nullptr;
    }

    if (source.colorType() == kN32_SkColorType) {
        switch (paint.getBlendMode()) {
            case SkBlendMode::kSrc:
            case SkBlendMode::kSrcOver:
                return allocator->make<Sprite_D16_S32>(source, paint.getBlendMode());
            default:
                break;
        }
    }
    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static unsigned div255(unsigned a, unsigned b) {
    return (a * b * 257 + 127) >> 16;
}

static void S32_src_da8(uint8_t dst[], const SkPMColor src[], int count) {
    for (int i = 0; i < count; ++i) {
        dst[i] = SkGetPackedA32(src[i]);
    }
}

static void S32_srcover_da8(uint8_t dst[], const SkPMColor src[], int count) {
    for (int i = 0; i < count; ++i) {
        SkPMColor c = src[i];
        if (c) {
            unsigned a = SkGetPackedA32(c);
            if (a == 0xFF) {
                dst[i] = 0xFF;
            } else {
                dst[i] = a + div255(255 - a, dst[i]);
            }
        }
    }
}

class Sprite_D8_S32 : public SkSpriteBlitter {
public:
    Sprite_D8_S32(const SkPixmap& src, SkBlendMode mode)  : INHERITED(src) {
        SkASSERT(src.colorType() == kN32_SkColorType);
        SkASSERT(mode == SkBlendMode::kSrc || mode == SkBlendMode::kSrcOver);

        fUseSrcOver = (mode == SkBlendMode::kSrcOver) && !src.isOpaque();
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint8_t* SK_RESTRICT dst = fDst.writable_addr8(x, y);
        const uint32_t* SK_RESTRICT src = fSource.addr32(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();

        do {
            if (fUseSrcOver) {
                S32_srcover_da8(dst, src, width);
            } else {
                S32_src_da8(dst, src, width);
            }

            dst = (uint8_t* SK_RESTRICT)((char*)dst + dstRB);
            src = (const uint32_t* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    bool fUseSrcOver;

    typedef SkSpriteBlitter INHERITED;
};

SkSpriteBlitter* SkSpriteBlitter::ChooseLA8(const SkPixmap& source, const SkPaint& paint,
                                            SkArenaAlloc* allocator) {
    SkASSERT(allocator != nullptr);

    if (paint.getColorFilter() != nullptr) {
        return nullptr;
    }
    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }

    U8CPU alpha = paint.getAlpha();
    if (alpha != 0xFF) {
        return nullptr;
    }

    if (source.colorType() == kN32_SkColorType) {
        switch (paint.getBlendMode()) {
            case SkBlendMode::kSrc:
            case SkBlendMode::kSrcOver:
                return allocator->make<Sprite_D8_S32>(source, paint.getBlendMode());
            default:
                break;
        }
    }
    return nullptr;
}
