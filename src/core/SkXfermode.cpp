
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkXfermode.h"
#include "SkXfermode_opts_SSE2.h"
#include "SkXfermode_proccoeff.h"
#include "Sk4px.h"
#include "SkColorPriv.h"
#include "SkLazyPtr.h"
#include "SkMathPriv.h"
#include "SkPMFloat.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkUtilsArm.h"
#include "SkWriteBuffer.h"

// When implemented, the Sk4f and Sk4px xfermodes beat src/opts/SkXfermodes_opts_SSE2's.
// When implemented, the Sk4px, but not Sk4f, xfermodes beat src/opts/SkXfermodes_arm_neon's.
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #define SK_4F_XFERMODES_ARE_FAST
    #define SK_4PX_XFERMODES_ARE_FAST
#elif defined(SK_ARM_HAS_NEON)
    #define SK_4PX_XFERMODES_ARE_FAST
#endif

#if !SK_ARM_NEON_IS_NONE
    #include "SkXfermode_opts_arm_neon.h"
#endif

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

const ProcCoeff gProcCoeffs[] = {
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
    { screen_modeproc,  SkXfermode::kOne_Coeff,     SkXfermode::kISC_Coeff },
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

bool SkXfermode::asMode(Mode* mode) const {
    return false;
}

bool SkXfermode::asFragmentProcessor(GrFragmentProcessor**, GrTexture*) const {
    return false;
}

bool SkXfermode::asXPFactory(GrXPFactory**) const {
    return false;
}


#if SK_SUPPORT_GPU
#include "effects/GrPorterDuffXferProcessor.h"

bool SkXfermode::AsXPFactory(SkXfermode* xfermode, GrXPFactory** xpf) {
    if (NULL == xfermode) {
        if (xpf) {
            *xpf = GrPorterDuffXPFactory::Create(kSrcOver_Mode);
        }
        return true;
    } else {
        return xfermode->asXPFactory(xpf);
    }
}
#else
bool SkXfermode::AsXPFactory(SkXfermode* xfermode, GrXPFactory** xpf) {
    return false;
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

bool SkXfermode::supportsCoverageAsAlpha() const {
    return false;
}

bool SkXfermode::isOpaque(SkXfermode::SrcColorOpacity opacityType) const {
    return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

SkFlattenable* SkProcCoeffXfermode::CreateProc(SkReadBuffer& buffer) {
    uint32_t mode32 = buffer.read32();
    if (!buffer.validate(mode32 < SK_ARRAY_COUNT(gProcCoeffs))) {
        return NULL;
    }
    return SkXfermode::Create((SkXfermode::Mode)mode32);
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

void SkProcCoeffXfermode::xfer16(uint16_t* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src, int count,
                                 const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (proc) {
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

void SkProcCoeffXfermode::xferA8(SkAlpha* SK_RESTRICT dst,
                                 const SkPMColor* SK_RESTRICT src, int count,
                                 const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;

    if (proc) {
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

#if SK_SUPPORT_GPU
#include "effects/GrCustomXfermode.h"

bool SkProcCoeffXfermode::asFragmentProcessor(GrFragmentProcessor** fp,
                                              GrTexture* background) const {
    if (GrCustomXfermode::IsSupportedMode(fMode)) {
        if (fp) {
            *fp = GrCustomXfermode::CreateFP(fMode, background);
            SkASSERT(*fp);
        }
        return true;
    }
    return false;
}

bool SkProcCoeffXfermode::asXPFactory(GrXPFactory** xp) const {
    if (CANNOT_USE_COEFF != fSrcCoeff) {
        if (xp) {
            *xp = GrPorterDuffXPFactory::Create(fMode);
            SkASSERT(*xp);
        }
        return true;
    }

    if (GrCustomXfermode::IsSupportedMode(fMode)) {
        if (xp) {
            *xp = GrCustomXfermode::CreateXPFactory(fMode);
            SkASSERT(*xp);
        }
        return true;
    }
    return false;
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
    SK_COMPILE_ASSERT(SK_ARRAY_COUNT(gModeStrings) == kLastMode + 1, mode_count);
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

///////////////////////////////////////////////////////////////////////////////

class SkClearXfermode : public SkProcCoeffXfermode {
public:
    static SkClearXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkClearXfermode, (rec));
    }

    void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const override;
    void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) const override;

    SK_TO_STRING_OVERRIDE()

private:
    SkClearXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kClear_Mode) {}

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

#ifndef SK_IGNORE_TO_STRING
void SkClearXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class SkSrcXfermode : public SkProcCoeffXfermode {
public:
    static SkSrcXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkSrcXfermode, (rec));
    }

    void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const override;
    void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) const override;

    SK_TO_STRING_OVERRIDE()

private:
    SkSrcXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kSrc_Mode) {}
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
#ifndef SK_IGNORE_TO_STRING
void SkSrcXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

class SkDstInXfermode : public SkProcCoeffXfermode {
public:
    static SkDstInXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkDstInXfermode, (rec));
    }

    void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const override;

    SK_TO_STRING_OVERRIDE()

private:
    SkDstInXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstIn_Mode) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkDstInXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT src, int count,
                             const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src);

    if (count <= 0) {
        return;
    }
    if (aa) {
        return this->INHERITED::xfer32(dst, src, count, aa);
    }

    do {
        unsigned a = SkGetPackedA32(*src);
        *dst = SkAlphaMulQ(*dst, SkAlpha255To256(a));
        dst++;
        src++;
    } while (--count != 0);
}

#ifndef SK_IGNORE_TO_STRING
void SkDstInXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

/* These modes can merge coverage into src-alpha
 *
{ dst_modeproc,     SkXfermode::kZero_Coeff,    SkXfermode::kOne_Coeff },
{ srcover_modeproc, SkXfermode::kOne_Coeff,     SkXfermode::kISA_Coeff },
{ dstover_modeproc, SkXfermode::kIDA_Coeff,     SkXfermode::kOne_Coeff },
{ dstout_modeproc,  SkXfermode::kZero_Coeff,    SkXfermode::kISA_Coeff },
{ srcatop_modeproc, SkXfermode::kDA_Coeff,      SkXfermode::kISA_Coeff },
{ xor_modeproc,     SkXfermode::kIDA_Coeff,     SkXfermode::kISA_Coeff },
{ plus_modeproc,    SkXfermode::kOne_Coeff,     SkXfermode::kOne_Coeff },
{ screen_modeproc,  SkXfermode::kOne_Coeff,     SkXfermode::kISC_Coeff },
*/

static const float gInv255 = 0.0039215683f; //  (1.0f / 255) - ULP == SkBits2Float(0x3B808080)

static Sk4f ramp(const Sk4f& v0, const Sk4f& v1, const Sk4f& t) {
    return v0 + (v1 - v0) * t;
}

static Sk4f clamp_255(const Sk4f& value) {
    return Sk4f::Min(Sk4f(255), value);
}

static Sk4f clamp_0_255(const Sk4f& value) {
    return Sk4f::Max(Sk4f(0), Sk4f::Min(Sk4f(255), value));
}

/**
 *  Some modes can, due to very slight numerical error, generate "invalid" pmcolors...
 *
 *  e.g.
 *      alpha = 100.9999
 *      red   = 101
 *
 *  or
 *      alpha = 255.0001
 *
 *  If we know we're going to write-out the values as bytes, we can relax these somewhat,
 *  since we only really need to enforce that the bytes are valid premul...
 *
 *  To that end, this method asserts that the resulting pmcolor will be valid, but does not call
 *  SkPMFloat::isValid(), as that would fire sometimes, but not result in a bad pixel.
 */
static inline SkPMFloat check_as_pmfloat(const Sk4f& value) {
    SkPMFloat pm = value;
#ifdef SK_DEBUG
    (void)pm.round();
#endif
    return pm;
}

//  kSrcATop_Mode,  //!< [Da, Sc * Da + (1 - Sa) * Dc]
struct SrcATop4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        const Sk4f inv255(gInv255);
        return check_as_pmfloat(dst + (src * Sk4f(dst.a()) - dst * Sk4f(src.a())) * inv255);
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        return Sk4px::Wide(src.mulWiden(dst.alphas()) + dst.mulWiden(src.alphas().inv()))
            .div255RoundNarrow();
    }
    static const bool kFoldCoverageIntoSrcAlpha = true;
    static const SkXfermode::Mode kMode = SkXfermode::kSrcATop_Mode;
};

//  kDstATop_Mode,  //!< [Sa, Sa * Dc + Sc * (1 - Da)]
struct DstATop4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        return SrcATop4f::Xfer(dst, src);
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        return SrcATop4f::Xfer(dst, src);
    }
    static const bool kFoldCoverageIntoSrcAlpha = false;
    static const SkXfermode::Mode kMode = SkXfermode::kDstATop_Mode;
};

//  kXor_Mode   [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + (1 - Sa) * Dc]
struct Xor4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        const Sk4f inv255(gInv255);
        return check_as_pmfloat(src + dst - (src * Sk4f(dst.a()) + dst * Sk4f(src.a())) * inv255);
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        return Sk4px::Wide(src.mulWiden(dst.alphas().inv()) + dst.mulWiden(src.alphas().inv()))
            .div255RoundNarrow();
    }
    static const bool kFoldCoverageIntoSrcAlpha = true;
    static const SkXfermode::Mode kMode = SkXfermode::kXor_Mode;
};

//  kPlus_Mode   [Sa + Da, Sc + Dc]
struct Plus4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        return check_as_pmfloat(clamp_255(src + dst));
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        return src.saturatedAdd(dst);
    }
    static const bool kFoldCoverageIntoSrcAlpha = false;
    static const SkXfermode::Mode kMode = SkXfermode::kPlus_Mode;
};

//  kModulate_Mode   [Sa * Da, Sc * Dc]
struct Modulate4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        const Sk4f inv255(gInv255);
        return check_as_pmfloat(src * dst * inv255);
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        return src.mulWiden(dst).div255RoundNarrow();
    }
    static const bool kFoldCoverageIntoSrcAlpha = false;
    static const SkXfermode::Mode kMode = SkXfermode::kModulate_Mode;
};

//  kScreen_Mode   [S + D - S * D]
struct Screen4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        const Sk4f inv255(gInv255);
        return check_as_pmfloat(src + dst - src * dst * inv255);
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        // Doing the math as S + (1-S)*D or S + (D - S*D) means the add and subtract can be done
        // in 8-bit space without overflow.  S + (1-S)*D is a touch faster because inv() is cheap.
        return src + src.inv().mulWiden(dst).div255RoundNarrow();
    }
    static const bool kFoldCoverageIntoSrcAlpha = true;
    static const SkXfermode::Mode kMode = SkXfermode::kScreen_Mode;
};

struct Multiply4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        const Sk4f inv255(gInv255);
        Sk4f sa = Sk4f(src.a());
        Sk4f da = Sk4f(dst.a());
        Sk4f sc = src;
        Sk4f dc = dst;
        Sk4f rc = sc + dc + (sc * (dc - da) - dc * sa) * inv255;
        // ra = srcover(sa, da), but the calc for rc happens to accomplish this for us
        return check_as_pmfloat(clamp_0_255(rc));
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        return Sk4px::Wide(src.mulWiden(dst.alphas().inv()) +
                           dst.mulWiden(src.alphas().inv()) +
                           src.mulWiden(dst))
            .div255RoundNarrow();
    }
    static const bool kFoldCoverageIntoSrcAlpha = false;
    static const SkXfermode::Mode kMode = SkXfermode::kMultiply_Mode;
};

// [ sa + da - sa*da, sc + dc - 2*min(sc*da, dc*sa) ]  (And notice sa*da == min(sa*da, da*sa).)
struct Difference4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        const Sk4f inv255(gInv255);
        Sk4f sa = Sk4f(src.a());
        Sk4f da = Sk4f(dst.a());
        Sk4f sc = src;
        Sk4f dc = dst;
        Sk4f min = Sk4f::Min(sc * da, dc * sa) * inv255;
        Sk4f ra = sc + dc - min;
        return check_as_pmfloat(ra - min * SkPMFloat(0, 1, 1, 1));
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        auto m = Sk4px::Wide(Sk16h::Min(src.mulWiden(dst.alphas()), dst.mulWiden(src.alphas())))
            .div255RoundNarrow();
        // There's no chance of underflow, and if we subtract m before adding src+dst, no overflow.
        return (src - m) + (dst - m.zeroAlphas());
    }
    static const bool kFoldCoverageIntoSrcAlpha = false;
    static const SkXfermode::Mode kMode = SkXfermode::kDifference_Mode;
};

// [ sa + da - sa*da, sc + dc - 2*sc*dc ]
struct Exclusion4f {
    static SkPMFloat Xfer(const SkPMFloat& src, const SkPMFloat& dst) {
        const Sk4f inv255(gInv255);
        Sk4f sc = src;
        Sk4f dc = dst;
        Sk4f prod = sc * dc * inv255;
        Sk4f ra = sc + dc - prod;
        return check_as_pmfloat(ra - prod * SkPMFloat(0, 1, 1, 1));
    }
    static Sk4px Xfer(const Sk4px& src, const Sk4px& dst) {
        auto p = src.mulWiden(dst).div255RoundNarrow();
        // There's no chance of underflow, and if we subtract p before adding src+dst, no overflow.
        return (src - p) + (dst - p.zeroAlphas());
    }
    static const bool kFoldCoverageIntoSrcAlpha = false;
    static const SkXfermode::Mode kMode = SkXfermode::kExclusion_Mode;
};

template <typename ProcType>
class SkT4fXfermode : public SkProcCoeffXfermode {
public:
    static SkXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkT4fXfermode, (rec));
    }

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        if (NULL == aa) {
            for (int i = 0; i < n; ++i) {
                dst[i] = ProcType::Xfer(SkPMFloat(src[i]), SkPMFloat(dst[i])).round();
            }
        } else {
            for (int i = 0; i < n; ++i) {
                const Sk4f aa4 = Sk4f(aa[i] * gInv255);
                SkPMFloat dstF(dst[i]);
                SkPMFloat srcF(src[i]);
                Sk4f res;
                if (ProcType::kFoldCoverageIntoSrcAlpha) {
                    Sk4f src4 = srcF;
                    res = ProcType::Xfer(src4 * aa4, dstF);
                } else {
                    res = ramp(dstF, ProcType::Xfer(srcF, dstF), aa4);
                }
                dst[i] = SkPMFloat(res).round();
            }
        }
    }

private:
    SkT4fXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, ProcType::kMode) {}

    typedef SkProcCoeffXfermode INHERITED;
};

template <typename ProcType>
class SkT4pxXfermode : public SkProcCoeffXfermode {
public:
    static SkXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkT4pxXfermode, (rec));
    }

    void xfer32(SkPMColor dst[], const SkPMColor src[], int n, const SkAlpha aa[]) const override {
        if (NULL == aa) {
            Sk4px::MapDstSrc(n, dst, src, [&](const Sk4px& dst4, const Sk4px& src4) {
                return ProcType::Xfer(src4, dst4);
            });
        } else {
            Sk4px::MapDstSrcAlpha(n, dst, src, aa,
                    [&](const Sk4px& dst4, const Sk4px& src4, const Sk16b& alpha) {
                // We can't exploit kFoldCoverageIntoSrcAlpha. That requires >=24-bit intermediates.
                Sk4px res4 = ProcType::Xfer(src4, dst4);
                return Sk4px::Wide(res4.mulWiden(alpha) + dst4.mulWiden(Sk4px(alpha).inv()))
                           .div255RoundNarrow();
            });
        }
    }

private:
    SkT4pxXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, ProcType::kMode) {}

    typedef SkProcCoeffXfermode INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class SkDstOutXfermode : public SkProcCoeffXfermode {
public:
    static SkDstOutXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkDstOutXfermode, (rec));
    }

    void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const override;

    SK_TO_STRING_OVERRIDE()

private:
    SkDstOutXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstOut_Mode) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkDstOutXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src, int count,
                              const SkAlpha* SK_RESTRICT aa) const {
    SkASSERT(dst && src);

    if (count <= 0) {
        return;
    }
    if (aa) {
        return this->INHERITED::xfer32(dst, src, count, aa);
    }

    do {
        unsigned a = SkGetPackedA32(*src);
        *dst = SkAlphaMulQ(*dst, SkAlpha255To256(255 - a));
        dst++;
        src++;
    } while (--count != 0);
}

#ifndef SK_IGNORE_TO_STRING
void SkDstOutXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

extern SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec, SkXfermode::Mode mode);
extern SkXfermodeProc SkPlatformXfermodeProcFactory(SkXfermode::Mode mode);

// Technically, can't be static and passed as a template parameter.  So we use anonymous namespace.
namespace {
SkXfermode* create_mode(int iMode) {
    SkXfermode::Mode mode = (SkXfermode::Mode)iMode;

    ProcCoeff rec = gProcCoeffs[mode];
    SkXfermodeProc pp = SkPlatformXfermodeProcFactory(mode);
    if (pp != NULL) {
        rec.fProc = pp;
    }

#if defined(SK_4PX_XFERMODES_ARE_FAST) && !defined(SK_PREFER_LEGACY_FLOAT_XFERMODES)
    switch (mode) {
        case SkXfermode::kSrcATop_Mode:    return SkT4pxXfermode<SrcATop4f>::Create(rec);
        case SkXfermode::kDstATop_Mode:    return SkT4pxXfermode<DstATop4f>::Create(rec);
        case SkXfermode::kXor_Mode:        return SkT4pxXfermode<Xor4f>::Create(rec);
        case SkXfermode::kPlus_Mode:       return SkT4pxXfermode<Plus4f>::Create(rec);
        case SkXfermode::kModulate_Mode:   return SkT4pxXfermode<Modulate4f>::Create(rec);
        case SkXfermode::kScreen_Mode:     return SkT4pxXfermode<Screen4f>::Create(rec);
        case SkXfermode::kMultiply_Mode:   return SkT4pxXfermode<Multiply4f>::Create(rec);
        case SkXfermode::kDifference_Mode: return SkT4pxXfermode<Difference4f>::Create(rec);
        case SkXfermode::kExclusion_Mode:  return SkT4pxXfermode<Exclusion4f>::Create(rec);
        default: break;
    }
#endif

#if defined(SK_4F_XFERMODES_ARE_FAST)
    switch (mode) {
        case SkXfermode::kSrcATop_Mode:    return SkT4fXfermode<SrcATop4f>::Create(rec);
        case SkXfermode::kDstATop_Mode:    return SkT4fXfermode<DstATop4f>::Create(rec);
        case SkXfermode::kXor_Mode:        return SkT4fXfermode<Xor4f>::Create(rec);
        case SkXfermode::kPlus_Mode:       return SkT4fXfermode<Plus4f>::Create(rec);
        case SkXfermode::kModulate_Mode:   return SkT4fXfermode<Modulate4f>::Create(rec);
        case SkXfermode::kScreen_Mode:     return SkT4fXfermode<Screen4f>::Create(rec);
        case SkXfermode::kMultiply_Mode:   return SkT4fXfermode<Multiply4f>::Create(rec);
        case SkXfermode::kDifference_Mode: return SkT4fXfermode<Difference4f>::Create(rec);
        case SkXfermode::kExclusion_Mode:  return SkT4fXfermode<Exclusion4f>::Create(rec);
        default: break;
    }
#endif

    SkXfermode* xfer = NULL;

    // check if we have a platform optim for that
    SkProcCoeffXfermode* xfm = SkPlatformXfermodeFactory(rec, mode);
    if (xfm != NULL) {
        xfer = xfm;
    } else {
        // All modes can in theory be represented by the ProcCoeff rec, since
        // it contains function ptrs. However, a few modes are both simple and
        // commonly used, so we call those out for their own subclasses here.
        switch (mode) {
            case SkXfermode::kClear_Mode:
                xfer = SkClearXfermode::Create(rec);
                break;
            case SkXfermode::kSrc_Mode:
                xfer = SkSrcXfermode::Create(rec);
                break;
            case SkXfermode::kSrcOver_Mode:
                SkASSERT(false);    // should not land here
                break;
            case SkXfermode::kDstIn_Mode:
                xfer = SkDstInXfermode::Create(rec);
                break;
            case SkXfermode::kDstOut_Mode:
                xfer = SkDstOutXfermode::Create(rec);
                break;
            default:
                // no special-case, just rely in the rec and its function-ptrs
                xfer = SkNEW_ARGS(SkProcCoeffXfermode, (rec, mode));
                break;
        }
    }
    return xfer;
}
}  // namespace

SK_DECLARE_STATIC_LAZY_PTR_ARRAY(SkXfermode, cached, SkXfermode::kLastMode + 1, create_mode);

SkXfermode* SkXfermode::Create(Mode mode) {
    SkASSERT(SK_ARRAY_COUNT(gProcCoeffs) == kModeCount);

    if ((unsigned)mode >= kModeCount) {
        // report error
        return NULL;
    }

    // Skia's "default" mode is srcover. NULL in SkPaint is interpreted as srcover
    // so we can just return NULL from the factory.
    if (kSrcOver_Mode == mode) {
        return NULL;
    }

    return SkSafeRef(cached[mode]);
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

bool SkXfermode::IsMode(const SkXfermode* xfer, Mode mode) {
    // if xfer==null then the mode is srcover
    Mode m = kSrcOver_Mode;
    if (xfer && !xfer->asMode(&m)) {
        return false;
    }
    return mode == m;
}

bool SkXfermode::SupportsCoverageAsAlpha(const SkXfermode* xfer) {
    // if xfer is NULL we treat it as srcOver which always supports coverageAsAlpha
    if (!xfer) {
        return true;
    }

    return xfer->supportsCoverageAsAlpha();
}

bool SkXfermode::IsOpaque(const SkXfermode* xfer, SrcColorOpacity opacityType) {
    // if xfer is NULL we treat it as srcOver which is opaque if our src is opaque
    if (!xfer) {
        return SkXfermode::kOpaque_SrcColorOpacity == opacityType;
    }

    return xfer->isOpaque(opacityType);
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
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
