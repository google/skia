
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkMathPriv.h"
#include "SkString.h"

SK_DEFINE_INST_COUNT(SkXfermode)

#define SkAlphaMulAlpha(a, b)   SkMulDiv255Round(a, b)

#if 0
// idea for higher precision blends in xfer procs (and slightly faster)
// see DstATop as a probable caller
static U8CPU mulmuldiv255round(U8CPU a, U8CPU b, U8CPU c, U8CPU d) {
    SkASSERT(a <= 255);
    SkASSERT(b <= 255);
    SkASSERT(c <= 255);
    SkASSERT(d <= 255);
    unsigned prod = SkMulS16(a, b) + SkMulS16(c, d) + 128;
    unsigned result = (prod + (prod >> 8)) >> 8;
    SkASSERT(result <= 255);
    return result;
}
#endif

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

static inline int clamp_max(int value, int max) {
    if (value > max) {
        value = max;
    }
    return value;
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
static inline int exclusion_byte(int sc, int dc, int sa, int da) {
    // this equations is wacky, wait for SVG to confirm it
    int r = sc * da + dc * sa - 2 * sc * dc + sc * (255 - da) + dc * (255 - sa);
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
    if(n < 0) {
       *r = L + SkMulDiv(*r - L, L, L - n);
       *g = L + SkMulDiv(*g - L, L, L - n);
       *b = L + SkMulDiv(*b - L, L, L - n);
    }

    if (x > a) {
       *r = L + SkMulDiv(*r - L, a - L, x - L);
       *g = L + SkMulDiv(*g - L, a - L, x - L);
       *b = L + SkMulDiv(*b - L, a - L, x - L);
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


struct ProcCoeff {
    SkXfermodeProc      fProc;
    SkXfermode::Coeff   fSC;
    SkXfermode::Coeff   fDC;
};

#define CANNOT_USE_COEFF    SkXfermode::Coeff(-1)

static const ProcCoeff gProcCoeffs[] = {
    { clear_modeproc,   SkXfermode::kZero_Coeff,    SkXfermode::kZero_Coeff },
    { src_modeproc,     SkXfermode::kOne_Coeff,     SkXfermode::kZero_Coeff },
    { dst_modeproc,     SkXfermode::kZero_Coeff,    SkXfermode::kOne_Coeff },
    { srcover_modeproc, SkXfermode::kOne_Coeff,     SkXfermode::kISA_Coeff },
    { dstover_modeproc, SkXfermode::kIDA_Coeff,     SkXfermode::kOne_Coeff },
    { srcin_modeproc,   SkXfermode::kDA_Coeff,      SkXfermode::kZero_Coeff },
    { dstin_modeproc,   SkXfermode::kZero_Coeff,    SkXfermode::kSA_Coeff },
    { srcout_modeproc,  SkXfermode::kIDA_Coeff,     SkXfermode::kZero_Coeff },
    { dstout_modeproc,  SkXfermode::kZero_Coeff,    SkXfermode::kISA_Coeff },
    { srcatop_modeproc, SkXfermode::kDA_Coeff,      SkXfermode::kISA_Coeff },
    { dstatop_modeproc, SkXfermode::kIDA_Coeff,     SkXfermode::kSA_Coeff },
    { xor_modeproc,     SkXfermode::kIDA_Coeff,     SkXfermode::kISA_Coeff },

    { plus_modeproc,    SkXfermode::kOne_Coeff,     SkXfermode::kOne_Coeff },
    { modulate_modeproc,SkXfermode::kZero_Coeff,    SkXfermode::kSC_Coeff },
    { screen_modeproc,      CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { overlay_modeproc,     CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { darken_modeproc,      CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { lighten_modeproc,     CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { colordodge_modeproc,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { colorburn_modeproc,   CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { hardlight_modeproc,   CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { softlight_modeproc,   CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { difference_modeproc,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { exclusion_modeproc,   CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { multiply_modeproc,    CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { hue_modeproc,         CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { saturation_modeproc,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { color_modeproc,       CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
    { luminosity_modeproc,  CANNOT_USE_COEFF,       CANNOT_USE_COEFF },
};

///////////////////////////////////////////////////////////////////////////////

bool SkXfermode::asCoeff(Coeff* src, Coeff* dst) const {
    return false;
}

bool SkXfermode::asMode(Mode* mode) const {
    return false;
}

bool SkXfermode::asNewEffectOrCoeff(GrContext*, GrEffectRef**, Coeff* src, Coeff* dst) const {
    return this->asCoeff(src, dst);
}

bool SkXfermode::AsNewEffectOrCoeff(SkXfermode* xfermode,
                                    GrContext* context,
                                    GrEffectRef** effect,
                                    Coeff* src,
                                    Coeff* dst) {
    if (NULL == xfermode) {
        return ModeAsCoeff(kSrcOver_Mode, src, dst);
    } else {
        return xfermode->asNewEffectOrCoeff(context, effect, src, dst);
    }
}

SkPMColor SkXfermode::xferColor(SkPMColor src, SkPMColor dst) const{
    // no-op. subclasses should override this
    return dst;
}

void SkXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                        const SkPMColor* SK_RESTRICT src, int count,
                        const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (NULL == aa) {
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

    if (NULL == aa) {
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

void SkXfermode::xfer4444(SkPMColor16* SK_RESTRICT dst,
                          const SkPMColor* SK_RESTRICT src, int count,
                          const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (NULL == aa) {
        for (int i = count - 1; i >= 0; --i) {
            SkPMColor dstC = SkPixel4444ToPixel32(dst[i]);
            dst[i] = SkPixel32ToPixel4444(this->xferColor(src[i], dstC));
        }
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0 != a) {
                SkPMColor dstC = SkPixel4444ToPixel32(dst[i]);
                SkPMColor C = this->xferColor(src[i], dstC);
                if (0xFF != a) {
                    C = SkFourByteInterp(C, dstC, a);
                }
                dst[i] = SkPixel32ToPixel4444(C);
            }
        }
    }
}

void SkXfermode::xferA8(SkAlpha* SK_RESTRICT dst,
                        const SkPMColor src[], int count,
                        const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (NULL == aa) {
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

///////////////////////////////////////////////////////////////////////////////

void SkProcXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                            const SkPMColor* SK_RESTRICT src, int count,
                            const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (NULL != proc) {
        if (NULL == aa) {
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

void SkProcXfermode::xfer16(uint16_t* SK_RESTRICT dst,
                            const SkPMColor* SK_RESTRICT src, int count,
                            const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (NULL != proc) {
        if (NULL == aa) {
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

void SkProcXfermode::xfer4444(SkPMColor16* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src, int count,
                              const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (NULL != proc) {
        if (NULL == aa) {
            for (int i = count - 1; i >= 0; --i) {
                SkPMColor dstC = SkPixel4444ToPixel32(dst[i]);
                dst[i] = SkPixel32ToPixel4444(proc(src[i], dstC));
            }
        } else {
            for (int i = count - 1; i >= 0; --i) {
                unsigned a = aa[i];
                if (0 != a) {
                    SkPMColor dstC = SkPixel4444ToPixel32(dst[i]);
                    SkPMColor C = proc(src[i], dstC);
                    if (0xFF != a) {
                        C = SkFourByteInterp(C, dstC, a);
                    }
                    dst[i] = SkPixel32ToPixel4444(C);
                }
            }
        }
    }
}

void SkProcXfermode::xferA8(SkAlpha* SK_RESTRICT dst,
                            const SkPMColor* SK_RESTRICT src, int count,
                            const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (NULL != proc) {
        if (NULL == aa) {
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

SkProcXfermode::SkProcXfermode(SkFlattenableReadBuffer& buffer)
        : SkXfermode(buffer) {
    fProc = NULL;
    if (!buffer.isCrossProcess()) {
        fProc = (SkXfermodeProc)buffer.readFunctionPtr();
    }
}

void SkProcXfermode::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    if (!buffer.isCrossProcess()) {
        buffer.writeFunctionPtr((void*)fProc);
    }
}

#ifdef SK_DEVELOPER
void SkProcXfermode::toString(SkString* str) const {
    str->appendf("SkProcXfermode: %p", fProc);
}
#endif

//////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrEffect.h"
#include "GrEffectUnitTest.h"
#include "GrTBackendEffectFactory.h"
#include "gl/GrGLEffect.h"

/**
 * GrEffect that implements the kDarken_Mode Xfermode. It requires access to the dst pixel color
 * in the shader. TODO: Make this work for all non-Coeff SkXfermode::Mode values.
 */
class DarkenEffect : public GrEffect {
public:
    static GrEffectRef* Create() {
        static AutoEffectUnref gEffect(SkNEW(DarkenEffect));
        return CreateEffectRef(gEffect);
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<DarkenEffect>::getInstance();
    }

    static const char* Name() { return "XfermodeDarken"; }

    class GLEffect : public GrGLEffect {
    public:
        GLEffect(const GrBackendEffectFactory& factory, const GrDrawEffect&)
            : GrGLEffect(factory ) {
        }
        virtual void emitCode(GrGLShaderBuilder* builder,
                              const GrDrawEffect& drawEffect,
                              EffectKey key,
                              const char* outputColor,
                              const char* inputColor,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            const char* dstColorName = builder->dstColor();
            GrAssert(NULL != dstColorName);
            if (NULL == inputColor) {
                // the input color is solid white
                builder->fsCodeAppendf("\t\t%s.a = 1.0;\n", outputColor);
                builder->fsCodeAppendf("\t\t%s.rgb = vec3(1.0, 1.0, 1.0) - %s.aaa + %s.rgb;\n",
                                       outputColor, dstColorName, dstColorName);
            } else {
                builder->fsCodeAppendf("\t\t%s.a = 1.0 - (1.0 - %s.a) * (1.0 - %s.a);\n",
                                       outputColor, dstColorName, inputColor);
                builder->fsCodeAppendf("\t\t%s.rgb = min((1.0 - %s.a) * %s.rgb + %s.rgb,"
                                                       " (1.0 - %s.a) * %s.rgb + %s.rgb);\n",
                                       outputColor,
                                       inputColor, dstColorName, inputColor,
                                       dstColorName, inputColor, dstColorName);
            }
        }

        static inline EffectKey GenKey(const GrDrawEffect&, const GrGLCaps&) { return 0; }

        virtual void setData(const GrGLUniformManager&, const GrDrawEffect&) SK_OVERRIDE {}

    private:
        typedef GrGLEffect INHERITED;
    };

    GR_DECLARE_EFFECT_TEST;

private:
    DarkenEffect() { this->setWillReadDst(); }
    virtual bool onIsEqual(const GrEffect& other) const SK_OVERRIDE { return true; }

    typedef GrEffect INHERITED;
};

GR_DEFINE_EFFECT_TEST(DarkenEffect);
GrEffectRef* DarkenEffect::TestCreate(SkMWCRandom*,
                                      GrContext*,
                                      const GrDrawTargetCaps&,
                                      GrTexture*[]) {
    static AutoEffectUnref gEffect(SkNEW(DarkenEffect));
    return CreateEffectRef(gEffect);
}

#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class SkProcCoeffXfermode : public SkProcXfermode {
public:
    SkProcCoeffXfermode(const ProcCoeff& rec, Mode mode)
            : INHERITED(rec.fProc) {
        fMode = mode;
        // these may be valid, or may be CANNOT_USE_COEFF
        fSrcCoeff = rec.fSC;
        fDstCoeff = rec.fDC;
    }

    virtual bool asMode(Mode* mode) const SK_OVERRIDE {
        if (mode) {
            *mode = fMode;
        }
        return true;
    }

    virtual bool asCoeff(Coeff* sc, Coeff* dc) const SK_OVERRIDE {
        if (CANNOT_USE_COEFF == fSrcCoeff) {
            return false;
        }

        if (sc) {
            *sc = fSrcCoeff;
        }
        if (dc) {
            *dc = fDstCoeff;
        }
        return true;
    }

#if SK_SUPPORT_GPU
    virtual bool asNewEffectOrCoeff(GrContext*,
                                    GrEffectRef** effect,
                                    Coeff* src,
                                    Coeff* dst) const SK_OVERRIDE {
        if (this->asCoeff(src, dst)) {
            return true;
        }
        if (kDarken_Mode == fMode) {
            if (NULL != effect) {
                *effect = DarkenEffect::Create();
            }
            return true;
        }
        return false;
    }
#endif

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcCoeffXfermode)

protected:
    SkProcCoeffXfermode(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fMode = (SkXfermode::Mode)buffer.read32();

        const ProcCoeff& rec = gProcCoeffs[fMode];
        // these may be valid, or may be CANNOT_USE_COEFF
        fSrcCoeff = rec.fSC;
        fDstCoeff = rec.fDC;
        // now update our function-ptr in the super class
        this->INHERITED::setProc(rec.fProc);
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE {
        this->INHERITED::flatten(buffer);
        buffer.write32(fMode);
    }

private:
    Mode    fMode;
    Coeff   fSrcCoeff, fDstCoeff;

    typedef SkProcXfermode INHERITED;
};

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
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(gModeStrings) == kLastMode + 1, mode_count);
}

#ifdef SK_DEVELOPER
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

///////////////////////////////////////////////////////////////////////////////

class SkClearXfermode : public SkProcCoeffXfermode {
public:
    SkClearXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kClear_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;
    virtual void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkClearXfermode)

private:
    SkClearXfermode(SkFlattenableReadBuffer& buffer)
        : SkProcCoeffXfermode(buffer) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkClearXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT, int count,
                             const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && count >= 0);

    if (NULL == aa) {
        memset(dst, 0, count << 2);
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0xFF == a) {
                dst[i] = 0;
            } else if (a != 0) {
                dst[i] = SkAlphaMulQ(dst[i], SkAlpha255To256(255 - a));
            }
        }
    }
}
void SkClearXfermode::xferA8(SkAlpha* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT, int count,
                             const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && count >= 0);

    if (NULL == aa) {
        memset(dst, 0, count);
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0xFF == a) {
                dst[i] = 0;
            } else if (0 != a) {
                dst[i] = SkAlphaMulAlpha(dst[i], 255 - a);
            }
        }
    }
}

#ifdef SK_DEVELOPER
void SkClearXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class SkSrcXfermode : public SkProcCoeffXfermode {
public:
    SkSrcXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kSrc_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;
    virtual void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSrcXfermode)

private:
    SkSrcXfermode(SkFlattenableReadBuffer& buffer)
        : SkProcCoeffXfermode(buffer) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkSrcXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src, int count,
                           const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (NULL == aa) {
        memcpy(dst, src, count << 2);
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (a == 0xFF) {
                dst[i] = src[i];
            } else if (a != 0) {
                dst[i] = SkFourByteInterp(src[i], dst[i], a);
            }
        }
    }
}

void SkSrcXfermode::xferA8(SkAlpha* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src, int count,
                           const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    if (NULL == aa) {
        for (int i = count - 1; i >= 0; --i) {
            dst[i] = SkToU8(SkGetPackedA32(src[i]));
        }
    } else {
        for (int i = count - 1; i >= 0; --i) {
            unsigned a = aa[i];
            if (0 != a) {
                unsigned srcA = SkGetPackedA32(src[i]);
                if (a == 0xFF) {
                    dst[i] = SkToU8(srcA);
                } else {
                    dst[i] = SkToU8(SkAlphaBlend(srcA, dst[i], a));
                }
            }
        }
    }
}
#ifdef SK_DEVELOPER
void SkSrcXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class SkDstInXfermode : public SkProcCoeffXfermode {
public:
    SkDstInXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstIn_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDstInXfermode)

private:
    SkDstInXfermode(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkDstInXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT src, int count,
                             const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src);

    if (count <= 0) {
        return;
    }
    if (NULL != aa) {
        return this->INHERITED::xfer32(dst, src, count, aa);
    }

    do {
        unsigned a = SkGetPackedA32(*src);
        *dst = SkAlphaMulQ(*dst, SkAlpha255To256(a));
        dst++;
        src++;
    } while (--count != 0);
}

#ifdef SK_DEVELOPER
void SkDstInXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class SkDstOutXfermode : public SkProcCoeffXfermode {
public:
    SkDstOutXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstOut_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDstOutXfermode)

private:
    SkDstOutXfermode(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkDstOutXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src, int count,
                              const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src);

    if (count <= 0) {
        return;
    }
    if (NULL != aa) {
        return this->INHERITED::xfer32(dst, src, count, aa);
    }

    do {
        unsigned a = SkGetPackedA32(*src);
        *dst = SkAlphaMulQ(*dst, SkAlpha255To256(255 - a));
        dst++;
        src++;
    } while (--count != 0);
}

#ifdef SK_DEVELOPER
void SkDstOutXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

SkXfermode* SkXfermode::Create(Mode mode) {
    SkASSERT(SK_ARRAY_COUNT(gProcCoeffs) == kModeCount);
    SkASSERT((unsigned)mode < kModeCount);

    const ProcCoeff& rec = gProcCoeffs[mode];

    switch (mode) {
        case kClear_Mode:
            return SkNEW_ARGS(SkClearXfermode, (rec));
        case kSrc_Mode:
            return SkNEW_ARGS(SkSrcXfermode, (rec));
        case kSrcOver_Mode:
            return NULL;
        case kDstIn_Mode:
            return SkNEW_ARGS(SkDstInXfermode, (rec));
        case kDstOut_Mode:
            return SkNEW_ARGS(SkDstOutXfermode, (rec));
        default:
            return SkNEW_ARGS(SkProcCoeffXfermode, (rec, mode));
    }
}

SkXfermodeProc SkXfermode::GetProc(Mode mode) {
    SkXfermodeProc  proc = NULL;
    if ((unsigned)mode < kModeCount) {
        proc = gProcCoeffs[mode].fProc;
    }
    return proc;
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
    if (NULL == xfer) {
        if (mode) {
            *mode = kSrcOver_Mode;
        }
        return true;
    }
    return xfer->asMode(mode);
}

bool SkXfermode::AsCoeff(const SkXfermode* xfer, Coeff* src, Coeff* dst) {
    if (NULL == xfer) {
        return ModeAsCoeff(kSrcOver_Mode, src, dst);
    }
    return xfer->asCoeff(src, dst);
}

bool SkXfermode::IsMode(const SkXfermode* xfer, Mode mode) {
    // if xfer==null then the mode is srcover
    Mode m = kSrcOver_Mode;
    if (xfer && !xfer->asMode(&m)) {
        return false;
    }
    return mode == m;
}

///////////////////////////////////////////////////////////////////////////////
//////////// 16bit xfermode procs

#ifdef SK_DEBUG
static bool require_255(SkPMColor src) { return SkGetPackedA32(src) == 0xFF; }
static bool require_0(SkPMColor src) { return SkGetPackedA32(src) == 0; }
#endif

static uint16_t src_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    return SkPixel32ToPixel16(src);
}

static uint16_t dst_modeproc16(SkPMColor src, uint16_t dst) {
    return dst;
}

static uint16_t srcover_modeproc16_0(SkPMColor src, uint16_t dst) {
    SkASSERT(require_0(src));
    return dst;
}

static uint16_t srcover_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    return SkPixel32ToPixel16(src);
}

static uint16_t dstover_modeproc16_0(SkPMColor src, uint16_t dst) {
    SkASSERT(require_0(src));
    return dst;
}

static uint16_t dstover_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    return dst;
}

static uint16_t srcin_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    return SkPixel32ToPixel16(src);
}

static uint16_t dstin_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    return dst;
}

static uint16_t dstout_modeproc16_0(SkPMColor src, uint16_t dst) {
    SkASSERT(require_0(src));
    return dst;
}

static uint16_t srcatop_modeproc16(SkPMColor src, uint16_t dst) {
    unsigned isa = 255 - SkGetPackedA32(src);

    return SkPackRGB16(
           SkPacked32ToR16(src) + SkAlphaMulAlpha(SkGetPackedR16(dst), isa),
           SkPacked32ToG16(src) + SkAlphaMulAlpha(SkGetPackedG16(dst), isa),
           SkPacked32ToB16(src) + SkAlphaMulAlpha(SkGetPackedB16(dst), isa));
}

static uint16_t srcatop_modeproc16_0(SkPMColor src, uint16_t dst) {
    SkASSERT(require_0(src));
    return dst;
}

static uint16_t srcatop_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    return SkPixel32ToPixel16(src);
}

static uint16_t dstatop_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    return dst;
}

/*********
    darken and lighten boil down to this.

    darken  = (1 - Sa) * Dc + min(Sc, Dc)
    lighten = (1 - Sa) * Dc + max(Sc, Dc)

    if (Sa == 0) these become
        darken  = Dc + min(0, Dc) = 0
        lighten = Dc + max(0, Dc) = Dc

    if (Sa == 1) these become
        darken  = min(Sc, Dc)
        lighten = max(Sc, Dc)
*/

static uint16_t darken_modeproc16_0(SkPMColor src, uint16_t dst) {
    SkASSERT(require_0(src));
    return 0;
}

static uint16_t darken_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    unsigned r = SkFastMin32(SkPacked32ToR16(src), SkGetPackedR16(dst));
    unsigned g = SkFastMin32(SkPacked32ToG16(src), SkGetPackedG16(dst));
    unsigned b = SkFastMin32(SkPacked32ToB16(src), SkGetPackedB16(dst));
    return SkPackRGB16(r, g, b);
}

static uint16_t lighten_modeproc16_0(SkPMColor src, uint16_t dst) {
    SkASSERT(require_0(src));
    return dst;
}

static uint16_t lighten_modeproc16_255(SkPMColor src, uint16_t dst) {
    SkASSERT(require_255(src));
    unsigned r = SkMax32(SkPacked32ToR16(src), SkGetPackedR16(dst));
    unsigned g = SkMax32(SkPacked32ToG16(src), SkGetPackedG16(dst));
    unsigned b = SkMax32(SkPacked32ToB16(src), SkGetPackedB16(dst));
    return SkPackRGB16(r, g, b);
}

struct Proc16Rec {
    SkXfermodeProc16    fProc16_0;
    SkXfermodeProc16    fProc16_255;
    SkXfermodeProc16    fProc16_General;
};

static const Proc16Rec gModeProcs16[] = {
    { NULL,                 NULL,                   NULL            }, // CLEAR
    { NULL,                 src_modeproc16_255,     NULL            },
    { dst_modeproc16,       dst_modeproc16,         dst_modeproc16  },
    { srcover_modeproc16_0, srcover_modeproc16_255, NULL            },
    { dstover_modeproc16_0, dstover_modeproc16_255, NULL            },
    { NULL,                 srcin_modeproc16_255,   NULL            },
    { NULL,                 dstin_modeproc16_255,   NULL            },
    { NULL,                 NULL,                   NULL            },// SRC_OUT
    { dstout_modeproc16_0,  NULL,                   NULL            },
    { srcatop_modeproc16_0, srcatop_modeproc16_255, srcatop_modeproc16  },
    { NULL,                 dstatop_modeproc16_255, NULL            },
    { NULL,                 NULL,                   NULL            }, // XOR

    { NULL,                 NULL,                   NULL            }, // plus
    { NULL,                 NULL,                   NULL            }, // modulate
    { NULL,                 NULL,                   NULL            }, // screen
    { NULL,                 NULL,                   NULL            }, // overlay
    { darken_modeproc16_0,  darken_modeproc16_255,  NULL            }, // darken
    { lighten_modeproc16_0, lighten_modeproc16_255, NULL            }, // lighten
    { NULL,                 NULL,                   NULL            }, // colordodge
    { NULL,                 NULL,                   NULL            }, // colorburn
    { NULL,                 NULL,                   NULL            }, // hardlight
    { NULL,                 NULL,                   NULL            }, // softlight
    { NULL,                 NULL,                   NULL            }, // difference
    { NULL,                 NULL,                   NULL            }, // exclusion
    { NULL,                 NULL,                   NULL            }, // multiply
    { NULL,                 NULL,                   NULL            }, // hue
    { NULL,                 NULL,                   NULL            }, // saturation
    { NULL,                 NULL,                   NULL            }, // color
    { NULL,                 NULL,                   NULL            }, // luminosity
};

SkXfermodeProc16 SkXfermode::GetProc16(Mode mode, SkColor srcColor) {
    SkXfermodeProc16  proc16 = NULL;
    if ((unsigned)mode < kModeCount) {
        const Proc16Rec& rec = gModeProcs16[mode];
        unsigned a = SkColorGetA(srcColor);

        if (0 == a) {
            proc16 = rec.fProc16_0;
        } else if (255 == a) {
            proc16 = rec.fProc16_255;
        } else {
            proc16 = rec.fProc16_General;
        }
    }
    return proc16;
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkProcCoeffXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkClearXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkSrcXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDstInXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkDstOutXfermode)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
