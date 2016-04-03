/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermode.h"
#include "SkXfermode_proccoeff.h"
#include "SkColorPriv.h"
#include "SkMathPriv.h"
#include "SkOncePtr.h"
#include "SkOpts.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkWriteBuffer.h"
#include "SkPM4f.h"

#define SkAlphaMulAlpha(a, b)   SkMulDiv255Round(a, b)

static inline unsigned saturated_add(unsigned a, unsigned b) {
    SkASSERT(a <= 255);
    SkASSERT(b <= 255);
    unsigned sum = a + b;
    if (sum > 255) {
        sum = 255;
    }
    return sum;
}

static inline int clamp_signed_byte(int n) {
    if (n < 0) {
        n = 0;
    } else if (n > 255) {
        n = 255;
    }
    return n;
}

static inline int clamp_div255round(int prod) {
    if (prod <= 0) {
        return 0;
    } else if (prod >= 255*255) {
        return 255;
    } else {
        return SkDiv255Round(prod);
    }
}

///////////////////////////////////////////////////////////////////////////////
#include "SkNx.h"

static Sk4f     alpha(const Sk4f& color) { return Sk4f(color[3]); }
static Sk4f inv_alpha(const Sk4f& color) { return Sk4f(1 - color[3]); }
static Sk4f     pin_1(const Sk4f& value) { return Sk4f::Min(value, Sk4f(1)); }

static Sk4f color_alpha(const Sk4f& color, float newAlpha) {
    return Sk4f(color[0], color[1], color[2], newAlpha);
}
static Sk4f color_alpha(const Sk4f& color, const Sk4f& newAlpha) {
    return color_alpha(color, newAlpha[3]);
}

static Sk4f set_argb(float a, float r, float g, float b) {
    if (0 == SkPM4f::R) {
        return Sk4f(r, g, b, a);
    } else {
        return Sk4f(b, g, r, a);
    }
}

static Sk4f    clear_4f(const Sk4f& s, const Sk4f& d) { return Sk4f(0); }
static Sk4f      src_4f(const Sk4f& s, const Sk4f& d) { return s; }
static Sk4f      dst_4f(const Sk4f& s, const Sk4f& d) { return d; }
static Sk4f  srcover_4f(const Sk4f& s, const Sk4f& d) { return s + inv_alpha(s) * d; }
static Sk4f  dstover_4f(const Sk4f& s, const Sk4f& d) { return d + inv_alpha(d) * s; }
static Sk4f    srcin_4f(const Sk4f& s, const Sk4f& d) { return s * alpha(d); }
static Sk4f    dstin_4f(const Sk4f& s, const Sk4f& d) { return d * alpha(s); }
static Sk4f   srcout_4f(const Sk4f& s, const Sk4f& d) { return s * inv_alpha(d); }
static Sk4f   dstout_4f(const Sk4f& s, const Sk4f& d) { return d * inv_alpha(s); }
static Sk4f  srcatop_4f(const Sk4f& s, const Sk4f& d) { return s * alpha(d) + d * inv_alpha(s); }
static Sk4f  dstatop_4f(const Sk4f& s, const Sk4f& d) { return d * alpha(s) + s * inv_alpha(d); }
static Sk4f      xor_4f(const Sk4f& s, const Sk4f& d) { return s * inv_alpha(d) + d * inv_alpha(s);}
static Sk4f     plus_4f(const Sk4f& s, const Sk4f& d) { return pin_1(s + d); }
static Sk4f modulate_4f(const Sk4f& s, const Sk4f& d) { return s * d; }
static Sk4f   screen_4f(const Sk4f& s, const Sk4f& d) { return s + d - s * d; }

static Sk4f multiply_4f(const Sk4f& s, const Sk4f& d) {
    return s * inv_alpha(d) + d * inv_alpha(s) + s * d;
}

static Sk4f overlay_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f sa = alpha(s);
    Sk4f da = alpha(d);
    Sk4f two = Sk4f(2);
    Sk4f rc = (two * d <= da).thenElse(two * s * d,
                                       sa * da - two * (da - d) * (sa - s));
    return pin_1(s + d - s * da + color_alpha(rc - d * sa, 0));
}

static Sk4f hardlight_4f(const Sk4f& s, const Sk4f& d) {
    return overlay_4f(d, s);
}

static Sk4f darken_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f sa = alpha(s);
    Sk4f da = alpha(d);
    return s + d - Sk4f::Max(s * da, d * sa);
}

static Sk4f lighten_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f sa = alpha(s);
    Sk4f da = alpha(d);
    return s + d - Sk4f::Min(s * da, d * sa);
}

static Sk4f colordodge_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f sa = alpha(s);
    Sk4f da = alpha(d);
    Sk4f isa = Sk4f(1) - sa;
    Sk4f ida = Sk4f(1) - da;

    Sk4f srcover = s + d * isa;
    Sk4f dstover = d + s * ida;
    Sk4f otherwise = sa * Sk4f::Min(da, (d * sa) / (sa - s)) + s * ida + d * isa;

    // Order matters here, preferring d==0 over s==sa.
    auto colors = (d == Sk4f(0)).thenElse(dstover,
                                          (s == sa).thenElse(srcover,
                                                             otherwise));
    return color_alpha(colors, srcover);
}

static Sk4f colorburn_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f sa  = alpha(s);
    Sk4f da  = alpha(d);
    Sk4f isa = Sk4f(1) - sa;
    Sk4f ida = Sk4f(1) - da;

    Sk4f srcover = s + d * isa;
    Sk4f dstover = d + s * ida;
    Sk4f otherwise = sa * (da - Sk4f::Min(da, (da - d) * sa / s)) + s * ida + d * isa;

    // Order matters here, preferring d==da over s==0.
    auto colors = (d == da).thenElse(dstover,
                                     (s == Sk4f(0)).thenElse(srcover,
                                                             otherwise));
    return color_alpha(colors, srcover);
}

static Sk4f softlight_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f sa  = alpha(s);
    Sk4f da  = alpha(d);
    Sk4f isa = Sk4f(1) - sa;
    Sk4f ida = Sk4f(1) - da;

    // Some common terms.
    Sk4f m  = (da > Sk4f(0)).thenElse(d / da, Sk4f(0));
    Sk4f s2 = Sk4f(2) * s;
    Sk4f m4 = Sk4f(4) * m;

    // The logic forks three ways:
    //    1. dark src?
    //    2. light src, dark dst?
    //    3. light src, light dst?
    Sk4f darkSrc = d * (sa + (s2 - sa) * (Sk4f(1) - m));            // Used in case 1.
    Sk4f darkDst = (m4 * m4 + m4) * (m - Sk4f(1)) + Sk4f(7) * m;    // Used in case 2.
    Sk4f liteDst = m.sqrt() - m;                                    // Used in case 3.
    Sk4f liteSrc = d * sa + da * (s2 - sa) * (Sk4f(4) * d <= da).thenElse(darkDst,
                                                                          liteDst); // Case 2 or 3?

    return color_alpha(s * ida + d * isa + (s2 <= sa).thenElse(darkSrc, liteSrc), // Case 1 or 2/3?
                       s + d * isa);
}

static Sk4f difference_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f min = Sk4f::Min(s * alpha(d), d * alpha(s));
    return s + d - min - color_alpha(min, 0);
}

static Sk4f exclusion_4f(const Sk4f& s, const Sk4f& d) {
    Sk4f product = s * d;
    return s + d - product - color_alpha(product, 0);
}

////////////////////////////////////////////////////

// The CSS compositing spec introduces the following formulas:
// (See https://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html#blendingnonseparable)
// SkComputeLuminance is similar to this formula but it uses the new definition from Rec. 709
// while PDF and CG uses the one from Rec. Rec. 601
// See http://www.glennchan.info/articles/technical/hd-versus-sd-color-space/hd-versus-sd-color-space.htm
static inline float Lum(float r, float g, float b) {
    return r * 0.2126f + g * 0.7152f + b * 0.0722f;
}

static inline float max(float a, float b, float c) {
    return SkTMax(a, SkTMax(b, c));
}

static inline float min(float a, float b, float c) {
    return SkTMin(a, SkTMin(b, c));
}

static inline float Sat(float r, float g, float b) {
    return max(r, g, b) - min(r, g, b);
}

static inline void setSaturationComponents(float* Cmin, float* Cmid, float* Cmax, float s) {
    if(*Cmax > *Cmin) {
        *Cmid = (*Cmid - *Cmin) * s / (*Cmax - *Cmin);
        *Cmax = s;
    } else {
        *Cmax = 0;
        *Cmid = 0;
    }
    *Cmin = 0;
}

static inline void SetSat(float* r, float* g, float* b, float s) {
    if(*r <= *g) {
        if(*g <= *b) {
            setSaturationComponents(r, g, b, s);
        } else if(*r <= *b) {
            setSaturationComponents(r, b, g, s);
        } else {
            setSaturationComponents(b, r, g, s);
        }
    } else if(*r <= *b) {
        setSaturationComponents(g, r, b, s);
    } else if(*g <= *b) {
        setSaturationComponents(g, b, r, s);
    } else {
        setSaturationComponents(b, g, r, s);
    }
}

static inline void clipColor(float* r, float* g, float* b, float a) {
    float L = Lum(*r, *g, *b);
    float n = min(*r, *g, *b);
    float x = max(*r, *g, *b);
    float denom;
    if ((n < 0) && (denom = L - n)) { // Compute denom and make sure it's non zero
        float scale = L / denom;
        *r = L + (*r - L) * scale;
        *g = L + (*g - L) * scale;
        *b = L + (*b - L) * scale;
    }

    if ((x > a) && (denom = x - L)) { // Compute denom and make sure it's non zero
        float scale = (a - L) / denom;
        *r = L + (*r - L) * scale;
        *g = L + (*g - L) * scale;
        *b = L + (*b - L) * scale;
    }
}

static inline void SetLum(float* r, float* g, float* b, float a, float l) {
    float d = l - Lum(*r, *g, *b);
    *r += d;
    *g += d;
    *b += d;
    clipColor(r, g, b, a);
}

static Sk4f hue_4f(const Sk4f& s, const Sk4f& d) {
    float sa = s[SkPM4f::A];
    float sr = s[SkPM4f::R];
    float sg = s[SkPM4f::G];
    float sb = s[SkPM4f::B];

    float da = d[SkPM4f::A];
    float dr = d[SkPM4f::R];
    float dg = d[SkPM4f::G];
    float db = d[SkPM4f::B];

    float Sr = sr;
    float Sg = sg;
    float Sb = sb;
    SetSat(&Sr, &Sg, &Sb, Sat(dr, dg, db) * sa);
    SetLum(&Sr, &Sg, &Sb, sa * da, Lum(dr, dg, db) * sa);

    return color_alpha(s * inv_alpha(d) + d * inv_alpha(s) + set_argb(0, Sr, Sg, Sb),
                       sa + da - sa * da);
}

static Sk4f saturation_4f(const Sk4f& s, const Sk4f& d) {
    float sa = s[SkPM4f::A];
    float sr = s[SkPM4f::R];
    float sg = s[SkPM4f::G];
    float sb = s[SkPM4f::B];

    float da = d[SkPM4f::A];
    float dr = d[SkPM4f::R];
    float dg = d[SkPM4f::G];
    float db = d[SkPM4f::B];

    float Dr = dr;
    float Dg = dg;
    float Db = db;
    SetSat(&Dr, &Dg, &Db, Sat(sr, sg, sb) * da);
    SetLum(&Dr, &Dg, &Db, sa * da, Lum(dr, dg, db) * sa);

    return color_alpha(s * inv_alpha(d) + d * inv_alpha(s) + set_argb(0, Dr, Dg, Db),
                       sa + da - sa * da);
}

static Sk4f color_4f(const Sk4f& s, const Sk4f& d) {
    float sa = s[SkPM4f::A];
    float sr = s[SkPM4f::R];
    float sg = s[SkPM4f::G];
    float sb = s[SkPM4f::B];

    float da = d[SkPM4f::A];
    float dr = d[SkPM4f::R];
    float dg = d[SkPM4f::G];
    float db = d[SkPM4f::B];

    float Sr = sr;
    float Sg = sg;
    float Sb = sb;
    SetLum(&Sr, &Sg, &Sb, sa * da, Lum(dr, dg, db) * sa);

    Sk4f res = color_alpha(s * inv_alpha(d) + d * inv_alpha(s) + set_argb(0, Sr, Sg, Sb),
                           sa + da - sa * da);
    // Can return tiny negative values ...
    return Sk4f::Max(res, Sk4f(0));
}

static Sk4f luminosity_4f(const Sk4f& s, const Sk4f& d) {
    float sa = s[SkPM4f::A];
    float sr = s[SkPM4f::R];
    float sg = s[SkPM4f::G];
    float sb = s[SkPM4f::B];

    float da = d[SkPM4f::A];
    float dr = d[SkPM4f::R];
    float dg = d[SkPM4f::G];
    float db = d[SkPM4f::B];

    float Dr = dr;
    float Dg = dg;
    float Db = db;
    SetLum(&Dr, &Dg, &Db, sa * da, Lum(sr, sg, sb) * da);

    Sk4f res = color_alpha(s * inv_alpha(d) + d * inv_alpha(s) + set_argb(0, Dr, Dg, Db),
                           sa + da - sa * da);
    // Can return tiny negative values ...
    return Sk4f::Max(res, Sk4f(0));
}

///////////////////////////////////////////////////////////////////////////////

//  kClear_Mode,    //!< [0, 0]
static SkPMColor clear_modeproc(SkPMColor src, SkPMColor dst) {
    return 0;
}

//  kSrc_Mode,      //!< [Sa, Sc]
static SkPMColor src_modeproc(SkPMColor src, SkPMColor dst) {
    return src;
}

//  kDst_Mode,      //!< [Da, Dc]
static SkPMColor dst_modeproc(SkPMColor src, SkPMColor dst) {
    return dst;
}

//  kSrcOver_Mode,  //!< [Sa + Da - Sa*Da, Sc + (1 - Sa)*Dc]
static SkPMColor srcover_modeproc(SkPMColor src, SkPMColor dst) {
#if 0
    // this is the old, more-correct way, but it doesn't guarantee that dst==255
    // will always stay opaque
    return src + SkAlphaMulQ(dst, SkAlpha255To256(255 - SkGetPackedA32(src)));
#else
    // this is slightly faster, but more importantly guarantees that dst==255
    // will always stay opaque
    return src + SkAlphaMulQ(dst, 256 - SkGetPackedA32(src));
#endif
}

//  kDstOver_Mode,  //!< [Sa + Da - Sa*Da, Dc + (1 - Da)*Sc]
static SkPMColor dstover_modeproc(SkPMColor src, SkPMColor dst) {
    // this is the reverse of srcover, just flipping src and dst
    // see srcover's comment about the 256 for opaqueness guarantees
    return dst + SkAlphaMulQ(src, 256 - SkGetPackedA32(dst));
}

//  kSrcIn_Mode,    //!< [Sa * Da, Sc * Da]
static SkPMColor srcin_modeproc(SkPMColor src, SkPMColor dst) {
    return SkAlphaMulQ(src, SkAlpha255To256(SkGetPackedA32(dst)));
}

//  kDstIn_Mode,    //!< [Sa * Da, Sa * Dc]
static SkPMColor dstin_modeproc(SkPMColor src, SkPMColor dst) {
    return SkAlphaMulQ(dst, SkAlpha255To256(SkGetPackedA32(src)));
}

//  kSrcOut_Mode,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
static SkPMColor srcout_modeproc(SkPMColor src, SkPMColor dst) {
    return SkAlphaMulQ(src, SkAlpha255To256(255 - SkGetPackedA32(dst)));
}

//  kDstOut_Mode,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
static SkPMColor dstout_modeproc(SkPMColor src, SkPMColor dst) {
    return SkAlphaMulQ(dst, SkAlpha255To256(255 - SkGetPackedA32(src)));
}

//  kSrcATop_Mode,  //!< [Da, Sc * Da + (1 - Sa) * Dc]
static SkPMColor srcatop_modeproc(SkPMColor src, SkPMColor dst) {
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned isa = 255 - sa;

    return SkPackARGB32(da,
                        SkAlphaMulAlpha(da, SkGetPackedR32(src)) +
                            SkAlphaMulAlpha(isa, SkGetPackedR32(dst)),
                        SkAlphaMulAlpha(da, SkGetPackedG32(src)) +
                            SkAlphaMulAlpha(isa, SkGetPackedG32(dst)),
                        SkAlphaMulAlpha(da, SkGetPackedB32(src)) +
                            SkAlphaMulAlpha(isa, SkGetPackedB32(dst)));
}

//  kDstATop_Mode,  //!< [Sa, Sa * Dc + Sc * (1 - Da)]
static SkPMColor dstatop_modeproc(SkPMColor src, SkPMColor dst) {
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned ida = 255 - da;

    return SkPackARGB32(sa,
                        SkAlphaMulAlpha(ida, SkGetPackedR32(src)) +
                            SkAlphaMulAlpha(sa, SkGetPackedR32(dst)),
                        SkAlphaMulAlpha(ida, SkGetPackedG32(src)) +
                            SkAlphaMulAlpha(sa, SkGetPackedG32(dst)),
                        SkAlphaMulAlpha(ida, SkGetPackedB32(src)) +
                            SkAlphaMulAlpha(sa, SkGetPackedB32(dst)));
}

//  kXor_Mode   [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + (1 - Sa) * Dc]
static SkPMColor xor_modeproc(SkPMColor src, SkPMColor dst) {
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned isa = 255 - sa;
    unsigned ida = 255 - da;

    return SkPackARGB32(sa + da - (SkAlphaMulAlpha(sa, da) << 1),
                        SkAlphaMulAlpha(ida, SkGetPackedR32(src)) +
                            SkAlphaMulAlpha(isa, SkGetPackedR32(dst)),
                        SkAlphaMulAlpha(ida, SkGetPackedG32(src)) +
                            SkAlphaMulAlpha(isa, SkGetPackedG32(dst)),
                        SkAlphaMulAlpha(ida, SkGetPackedB32(src)) +
                            SkAlphaMulAlpha(isa, SkGetPackedB32(dst)));
}

///////////////////////////////////////////////////////////////////////////////

// kPlus_Mode
static SkPMColor plus_modeproc(SkPMColor src, SkPMColor dst) {
    unsigned b = saturated_add(SkGetPackedB32(src), SkGetPackedB32(dst));
    unsigned g = saturated_add(SkGetPackedG32(src), SkGetPackedG32(dst));
    unsigned r = saturated_add(SkGetPackedR32(src), SkGetPackedR32(dst));
    unsigned a = saturated_add(SkGetPackedA32(src), SkGetPackedA32(dst));
    return SkPackARGB32(a, r, g, b);
}

// kModulate_Mode
static SkPMColor modulate_modeproc(SkPMColor src, SkPMColor dst) {
    int a = SkAlphaMulAlpha(SkGetPackedA32(src), SkGetPackedA32(dst));
    int r = SkAlphaMulAlpha(SkGetPackedR32(src), SkGetPackedR32(dst));
    int g = SkAlphaMulAlpha(SkGetPackedG32(src), SkGetPackedG32(dst));
    int b = SkAlphaMulAlpha(SkGetPackedB32(src), SkGetPackedB32(dst));
    return SkPackARGB32(a, r, g, b);
}

static inline int srcover_byte(int a, int b) {
    return a + b - SkAlphaMulAlpha(a, b);
}

// kMultiply_Mode
// B(Cb, Cs) = Cb x Cs
// multiply uses its own version of blendfunc_byte because sa and da are not needed
static int blendfunc_multiply_byte(int sc, int dc, int sa, int da) {
    return clamp_div255round(sc * (255 - da)  + dc * (255 - sa)  + sc * dc);
}

static SkPMColor multiply_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = blendfunc_multiply_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = blendfunc_multiply_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = blendfunc_multiply_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kScreen_Mode
static SkPMColor screen_modeproc(SkPMColor src, SkPMColor dst) {
    int a = srcover_byte(SkGetPackedA32(src), SkGetPackedA32(dst));
    int r = srcover_byte(SkGetPackedR32(src), SkGetPackedR32(dst));
    int g = srcover_byte(SkGetPackedG32(src), SkGetPackedG32(dst));
    int b = srcover_byte(SkGetPackedB32(src), SkGetPackedB32(dst));
    return SkPackARGB32(a, r, g, b);
}

// kOverlay_Mode
static inline int overlay_byte(int sc, int dc, int sa, int da) {
    int tmp = sc * (255 - da) + dc * (255 - sa);
    int rc;
    if (2 * dc <= da) {
        rc = 2 * sc * dc;
    } else {
        rc = sa * da - 2 * (da - dc) * (sa - sc);
    }
    return clamp_div255round(rc + tmp);
}
static SkPMColor overlay_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = overlay_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = overlay_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = overlay_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kDarken_Mode
static inline int darken_byte(int sc, int dc, int sa, int da) {
    int sd = sc * da;
    int ds = dc * sa;
    if (sd < ds) {
        // srcover
        return sc + dc - SkDiv255Round(ds);
    } else {
        // dstover
        return dc + sc - SkDiv255Round(sd);
    }
}
static SkPMColor darken_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = darken_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = darken_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = darken_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kLighten_Mode
static inline int lighten_byte(int sc, int dc, int sa, int da) {
    int sd = sc * da;
    int ds = dc * sa;
    if (sd > ds) {
        // srcover
        return sc + dc - SkDiv255Round(ds);
    } else {
        // dstover
        return dc + sc - SkDiv255Round(sd);
    }
}
static SkPMColor lighten_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = lighten_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = lighten_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = lighten_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kColorDodge_Mode
static inline int colordodge_byte(int sc, int dc, int sa, int da) {
    int diff = sa - sc;
    int rc;
    if (0 == dc) {
        return SkAlphaMulAlpha(sc, 255 - da);
    } else if (0 == diff) {
        rc = sa * da + sc * (255 - da) + dc * (255 - sa);
    } else {
        diff = dc * sa / diff;
        rc = sa * ((da < diff) ? da : diff) + sc * (255 - da) + dc * (255 - sa);
    }
    return clamp_div255round(rc);
}
static SkPMColor colordodge_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = colordodge_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = colordodge_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = colordodge_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kColorBurn_Mode
static inline int colorburn_byte(int sc, int dc, int sa, int da) {
    int rc;
    if (dc == da) {
        rc = sa * da + sc * (255 - da) + dc * (255 - sa);
    } else if (0 == sc) {
        return SkAlphaMulAlpha(dc, 255 - sa);
    } else {
        int tmp = (da - dc) * sa / sc;
        rc = sa * (da - ((da < tmp) ? da : tmp))
            + sc * (255 - da) + dc * (255 - sa);
    }
    return clamp_div255round(rc);
}
static SkPMColor colorburn_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = colorburn_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = colorburn_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = colorburn_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kHardLight_Mode
static inline int hardlight_byte(int sc, int dc, int sa, int da) {
    int rc;
    if (2 * sc <= sa) {
        rc = 2 * sc * dc;
    } else {
        rc = sa * da - 2 * (da - dc) * (sa - sc);
    }
    return clamp_div255round(rc + sc * (255 - da) + dc * (255 - sa));
}
static SkPMColor hardlight_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = hardlight_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = hardlight_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = hardlight_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// returns 255 * sqrt(n/255)
static U8CPU sqrt_unit_byte(U8CPU n) {
    return SkSqrtBits(n, 15+4);
}

// kSoftLight_Mode
static inline int softlight_byte(int sc, int dc, int sa, int da) {
    int m = da ? dc * 256 / da : 0;
    int rc;
    if (2 * sc <= sa) {
        rc = dc * (sa + ((2 * sc - sa) * (256 - m) >> 8));
    } else if (4 * dc <= da) {
        int tmp = (4 * m * (4 * m + 256) * (m - 256) >> 16) + 7 * m;
        rc = dc * sa + (da * (2 * sc - sa) * tmp >> 8);
    } else {
        int tmp = sqrt_unit_byte(m) - m;
        rc = dc * sa + (da * (2 * sc - sa) * tmp >> 8);
    }
    return clamp_div255round(rc + sc * (255 - da) + dc * (255 - sa));
}
static SkPMColor softlight_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = softlight_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = softlight_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = softlight_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kDifference_Mode
static inline int difference_byte(int sc, int dc, int sa, int da) {
    int tmp = SkMin32(sc * da, dc * sa);
    return clamp_signed_byte(sc + dc - 2 * SkDiv255Round(tmp));
}
static SkPMColor difference_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = difference_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = difference_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = difference_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// kExclusion_Mode
static inline int exclusion_byte(int sc, int dc, int, int) {
    // this equations is wacky, wait for SVG to confirm it
    //int r = sc * da + dc * sa - 2 * sc * dc + sc * (255 - da) + dc * (255 - sa);

    // The above equation can be simplified as follows
    int r = 255*(sc + dc) - 2 * sc * dc;
    return clamp_div255round(r);
}
static SkPMColor exclusion_modeproc(SkPMColor src, SkPMColor dst) {
    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = exclusion_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = exclusion_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = exclusion_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    return SkPackARGB32(a, r, g, b);
}

// The CSS compositing spec introduces the following formulas:
// (See https://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html#blendingnonseparable)
// SkComputeLuminance is similar to this formula but it uses the new definition from Rec. 709
// while PDF and CG uses the one from Rec. Rec. 601
// See http://www.glennchan.info/articles/technical/hd-versus-sd-color-space/hd-versus-sd-color-space.htm
static inline int Lum(int r, int g, int b)
{
    return SkDiv255Round(r * 77 + g * 150 + b * 28);
}

static inline int min2(int a, int b) { return a < b ? a : b; }
static inline int max2(int a, int b) { return a > b ? a : b; }
#define minimum(a, b, c) min2(min2(a, b), c)
#define maximum(a, b, c) max2(max2(a, b), c)

static inline int Sat(int r, int g, int b) {
    return maximum(r, g, b) - minimum(r, g, b);
}

static inline void setSaturationComponents(int* Cmin, int* Cmid, int* Cmax, int s) {
    if(*Cmax > *Cmin) {
        *Cmid =  SkMulDiv(*Cmid - *Cmin, s, *Cmax - *Cmin);
        *Cmax = s;
    } else {
        *Cmax = 0;
        *Cmid = 0;
    }

    *Cmin = 0;
}

static inline void SetSat(int* r, int* g, int* b, int s) {
    if(*r <= *g) {
        if(*g <= *b) {
            setSaturationComponents(r, g, b, s);
        } else if(*r <= *b) {
            setSaturationComponents(r, b, g, s);
        } else {
            setSaturationComponents(b, r, g, s);
        }
    } else if(*r <= *b) {
        setSaturationComponents(g, r, b, s);
    } else if(*g <= *b) {
        setSaturationComponents(g, b, r, s);
    } else {
        setSaturationComponents(b, g, r, s);
    }
}

static inline void clipColor(int* r, int* g, int* b, int a) {
    int L = Lum(*r, *g, *b);
    int n = minimum(*r, *g, *b);
    int x = maximum(*r, *g, *b);
    int denom;
    if ((n < 0) && (denom = L - n)) { // Compute denom and make sure it's non zero
       *r = L + SkMulDiv(*r - L, L, denom);
       *g = L + SkMulDiv(*g - L, L, denom);
       *b = L + SkMulDiv(*b - L, L, denom);
    }

    if ((x > a) && (denom = x - L)) { // Compute denom and make sure it's non zero
       int numer = a - L;
       *r = L + SkMulDiv(*r - L, numer, denom);
       *g = L + SkMulDiv(*g - L, numer, denom);
       *b = L + SkMulDiv(*b - L, numer, denom);
    }
}

static inline void SetLum(int* r, int* g, int* b, int a, int l) {
  int d = l - Lum(*r, *g, *b);
  *r +=  d;
  *g +=  d;
  *b +=  d;

  clipColor(r, g, b, a);
}

// non-separable blend modes are done in non-premultiplied alpha
#define  blendfunc_nonsep_byte(sc, dc, sa, da, blendval) \
  clamp_div255round(sc * (255 - da) +  dc * (255 - sa) + blendval)

// kHue_Mode
// B(Cb, Cs) = SetLum(SetSat(Cs, Sat(Cb)), Lum(Cb))
// Create a color with the hue of the source color and the saturation and luminosity of the backdrop color.
static SkPMColor hue_modeproc(SkPMColor src, SkPMColor dst) {
    int sr = SkGetPackedR32(src);
    int sg = SkGetPackedG32(src);
    int sb = SkGetPackedB32(src);
    int sa = SkGetPackedA32(src);

    int dr = SkGetPackedR32(dst);
    int dg = SkGetPackedG32(dst);
    int db = SkGetPackedB32(dst);
    int da = SkGetPackedA32(dst);
    int Sr, Sg, Sb;

    if(sa && da) {
        Sr = sr * sa;
        Sg = sg * sa;
        Sb = sb * sa;
        SetSat(&Sr, &Sg, &Sb, Sat(dr, dg, db) * sa);
        SetLum(&Sr, &Sg, &Sb, sa * da, Lum(dr, dg, db) * sa);
    } else {
        Sr = 0;
        Sg = 0;
        Sb = 0;
    }

    int a = srcover_byte(sa, da);
    int r = blendfunc_nonsep_byte(sr, dr, sa, da, Sr);
    int g = blendfunc_nonsep_byte(sg, dg, sa, da, Sg);
    int b = blendfunc_nonsep_byte(sb, db, sa, da, Sb);
    return SkPackARGB32(a, r, g, b);
}

// kSaturation_Mode
// B(Cb, Cs) = SetLum(SetSat(Cb, Sat(Cs)), Lum(Cb))
// Create a color with the saturation of the source color and the hue and luminosity of the backdrop color.
static SkPMColor saturation_modeproc(SkPMColor src, SkPMColor dst) {
    int sr = SkGetPackedR32(src);
    int sg = SkGetPackedG32(src);
    int sb = SkGetPackedB32(src);
    int sa = SkGetPackedA32(src);

    int dr = SkGetPackedR32(dst);
    int dg = SkGetPackedG32(dst);
    int db = SkGetPackedB32(dst);
    int da = SkGetPackedA32(dst);
    int Dr, Dg, Db;

    if(sa && da) {
        Dr = dr * sa;
        Dg = dg * sa;
        Db = db * sa;
        SetSat(&Dr, &Dg, &Db, Sat(sr, sg, sb) * da);
        SetLum(&Dr, &Dg, &Db, sa * da, Lum(dr, dg, db) * sa);
    } else {
        Dr = 0;
        Dg = 0;
        Db = 0;
    }

    int a = srcover_byte(sa, da);
    int r = blendfunc_nonsep_byte(sr, dr, sa, da, Dr);
    int g = blendfunc_nonsep_byte(sg, dg, sa, da, Dg);
    int b = blendfunc_nonsep_byte(sb, db, sa, da, Db);
    return SkPackARGB32(a, r, g, b);
}

// kColor_Mode
// B(Cb, Cs) = SetLum(Cs, Lum(Cb))
// Create a color with the hue and saturation of the source color and the luminosity of the backdrop color.
static SkPMColor color_modeproc(SkPMColor src, SkPMColor dst) {
    int sr = SkGetPackedR32(src);
    int sg = SkGetPackedG32(src);
    int sb = SkGetPackedB32(src);
    int sa = SkGetPackedA32(src);

    int dr = SkGetPackedR32(dst);
    int dg = SkGetPackedG32(dst);
    int db = SkGetPackedB32(dst);
    int da = SkGetPackedA32(dst);
    int Sr, Sg, Sb;

    if(sa && da) {
        Sr = sr * da;
        Sg = sg * da;
        Sb = sb * da;
        SetLum(&Sr, &Sg, &Sb, sa * da, Lum(dr, dg, db) * sa);
    } else {
        Sr = 0;
        Sg = 0;
        Sb = 0;
    }

    int a = srcover_byte(sa, da);
    int r = blendfunc_nonsep_byte(sr, dr, sa, da, Sr);
    int g = blendfunc_nonsep_byte(sg, dg, sa, da, Sg);
    int b = blendfunc_nonsep_byte(sb, db, sa, da, Sb);
    return SkPackARGB32(a, r, g, b);
}

// kLuminosity_Mode
// B(Cb, Cs) = SetLum(Cb, Lum(Cs))
// Create a color with the luminosity of the source color and the hue and saturation of the backdrop color.
static SkPMColor luminosity_modeproc(SkPMColor src, SkPMColor dst) {
    int sr = SkGetPackedR32(src);
    int sg = SkGetPackedG32(src);
    int sb = SkGetPackedB32(src);
    int sa = SkGetPackedA32(src);

    int dr = SkGetPackedR32(dst);
    int dg = SkGetPackedG32(dst);
    int db = SkGetPackedB32(dst);
    int da = SkGetPackedA32(dst);
    int Dr, Dg, Db;

    if(sa && da) {
        Dr = dr * sa;
        Dg = dg * sa;
        Db = db * sa;
        SetLum(&Dr, &Dg, &Db, sa * da, Lum(sr, sg, sb) * da);
    } else {
        Dr = 0;
        Dg = 0;
        Db = 0;
    }

    int a = srcover_byte(sa, da);
    int r = blendfunc_nonsep_byte(sr, dr, sa, da, Dr);
    int g = blendfunc_nonsep_byte(sg, dg, sa, da, Dg);
    int b = blendfunc_nonsep_byte(sb, db, sa, da, Db);
    return SkPackARGB32(a, r, g, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkPM4f as_pm4f(const Sk4f& x) {
    SkPM4f pm4;
    x.store(pm4.fVec);
    return pm4;
}

static Sk4f as_4f(const SkPM4f& pm4) {
    return Sk4f::Load(pm4.fVec);
}

static void assert_unit(const SkPM4f& r) {
#ifdef SK_DEBUG
    const float eps = 0.00001f;
    const float min = 0 - eps;
    const float max = 1 + eps;
    for (int i = 0; i < 4; ++i) {
        SkASSERT(r.fVec[i] >= min && r.fVec[i] <= max);
    }
#endif
}

template <Sk4f (blend)(const Sk4f&, const Sk4f&)> SkPM4f proc_4f(const SkPM4f& s, const SkPM4f& d) {
    assert_unit(s);
    assert_unit(d);
    SkPM4f r = as_pm4f(blend(as_4f(s), as_4f(d)));
    assert_unit(r);
    return r;
}

const ProcCoeff gProcCoeffs[] = {
    { clear_modeproc,       proc_4f<clear_4f>,      SkXfermode::kZero_Coeff,    SkXfermode::kZero_Coeff },
    { src_modeproc,         proc_4f<src_4f>,        SkXfermode::kOne_Coeff,     SkXfermode::kZero_Coeff },
    { dst_modeproc,         proc_4f<dst_4f>,        SkXfermode::kZero_Coeff,    SkXfermode::kOne_Coeff  },
    { srcover_modeproc,     proc_4f<srcover_4f>,    SkXfermode::kOne_Coeff,     SkXfermode::kISA_Coeff  },
    { dstover_modeproc,     proc_4f<dstover_4f>,    SkXfermode::kIDA_Coeff,     SkXfermode::kOne_Coeff  },
    { srcin_modeproc,       proc_4f<srcin_4f>,      SkXfermode::kDA_Coeff,      SkXfermode::kZero_Coeff },
    { dstin_modeproc,       proc_4f<dstin_4f>,      SkXfermode::kZero_Coeff,    SkXfermode::kSA_Coeff   },
    { srcout_modeproc,      proc_4f<srcout_4f>,     SkXfermode::kIDA_Coeff,     SkXfermode::kZero_Coeff },
    { dstout_modeproc,      proc_4f<dstout_4f>,     SkXfermode::kZero_Coeff,    SkXfermode::kISA_Coeff  },
    { srcatop_modeproc,     proc_4f<srcatop_4f>,    SkXfermode::kDA_Coeff,      SkXfermode::kISA_Coeff  },
    { dstatop_modeproc,     proc_4f<dstatop_4f>,    SkXfermode::kIDA_Coeff,     SkXfermode::kSA_Coeff   },
    { xor_modeproc,         proc_4f<xor_4f>,        SkXfermode::kIDA_Coeff,     SkXfermode::kISA_Coeff  },

    { plus_modeproc,        proc_4f<plus_4f>,       SkXfermode::kOne_Coeff,     SkXfermode::kOne_Coeff  },
    { modulate_modeproc,    proc_4f<modulate_4f>,   SkXfermode::kZero_Coeff,    SkXfermode::kSC_Coeff   },
    { screen_modeproc,      proc_4f<screen_4f>,     SkXfermode::kOne_Coeff,     SkXfermode::kISC_Coeff  },
    { overlay_modeproc,     proc_4f<overlay_4f>,    CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { darken_modeproc,      proc_4f<darken_4f>,     CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { lighten_modeproc,     proc_4f<lighten_4f>,    CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { colordodge_modeproc,  proc_4f<colordodge_4f>, CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { colorburn_modeproc,   proc_4f<colorburn_4f>,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { hardlight_modeproc,   proc_4f<hardlight_4f>,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { softlight_modeproc,   proc_4f<softlight_4f>,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { difference_modeproc,  proc_4f<difference_4f>, CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { exclusion_modeproc,   proc_4f<exclusion_4f>,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { multiply_modeproc,    proc_4f<multiply_4f>,   CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { hue_modeproc,         proc_4f<hue_4f>,        CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { saturation_modeproc,  proc_4f<saturation_4f>, CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { color_modeproc,       proc_4f<color_4f>,      CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { luminosity_modeproc,  proc_4f<luminosity_4f>, CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
};

///////////////////////////////////////////////////////////////////////////////

bool SkXfermode::asMode(Mode* mode) const {
    return false;
}

#if SK_SUPPORT_GPU
const GrFragmentProcessor* SkXfermode::getFragmentProcessorForImageFilter(
                                                                const GrFragmentProcessor*) const {
    // This should never be called.
    // TODO: make pure virtual in SkXfermode once Android update lands
    SkASSERT(0);
    return nullptr;
}

GrXPFactory* SkXfermode::asXPFactory() const {
    // This should never be called.
    // TODO: make pure virtual in SkXfermode once Android update lands
    SkASSERT(0);
    return nullptr;
}
#endif

SkPMColor SkXfermode::xferColor(SkPMColor src, SkPMColor dst) const{
    // no-op. subclasses should override this
    return dst;
}

void SkXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                        const SkPMColor* SK_RESTRICT src, int count,
                        const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (nullptr == aa) {
        for (int i = count - 1; i >= 0; --i) {
            dst[i] = this->xferColor(src[i], dst[i]);
        }
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0 != a) {
                SkPMColor dstC = dst[i];
                SkPMColor C = this->xferColor(src[i], dstC);
                if (0xFF != a) {
                    C = SkFourByteInterp(C, dstC, a);
                }
                dst[i] = C;
            }
        }
    }
}

void SkXfermode::xfer16(uint16_t* dst,
                        const SkPMColor* SK_RESTRICT src, int count,
                        const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (nullptr == aa) {
        for (int i = count - 1; i >= 0; --i) {
            SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
            dst[i] = SkPixel32ToPixel16_ToU16(this->xferColor(src[i], dstC));
        }
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0 != a) {
                SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
                SkPMColor C = this->xferColor(src[i], dstC);
                if (0xFF != a) {
                    C = SkFourByteInterp(C, dstC, a);
                }
                dst[i] = SkPixel32ToPixel16_ToU16(C);
            }
        }
    }
}

void SkXfermode::xferA8(SkAlpha* SK_RESTRICT dst,
                        const SkPMColor src[], int count,
                        const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (nullptr == aa) {
        for (int i = count - 1; i >= 0; --i) {
            SkPMColor res = this->xferColor(src[i], (dst[i] << SK_A32_SHIFT));
            dst[i] = SkToU8(SkGetPackedA32(res));
        }
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0 != a) {
                SkAlpha dstA = dst[i];
                unsigned A = SkGetPackedA32(this->xferColor(src[i],
                                            (SkPMColor)(dstA << SK_A32_SHIFT)));
                if (0xFF != a) {
                    A = SkAlphaBlend(A, dstA, SkAlpha255To256(a));
                }
                dst[i] = SkToU8(A);
            }
        }
    }
}

bool SkXfermode::supportsCoverageAsAlpha() const {
    return false;
}

bool SkXfermode::isOpaque(SkXfermode::SrcColorOpacity opacityType) const {
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkProcCoeffXfermode::CreateProc(SkReadBuffer& buffer) {
    uint32_t mode32 = buffer.read32();
    if (!buffer.validate(mode32 < SK_ARRAY_COUNT(gProcCoeffs))) {
        return nullptr;
    }
    return SkXfermode::Make((SkXfermode::Mode)mode32);
}

void SkProcCoeffXfermode::flatten(SkWriteBuffer& buffer) const {
    buffer.write32(fMode);
}

bool SkProcCoeffXfermode::asMode(Mode* mode) const {
    if (mode) {
        *mode = fMode;
    }
    return true;
}

bool SkProcCoeffXfermode::supportsCoverageAsAlpha() const {
    if (CANNOT_USE_COEFF == fSrcCoeff) {
        return false;
    }

    switch (fDstCoeff) {
        case SkXfermode::kOne_Coeff:
        case SkXfermode::kISA_Coeff:
        case SkXfermode::kISC_Coeff:
            return true;
        default:
            return false;
    }
}

bool SkProcCoeffXfermode::isOpaque(SkXfermode::SrcColorOpacity opacityType) const {
    if (CANNOT_USE_COEFF == fSrcCoeff) {
        return false;
    }

    if (SkXfermode::kDA_Coeff == fSrcCoeff || SkXfermode::kDC_Coeff == fSrcCoeff ||
        SkXfermode::kIDA_Coeff == fSrcCoeff || SkXfermode::kIDC_Coeff == fSrcCoeff) {
        return false;
    }

    switch (fDstCoeff) {
        case SkXfermode::kZero_Coeff:
            return true;
        case SkXfermode::kISA_Coeff:
            return SkXfermode::kOpaque_SrcColorOpacity == opacityType;
        case SkXfermode::kSA_Coeff:
            return SkXfermode::kTransparentBlack_SrcColorOpacity == opacityType ||
                   SkXfermode::kTransparentAlpha_SrcColorOpacity == opacityType;
        case SkXfermode::kSC_Coeff:
            return SkXfermode::kTransparentBlack_SrcColorOpacity == opacityType;
        default:
            return false;
    }

}

void SkProcCoeffXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src, int count,
                                 const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (proc) {
        if (nullptr == aa) {
            for (int i = count - 1; i >= 0; --i) {
                dst[i] = proc(src[i], dst[i]);
            }
        } else {
            for (int i = count - 1; i >= 0; --i) {
                unsigned a = aa[i];
                if (0 != a) {
                    SkPMColor dstC = dst[i];
                    SkPMColor C = proc(src[i], dstC);
                    if (a != 0xFF) {
                        C = SkFourByteInterp(C, dstC, a);
                    }
                    dst[i] = C;
                }
            }
        }
    }
}

void SkProcCoeffXfermode::xfer16(uint16_t* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src, int count,
                                 const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (proc) {
        if (nullptr == aa) {
            for (int i = count - 1; i >= 0; --i) {
                SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
                dst[i] = SkPixel32ToPixel16_ToU16(proc(src[i], dstC));
            }
        } else {
            for (int i = count - 1; i >= 0; --i) {
                unsigned a = aa[i];
                if (0 != a) {
                    SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
                    SkPMColor C = proc(src[i], dstC);
                    if (0xFF != a) {
                        C = SkFourByteInterp(C, dstC, a);
                    }
                    dst[i] = SkPixel32ToPixel16_ToU16(C);
                }
            }
        }
    }
}

void SkProcCoeffXfermode::xferA8(SkAlpha* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src, int count,
                                 const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (proc) {
        if (nullptr == aa) {
            for (int i = count - 1; i >= 0; --i) {
                SkPMColor res = proc(src[i], dst[i] << SK_A32_SHIFT);
                dst[i] = SkToU8(SkGetPackedA32(res));
            }
        } else {
            for (int i = count - 1; i >= 0; --i) {
                unsigned a = aa[i];
                if (0 != a) {
                    SkAlpha dstA = dst[i];
                    SkPMColor res = proc(src[i], dstA << SK_A32_SHIFT);
                    unsigned A = SkGetPackedA32(res);
                    if (0xFF != a) {
                        A = SkAlphaBlend(A, dstA, SkAlpha255To256(a));
                    }
                    dst[i] = SkToU8(A);
                }
            }
        }
    }
}

#if SK_SUPPORT_GPU
#include "effects/GrCustomXfermode.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"

const GrFragmentProcessor* SkProcCoeffXfermode::getFragmentProcessorForImageFilter(
                                                            const GrFragmentProcessor* dst) const {
    SkASSERT(dst);
    return GrXfermodeFragmentProcessor::CreateFromDstProcessor(dst, fMode);
}

GrXPFactory* SkProcCoeffXfermode::asXPFactory() const {
    if (CANNOT_USE_COEFF != fSrcCoeff) {
        GrXPFactory* result = GrPorterDuffXPFactory::Create(fMode);
        SkASSERT(result);
        return result;
    }

    SkASSERT(GrCustomXfermode::IsSupportedMode(fMode));
    return GrCustomXfermode::CreateXPFactory(fMode);
}
#endif

const char* SkXfermode::ModeName(Mode mode) {
    SkASSERT((unsigned) mode <= (unsigned)kLastMode);
    const char* gModeStrings[] = {
        "Clear", "Src", "Dst", "SrcOver", "DstOver", "SrcIn", "DstIn",
        "SrcOut", "DstOut", "SrcATop", "DstATop", "Xor", "Plus",
        "Modulate", "Screen", "Overlay", "Darken", "Lighten", "ColorDodge",
        "ColorBurn", "HardLight", "SoftLight", "Difference", "Exclusion",
        "Multiply", "Hue", "Saturation", "Color",  "Luminosity"
    };
    return gModeStrings[mode];
    static_assert(SK_ARRAY_COUNT(gModeStrings) == kLastMode + 1, "mode_count");
}

#ifndef SK_IGNORE_TO_STRING
void SkProcCoeffXfermode::toString(SkString* str) const {
    str->append("SkProcCoeffXfermode: ");

    str->append("mode: ");
    str->append(ModeName(fMode));

    static const char* gCoeffStrings[kCoeffCount] = {
        "Zero", "One", "SC", "ISC", "DC", "IDC", "SA", "ISA", "DA", "IDA"
    };

    str->append(" src: ");
    if (CANNOT_USE_COEFF == fSrcCoeff) {
        str->append("can't use");
    } else {
        str->append(gCoeffStrings[fSrcCoeff]);
    }

    str->append(" dst: ");
    if (CANNOT_USE_COEFF == fDstCoeff) {
        str->append("can't use");
    } else {
        str->append(gCoeffStrings[fDstCoeff]);
    }
}
#endif


SK_DECLARE_STATIC_ONCE_PTR(SkXfermode, cached[SkXfermode::kLastMode + 1]);

sk_sp<SkXfermode> SkXfermode::Make(Mode mode) {
    SkASSERT(SK_ARRAY_COUNT(gProcCoeffs) == kModeCount);

    if ((unsigned)mode >= kModeCount) {
        // report error
        return nullptr;
    }

    // Skia's "default" mode is srcover. nullptr in SkPaint is interpreted as srcover
    // so we can just return nullptr from the factory.
    if (kSrcOver_Mode == mode) {
        return nullptr;
    }

    return sk_ref_sp(cached[mode].get([=]{
        ProcCoeff rec = gProcCoeffs[mode];
        if (auto xfermode = SkOpts::create_xfermode(rec, mode)) {
            return xfermode;
        }
        return (SkXfermode*) new SkProcCoeffXfermode(rec, mode);
    }));
}

SkXfermodeProc SkXfermode::GetProc(Mode mode) {
    SkXfermodeProc  proc = nullptr;
    if ((unsigned)mode < kModeCount) {
        proc = gProcCoeffs[mode].fProc;
    }
    return proc;
}

SkXfermodeProc4f SkXfermode::GetProc4f(Mode mode) {
    SkXfermodeProc4f  proc = nullptr;
    if ((unsigned)mode < kModeCount) {
        proc = gProcCoeffs[mode].fProc4f;
    }
    return proc;
}

static SkPM4f missing_proc4f(const SkPM4f& src, const SkPM4f& dst) {
    return src;
}

SkXfermodeProc4f SkXfermode::getProc4f() const {
    Mode mode;
    return this->asMode(&mode) ? GetProc4f(mode) : missing_proc4f;
}

bool SkXfermode::ModeAsCoeff(Mode mode, Coeff* src, Coeff* dst) {
    SkASSERT(SK_ARRAY_COUNT(gProcCoeffs) == kModeCount);

    if ((unsigned)mode >= (unsigned)kModeCount) {
        // illegal mode parameter
        return false;
    }

    const ProcCoeff& rec = gProcCoeffs[mode];

    if (CANNOT_USE_COEFF == rec.fSC) {
        return false;
    }

    SkASSERT(CANNOT_USE_COEFF != rec.fDC);
    if (src) {
        *src = rec.fSC;
    }
    if (dst) {
        *dst = rec.fDC;
    }
    return true;
}

bool SkXfermode::AsMode(const SkXfermode* xfer, Mode* mode) {
    if (nullptr == xfer) {
        if (mode) {
            *mode = kSrcOver_Mode;
        }
        return true;
    }
    return xfer->asMode(mode);
}

bool SkXfermode::IsMode(const SkXfermode* xfer, Mode mode) {
    // if xfer==null then the mode is srcover
    Mode m = kSrcOver_Mode;
    if (xfer && !xfer->asMode(&m)) {
        return false;
    }
    return mode == m;
}

bool SkXfermode::SupportsCoverageAsAlpha(const SkXfermode* xfer) {
    // if xfer is nullptr we treat it as srcOver which always supports coverageAsAlpha
    if (!xfer) {
        return true;
    }

    return xfer->supportsCoverageAsAlpha();
}

bool SkXfermode::IsOpaque(const SkXfermode* xfer, SrcColorOpacity opacityType) {
    // if xfer is nullptr we treat it as srcOver which is opaque if our src is opaque
    if (!xfer) {
        return SkXfermode::kOpaque_SrcColorOpacity == opacityType;
    }

    return xfer->isOpaque(opacityType);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkProcCoeffXfermode)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
