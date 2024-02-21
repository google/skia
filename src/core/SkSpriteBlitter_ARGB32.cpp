/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorType.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkCPUTypes.h"
#include "src/base/SkArenaAlloc.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkSpriteBlitter.h"

#include <cstddef>
#include <cstdint>

///////////////////////////////////////////////////////////////////////////////

class Sprite_D32_S32 : public SkSpriteBlitter {
public:
    Sprite_D32_S32(const SkPixmap& src, U8CPU alpha)  : INHERITED(src) {
        SkASSERT(src.colorType() == kN32_SkColorType);

        unsigned flags32 = 0;
        if (255 != alpha) {
            flags32 |= SkBlitRow::kGlobalAlpha_Flag32;
        }
        if (!src.isOpaque()) {
            flags32 |= SkBlitRow::kSrcPixelAlpha_Flag32;
        }

        fProc32 = SkBlitRow::Factory32(flags32);
        fAlpha = alpha;
    }

    void blitRect(int x, int y, int width, int height) override {
        SkASSERT(width > 0 && height > 0);
        uint32_t* SK_RESTRICT dst = fDst.writable_addr32(x, y);
        const uint32_t* SK_RESTRICT src = fSource.addr32(x - fLeft, y - fTop);
        size_t dstRB = fDst.rowBytes();
        size_t srcRB = fSource.rowBytes();
        SkBlitRow::Proc32 proc = fProc32;
        U8CPU             alpha = fAlpha;

        do {
            proc(dst, src, width, alpha);
            dst = (uint32_t* SK_RESTRICT)((char*)dst + dstRB);
            src = (const uint32_t* SK_RESTRICT)((const char*)src + srcRB);
        } while (--height != 0);
    }

private:
    SkBlitRow::Proc32   fProc32;
    U8CPU               fAlpha;

    using INHERITED = SkSpriteBlitter;
};

///////////////////////////////////////////////////////////////////////////////

SkSpriteBlitter* SkSpriteBlitter::ChooseL32(const SkPixmap& source, const SkPaint& paint,
                                            SkArenaAlloc* allocator) {
    SkASSERT(allocator != nullptr);

    if (paint.getColorFilter() != nullptr) {
        return nullptr;
    }
    if (paint.getMaskFilter() != nullptr) {
        return nullptr;
    }
    if (source.colorType() == kN32_SkColorType && paint.isSrcOver()) {
        // this can handle alpha, but not xfermode
        return allocator->make<Sprite_D32_S32>(source, paint.getAlpha());
    }
    return nullptr;
}
