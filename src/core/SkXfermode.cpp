/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkXfermode.h"
#include "SkColorPriv.h"

#define SkAlphaMulAlpha(a, b)   SkMulDiv255Round(a, b)

static SkPMColor SkFourByteInterp(SkPMColor src, SkPMColor dst, U8CPU alpha) {
    unsigned scale = SkAlpha255To256(alpha);

    unsigned a = SkAlphaBlend(SkGetPackedA32(src), SkGetPackedA32(dst), scale);
    unsigned r = SkAlphaBlend(SkGetPackedR32(src), SkGetPackedR32(dst), scale);
    unsigned g = SkAlphaBlend(SkGetPackedG32(src), SkGetPackedG32(dst), scale);
    unsigned b = SkAlphaBlend(SkGetPackedB32(src), SkGetPackedB32(dst), scale);

    return SkPackARGB32(a, r, g, b);
}

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

///////////////////////////////////////////////////////////////////////////////

bool SkXfermode::asCoeff(Coeff* src, Coeff* dst) {
    return false;
}

SkPMColor SkXfermode::xferColor(SkPMColor src, SkPMColor dst) {
    // no-op. subclasses should override this
    return dst;
}

void SkXfermode::xfer32(SK_RESTRICT SkPMColor dst[],
                        const SK_RESTRICT SkPMColor src[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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

void SkXfermode::xfer16(SK_RESTRICT uint16_t dst[],
                        const SK_RESTRICT SkPMColor src[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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

void SkXfermode::xfer4444(SK_RESTRICT SkPMColor16 dst[],
                          const SK_RESTRICT SkPMColor src[], int count,
                          const SK_RESTRICT SkAlpha aa[])
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

void SkXfermode::xferA8(SK_RESTRICT SkAlpha dst[],
                        const SkPMColor src[], int count,
                        const SK_RESTRICT SkAlpha aa[])
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

void SkProcXfermode::xfer32(SK_RESTRICT SkPMColor dst[],
                            const SK_RESTRICT SkPMColor src[], int count,
                            const SK_RESTRICT SkAlpha aa[]) {
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

void SkProcXfermode::xfer16(SK_RESTRICT uint16_t dst[],
                            const SK_RESTRICT SkPMColor src[], int count,
                            const SK_RESTRICT SkAlpha aa[]) {
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

void SkProcXfermode::xfer4444(SK_RESTRICT SkPMColor16 dst[],
                              const SK_RESTRICT SkPMColor src[], int count,
                              const SK_RESTRICT SkAlpha aa[]) {
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

void SkProcXfermode::xferA8(SK_RESTRICT SkAlpha dst[],
                            const SK_RESTRICT SkPMColor src[], int count,
                            const SK_RESTRICT SkAlpha aa[]) {
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
    fProc = (SkXfermodeProc)buffer.readFunctionPtr();
}

void SkProcXfermode::flatten(SkFlattenableWriteBuffer& buffer) {
    buffer.writeFunctionPtr((void*)fProc);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class SkProcCoeffXfermode : public SkProcXfermode {
public:
    SkProcCoeffXfermode(SkXfermodeProc proc, Coeff sc, Coeff dc)
            : INHERITED(proc), fSrcCoeff(sc), fDstCoeff(dc) {
    }
    
    virtual bool asCoeff(Coeff* sc, Coeff* dc) {
        if (sc) {
            *sc = fSrcCoeff;
        }
        if (dc) {
            *dc = fDstCoeff;
        }
        return true;
    }
    
    virtual Factory getFactory() { return CreateProc; }
    virtual void flatten(SkFlattenableWriteBuffer& buffer) {
        this->INHERITED::flatten(buffer);
        buffer.write32(fSrcCoeff);
        buffer.write32(fDstCoeff);
    }

protected:
    SkProcCoeffXfermode(SkFlattenableReadBuffer& buffer)
            : INHERITED(buffer) {
        fSrcCoeff = (Coeff)buffer.readU32();
        fDstCoeff = (Coeff)buffer.readU32();
    }
    
private:
    Coeff   fSrcCoeff, fDstCoeff;
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkProcCoeffXfermode, (buffer)); }

    typedef SkProcXfermode INHERITED;
};

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


// kDarken_Mode,   [Sa + Da - Sa·Da, Sc·(1 - Da) + Dc·(1 - Sa) + min(Sc, Dc)]

static inline unsigned darken_p(unsigned src, unsigned dst,
                                unsigned src_mul, unsigned dst_mul) {
    return ((dst_mul * src + src_mul * dst) >> 8) + SkMin32(src, dst);
}

static SkPMColor darken_modeproc(SkPMColor src, SkPMColor dst) {
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned src_scale = SkAlpha255To256(255 - sa);
    unsigned dst_scale = SkAlpha255To256(255 - da);

    unsigned ra = sa + da - SkAlphaMulAlpha(sa, da);
    unsigned rr = darken_p(SkGetPackedR32(src), SkGetPackedR32(dst),
                           src_scale, dst_scale);
    unsigned rg = darken_p(SkGetPackedG32(src), SkGetPackedG32(dst),
                           src_scale, dst_scale);
    unsigned rb = darken_p(SkGetPackedB32(src), SkGetPackedB32(dst),
                           src_scale, dst_scale);

    return SkPackARGB32(ra, SkFastMin32(rr, ra),
                        SkFastMin32(rg, ra), SkFastMin32(rb, ra));
}

// kLighten_Mode,  [Sa + Da - Sa·Da, Sc·(1 - Da) + Dc·(1 - Sa) + max(Sc, Dc)]
static inline unsigned lighten_p(unsigned src, unsigned dst,
                                 unsigned src_mul, unsigned dst_mul) {
    return ((dst_mul * src + src_mul * dst) >> 8) + SkMax32(src, dst);
}

static SkPMColor lighten_modeproc(SkPMColor src, SkPMColor dst) {
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned src_scale = SkAlpha255To256(255 - sa);
    unsigned dst_scale = SkAlpha255To256(255 - da);
    
    unsigned ra = sa + da - SkAlphaMulAlpha(sa, da);
    unsigned rr = lighten_p(SkGetPackedR32(src), SkGetPackedR32(dst),
                            src_scale, dst_scale);
    unsigned rg = lighten_p(SkGetPackedG32(src), SkGetPackedG32(dst),
                            src_scale, dst_scale);
    unsigned rb = lighten_p(SkGetPackedB32(src), SkGetPackedB32(dst),
                            src_scale, dst_scale);

    return SkPackARGB32(ra, SkFastMin32(rr, ra),
                        SkFastMin32(rg, ra), SkFastMin32(rb, ra));
}
    
static SkPMColor mult_modeproc(SkPMColor src, SkPMColor dst) {
    int a = SkAlphaMulAlpha(SkGetPackedA32(src), SkGetPackedA32(dst));
    int r = SkAlphaMulAlpha(SkGetPackedR32(src), SkGetPackedR32(dst));
    int g = SkAlphaMulAlpha(SkGetPackedG32(src), SkGetPackedG32(dst));
    int b = SkAlphaMulAlpha(SkGetPackedB32(src), SkGetPackedB32(dst));
    return SkPackARGB32(a, r, g, b);
}

static inline int screen_byte(int a, int b) {
    return a + b - SkAlphaMulAlpha(a, b);
}

static SkPMColor screen_modeproc(SkPMColor src, SkPMColor dst) {
    int a = screen_byte(SkGetPackedA32(src), SkGetPackedA32(dst));
    int r = screen_byte(SkGetPackedR32(src), SkGetPackedR32(dst));
    int g = screen_byte(SkGetPackedG32(src), SkGetPackedG32(dst));
    int b = screen_byte(SkGetPackedB32(src), SkGetPackedB32(dst));
    return SkPackARGB32(a, r, g, b);
}

///////////////////////////////////////////////////////////////////////////////

class SkClearXfermode : public SkProcCoeffXfermode {
public:
    SkClearXfermode() : SkProcCoeffXfermode(clear_modeproc,
                                            kZero_Coeff, kZero_Coeff) {}

    virtual void xfer32(SK_RESTRICT SkPMColor dst[],
                        const SK_RESTRICT SkPMColor[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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
    virtual void xferA8(SK_RESTRICT SkAlpha dst[],
                        const SK_RESTRICT SkPMColor[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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
    
    virtual Factory getFactory() { return CreateProc; }

private:
    SkClearXfermode(SkFlattenableReadBuffer& buffer)
        : SkProcCoeffXfermode(buffer) {}
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkClearXfermode, (buffer));
    }
};

///////////////////////////////////////////////////////////////////////////////

class SkSrcXfermode : public SkProcCoeffXfermode {
public:
    SkSrcXfermode() : SkProcCoeffXfermode(src_modeproc,
                                          kOne_Coeff, kZero_Coeff) {}

    virtual void xfer32(SK_RESTRICT SkPMColor dst[],
                        const SK_RESTRICT SkPMColor src[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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

    virtual void xferA8(SK_RESTRICT SkAlpha dst[],
                        const SK_RESTRICT SkPMColor src[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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
    
    virtual Factory getFactory() { return CreateProc; }

private:
    SkSrcXfermode(SkFlattenableReadBuffer& buffer)
        : SkProcCoeffXfermode(buffer) {}
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkSrcXfermode, (buffer));
    }
};

class SkDstInXfermode : public SkProcCoeffXfermode {
public:
    SkDstInXfermode() : SkProcCoeffXfermode(dstin_modeproc,
                                            kZero_Coeff, kSA_Coeff) {}
    
    virtual void xfer32(SK_RESTRICT SkPMColor dst[],
                        const SK_RESTRICT SkPMColor src[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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
    
    virtual Factory getFactory() { return CreateProc; }
    
private:
    SkDstInXfermode(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {}
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkDstInXfermode, (buffer));
    }
    
    typedef SkProcCoeffXfermode INHERITED;
};

class SkDstOutXfermode : public SkProcCoeffXfermode {
public:
    SkDstOutXfermode() : SkProcCoeffXfermode(dstout_modeproc,
                                             kZero_Coeff, kISA_Coeff) {}
    
    virtual void xfer32(SK_RESTRICT SkPMColor dst[],
                        const SK_RESTRICT SkPMColor src[], int count,
                        const SK_RESTRICT SkAlpha aa[]) {
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
    
    virtual Factory getFactory() { return CreateProc; }
    
private:
    SkDstOutXfermode(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {}
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkDstOutXfermode, (buffer));
    }
    
    typedef SkProcCoeffXfermode INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

#include "SkPorterDuff.h"

struct ProcCoeff {
    SkXfermodeProc      fProc;
    SkXfermode::Coeff   fSC;
    SkXfermode::Coeff   fDC;
};

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
    // these two can't be represented as coefficients
    { darken_modeproc,  SkXfermode::Coeff(-1),      SkXfermode::Coeff(-1) },
    { lighten_modeproc, SkXfermode::Coeff(-1),      SkXfermode::Coeff(-1) },
    // these can use coefficients
    { mult_modeproc,    SkXfermode::kZero_Coeff,    SkXfermode::kSC_Coeff },
    { screen_modeproc,  SkXfermode::kOne_Coeff,     SkXfermode::kISC_Coeff }
};

SkXfermode* SkPorterDuff::CreateXfermode(SkPorterDuff::Mode mode) {
    SkASSERT(SK_ARRAY_COUNT(gProcCoeffs) == SkPorterDuff::kModeCount);
    SkASSERT((unsigned)mode < SkPorterDuff::kModeCount);

    switch (mode) {
        case kClear_Mode:
            return SkNEW(SkClearXfermode);
        case kSrc_Mode:
            return SkNEW(SkSrcXfermode);
        case kSrcOver_Mode:
            return NULL;
        case kDstIn_Mode:
            return SkNEW(SkDstInXfermode);
        case kDstOut_Mode:
            return SkNEW(SkDstOutXfermode);
        // these two can't be represented with Coeff
        case kDarken_Mode:
            return SkNEW_ARGS(SkProcXfermode, (darken_modeproc));
        case kLighten_Mode:
            return SkNEW_ARGS(SkProcXfermode, (lighten_modeproc));
        // use the table 
        default: {
            const ProcCoeff& rec = gProcCoeffs[mode];
            SkASSERT((unsigned)rec.fSC < SkXfermode::kCoeffCount);
            SkASSERT((unsigned)rec.fDC < SkXfermode::kCoeffCount);
            return SkNEW_ARGS(SkProcCoeffXfermode, (rec.fProc,
                                                    rec.fSC, rec.fDC));
        }
    }
}

bool SkPorterDuff::IsMode(SkXfermode* xfer, Mode* mode) {
    if (NULL == xfer) {
        if (mode) {
            *mode = kSrcOver_Mode;
        }
        return true;
    }

    SkXfermode::Coeff sc, dc;
    if (xfer->asCoeff(&sc, &dc)) {
        SkASSERT((unsigned)sc < (unsigned)SkXfermode::kCoeffCount);
        SkASSERT((unsigned)dc < (unsigned)SkXfermode::kCoeffCount);
        
        const ProcCoeff* rec = gProcCoeffs;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gProcCoeffs); i++) {
            if (rec[i].fSC == sc && rec[i].fDC == dc) {
                if (mode) {
                    *mode = SkPorterDuff::Mode(i);
                }
                return true;
            }
        }
    }

    // no coefficients, or not found in our table
    return false;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUGx
static void unit_test() {
    for (unsigned a = 0; a <= 255; a++) {
        for (unsigned c = 0; c <= a; c++) {
            SkPMColor pm = SkPackARGB32(a, c, c, c);
            for (unsigned aa = 0; aa <= 255; aa++) {
                for (unsigned cc = 0; cc <= aa; cc++) {
                    SkPMColor pm2 = SkPackARGB32(aa, cc, cc, cc);
                    
                    const size_t N = SK_ARRAY_COUNT(gProcCoeffs);
                    for (size_t i = 0; i < N; i++) {
                        gProcCoeffs[i].fProc(pm, pm2);
                    }
                }
            }
        }
    }            
}
#endif

SkXfermodeProc SkPorterDuff::GetXfermodeProc(Mode mode) {
#ifdef SK_DEBUGx
    static bool gUnitTest;
    if (!gUnitTest) {
        gUnitTest = true;
        unit_test();
    }
#endif

    SkXfermodeProc  proc = NULL;

    if ((unsigned)mode < SkPorterDuff::kModeCount) {
        proc = gProcCoeffs[mode].fProc;
    }
    return proc;
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

static const Proc16Rec gPorterDuffModeProcs16[] = {
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
    { darken_modeproc16_0,  darken_modeproc16_255,  NULL            },
    { lighten_modeproc16_0, lighten_modeproc16_255, NULL            },
    { NULL,                 NULL,                   NULL            },//multiply
    { NULL,                 NULL,                   NULL            }// screen
};

SkXfermodeProc16 SkPorterDuff::GetXfermodeProc16(Mode mode, SkColor srcColor) {
    SkXfermodeProc16  proc16 = NULL;

    if ((unsigned)mode < SkPorterDuff::kModeCount) {
        const Proc16Rec& rec = gPorterDuffModeProcs16[mode];
        
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

