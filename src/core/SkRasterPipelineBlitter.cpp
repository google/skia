/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitter.h"
#include "SkColor.h"
#include "SkColorFilter.h"
#include "SkHalf.h"
#include "SkPM4f.h"
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
    void append_load_d(SkRasterPipeline*, const void*) const;
    void append_store (SkRasterPipeline*,       void*) const;

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

// Clamp colors into [0,1] premul (e.g. just before storing back to memory).
static void SK_VECTORCALL clamp_01_premul(SkRasterPipeline::Stage* st, size_t x,
                                          Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                          Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    a = Sk4f::Max(a, 0.0f);
    r = Sk4f::Max(r, 0.0f);
    g = Sk4f::Max(g, 0.0f);
    b = Sk4f::Max(b, 0.0f);

    a = Sk4f::Min(a, 1.0f);
    r = Sk4f::Min(r, a);
    g = Sk4f::Min(g, a);
    b = Sk4f::Min(b, a);

    st->next(x, r,g,b,a, dr,dg,db,da);
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

static void from_565(const Sk4h& _565, Sk4f* r, Sk4f* g, Sk4f* b) {
    Sk4i _32_bit = SkNx_cast<int>(_565);

    *r = SkNx_cast<float>(_32_bit & SK_R16_MASK_IN_PLACE) * (1.0f / SK_R16_MASK_IN_PLACE);
    *g = SkNx_cast<float>(_32_bit & SK_G16_MASK_IN_PLACE) * (1.0f / SK_G16_MASK_IN_PLACE);
    *b = SkNx_cast<float>(_32_bit & SK_B16_MASK_IN_PLACE) * (1.0f / SK_B16_MASK_IN_PLACE);
}

static Sk4h to_565(const Sk4f& r, const Sk4f& g, const Sk4f& b) {
    return SkNx_cast<uint16_t>( Sk4f_round(r * SK_R16_MASK) << SK_R16_SHIFT
                              | Sk4f_round(g * SK_G16_MASK) << SK_G16_SHIFT
                              | Sk4f_round(b * SK_B16_MASK) << SK_B16_SHIFT);
}

// s' = d(1-c) + sc, 4 pixels at a time for 565 coverage.
static void SK_VECTORCALL lerp_lcd16(SkRasterPipeline::Stage* st, size_t x,
                                     Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                     Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint16_t*>() + x;
    Sk4f cr, cg, cb;
    from_565(Sk4h::Load(ptr), &cr, &cg, &cb);

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
    from_565({*ptr,0,0,0}, &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0f;
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Load 4 565 dst pixels.
static void SK_VECTORCALL load_d_565(SkRasterPipeline::Stage* st, size_t x,
                                     Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                     Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint16_t*>() + x;

    from_565(Sk4h::Load(ptr), &dr,&dg,&db);
    da = 1.0f;
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Load 1 565 dst pixel.
static void SK_VECTORCALL load_d_565_1(SkRasterPipeline::Stage* st, size_t x,
                                       Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                       Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint16_t*>() + x;

    from_565({*ptr,0,0,0}, &dr,&dg,&db);
    da = 1.0f;
    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Store 4 565 pixels.
static void SK_VECTORCALL store_565(SkRasterPipeline::Stage* st, size_t x,
                                    Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                    Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<uint16_t*>() + x;
    to_565(r,g,b).store(ptr);
}

// Store 1 565 pixel.
static void SK_VECTORCALL store_565_1(SkRasterPipeline::Stage* st, size_t x,
                                      Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                      Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<uint16_t*>() + x;
    *ptr = to_565(r,g,b)[0];
}

// Load 4 F16 pixels.
static void SK_VECTORCALL load_d_f16(SkRasterPipeline::Stage* st, size_t x,
                                     Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                     Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint64_t*>() + x;

    Sk4h rh, gh, bh, ah;
    Sk4h_load4(ptr, &rh, &gh, &bh, &ah);

    dr = SkHalfToFloat_finite(rh);
    dg = SkHalfToFloat_finite(gh);
    db = SkHalfToFloat_finite(bh);
    da = SkHalfToFloat_finite(ah);

    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Load 1 F16 pixel.
static void SK_VECTORCALL load_d_f16_1(SkRasterPipeline::Stage* st, size_t x,
                                       Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                       Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<const uint64_t*>() + x;

    auto p0 = SkHalfToFloat_finite(ptr[0]);
    dr = { p0[0],0,0,0 };
    dg = { p0[1],0,0,0 };
    db = { p0[2],0,0,0 };
    da = { p0[3],0,0,0 };

    st->next(x, r,g,b,a, dr,dg,db,da);
}

// Store 4 F16 pixels.
static void SK_VECTORCALL store_f16(SkRasterPipeline::Stage* st, size_t x,
                                    Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                    Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<uint64_t*>() + x;

    Sk4h_store4(ptr, SkFloatToHalf_finite(r), SkFloatToHalf_finite(g),
                     SkFloatToHalf_finite(b), SkFloatToHalf_finite(a));
}

// Store 1 F16 pixel.
static void SK_VECTORCALL store_f16_1(SkRasterPipeline::Stage* st, size_t x,
                                      Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                      Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto ptr = st->ctx<uint64_t*>() + x;

    SkFloatToHalf_finite({r[0], g[0], b[0], a[0]}).store(ptr);
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

    da = SkNx_cast<float>(Sk4u::Load(ptr) >> SK_A32_SHIFT) * (1/255.0f);

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
    ( sk_linear_to_srgb_noclamp(r) << SK_R32_SHIFT
    | sk_linear_to_srgb_noclamp(g) << SK_G32_SHIFT
    | sk_linear_to_srgb_noclamp(b) << SK_B32_SHIFT
    |       Sk4f_round(255.0f * a) << SK_A32_SHIFT).store(dst);
}

// Tail variant of store_srgb() handling 1 pixel at a time.
static void SK_VECTORCALL store_srgb_1(SkRasterPipeline::Stage* st, size_t x,
                                       Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                       Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    auto dst = st->ctx<uint32_t*>() + x;
    Sk4i rgb = sk_linear_to_srgb_noclamp(swizzle_rb_if_bgra({ r[0], g[0], b[0], 0.0f }));

    uint32_t rgba;
    SkNx_cast<uint8_t>(rgb).store(&rgba);
    rgba |= (uint32_t)(255.0f * a[0] + 0.5f) << 24;
    *dst = rgba;
}

static bool supported(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kN32_SkColorType:      return info.gammaCloseToSRGB();
        case kRGBA_F16_SkColorType: return true;
        case kRGB_565_SkColorType:  return true;
        default:                    return false;
    }
}

template <typename Effect>
static bool append_effect_stages(const Effect* effect, SkRasterPipeline* pipeline) {
    return !effect || effect->appendStages(pipeline);
}


SkBlitter* SkRasterPipelineBlitter::Create(const SkPixmap& dst,
                                           const SkPaint& paint,
                                           SkTBlitterAllocator* alloc) {
    if (!supported(dst.info())) {
        return nullptr;
    }
    if (paint.getShader()) {
        return nullptr;  // TODO: need to work out how shaders and their contexts work
    }

    SkRasterPipeline shader, colorFilter, xfermode;
    if (!append_effect_stages(paint.getColorFilter(), &colorFilter) ||
        !append_effect_stages(paint.getXfermode(),    &xfermode   )) {
        return nullptr;
    }

    uint32_t paintColor = paint.getColor();

    SkColor4f color;
    if (SkImageInfoIsGammaCorrect(dst.info())) {
        color = SkColor4f::FromColor(paintColor);
    } else {
        swizzle_rb(SkNx_cast<float>(Sk4b::Load(&paintColor)) * (1/255.0f)).store(&color);
    }

    auto blitter = alloc->createT<SkRasterPipelineBlitter>(
            dst,
            shader, colorFilter, xfermode,
            color.premul());

    if (!paint.getShader()) {
        blitter->fShader.append(constant_color, &blitter->fPaintColor);
    }
    if (!paint.getXfermode()) {
        blitter->fXfermode.append(srcover);
    }

    return blitter;
}

void SkRasterPipelineBlitter::append_load_d(SkRasterPipeline* p, const void* dst) const {
    SkASSERT(supported(fDst.info()));

    switch (fDst.info().colorType()) {
        case kN32_SkColorType:
            if (fDst.info().gammaCloseToSRGB()) {
                p->append(load_d_srgb, load_d_srgb_1, dst);
            }
            break;
        case kRGBA_F16_SkColorType:
            p->append(load_d_f16, load_d_f16_1, dst);
            break;
        case kRGB_565_SkColorType:
            p->append(load_d_565, load_d_565_1, dst);
            break;
        default: break;
    }
}

void SkRasterPipelineBlitter::append_store(SkRasterPipeline* p, void* dst) const {
    SkASSERT(supported(fDst.info()));

    p->append(clamp_01_premul);
    switch (fDst.info().colorType()) {
        case kN32_SkColorType:
            if (fDst.info().gammaCloseToSRGB()) {
                p->append(store_srgb, store_srgb_1, dst);
            }
            break;
        case kRGBA_F16_SkColorType:
            p->append(store_f16, store_f16_1, dst);
            break;
        case kRGB_565_SkColorType:
            p->append(store_565, store_565_1, dst);
            break;
        default: break;
    }
}

void SkRasterPipelineBlitter::blitH(int x, int y, int w) {
    auto dst = fDst.writable_addr(0,y);

    SkRasterPipeline p;
    p.extend(fShader);
    p.extend(fColorFilter);
    this->append_load_d(&p, dst);
    p.extend(fXfermode);
    this->append_store(&p, dst);

    p.run(x, w);
}

void SkRasterPipelineBlitter::blitAntiH(int x, int y, const SkAlpha aa[], const int16_t runs[]) {
    auto dst = fDst.writable_addr(0,y);
    float coverage;

    SkRasterPipeline p;
    p.extend(fShader);
    p.extend(fColorFilter);
    this->append_load_d(&p, dst);
    p.extend(fXfermode);
    p.append(lerp_constant_float, &coverage);
    this->append_store(&p, dst);

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
        this->append_load_d(&p, dst);
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
        this->append_store(&p, dst);

        p.run(x, clip.width());
    }
}
