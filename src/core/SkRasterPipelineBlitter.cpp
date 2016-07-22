/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitter.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkShader.h"
#include "SkSRGB.h"
#include "SkXfermode.h"


class SkRasterPipelineBlitter : public SkBlitter {
public:
    static SkBlitter* Create(const SkPixmap&, const SkPaint&, SkTBlitterAllocator*);

    SkRasterPipelineBlitter(SkPixmap dst,
                            SkRasterPipeline shader,
                            SkRasterPipeline colorFilter,
                            SkRasterPipeline xfermode,
                            SkPM4f paintColor)
        : fDst(dst)
        , fShader(shader)
        , fColorFilter(colorFilter)
        , fXfermode(xfermode)
        , fPaintColor(paintColor)
    {}

    void blitH    (int x, int y, int w)                            override;
    void blitAntiH(int x, int y, const SkAlpha[], const int16_t[]) override;
    void blitMask (const SkMask&, const SkIRect& clip)             override;

    // TODO: The default implementations of the other blits look fine,
    // but some of them like blitV could probably benefit from custom
    // blits using something like a SkRasterPipeline::runFew() method.

private:
    SkPixmap         fDst;
    SkRasterPipeline fShader, fColorFilter, fXfermode;
    SkPM4f           fPaintColor;

    typedef SkBlitter INHERITED;
};

SkBlitter* SkCreateRasterPipelineBlitter(const SkPixmap& dst,
                                         const SkPaint& paint,
                                         SkTBlitterAllocator* alloc) {
    return SkRasterPipelineBlitter::Create(dst, paint, alloc);
}


// The default shader produces a constant color (from the SkPaint).
static void SK_VECTORCALL constant_color(SkRasterPipeline::Stage* st, size_t x,
                                         Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                         Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto color = st->ctx<const SkPM4f*>();
    r = color->r();
    g = color->g();
    b = color->b();
    a = color->a();
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// The default transfer mode is srcover, s' = s + d*(1-sa).
static void SK_VECTORCALL srcover(SkRasterPipeline::Stage* st, size_t x,
                                  Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                  Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto A = 1.0f - a;
    r += dr*A;
    g += dg*A;
    b += db*A;
    a += da*A;
    st->next(x, r,g,b,a, dr,dg,db,da);
}

static Sk4f lerp(const Sk4f& from, const Sk4f& to, const Sk4f& cov) {
    return from + (to-from)*cov;
}

// s' = d(1-c) + sc, for a constant c.
static void SK_VECTORCALL lerp_constant_float(SkRasterPipeline::Stage* st, size_t x,
                                              Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                              Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    Sk4f c = *st->ctx<const float*>();

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// s' = d(1-c) + sc, 4 pixels at a time for 8-bit coverage.
static void SK_VECTORCALL lerp_a8(SkRasterPipeline::Stage* st, size_t x,
                                  Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                  Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint8_t*>() + x;
    Sk4f c = SkNx_cast<float>(Sk4b::Load(ptr)) * (1/255.0f);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Tail variant of lerp_a8() handling 1 pixel at a time.
static void SK_VECTORCALL lerp_a8_1(SkRasterPipeline::Stage* st, size_t x,
                                    Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                    Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint8_t*>() + x;
    Sk4f c = *ptr * (1/255.0f);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
    st->next(x, r,g,b,a, dr,dg,db,da);
}

static void upscale_lcd16(const Sk4h& lcd16, Sk4f* r, Sk4f* g, Sk4f* b) {
    Sk4i _32_bit = SkNx_cast<int>(lcd16);

    *r = SkNx_cast<float>(_32_bit & SK_R16_MASK_IN_PLACE) * (1.0f / SK_R16_MASK_IN_PLACE);
    *g = SkNx_cast<float>(_32_bit & SK_G16_MASK_IN_PLACE) * (1.0f / SK_G16_MASK_IN_PLACE);
    *b = SkNx_cast<float>(_32_bit & SK_B16_MASK_IN_PLACE) * (1.0f / SK_B16_MASK_IN_PLACE);
}

// s' = d(1-c) + sc, 4 pixels at a time for 565 coverage.
static void SK_VECTORCALL lerp_lcd16(SkRasterPipeline::Stage* st, size_t x,
                                     Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                     Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint16_t*>() + x;
    Sk4f cr, cg, cb;
    upscale_lcd16(Sk4h::Load(ptr), &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0f;
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Tail variant of lerp_lcd16() handling 1 pixel at a time.
static void SK_VECTORCALL lerp_lcd16_1(SkRasterPipeline::Stage* st, size_t x,
                                       Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                       Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint16_t*>() + x;
    Sk4f cr, cg, cb;
    upscale_lcd16({*ptr,0,0,0}, &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0f;
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Load 4 8-bit sRGB pixels from SkPMColor order to RGBA.
static void SK_VECTORCALL load_d_srgb(SkRasterPipeline::Stage* st, size_t x,
                                      Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                      Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint32_t*>() + x;

    dr = { sk_linear_from_srgb[(ptr[0] >> SK_R32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[1] >> SK_R32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[2] >> SK_R32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[3] >> SK_R32_SHIFT) & 0xff] };

    dg = { sk_linear_from_srgb[(ptr[0] >> SK_G32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[1] >> SK_G32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[2] >> SK_G32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[3] >> SK_G32_SHIFT) & 0xff] };

    db = { sk_linear_from_srgb[(ptr[0] >> SK_B32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[1] >> SK_B32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[2] >> SK_B32_SHIFT) & 0xff],
           sk_linear_from_srgb[(ptr[3] >> SK_B32_SHIFT) & 0xff] };

    // TODO: this >> doesn't really need mask if we make it logical instead of arithmetic.
    da = SkNx_cast<float>((Sk4i::Load(ptr) >> SK_A32_SHIFT) & 0xff) * (1/255.0f);

    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Tail variant of load_d_srgb() handling 1 pixel at a time.
static void SK_VECTORCALL load_d_srgb_1(SkRasterPipeline::Stage* st, size_t x,
                                        Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                        Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint32_t*>() + x;

    dr = { sk_linear_from_srgb[(*ptr >> SK_R32_SHIFT) & 0xff], 0,0,0 };
    dg = { sk_linear_from_srgb[(*ptr >> SK_G32_SHIFT) & 0xff], 0,0,0 };
    db = { sk_linear_from_srgb[(*ptr >> SK_B32_SHIFT) & 0xff], 0,0,0 };
    da = {        (1/255.0f) * (*ptr >> SK_A32_SHIFT)        , 0,0,0 };

    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Write out 4 pixels as 8-bit SkPMColor-order sRGB.
static void SK_VECTORCALL store_srgb(SkRasterPipeline::Stage* st, size_t x,
                                     Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                     Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto dst = st->ctx<uint32_t*>() + x;
    ( sk_linear_to_srgb(r) << SK_R32_SHIFT
    | sk_linear_to_srgb(g) << SK_G32_SHIFT
    | sk_linear_to_srgb(b) << SK_B32_SHIFT
    | Sk4f_round(255.0f*a) << SK_A32_SHIFT).store(dst);
}

// Tail variant of store_srgb() handling 1 pixel at a time.
static void SK_VECTORCALL store_srgb_1(SkRasterPipeline::Stage* st, size_t x,
                                       Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                       Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto dst = st->ctx<uint32_t*>() + x;
    *dst = Sk4f_toS32(swizzle_rb_if_bgra({ r[0], g[0], b[0], a[0] }));
}


template <typename Effect>
static bool append_effect_stages(const Effect* effect, SkRasterPipeline* pipeline) {
    return !effect || effect->appendStages(pipeline);
}


SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           SkTBlitterAllocator* alloc) {
    if (!dst.info().gammaCloseToSRGB()) {
        return nullptr;  // TODO: f16, etc.
    }
    if (paint.getShader()) {
        return nullptr;  // TODO: need to work out how shaders and their contexts work
    }

    SkRasterPipeline shader, colorFilter, xfermode;
    if (!append_effect_stages(paint.getColorFilter(), &colorFilter) ||
        !append_effect_stages(paint.getXfermode(),    &xfermode   )) {
        return nullptr;
    }

    auto blitter = alloc->createT<SkRasterPipelineBlitter>(
            dst,
            shader, colorFilter, xfermode,
            SkColor4f::FromColor(paint.getColor()).premul());

    if (!paint.getShader()) {
        blitter->fShader.append(constant_color, &blitter->fPaintColor);
    }
    if (!paint.getXfermode()) {
        blitter->fXfermode.append(srcover);
    }

    return blitter;
}

void SkRasterPipelineBlitter::blitH(int x, int y, int w) {
    auto dst = fDst.writable_addr(0,y);

    SkRasterPipeline p;
    p.extend(fShader);
    p.extend(fColorFilter);
    p.append(load_d_srgb, load_d_srgb_1, dst);
    p.extend(fXfermode);
    p.append(store_srgb, store_srgb_1, dst);

    p.run(x, w);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    auto dst = fDst.writable_addr(0,y);
    float coverage;

    SkRasterPipeline p;
    p.extend(fShader);
    p.extend(fColorFilter);
    p.append(load_d_srgb, load_d_srgb_1, dst);
    p.extend(fXfermode);
    p.append(lerp_constant_float, &coverage);
    p.append(store_srgb, store_srgb_1, dst);

    for (int16_t run = *runs; run > 0; run = *runs) {
        coverage = *aa * (1/255.0f);
        p.run(x, run);

        x    += run;
        runs += run;
        aa   += run;
    }
}

void SkRasterPipelineBlitter::blitMask(const SkMask& mask, const SkIRect& clip) {
    if (mask.fFormat == SkMask::kBW_Format) {
        // TODO: native BW masks?
        return INHERITED::blitMask(mask, clip);
    }

    int x = clip.left();
    for (int y = clip.top(); y < clip.bottom(); y++) {
        auto dst = fDst.writable_addr(0,y);

        SkRasterPipeline p;
        p.extend(fShader);
        p.extend(fColorFilter);
        p.append(load_d_srgb, load_d_srgb_1, dst);
        p.extend(fXfermode);
        switch (mask.fFormat) {
            case SkMask::kA8_Format:
                p.append(lerp_a8, lerp_a8_1, mask.getAddr8(x,y)-x);
                break;
            case SkMask::kLCD16_Format:
                p.append(lerp_lcd16, lerp_lcd16_1, mask.getAddrLCD16(x,y)-x);
                break;
            default: break;
        }
        p.append(store_srgb, store_srgb_1, dst);

        p.run(x, clip.width());
    }
}
