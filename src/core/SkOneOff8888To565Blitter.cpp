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
        if (paint.getFilterQuality() != kNone_SkFilterQuality) {
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
        if (mode != SkBlendMode::kSrc && mode != SkBlendMode::kSrcOver) {
            return nullptr;
        }

        return alloc->make<SkOneOff8888To565Blitter>(mode, src, dst);
    }

    SkOneOff8888To565Blitter(SkBlendMode mode, SkPixmap src, SkPixmap dst)
        : fMode(mode)
        , fSrc(src)
        , fDst(dst) {}

    void blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) override {
        fMode == SkBlendMode::kSrc ? this->blitAntiH_Src    (x,y,aa,runs)
                                   : this->blitAntiH_SrcOver(x,y,aa,runs);
    }

    void blitH(int x, int y, int w) override {
        fMode == SkBlendMode::kSrc ? this->blitH_Src    (x,y,w)
                                   : this->blitH_SrcOver(x,y,w);
    }

private:
    void blitAntiH_Src    (int x, int y, const SkAlpha[], const int16_t[]);
    void blitAntiH_SrcOver(int x, int y, const SkAlpha[], const int16_t[]);
    void blitH_Src        (int x, int y, int w);
    void blitH_SrcOver    (int x, int y, int w);

    SkBlendMode fMode;
    SkPixmap    fSrc,
                fDst;
};

void SkOneOff8888To565Blitter::blitAntiH_Src(int x, int y,
                                             const SkAlpha aa[], const int16_t runs[]) {
    uint16_t* dst = fDst.writable_addr16(x,y);
    for (int16_t run = *runs; run > 0; run = *runs) {
        for (int i = 0; i < run; i++) {
            dst[i] = 31 << 0;
        }

        dst  += run;
        aa   += run;
        runs += run;
    }
}

void SkOneOff8888To565Blitter::blitAntiH_SrcOver(int x, int y,
                                                 const SkAlpha aa[], const int16_t runs[]) {
    uint16_t* dst = fDst.writable_addr16(x,y);
    for (int16_t run = *runs; run > 0; run = *runs) {
        for (int i = 0; i < run; i++) {
            dst[i] = 63 << 5;
        }

        dst  += run;
        aa   += run;
        runs += run;
    }
}

void SkOneOff8888To565Blitter::blitH_Src(int x, int y, int w) {
    uint16_t* dst = fDst.writable_addr16(x,y);

    for (int i = 0; i < w; i++) {
        dst[i] = 31 << 11;
    }
}

void SkOneOff8888To565Blitter::blitH_SrcOver(int x, int y, int w) {
    uint16_t* dst = fDst.writable_addr16(x,y);

    for (int i = 0; i < w; i++) {
        dst[i] = 0;
    }
}

SkBlitter* SkCreateOneOff8888To565Blitter(const SkPixmap& dst,
                                          const SkPaint& paint,
                                          const SkMatrix& ctm,
                                          SkArenaAlloc* alloc) {
    return SkOneOff8888To565Blitter::Create(dst, paint, ctm, alloc);
}
