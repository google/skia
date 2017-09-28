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
        if (paint.getAlpha() != 0xff) {
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
        if (!image->peekPixels(&src) || src.colorType() != kN32_SkColorType
                                     || src.alphaType() != kPremul_SkAlphaType
                                     || src.colorSpace() != nullptr) {
            return nullptr;
        }

        SkMatrix dstToSrc;
        if (!ctm.isScaleTranslate() || !localM.isIdentity() || !ctm.invert(&dstToSrc)) {
            return nullptr;
        }

        auto mode = paint.getBlendMode();
        if (image->isOpaque() && mode == SkBlendMode::kSrcOver) {
            mode = SkBlendMode::kSrc;
        }
        if (mode != SkBlendMode::kSrc && mode != SkBlendMode::kSrcOver) {
            return nullptr;
        }

        return alloc->make<SkOneOff8888To565Blitter>(mode, src, dst, dstToSrc);
    }

    SkOneOff8888To565Blitter(SkBlendMode mode, SkPixmap src, SkPixmap dst, SkMatrix dstToSrc)
        : fMode(mode)
        , fSrc(src)
        , fDst(dst)
        , fDstToSrc(dstToSrc) {}

    void blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) override {
        fMode == SkBlendMode::kSrc ? this->blitAntiH_Src    (x,y,aa,runs)
                                   : this->blitAntiH_SrcOver(x,y,aa,runs);
    }

    void blitH(int x, int y, int w) override {
        fMode == SkBlendMode::kSrc ? this->blitH_Src    (x,y,w)
                                   : this->blitH_SrcOver(x,y,w);
    }

private:
    void blitAntiH_Src    (int x, int y, const SkAlpha[], const int16_t[]) const;
    void blitAntiH_SrcOver(int x, int y, const SkAlpha[], const int16_t[]) const;
    void blitH_Src        (int x, int y, int w) const;
    void blitH_SrcOver    (int x, int y, int w) const;

    const SkPMColor* srcAt(int x, int y, SkFixed* deltaX) const {
        *deltaX = SkFloatToFixed(fDstToSrc.getScaleX());

        SkPoint xy = fDstToSrc.mapXY(x + 0.5f, y + 0.5f);
        return fSrc.addr32(SkTPin(xy.fX, (SkScalar)0, (SkScalar)fSrc.width()  - 1),
                           SkTPin(xy.fY, (SkScalar)0, (SkScalar)fSrc.height() - 1));
    }

    int clampX(SkFixed off) const {
        return SkTPin(SkFixedFloorToInt(off), 0, fSrc.width() - 1);
    }

    SkBlendMode fMode;
    SkPixmap    fSrc,
                fDst;
    SkMatrix    fDstToSrc;
};

SkBlitter* SkCreateOneOff8888To565Blitter(const SkPixmap& dst,
                                          const SkPaint& paint,
                                          const SkMatrix& ctm,
                                          SkArenaAlloc* alloc) {
    return SkOneOff8888To565Blitter::Create(dst, paint, ctm, alloc);
}

void SkOneOff8888To565Blitter::blitAntiH_Src(int x, int y,
                                             const SkAlpha aa[], const int16_t runs[]) const {
    SkFixed dx, off = 0;
    const SkPMColor* src = this->srcAt(x,y, &dx);
    uint16_t* dst = fDst.writable_addr16(x,y);

    for (int16_t run = *runs; run > 0; run = *runs) {
        for (int i = 0; i < run; i++) {
            SkPMColor s = src[this->clampX(off)];
            dst[i] = SkPixel32ToPixel16(SkFourByteInterp(s, SkPixel16ToPixel32(dst[i]), aa[i]));
            off += dx;
        }

        dst  += run;
        aa   += run;
        runs += run;
    }
}

void SkOneOff8888To565Blitter::blitAntiH_SrcOver(int x, int y,
                                                 const SkAlpha aa[], const int16_t runs[]) const {
    SkFixed dx, off = 0;
    const SkPMColor* src = this->srcAt(x,y, &dx);
    uint16_t* dst = fDst.writable_addr16(x,y);

    for (int16_t run = *runs; run > 0; run = *runs) {
        for (int i = 0; i < run; i++) {
            SkPMColor s = src[this->clampX(off)];
            dst[i] = SkSrcOver32To16(SkAlphaMulQ(s, SkAlpha255To256(aa[i])), dst[i]);
            off += dx;
        }

        dst  += run;
        aa   += run;
        runs += run;
    }
}

void SkOneOff8888To565Blitter::blitH_Src(int x, int y, int w) const {
    SkFixed dx, off = 0;
    const SkPMColor* src = this->srcAt(x,y, &dx);
    uint16_t* dst = fDst.writable_addr16(x,y);

    for (int i = 0; i < w; i++) {
        SkPMColor s = src[this->clampX(off)];
        dst[i] = SkPixel32ToPixel16(s);
        off += dx;
    }
}

void SkOneOff8888To565Blitter::blitH_SrcOver(int x, int y, int w) const {
    SkFixed dx, off = 0;
    const SkPMColor* src = this->srcAt(x,y, &dx);
    uint16_t* dst = fDst.writable_addr16(x,y);

    for (int i = 0; i < w; i++) {
        SkPMColor s = src[this->clampX(off)];
        dst[i] = SkSrcOver32To16(s, dst[i]);
        off += dx;
    }
}
