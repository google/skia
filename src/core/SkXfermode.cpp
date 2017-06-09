/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlendModePriv.h"
#include "SkXfermode_proccoeff.h"
#include "SkColorPriv.h"
#include "SkMathPriv.h"
#include "SkOnce.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkWriteBuffer.h"
#include "SkPM4f.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "effects/GrCustomXfermode.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
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

///////////////////////////////////////////////////////////////////////////////////////////////////

const SkXfermodeProc gProcs[] = {
    clear_modeproc,
    src_modeproc,
    dst_modeproc,
    srcover_modeproc,
    dstover_modeproc,
    srcin_modeproc,
    dstin_modeproc,
    srcout_modeproc,
    dstout_modeproc,
    srcatop_modeproc,
    dstatop_modeproc,
    xor_modeproc,

    plus_modeproc,
    modulate_modeproc,
    screen_modeproc,
    overlay_modeproc,
    darken_modeproc,
    lighten_modeproc,
    colordodge_modeproc,
    colorburn_modeproc,
    hardlight_modeproc,
    softlight_modeproc,
    difference_modeproc,
    exclusion_modeproc,
    multiply_modeproc,
    hue_modeproc,
    saturation_modeproc,
    color_modeproc,
    luminosity_modeproc,
};

bool SkXfermode::asMode(SkBlendMode* mode) const {
    return false;
}

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkXfermode::makeFragmentProcessorForImageFilter(
                                                                sk_sp<GrFragmentProcessor>) const {
    // This should never be called.
    // TODO: make pure virtual in SkXfermode once Android update lands
    SkASSERT(0);
    return nullptr;
}

const GrXPFactory* SkXfermode::asXPFactory() const {
    // This should never be called.
    // TODO: make pure virtual in SkXfermode once Android update lands
    SkASSERT(0);
    return nullptr;
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

sk_sp<SkFlattenable> SkProcCoeffXfermode::CreateProc(SkReadBuffer& buffer) {
    uint32_t mode32 = buffer.read32();
    if (!buffer.validate(mode32 < SK_ARRAY_COUNT(gProcs))) {
        return nullptr;
    }
    return SkXfermode::Make((SkBlendMode)mode32);
}

void SkProcCoeffXfermode::flatten(SkWriteBuffer& buffer) const {
    buffer.write32((int)fMode);
}

bool SkProcCoeffXfermode::asMode(SkBlendMode* mode) const {
    if (mode) {
        *mode = fMode;
    }
    return true;
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

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkProcCoeffXfermode::makeFragmentProcessorForImageFilter(
                                                            sk_sp<GrFragmentProcessor> dst) const {
    SkASSERT(dst);
    return GrXfermodeFragmentProcessor::MakeFromDstProcessor(std::move(dst), fMode);
}

const GrXPFactory* SkProcCoeffXfermode::asXPFactory() const {
    if (SkBlendMode_AsCoeff(fMode, nullptr, nullptr)) {
        const GrXPFactory* result(GrPorterDuffXPFactory::Get(fMode));
        SkASSERT(result);
        return result;
    }

    SkASSERT(GrCustomXfermode::IsSupportedMode(fMode));
    return GrCustomXfermode::Get(fMode);
}
#endif

const char* SkBlendMode_Name(SkBlendMode mode) {
    SkASSERT((unsigned) mode <= (unsigned)SkBlendMode::kLastMode);
    const char* gModeStrings[] = {
        "Clear", "Src", "Dst", "SrcOver", "DstOver", "SrcIn", "DstIn",
        "SrcOut", "DstOut", "SrcATop", "DstATop", "Xor", "Plus",
        "Modulate", "Screen", "Overlay", "Darken", "Lighten", "ColorDodge",
        "ColorBurn", "HardLight", "SoftLight", "Difference", "Exclusion",
        "Multiply", "Hue", "Saturation", "Color",  "Luminosity"
    };
    return gModeStrings[(int)mode];
    static_assert(SK_ARRAY_COUNT(gModeStrings) == (size_t)SkBlendMode::kLastMode + 1, "mode_count");
}

#ifndef SK_IGNORE_TO_STRING
void SkProcCoeffXfermode::toString(SkString* str) const {
    str->append("SkProcCoeffXfermode: ");

    str->append("mode: ");
    str->append(SkBlendMode_Name(fMode));
}
#endif


sk_sp<SkXfermode> SkXfermode::Make(SkBlendMode mode) {
    if ((unsigned)mode > (unsigned)SkBlendMode::kLastMode) {
        // report error
        return nullptr;
    }

    // Skia's "default" mode is srcover. nullptr in SkPaint is interpreted as srcover
    // so we can just return nullptr from the factory.
    if (SkBlendMode::kSrcOver == mode) {
        return nullptr;
    }

    const int COUNT_BLENDMODES = (int)SkBlendMode::kLastMode + 1;
    SkASSERT(SK_ARRAY_COUNT(gProcs) == COUNT_BLENDMODES);

    static SkOnce        once[COUNT_BLENDMODES];
    static SkXfermode* cached[COUNT_BLENDMODES];

    once[(int)mode]([mode] {
        SkXfermodeProc proc = gProcs[(int)mode];
        if (auto xfermode = SkOpts::create_xfermode(proc, mode)) {
            cached[(int)mode] = xfermode;
        } else {
            cached[(int)mode] = new SkProcCoeffXfermode(proc, mode);
        }
    });
    return sk_ref_sp(cached[(int)mode]);
}

SkXfermodeProc SkXfermode::GetProc(SkBlendMode mode) {
    SkXfermodeProc  proc = nullptr;
    if ((unsigned)mode <= (unsigned)SkBlendMode::kLastMode) {
        proc = gProcs[(unsigned)mode];
    }
    return proc;
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkXfermode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkProcCoeffXfermode)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkXfermode::IsOpaque(SkBlendMode mode, SrcColorOpacity opacityType) {
    SkBlendModeCoeff src, dst;
    if (!SkBlendMode_AsCoeff(mode, &src, &dst)) {
        return false;
    }

    switch (src) {
        case SkBlendModeCoeff::kDA:
        case SkBlendModeCoeff::kDC:
        case SkBlendModeCoeff::kIDA:
        case SkBlendModeCoeff::kIDC:
            return false;
        default:
            break;
    }

    switch (dst) {
        case SkBlendModeCoeff::kZero:
            return true;
        case SkBlendModeCoeff::kISA:
            return kOpaque_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSA:
            return kTransparentBlack_SrcColorOpacity == opacityType ||
            kTransparentAlpha_SrcColorOpacity == opacityType;
        case SkBlendModeCoeff::kSC:
            return kTransparentBlack_SrcColorOpacity == opacityType;
        default:
            return false;
    }
    return false;
}

#if SK_SUPPORT_GPU
const GrXPFactory* SkBlendMode_AsXPFactory(SkBlendMode mode) {
    if (SkBlendMode_AsCoeff(mode, nullptr, nullptr)) {
        const GrXPFactory* result = GrPorterDuffXPFactory::Get(mode);
        SkASSERT(result);
        return result;
    }

    SkASSERT(GrCustomXfermode::IsSupportedMode(mode));
    return GrCustomXfermode::Get(mode);
}
#endif

