
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkXfermode.h"
#include "SkColorPriv.h"

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

// kMultiply_Mode
static SkPMColor multiply_modeproc(SkPMColor src, SkPMColor dst) {
    int a = SkAlphaMulAlpha(SkGetPackedA32(src), SkGetPackedA32(dst));
    int r = SkAlphaMulAlpha(SkGetPackedR32(src), SkGetPackedR32(dst));
    int g = SkAlphaMulAlpha(SkGetPackedG32(src), SkGetPackedG32(dst));
    int b = SkAlphaMulAlpha(SkGetPackedB32(src), SkGetPackedB32(dst));
    return SkPackARGB32(a, r, g, b);
}

// kScreen_Mode
static inline int srcover_byte(int a, int b) {
    return a + b - SkAlphaMulAlpha(a, b);
}
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
    if (0 == diff) {
        rc = sa * da + sc * (255 - da) + dc * (255 - sa);
        rc = SkDiv255Round(rc);
    } else {
        int tmp = (dc * sa << 15) / (da * diff);
        rc = SkDiv255Round(sa * da) * tmp >> 15;
        // don't clamp here, since we'll do it in our modeproc
    }
    return rc;
}
static SkPMColor colordodge_modeproc(SkPMColor src, SkPMColor dst) {
    // added to avoid div-by-zero in colordodge_byte
    if (0 == dst) {
        return src;
    }

    int sa = SkGetPackedA32(src);
    int da = SkGetPackedA32(dst);
    int a = srcover_byte(sa, da);
    int r = colordodge_byte(SkGetPackedR32(src), SkGetPackedR32(dst), sa, da);
    int g = colordodge_byte(SkGetPackedG32(src), SkGetPackedG32(dst), sa, da);
    int b = colordodge_byte(SkGetPackedB32(src), SkGetPackedB32(dst), sa, da);
    r = clamp_max(r, a);
    g = clamp_max(g, a);
    b = clamp_max(b, a);
    return SkPackARGB32(a, r, g, b);
}

// kColorBurn_Mode
static inline int colorburn_byte(int sc, int dc, int sa, int da) {
    int rc;
    if (dc == da && 0 == sc) {
        rc = sa * da + dc * (255 - sa);
    } else if (0 == sc) {
        return SkAlphaMulAlpha(dc, 255 - sa);
    } else {
        int tmp = (sa * (da - dc) * 256) / (sc * da);
        if (tmp > 256) {
            tmp = 256;
        }
        int tmp2 = sa * da;
        rc = tmp2 - (tmp2 * tmp >> 8) + sc * (255 - da) + dc * (255 - sa);
    }
    return SkDiv255Round(rc);
}
static SkPMColor colorburn_modeproc(SkPMColor src, SkPMColor dst) {
    // added to avoid div-by-zero in colorburn_byte
    if (0 == dst) {
        return src;
    }

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
    { multiply_modeproc,SkXfermode::kZero_Coeff,    SkXfermode::kSC_Coeff },
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
};

///////////////////////////////////////////////////////////////////////////////

bool SkXfermode::asCoeff(Coeff* src, Coeff* dst) {
    return false;
}

bool SkXfermode::asMode(Mode* mode) {
    return false;
}

SkPMColor SkXfermode::xferColor(SkPMColor src, SkPMColor dst) {
    // no-op. subclasses should override this
    return dst;
}

void SkXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                        const SkPMColor* SK_RESTRICT src, int count,
                        const SkAlpha* SK_RESTRICT aa) {
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
                        const SkAlpha* SK_RESTRICT aa) {
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
                          const SkAlpha* SK_RESTRICT aa)
{
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
                        const SkAlpha* SK_RESTRICT aa)
{
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
                            const SkAlpha* SK_RESTRICT aa) {
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
                            const SkAlpha* SK_RESTRICT aa) {
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
                              const SkAlpha* SK_RESTRICT aa) {
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
                            const SkAlpha* SK_RESTRICT aa) {
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
    // Might be a NULL if the Xfermode is recorded using the CrossProcess flag
    fProc = (SkXfermodeProc)buffer.readFunctionPtr();
}

void SkProcXfermode::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    if (buffer.isCrossProcess()) {
        // function pointer is only valid in the current process. Write a NULL
        // so it can't be accidentally used
        buffer.writeFunctionPtr(NULL);
    } else {
        buffer.writeFunctionPtr((void*)fProc);
    }
}

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

    virtual bool asMode(Mode* mode) {
        if (mode) {
            *mode = fMode;
        }
        return true;
    }

    virtual bool asCoeff(Coeff* sc, Coeff* dc) {
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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcCoeffXfermode)

protected:
    SkProcCoeffXfermode(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
        fMode = (SkXfermode::Mode)buffer.readU32();

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

///////////////////////////////////////////////////////////////////////////////

class SkClearXfermode : public SkProcCoeffXfermode {
public:
    SkClearXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kClear_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) SK_OVERRIDE;
    virtual void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkClearXfermode)

private:
    SkClearXfermode(SkFlattenableReadBuffer& buffer)
        : SkProcCoeffXfermode(buffer) {}

};

void SkClearXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT, int count,
                             const SkAlpha* SK_RESTRICT aa) {
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
                             const SkAlpha* SK_RESTRICT aa) {
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

///////////////////////////////////////////////////////////////////////////////

class SkSrcXfermode : public SkProcCoeffXfermode {
public:
    SkSrcXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kSrc_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) SK_OVERRIDE;
    virtual void xferA8(SkAlpha*, const SkPMColor*, int, const SkAlpha*) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSrcXfermode)

private:
    SkSrcXfermode(SkFlattenableReadBuffer& buffer)
        : SkProcCoeffXfermode(buffer) {}

};

void SkSrcXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                           const SkPMColor* SK_RESTRICT src, int count,
                           const SkAlpha* SK_RESTRICT aa) {
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
                           const SkAlpha* SK_RESTRICT aa) {
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

////////////////////////////////////////////////////////////////////////////////////

class SkDstInXfermode : public SkProcCoeffXfermode {
public:
    SkDstInXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstIn_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDstInXfermode)

private:
    SkDstInXfermode(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkDstInXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                             const SkPMColor* SK_RESTRICT src, int count,
                             const SkAlpha* SK_RESTRICT aa) {
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

/////////////////////////////////////////////////////////////////////////////////////////

class SkDstOutXfermode : public SkProcCoeffXfermode {
public:
    SkDstOutXfermode(const ProcCoeff& rec) : SkProcCoeffXfermode(rec, kDstOut_Mode) {}

    virtual void xfer32(SkPMColor*, const SkPMColor*, int, const SkAlpha*) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDstOutXfermode)

private:
    SkDstOutXfermode(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}

    typedef SkProcCoeffXfermode INHERITED;
};

void SkDstOutXfermode::xfer32(SkPMColor* SK_RESTRICT dst,
                              const SkPMColor* SK_RESTRICT src, int count,
                              const SkAlpha* SK_RESTRICT aa) {
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

bool SkXfermode::AsMode(SkXfermode* xfer, Mode* mode) {
    if (NULL == xfer) {
        if (mode) {
            *mode = kSrcOver_Mode;
        }
        return true;
    }
    return xfer->asMode(mode);
}

bool SkXfermode::AsCoeff(SkXfermode* xfer, Coeff* src, Coeff* dst) {
    if (NULL == xfer) {
        return ModeAsCoeff(kSrcOver_Mode, src, dst);
    }
    return xfer->asCoeff(src, dst);
}

bool SkXfermode::IsMode(SkXfermode* xfer, Mode mode) {
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
    { NULL,                 NULL,                   NULL            }, // multiply
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
