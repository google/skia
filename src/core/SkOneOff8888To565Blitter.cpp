/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArenaAlloc.h"
#include "SkBlitter.h"

class SkOneOff8888To565Blitter final : public SkBlitter {
public:
    static SkBlitter* Create(const SkPixmap& dst,
                             const SkPaint& paint,
                             const SkMatrix& ctm,
                             SkArenaAlloc* alloc) {
        if (dst.colorType() != kRGB_565_SkColorType) {
            return nullptr;
        }
        if (!paint.getShader()) {
            return nullptr;
        }
        if (paint.getColorFilter()) {
            return nullptr;
        }


        SkMatrix localM;
        SkShader::TileMode tiling_xy[2];
        SkImage* image = paint.getShader()->isAImage(&localM, tiling_xy);

        if (!image || tiling_xy[0] != SkShader::kClamp_TileMode
                   || tiling_xy[1] != SkShader::kClamp_TileMode) {
            return nullptr;
        }

        SkPixmap src;
        if (!image->peekPixels(&src) || src.colorType() != kN32_SkColorType) {
            return nullptr;
        }

        if (!ctm.isScaleTranslate() || !localM.isIdentity()) {
            return nullptr;
        }

        auto mode = paint.getBlendMode();
        if (image->isOpaque() && mode == SkBlendMode::kSrcOver) {
            mode = SkBlendMode::kSrc;
        }
        if (mode != SkBlendMode::kSrc) {
            return nullptr;
        }

        if (paint.getFilterQuality() != kNone_SkFilterQuality) {
            return nullptr;
        }

        return alloc->make<SkOneOff8888To565Blitter>();
    }

    void blitAntiH(int x, int y, const SkAlpha[], const int16_t[]) override {
        SkASSERT(false);
    }

    void blitH(int x, int y, int w) override {
        // TODO
    }

private:
};

SkBlitter* SkCreateOneOff8888To565Blitter(const SkPixmap& dst,
                                          const SkPaint& paint,
                                          const SkMatrix& ctm,
                                          SkArenaAlloc* alloc) {
    return SkOneOff8888To565Blitter::Create(dst, paint, ctm, alloc);
}
