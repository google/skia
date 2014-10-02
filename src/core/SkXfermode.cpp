
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkXfermode.h"
#include "SkXfermode_opts_SSE2.h"
#include "SkXfermode_proccoeff.h"
#include "SkColorPriv.h"
#include "SkLazyPtr.h"
#include "SkMathPriv.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkUtilsArm.h"
#include "SkWriteBuffer.h"

#if !SK_ARM_NEON_IS_NONE
#include "SkXfermode_opts_arm_neon.h"
#endif

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

bool SkXfermode::asCoeff(Coeff* src, Coeff* dst) const {
    return false;
}

bool SkXfermode::asMode(Mode* mode) const {
    return false;
}

bool SkXfermode::asFragmentProcessor(GrFragmentProcessor**, GrTexture*) const {
    return false;
}

bool SkXfermode::asFragmentProcessorOrCoeff(SkXfermode* xfermode, GrFragmentProcessor** fp,
                                            Coeff* src, Coeff* dst, GrTexture* background) {
    if (NULL == xfermode) {
        return ModeAsCoeff(kSrcOver_Mode, src, dst);
    } else if (xfermode->asCoeff(src, dst)) {
        return true;
    } else {
        return xfermode->asFragmentProcessor(fp, background);
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

//////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU

#include "GrProcessor.h"
#include "GrCoordTransform.h"
#include "GrProcessorUnitTest.h"
#include "GrTBackendProcessorFactory.h"
#include "gl/GrGLProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

/**
 * GrProcessor that implements the all the separable xfer modes that cannot be expressed as Coeffs.
 */
class XferEffect : public GrFragmentProcessor {
public:
    static bool IsSupportedMode(SkXfermode::Mode mode) {
        return mode > SkXfermode::kLastCoeffMode && mode <= SkXfermode::kLastMode;
    }

    static GrFragmentProcessor* Create(SkXfermode::Mode mode, GrTexture* background) {
        if (!IsSupportedMode(mode)) {
            return NULL;
        } else {
            return SkNEW_ARGS(XferEffect, (mode, background));
        }
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        *validFlags = 0;
    }

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendFragmentProcessorFactory<XferEffect>::getInstance();
    }

    static const char* Name() { return "XferEffect"; }

    SkXfermode::Mode mode() const { return fMode; }
    const GrTextureAccess&  backgroundAccess() const { return fBackgroundAccess; }

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        GLProcessor(const GrBackendProcessorFactory& factory, const GrProcessor&)
            : INHERITED(factory) {
        }
        virtual void emitCode(GrGLProgramBuilder* builder,
                              const GrFragmentProcessor& fp,
                              const GrProcessorKey& key,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray& coords,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            SkXfermode::Mode mode = fp.cast<XferEffect>().mode();
            const GrTexture* backgroundTex =
                    fp.cast<XferEffect>().backgroundAccess().getTexture();
            GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
            const char* dstColor;
            if (backgroundTex) {
                dstColor = "bgColor";
                fsBuilder->codeAppendf("\t\tvec4 %s = ", dstColor);
                fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].getType());
                fsBuilder->codeAppendf(";\n");
            } else {
                dstColor = fsBuilder->dstColor();
            }
            SkASSERT(dstColor);

            // We don't try to optimize for this case at all
            if (NULL == inputColor) {
                fsBuilder->codeAppendf("\t\tconst vec4 ones = vec4(1);\n");
                inputColor = "ones";
            }
            fsBuilder->codeAppendf("\t\t// SkXfermode::Mode: %s\n", SkXfermode::ModeName(mode));

            // These all perform src-over on the alpha channel.
            fsBuilder->codeAppendf("\t\t%s.a = %s.a + (1.0 - %s.a) * %s.a;\n",
                                    outputColor, inputColor, inputColor, dstColor);

            switch (mode) {
                case SkXfermode::kOverlay_Mode:
                    // Overlay is Hard-Light with the src and dst reversed
                    HardLight(fsBuilder, outputColor, dstColor, inputColor);
                    break;
                case SkXfermode::kDarken_Mode:
                    fsBuilder->codeAppendf("\t\t%s.rgb = min((1.0 - %s.a) * %s.rgb + %s.rgb, "
                                                            "(1.0 - %s.a) * %s.rgb + %s.rgb);\n",
                                            outputColor,
                                            inputColor, dstColor, inputColor,
                                            dstColor, inputColor, dstColor);
                    break;
                case SkXfermode::kLighten_Mode:
                    fsBuilder->codeAppendf("\t\t%s.rgb = max((1.0 - %s.a) * %s.rgb + %s.rgb, "
                                                            "(1.0 - %s.a) * %s.rgb + %s.rgb);\n",
                                            outputColor,
                                            inputColor, dstColor, inputColor,
                                            dstColor, inputColor, dstColor);
                    break;
                case SkXfermode::kColorDodge_Mode:
                    ColorDodgeComponent(fsBuilder, outputColor, inputColor, dstColor, 'r');
                    ColorDodgeComponent(fsBuilder, outputColor, inputColor, dstColor, 'g');
                    ColorDodgeComponent(fsBuilder, outputColor, inputColor, dstColor, 'b');
                    break;
                case SkXfermode::kColorBurn_Mode:
                    ColorBurnComponent(fsBuilder, outputColor, inputColor, dstColor, 'r');
                    ColorBurnComponent(fsBuilder, outputColor, inputColor, dstColor, 'g');
                    ColorBurnComponent(fsBuilder, outputColor, inputColor, dstColor, 'b');
                    break;
                case SkXfermode::kHardLight_Mode:
                    HardLight(fsBuilder, outputColor, inputColor, dstColor);
                    break;
                case SkXfermode::kSoftLight_Mode:
                    fsBuilder->codeAppendf("\t\tif (0.0 == %s.a) {\n", dstColor);
                    fsBuilder->codeAppendf("\t\t\t%s.rgba = %s;\n", outputColor, inputColor);
                    fsBuilder->codeAppendf("\t\t} else {\n");
                    SoftLightComponentPosDstAlpha(fsBuilder, outputColor, inputColor, dstColor, 'r');
                    SoftLightComponentPosDstAlpha(fsBuilder, outputColor, inputColor, dstColor, 'g');
                    SoftLightComponentPosDstAlpha(fsBuilder, outputColor, inputColor, dstColor, 'b');
                    fsBuilder->codeAppendf("\t\t}\n");
                    break;
                case SkXfermode::kDifference_Mode:
                    fsBuilder->codeAppendf("\t\t%s.rgb = %s.rgb + %s.rgb -"
                                                       "2.0 * min(%s.rgb * %s.a, %s.rgb * %s.a);\n",
                                           outputColor, inputColor, dstColor, inputColor, dstColor,
                                           dstColor, inputColor);
                    break;
                case SkXfermode::kExclusion_Mode:
                    fsBuilder->codeAppendf("\t\t%s.rgb = %s.rgb + %s.rgb - "
                                                        "2.0 * %s.rgb * %s.rgb;\n",
                                           outputColor, dstColor, inputColor, dstColor, inputColor);
                    break;
                case SkXfermode::kMultiply_Mode:
                    fsBuilder->codeAppendf("\t\t%s.rgb = (1.0 - %s.a) * %s.rgb + "
                                                        "(1.0 - %s.a) * %s.rgb + "
                                                         "%s.rgb * %s.rgb;\n",
                                           outputColor, inputColor, dstColor, dstColor, inputColor,
                                           inputColor, dstColor);
                    break;
                case SkXfermode::kHue_Mode: {
                    //  SetLum(SetSat(S * Da, Sat(D * Sa)), Sa*Da, D*Sa) + (1 - Sa) * D + (1 - Da) * S
                    SkString setSat, setLum;
                    AddSatFunction(fsBuilder, &setSat);
                    AddLumFunction(fsBuilder, &setLum);
                    fsBuilder->codeAppendf("\t\tvec4 dstSrcAlpha = %s * %s.a;\n",
                                           dstColor, inputColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb = %s(%s(%s.rgb * %s.a, dstSrcAlpha.rgb), dstSrcAlpha.a, dstSrcAlpha.rgb);\n",
                                           outputColor, setLum.c_str(), setSat.c_str(), inputColor,
                                           dstColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;\n",
                                           outputColor, inputColor, dstColor, dstColor, inputColor);
                    break;
                }
                case SkXfermode::kSaturation_Mode: {
                    // SetLum(SetSat(D * Sa, Sat(S * Da)), Sa*Da, D*Sa)) + (1 - Sa) * D + (1 - Da) * S
                    SkString setSat, setLum;
                    AddSatFunction(fsBuilder, &setSat);
                    AddLumFunction(fsBuilder, &setLum);
                    fsBuilder->codeAppendf("\t\tvec4 dstSrcAlpha = %s * %s.a;\n",
                                           dstColor, inputColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb = %s(%s(dstSrcAlpha.rgb, %s.rgb * %s.a), dstSrcAlpha.a, dstSrcAlpha.rgb);\n",
                                           outputColor, setLum.c_str(), setSat.c_str(), inputColor,
                                           dstColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;\n",
                                           outputColor, inputColor, dstColor, dstColor, inputColor);
                    break;
                }
                case SkXfermode::kColor_Mode: {
                    //  SetLum(S * Da, Sa* Da, D * Sa) + (1 - Sa) * D + (1 - Da) * S
                    SkString setLum;
                    AddLumFunction(fsBuilder, &setLum);
                    fsBuilder->codeAppendf("\t\tvec4 srcDstAlpha = %s * %s.a;\n",
                                           inputColor, dstColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb = %s(srcDstAlpha.rgb, srcDstAlpha.a, %s.rgb * %s.a);\n",
                                           outputColor, setLum.c_str(), dstColor, inputColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;\n",
                                           outputColor, inputColor, dstColor, dstColor, inputColor);
                    break;
                }
                case SkXfermode::kLuminosity_Mode: {
                    //  SetLum(D * Sa, Sa* Da, S * Da) + (1 - Sa) * D + (1 - Da) * S
                    SkString setLum;
                    AddLumFunction(fsBuilder, &setLum);
                    fsBuilder->codeAppendf("\t\tvec4 srcDstAlpha = %s * %s.a;\n",
                                           inputColor, dstColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb = %s(%s.rgb * %s.a, srcDstAlpha.a, srcDstAlpha.rgb);\n",
                                           outputColor, setLum.c_str(), dstColor, inputColor);
                    fsBuilder->codeAppendf("\t\t%s.rgb += (1.0 - %s.a) * %s.rgb + (1.0 - %s.a) * %s.rgb;\n",
                                           outputColor, inputColor, dstColor, dstColor, inputColor);
                    break;
                }
                default:
                    SkFAIL("Unknown XferEffect mode.");
                    break;
            }
        }

        static inline void GenKey(const GrProcessor& proc, const GrGLCaps&,
                                  GrProcessorKeyBuilder* b) {
            // The background may come from the dst or from a texture.
            uint32_t key = proc.numTextures();
            SkASSERT(key <= 1);
            key |= proc.cast<XferEffect>().mode() << 1;
            b->add32(key);
        }

    private:
        static void HardLight(GrGLFragmentShaderBuilder* fsBuilder,
                              const char* final,
                              const char* src,
                              const char* dst) {
            static const char kComponents[] = {'r', 'g', 'b'};
            for (size_t i = 0; i < SK_ARRAY_COUNT(kComponents); ++i) {
                char component = kComponents[i];
                fsBuilder->codeAppendf("\t\tif (2.0 * %s.%c <= %s.a) {\n", src, component, src);
                fsBuilder->codeAppendf("\t\t\t%s.%c = 2.0 * %s.%c * %s.%c;\n", final, component, src, component, dst, component);
                fsBuilder->codeAppend("\t\t} else {\n");
                fsBuilder->codeAppendf("\t\t\t%s.%c = %s.a * %s.a - 2.0 * (%s.a - %s.%c) * (%s.a - %s.%c);\n",
                                       final, component, src, dst, dst, dst, component, src, src, component);
                fsBuilder->codeAppend("\t\t}\n");
            }
            fsBuilder->codeAppendf("\t\t%s.rgb += %s.rgb * (1.0 - %s.a) + %s.rgb * (1.0 - %s.a);\n",
                                   final, src, dst, dst, src);
        }

        // Does one component of color-dodge
        static void ColorDodgeComponent(GrGLFragmentShaderBuilder* fsBuilder,
                                        const char* final,
                                        const char* src,
                                        const char* dst,
                                        const char component) {
            fsBuilder->codeAppendf("\t\tif (0.0 == %s.%c) {\n", dst, component);
            fsBuilder->codeAppendf("\t\t\t%s.%c = %s.%c * (1.0 - %s.a);\n",
                                   final, component, src, component, dst);
            fsBuilder->codeAppend("\t\t} else {\n");
            fsBuilder->codeAppendf("\t\t\tfloat d = %s.a - %s.%c;\n", src, src, component);
            fsBuilder->codeAppend("\t\t\tif (0.0 == d) {\n");
            fsBuilder->codeAppendf("\t\t\t\t%s.%c = %s.a * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);\n",
                                   final, component, src, dst, src, component, dst, dst, component,
                                   src);
            fsBuilder->codeAppend("\t\t\t} else {\n");
            fsBuilder->codeAppendf("\t\t\t\td = min(%s.a, %s.%c * %s.a / d);\n",
                                   dst, dst, component, src);
            fsBuilder->codeAppendf("\t\t\t\t%s.%c = d * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);\n",
                                   final, component, src, src, component, dst, dst, component, src);
            fsBuilder->codeAppend("\t\t\t}\n");
            fsBuilder->codeAppend("\t\t}\n");
        }

        // Does one component of color-burn
        static void ColorBurnComponent(GrGLFragmentShaderBuilder* fsBuilder,
                                       const char* final,
                                       const char* src,
                                       const char* dst,
                                       const char component) {
            fsBuilder->codeAppendf("\t\tif (%s.a == %s.%c) {\n", dst, dst, component);
            fsBuilder->codeAppendf("\t\t\t%s.%c = %s.a * %s.a + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);\n",
                                   final, component, src, dst, src, component, dst, dst, component,
                                   src);
            fsBuilder->codeAppendf("\t\t} else if (0.0 == %s.%c) {\n", src, component);
            fsBuilder->codeAppendf("\t\t\t%s.%c = %s.%c * (1.0 - %s.a);\n",
                                   final, component, dst, component, src);
            fsBuilder->codeAppend("\t\t} else {\n");
            fsBuilder->codeAppendf("\t\t\tfloat d = max(0.0, %s.a - (%s.a - %s.%c) * %s.a / %s.%c);\n",
                                   dst, dst, dst, component, src, src, component);
            fsBuilder->codeAppendf("\t\t\t%s.%c = %s.a * d + %s.%c * (1.0 - %s.a) + %s.%c * (1.0 - %s.a);\n",
                                   final, component, src, src, component, dst, dst, component, src);
            fsBuilder->codeAppend("\t\t}\n");
        }

        // Does one component of soft-light. Caller should have already checked that dst alpha > 0.
        static void SoftLightComponentPosDstAlpha(GrGLFragmentShaderBuilder* fsBuilder,
                                                  const char* final,
                                                  const char* src,
                                                  const char* dst,
                                                  const char component) {
            // if (2S < Sa)
            fsBuilder->codeAppendf("\t\t\tif (2.0 * %s.%c <= %s.a) {\n", src, component, src);
            // (D^2 (Sa-2 S))/Da+(1-Da) S+D (-Sa+2 S+1)
            fsBuilder->codeAppendf("\t\t\t\t%s.%c = (%s.%c*%s.%c*(%s.a - 2.0*%s.%c)) / %s.a + (1.0 - %s.a) * %s.%c + %s.%c*(-%s.a + 2.0*%s.%c + 1.0);\n",
                                   final, component, dst, component, dst, component, src, src,
                                   component, dst, dst, src, component, dst, component, src, src,
                                   component);
            // else if (4D < Da)
            fsBuilder->codeAppendf("\t\t\t} else if (4.0 * %s.%c <= %s.a) {\n",
                                   dst, component, dst);
            fsBuilder->codeAppendf("\t\t\t\tfloat DSqd = %s.%c * %s.%c;\n",
                                   dst, component, dst, component);
            fsBuilder->codeAppendf("\t\t\t\tfloat DCub = DSqd * %s.%c;\n", dst, component);
            fsBuilder->codeAppendf("\t\t\t\tfloat DaSqd = %s.a * %s.a;\n", dst, dst);
            fsBuilder->codeAppendf("\t\t\t\tfloat DaCub = DaSqd * %s.a;\n", dst);
            // (Da^3 (-S)+Da^2 (S-D (3 Sa-6 S-1))+12 Da D^2 (Sa-2 S)-16 D^3 (Sa-2 S))/Da^2
            fsBuilder->codeAppendf("\t\t\t\t%s.%c = (-DaCub*%s.%c + DaSqd*(%s.%c - %s.%c * (3.0*%s.a - 6.0*%s.%c - 1.0)) + 12.0*%s.a*DSqd*(%s.a - 2.0*%s.%c) - 16.0*DCub * (%s.a - 2.0*%s.%c)) / DaSqd;\n",
                                   final, component, src, component, src, component, dst, component,
                                   src, src, component, dst, src, src, component, src, src,
                                   component);
            fsBuilder->codeAppendf("\t\t\t} else {\n");
            // -sqrt(Da * D) (Sa-2 S)-Da S+D (Sa-2 S+1)+S
            fsBuilder->codeAppendf("\t\t\t\t%s.%c = -sqrt(%s.a*%s.%c)*(%s.a - 2.0*%s.%c) - %s.a*%s.%c + %s.%c*(%s.a - 2.0*%s.%c + 1.0) + %s.%c;\n",
                                    final, component, dst, dst, component, src, src, component, dst,
                                    src, component, dst, component, src, src, component, src,
                                    component);
            fsBuilder->codeAppendf("\t\t\t}\n");
        }

        // Adds a function that takes two colors and an alpha as input. It produces a color with the
        // hue and saturation of the first color, the luminosity of the second color, and the input
        // alpha. It has this signature:
        //      vec3 set_luminance(vec3 hueSatColor, float alpha, vec3 lumColor).
        static void AddLumFunction(GrGLFragmentShaderBuilder* fsBuilder, SkString* setLumFunction) {
            // Emit a helper that gets the luminance of a color.
            SkString getFunction;
            GrGLShaderVar getLumArgs[] = {
                GrGLShaderVar("color", kVec3f_GrSLType),
            };
            SkString getLumBody("\treturn dot(vec3(0.3, 0.59, 0.11), color);\n");
            fsBuilder->emitFunction(kFloat_GrSLType,
                                    "luminance",
                                    SK_ARRAY_COUNT(getLumArgs), getLumArgs,
                                    getLumBody.c_str(),
                                    &getFunction);

            // Emit the set luminance function.
            GrGLShaderVar setLumArgs[] = {
                GrGLShaderVar("hueSat", kVec3f_GrSLType),
                GrGLShaderVar("alpha", kFloat_GrSLType),
                GrGLShaderVar("lumColor", kVec3f_GrSLType),
            };
            SkString setLumBody;
            setLumBody.printf("\tfloat diff = %s(lumColor - hueSat);\n", getFunction.c_str());
            setLumBody.append("\tvec3 outColor = hueSat + diff;\n");
            setLumBody.appendf("\tfloat outLum = %s(outColor);\n", getFunction.c_str());
            setLumBody.append("\tfloat minComp = min(min(outColor.r, outColor.g), outColor.b);\n"
                              "\tfloat maxComp = max(max(outColor.r, outColor.g), outColor.b);\n"
                              "\tif (minComp < 0.0) {\n"
                              "\t\toutColor = outLum + ((outColor - vec3(outLum, outLum, outLum)) * outLum) / (outLum - minComp);\n"
                              "\t}\n"
                              "\tif (maxComp > alpha) {\n"
                              "\t\toutColor = outLum + ((outColor - vec3(outLum, outLum, outLum)) * (alpha - outLum)) / (maxComp - outLum);\n"
                              "\t}\n"
                              "\treturn outColor;\n");
            fsBuilder->emitFunction(kVec3f_GrSLType,
                                    "set_luminance",
                                    SK_ARRAY_COUNT(setLumArgs), setLumArgs,
                                    setLumBody.c_str(),
                                    setLumFunction);
        }

        // Adds a function that creates a color with the hue and luminosity of one input color and
        // the saturation of another color. It will have this signature:
        //      float set_saturation(vec3 hueLumColor, vec3 satColor)
        static void AddSatFunction(GrGLFragmentShaderBuilder* fsBuilder, SkString* setSatFunction) {
            // Emit a helper that gets the saturation of a color
            SkString getFunction;
            GrGLShaderVar getSatArgs[] = { GrGLShaderVar("color", kVec3f_GrSLType) };
            SkString getSatBody;
            getSatBody.printf("\treturn max(max(color.r, color.g), color.b) - "
                              "min(min(color.r, color.g), color.b);\n");
            fsBuilder->emitFunction(kFloat_GrSLType,
                                    "saturation",
                                    SK_ARRAY_COUNT(getSatArgs), getSatArgs,
                                    getSatBody.c_str(),
                                    &getFunction);

            // Emit a helper that sets the saturation given sorted input channels. This used
            // to use inout params for min, mid, and max components but that seems to cause
            // problems on PowerVR drivers. So instead it returns a vec3 where r, g ,b are the
            // adjusted min, mid, and max inputs, respectively.
            SkString helperFunction;
            GrGLShaderVar helperArgs[] = {
                GrGLShaderVar("minComp", kFloat_GrSLType),
                GrGLShaderVar("midComp", kFloat_GrSLType),
                GrGLShaderVar("maxComp", kFloat_GrSLType),
                GrGLShaderVar("sat", kFloat_GrSLType),
            };
            static const char kHelperBody[] = "\tif (minComp < maxComp) {\n"
                                              "\t\tvec3 result;\n"
                                              "\t\tresult.r = 0.0;\n"
                                              "\t\tresult.g = sat * (midComp - minComp) / (maxComp - minComp);\n"
                                              "\t\tresult.b = sat;\n"
                                              "\t\treturn result;\n"
                                              "\t} else {\n"
                                              "\t\treturn vec3(0, 0, 0);\n"
                                              "\t}\n";
            fsBuilder->emitFunction(kVec3f_GrSLType,
                                    "set_saturation_helper",
                                    SK_ARRAY_COUNT(helperArgs), helperArgs,
                                    kHelperBody,
                                    &helperFunction);

            GrGLShaderVar setSatArgs[] = {
                GrGLShaderVar("hueLumColor", kVec3f_GrSLType),
                GrGLShaderVar("satColor", kVec3f_GrSLType),
            };
            const char* helpFunc = helperFunction.c_str();
            SkString setSatBody;
            setSatBody.appendf("\tfloat sat = %s(satColor);\n"
                               "\tif (hueLumColor.r <= hueLumColor.g) {\n"
                               "\t\tif (hueLumColor.g <= hueLumColor.b) {\n"
                               "\t\t\thueLumColor.rgb = %s(hueLumColor.r, hueLumColor.g, hueLumColor.b, sat);\n"
                               "\t\t} else if (hueLumColor.r <= hueLumColor.b) {\n"
                               "\t\t\thueLumColor.rbg = %s(hueLumColor.r, hueLumColor.b, hueLumColor.g, sat);\n"
                               "\t\t} else {\n"
                               "\t\t\thueLumColor.brg = %s(hueLumColor.b, hueLumColor.r, hueLumColor.g, sat);\n"
                               "\t\t}\n"
                               "\t} else if (hueLumColor.r <= hueLumColor.b) {\n"
                               "\t\thueLumColor.grb = %s(hueLumColor.g, hueLumColor.r, hueLumColor.b, sat);\n"
                               "\t} else if (hueLumColor.g <= hueLumColor.b) {\n"
                               "\t\thueLumColor.gbr = %s(hueLumColor.g, hueLumColor.b, hueLumColor.r, sat);\n"
                               "\t} else {\n"
                               "\t\thueLumColor.bgr = %s(hueLumColor.b, hueLumColor.g, hueLumColor.r, sat);\n"
                               "\t}\n"
                               "\treturn hueLumColor;\n",
                               getFunction.c_str(), helpFunc, helpFunc, helpFunc, helpFunc,
                               helpFunc, helpFunc);
            fsBuilder->emitFunction(kVec3f_GrSLType,
                                    "set_saturation",
                                    SK_ARRAY_COUNT(setSatArgs), setSatArgs,
                                    setSatBody.c_str(),
                                    setSatFunction);

        }

        typedef GrGLFragmentProcessor INHERITED;
    };

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

private:
    XferEffect(SkXfermode::Mode mode, GrTexture* background)
        : fMode(mode) {
        if (background) {
            fBackgroundTransform.reset(kLocal_GrCoordSet, background);
            this->addCoordTransform(&fBackgroundTransform);
            fBackgroundAccess.reset(background);
            this->addTextureAccess(&fBackgroundAccess);
        } else {
            this->setWillReadDstColor();
        }
    }
    virtual bool onIsEqual(const GrProcessor& other) const SK_OVERRIDE {
        const XferEffect& s = other.cast<XferEffect>();
        return fMode == s.fMode &&
               fBackgroundAccess.getTexture() == s.fBackgroundAccess.getTexture();
    }

    SkXfermode::Mode fMode;
    GrCoordTransform fBackgroundTransform;
    GrTextureAccess  fBackgroundAccess;

    typedef GrFragmentProcessor INHERITED;
};

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(XferEffect);
GrFragmentProcessor* XferEffect::TestCreate(SkRandom* rand,
                                            GrContext*,
                                            const GrDrawTargetCaps&,
                                            GrTexture*[]) {
    int mode = rand->nextRangeU(SkXfermode::kLastCoeffMode + 1, SkXfermode::kLastSeparableMode);

    return SkNEW_ARGS(XferEffect, (static_cast<SkXfermode::Mode>(mode), NULL));
}

#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
SkProcCoeffXfermode::SkProcCoeffXfermode(SkReadBuffer& buffer) : INHERITED(buffer) {
    uint32_t mode32 = buffer.read32() % SK_ARRAY_COUNT(gProcCoeffs);
    if (mode32 >= SK_ARRAY_COUNT(gProcCoeffs)) {
        // out of range, just set to something harmless
        mode32 = SkXfermode::kSrcOut_Mode;
    }
    fMode = (SkXfermode::Mode)mode32;

    const ProcCoeff& rec = gProcCoeffs[fMode];
    fProc = rec.fProc;
    // these may be valid, or may be CANNOT_USE_COEFF
    fSrcCoeff = rec.fSC;
    fDstCoeff = rec.fDC;
}
#endif

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

bool SkProcCoeffXfermode::asCoeff(Coeff* sc, Coeff* dc) const {
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
bool SkProcCoeffXfermode::asFragmentProcessor(GrFragmentProcessor** fp,
                                              GrTexture* background) const {
    if (XferEffect::IsSupportedMode(fMode)) {
        if (fp) {
            *fp = XferEffect::Create(fMode, background);
            SkASSERT(*fp);
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

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;
    virtual void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()

private:
    SkClearXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kClear_Mode) {}
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkClearXfermode(SkReadBuffer& buffer) : SkProcCoeffXfermode(buffer) {}
#endif

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

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;
    virtual void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()

private:
    SkSrcXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kSrc_Mode) {}
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkSrcXfermode(SkReadBuffer& buffer) : SkProcCoeffXfermode(buffer) {}
#endif
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

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()

private:
    SkDstInXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstIn_Mode) {}
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkDstInXfermode(SkReadBuffer& buffer) : INHERITED(buffer) {}
#endif

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

class SkDstOutXfermode : public SkProcCoeffXfermode {
public:
    static SkDstOutXfermode* Create(const ProcCoeff& rec) {
        return SkNEW_ARGS(SkDstOutXfermode, (rec));
    }

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()

private:
    SkDstOutXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstOut_Mode) {}
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkDstOutXfermode(SkReadBuffer& buffer) : INHERITED(buffer) {}
#endif

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

    SK_DECLARE_STATIC_LAZY_PTR_ARRAY(SkXfermode, cached, kModeCount, create_mode);
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
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
