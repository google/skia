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
SK_RASTER_STAGE(clamp_01_premul) {
    a = Sk4f::Max(a, 0.0f);
    r = Sk4f::Max(r, 0.0f);
    g = Sk4f::Max(g, 0.0f);
    b = Sk4f::Max(b, 0.0f);

    a = Sk4f::Min(a, 1.0f);
    r = Sk4f::Min(r, a);
    g = Sk4f::Min(g, a);
    b = Sk4f::Min(b, a);
}

// The default shader produces a constant color (from the SkPaint).
SK_RASTER_STAGE(constant_color) {
    auto color = (const SkPM4f*)ctx;
    r = color->r();
    g = color->g();
    b = color->b();
    a = color->a();
}

// The default transfer mode is srcover, s' = s + d*(1-sa).
SK_RASTER_STAGE(srcover) {
    r += dr*(1.0f - a);
    g += dg*(1.0f - a);
    b += db*(1.0f - a);
    a += da*(1.0f - a);
}

static Sk4f lerp(const Sk4f& from, const Sk4f& to, const Sk4f& cov) {
    return from + (to-from)*cov;
}

// s' = d(1-c) + sc, for a constant c.
SK_RASTER_STAGE(lerp_constant_float) {
    Sk4f c = *(const float*)ctx;

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

template <typename T>
static SkNx<4,T> load_tail(size_t tail, const T* src) {
    if (tail) {
       return SkNx<4,T>(src[0], (tail>1 ? src[1] : 0), (tail>2 ? src[2] : 0), 0);
    }
    return SkNx<4,T>::Load(src);
}

template <typename T>
static void store_tail(size_t tail, const SkNx<4,T>& v, T* dst) {
    switch(tail) {
        case 0: return v.store(dst);
        case 3: dst[2] = v[2];
        case 2: dst[1] = v[1];
        case 1: dst[0] = v[0];
    }
}

// s' = d(1-c) + sc for 8-bit c.
SK_RASTER_STAGE(lerp_a8) {
    auto ptr = (const uint8_t*)ctx + x;

    Sk4f c = SkNx_cast<float>(load_tail(tail, ptr)) * (1/255.0f);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
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

// s' = d(1-c) + sc for 565 c.
SK_RASTER_STAGE(lerp_lcd16) {
    auto ptr = (const uint16_t*)ctx + x;
    Sk4f cr, cg, cb;
    from_565(load_tail(tail, ptr), &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0f;
}

SK_RASTER_STAGE(load_d_565) {
    auto ptr = (const uint16_t*)ctx + x;
    from_565(load_tail(tail, ptr), &dr,&dg,&db);
    da = 1.0f;
}

SK_RASTER_STAGE(store_565) {
    auto ptr = (uint16_t*)ctx + x;
    store_tail(tail, to_565(r,g,b), ptr);
}

SK_RASTER_STAGE(load_d_f16) {
    auto ptr = (const uint64_t*)ctx + x;

    if (tail) {
        auto p0 =          SkHalfToFloat_finite_ftz(ptr[0])          ,
             p1 = tail>1 ? SkHalfToFloat_finite_ftz(ptr[1]) : Sk4f{0},
             p2 = tail>2 ? SkHalfToFloat_finite_ftz(ptr[2]) : Sk4f{0};
        dr = { p0[0],p1[0],p2[0],0 };
        dg = { p0[1],p1[1],p2[1],0 };
        db = { p0[2],p1[2],p2[2],0 };
        da = { p0[3],p1[3],p2[3],0 };
        return;
    }

    Sk4h rh, gh, bh, ah;
    Sk4h_load4(ptr, &rh, &gh, &bh, &ah);
    dr = SkHalfToFloat_finite_ftz(rh);
    dg = SkHalfToFloat_finite_ftz(gh);
    db = SkHalfToFloat_finite_ftz(bh);
    da = SkHalfToFloat_finite_ftz(ah);
}

SK_RASTER_STAGE(store_f16) {
    auto ptr = (uint64_t*)ctx + x;

    switch (tail) {
        case 0: return Sk4h_store4(ptr, SkFloatToHalf_finite_ftz(r), SkFloatToHalf_finite_ftz(g),
                                        SkFloatToHalf_finite_ftz(b), SkFloatToHalf_finite_ftz(a));

        case 3: SkFloatToHalf_finite_ftz({r[2], g[2], b[2], a[2]}).store(ptr+2);
        case 2: SkFloatToHalf_finite_ftz({r[1], g[1], b[1], a[1]}).store(ptr+1);
        case 1: SkFloatToHalf_finite_ftz({r[0], g[0], b[0], a[0]}).store(ptr+0);
    }
}

// Load 8-bit SkPMColor-order sRGB.
SK_RASTER_STAGE(load_d_srgb) {
    auto ptr = (const uint32_t*)ctx + x;

    if (tail) {
        float rs[] = {0,0,0,0},
              gs[] = {0,0,0,0},
              bs[] = {0,0,0,0},
              as[] = {0,0,0,0};
        for (size_t i = 0; i < tail; i++) {
            rs[i] = sk_linear_from_srgb[(ptr[i] >> SK_R32_SHIFT) & 0xff];
            gs[i] = sk_linear_from_srgb[(ptr[i] >> SK_G32_SHIFT) & 0xff];
            bs[i] = sk_linear_from_srgb[(ptr[i] >> SK_B32_SHIFT) & 0xff];
            as[i] =       (1/255.0f) *  (ptr[i] >> SK_A32_SHIFT)        ;
        }
        dr = Sk4f::Load(rs);
        dg = Sk4f::Load(gs);
        db = Sk4f::Load(bs);
        da = Sk4f::Load(as);
        return;
    }

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
}

// Store 8-bit SkPMColor-order sRGB.
SK_RASTER_STAGE(store_srgb) {
    auto ptr = (uint32_t*)ctx + x;
    store_tail(tail, ( sk_linear_to_srgb_noclamp(r) << SK_R32_SHIFT
                     | sk_linear_to_srgb_noclamp(g) << SK_G32_SHIFT
                     | sk_linear_to_srgb_noclamp(b) << SK_B32_SHIFT
                     |       Sk4f_round(255.0f * a) << SK_A32_SHIFT), (int*)ptr);
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
        blitter->fShader.append<constant_color>(&blitter->fPaintColor);
    }
    if (!paint.getXfermode()) {
        blitter->fXfermode.append<srcover>();
    }

    return blitter;
}

void SkRasterPipelineBlitter::append_load_d(SkRasterPipeline* p, const void* dst) const {
    SkASSERT(supported(fDst.info()));

    switch (fDst.info().colorType()) {
        case kN32_SkColorType:
            if (fDst.info().gammaCloseToSRGB()) {
                p->append<load_d_srgb>(dst);
            }
            break;
        case kRGBA_F16_SkColorType:
            p->append<load_d_f16>(dst);
            break;
        case kRGB_565_SkColorType:
            p->append<load_d_565>(dst);
            break;
        default: break;
    }
}

template <SkRasterPipeline::EasyFn fn>
static void clamp_01_premul_then(void* ctx, size_t x, size_t tail,
                                 Sk4f&  r, Sk4f&  g, Sk4f&  b, Sk4f&  a,
                                 Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da) {
    clamp_01_premul(nullptr, x,tail, r,g,b,a, dr,dg,db,da);
                 fn(    ctx, x,tail, r,g,b,a, dr,dg,db,da);
}

void SkRasterPipelineBlitter::append_store(SkRasterPipeline* p, void* dst) const {
    SkASSERT(supported(fDst.info()));

    switch (fDst.info().colorType()) {
        case kN32_SkColorType:
            if (fDst.info().gammaCloseToSRGB()) {
                p->last<clamp_01_premul_then<store_srgb>>(dst);
            }
            break;
        case kRGBA_F16_SkColorType:
            p->last<clamp_01_premul_then<store_f16>>(dst);
            break;
        case kRGB_565_SkColorType:
            p->last<clamp_01_premul_then<store_565>>(dst);
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
    p.append<lerp_constant_float>(&coverage);
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
                p.append<lerp_a8>(mask.getAddr8(x,y)-x);
                break;
            case SkMask::kLCD16_Format:
                p.append<lerp_lcd16>(mask.getAddrLCD16(x,y)-x);
                break;
            default: break;
        }
        this->append_store(&p, dst);

        p.run(x, clip.width());
    }
}
