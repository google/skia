/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImageInfo.h"
#include "SkXfermode.h"
#include "SkHalf.h"
#include "SkImage.h"
#include "SkPM4f.h"

static SkPMColor f16_to_pmcolor(uint64_t src) {
    SkPMColor dst;
    const SkHalf* sptr = reinterpret_cast<const SkHalf*>(&src);
    uint8_t* dptr = reinterpret_cast<uint8_t*>(&dst);
    for (int i = 0; i < 4; ++i) {
        float f = SkHalfToFloat(sptr[i]);
        dptr[i] = SkToU8((int)(f * 255 + 0.5f));
    }
    return dst;
}

static SkPMColor u16_to_pmcolor(uint64_t src) {
    SkPMColor dst;
    const uint16_t* sptr = reinterpret_cast<const uint16_t*>(&src);
    uint8_t* dptr = reinterpret_cast<uint8_t*>(&dst);
    for (int i = 0; i < 4; ++i) {
        dptr[i] = sptr[i] >> 8;
    }
    return dst;
}

static SkImage* new_u64_image(const SkBitmap& src, uint32_t flags) {
    SkBitmap dst;
    dst.allocN32Pixels(src.width(), src.height());
    SkPixmap srcPM, dstPM;
    src.peekPixels(&srcPM);
    dst.peekPixels(&dstPM);

    for (int y = 0; y < srcPM.height(); ++y) {
        for (int x = 0; x < srcPM.width(); ++x) {
            uint64_t srcP = *srcPM.addr64(x, y);
            uint32_t* dstP = dstPM.writable_addr32(x, y);

            if (flags & SkXfermode::kDstIsFloat16_U64Flag) {
                *dstP = f16_to_pmcolor(srcP);
            } else {
                *dstP = u16_to_pmcolor(srcP);
            }
        }
    }
    return SkImage::NewRasterCopy(dstPM.info(), dstPM.addr(), dstPM.rowBytes());
}

static void draw_rect(SkCanvas* canvas, const SkRect& r, SkColor c, uint32_t u64_flags,
                      const SkAlpha aa[]) {
    const SkIRect ir = r.round();
    const SkImageInfo info = SkImageInfo::Make(ir.width(), ir.height(),
                                               kRGBA_F16_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(info);
    SkPixmap pm;
    bm.peekPixels(&pm);
    memset(pm.writable_addr(), 0, pm.getSafeSize());

    if (SkColorGetA(c) == 0xFF) {
        u64_flags |= SkXfermode::kSrcIsOpaque_PM4fFlag;
    }

    const SkXfermode::U64State state { nullptr, u64_flags };

    const SkPM4f src = SkColor4f::FromColor(c).premul();
    auto proc1 = SkXfermode::GetU64Proc1(SkXfermode::kSrcOver_Mode, u64_flags);
    for (int y = 0; y < ir.height()/2; ++y) {
        proc1(state, pm.writable_addr64(0, y), src, ir.width(), aa);
    }
    
    SkPM4f buffer[1000];
    for (int i = 0; i < ir.width(); ++i) {
        buffer[i] = src;
    }
    auto procN = SkXfermode::GetU64ProcN(SkXfermode::kSrcOver_Mode, u64_flags);
    for (int y = ir.height()/2 + 1; y < ir.height(); ++y) {
        procN(state, pm.writable_addr64(0, y), buffer, ir.width(), aa);
    }

    SkAutoTUnref<SkImage> image(new_u64_image(bm, u64_flags));
    canvas->drawImage(image, r.left(), r.top(), nullptr);
}

/*
 *  Test SkXfer4fProcs directly for src-over, comparing them to current SkColor blits.
 */
DEF_SIMPLE_GM(xfer_u64_srcover, canvas, 580, 760) {
    const int IW = 50;
    const SkScalar W = IW;
    const SkScalar H = 100;

    const int32_t flags[] = {
        -1,                                 // normal
        0,                                  // U16 components
        SkXfermode::kDstIsFloat16_U64Flag,  // F16 components
    };
    const SkColor colors[] = {
        SK_ColorBLACK, SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
        0x88000000, 0x88FF0000, 0x8800FF00, 0x880000FF
    };
    
    uint8_t aa_scanline[IW];
    for (int i = 0; i < IW; ++i) {
        aa_scanline[i] = i * 255 / (IW - 1);
    }
    uint8_t const* aa_table[] = { nullptr, aa_scanline };

    SkBitmap mask;
    mask.installPixels(SkImageInfo::MakeA8(IW, 1), aa_scanline, IW);

    canvas->translate(20, 20);

    const SkRect r = SkRect::MakeWH(W, H);
    for (const uint8_t* aa : aa_table) {
        canvas->save();
        for (auto flag : flags) {
            canvas->save();
            for (SkColor c : colors) {
                if (flag < 0) {
                    SkPaint p;
                    p.setColor(c);
                    if (aa) {
                        canvas->drawBitmapRect(mask, r, &p);
                    } else {
                        canvas->drawRect(r, p);
                    }
                } else {
                    draw_rect(canvas, r, c, flag, aa);
                }
                canvas->translate(W + 20, 0);
            }
            canvas->restore();
            canvas->translate(0, H + 20);
        }
        canvas->restore();
        canvas->translate(0, (H + 20) * SK_ARRAY_COUNT(flags) + 20);
    }
}
