/* libs/graphics/sgl/SkXfermode.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkXfermode.h"
#include "SkColorPriv.h"

static inline U8CPU SkAlphaMulAlpha(U8CPU a, U8CPU b)
{
    unsigned ab = a * b;
#if 0
    return ab / 255;
#else
    return ((ab << 8) + ab + 257) >> 16;
#endif
}

void SkXfermode::xfer32(SkPMColor dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    // override in subclass
}

void SkXfermode::xfer16(uint16_t dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    // override in subclass
}

void SkXfermode::xferA8(SkAlpha dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    // override in subclass
}

//////////////////////////////////////////////////////////////////////////////////

static SkPMColor SkFourByteInterp(SkPMColor src, SkPMColor dst, U8CPU alpha)
{
    unsigned scale = SkAlpha255To256(alpha);

    unsigned a = SkAlphaBlend(SkGetPackedA32(src), SkGetPackedA32(dst), scale);
    unsigned r = SkAlphaBlend(SkGetPackedR32(src), SkGetPackedR32(dst), scale);
    unsigned g = SkAlphaBlend(SkGetPackedG32(src), SkGetPackedG32(dst), scale);
    unsigned b = SkAlphaBlend(SkGetPackedB32(src), SkGetPackedB32(dst), scale);

    return SkPackARGB32(a, r, g, b);
}

void SkProcXfermode::xfer32(SkPMColor dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;
    if (proc)
    {
        if (NULL == aa)     
        {
            for (int i = count - 1; i >= 0; --i)
                dst[i] = proc(src[i], dst[i]);
        }
        else
        {
            for (int i = count - 1; i >= 0; --i)
            {
                unsigned a = aa[i];
                if (a != 0)
                {
                    SkPMColor dstC = dst[i];
                    SkPMColor C = proc(src[i], dstC);
                    if (a != 0xFF)
                        C = SkFourByteInterp(C, dstC, a);
                    dst[i] = C;
                }
            }
        }
    }
}

void SkProcXfermode::xfer16(uint16_t dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;
    if (proc)
    {
        if (NULL == aa)     
        {
            for (int i = count - 1; i >= 0; --i)
            {
                SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
                dst[i] = SkPixel32ToPixel16_ToU16(proc(src[i], dstC));
            }
        }
        else
        {
            for (int i = count - 1; i >= 0; --i)
            {
                unsigned a = aa[i];
                if (a != 0)
                {
                    SkPMColor dstC = SkPixel16ToPixel32(dst[i]);
                    SkPMColor C = proc(src[i], dstC);
                    if (a != 0xFF)
                        C = SkFourByteInterp(C, dstC, a);
                    dst[i] = SkPixel32ToPixel16_ToU16(C);
                }
            }
        }
    }
}

void SkProcXfermode::xferA8(SkAlpha dst[], const SkPMColor src[], int count, const SkAlpha aa[])
{
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = fProc;
    if (proc)
    {
        if (NULL == aa)     
        {
            for (int i = count - 1; i >= 0; --i)
                dst[i] = SkToU8(SkGetPackedA32(proc(src[i], (SkPMColor)(dst[i] << SK_A32_SHIFT))));
        }
        else
        {
            for (int i = count - 1; i >= 0; --i)
            {
                unsigned a = aa[i];
                if (a != 0)
                {
                    SkAlpha dstA = dst[i];
                    unsigned A = SkGetPackedA32(proc(src[i], (SkPMColor)(dstA << SK_A32_SHIFT)));
                    if (a != 0xFF)
                        A = SkAlphaBlend(A, dstA, SkAlpha255To256(a));
                    dst[i] = SkToU8(A);
                }
            }
        }
    }
}

SkProcXfermode::SkProcXfermode(SkRBuffer& buffer) : SkXfermode(buffer)
{
    fProc = (SkXfermodeProc)buffer.readPtr();
}

void SkProcXfermode::flatten(SkWBuffer& buffer)
{
    this->INHERITED::flatten(buffer);
    
    buffer.writePtr((void*)fProc);
}

SkFlattenable::Factory SkProcXfermode::getFactory()
{
    return CreateProc;
}

SkFlattenable* SkProcXfermode::CreateProc(SkRBuffer& buffer)
{
    return SkNEW_ARGS(SkProcXfermode, (buffer));
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

//  kClear_Mode,    //!< [0, 0]
static SkPMColor clear_modeproc(SkPMColor src, SkPMColor dst)
{
    return 0;
}

//  kSrc_Mode,      //!< [Sa, Sc]
static SkPMColor src_modeproc(SkPMColor src, SkPMColor dst)
{
    return src;
}

//  kDst_Mode,      //!< [Da, Dc]
static SkPMColor dst_modeproc(SkPMColor src, SkPMColor dst)
{
    return dst;
}

//  kSrcOver_Mode,  //!< [Sa + (1 - Sa)*Da, Sc + (1 - Sa)*Dc] this is the default mode
static SkPMColor srcover_modeproc(SkPMColor src, SkPMColor dst)
{
    return src + SkAlphaMulQ(dst, SkAlpha255To256(255 - SkGetPackedA32(src)));
}

//  kDstOver_Mode,  //!< [Sa + (1 - Sa)*Da, Dc + (1 - Da)*Sc]
static SkPMColor dstover_modeproc(SkPMColor src, SkPMColor dst)
{
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);

    return SkPackARGB32(sa + da - SkAlphaMulAlpha(sa, da),
                        SkGetPackedR32(dst) + SkAlphaMulAlpha(255 - da, SkGetPackedR32(src)),
                        SkGetPackedG32(dst) + SkAlphaMulAlpha(255 - da, SkGetPackedG32(src)),
                        SkGetPackedB32(dst) + SkAlphaMulAlpha(255 - da, SkGetPackedB32(src)));
}

//  kSrcIn_Mode,    //!< [Sa * Da, Sc * Da]
static SkPMColor srcin_modeproc(SkPMColor src, SkPMColor dst)
{
    return SkAlphaMulQ(src, SkAlpha255To256(SkGetPackedA32(dst)));
}

//  kDstIn_Mode,    //!< [Sa * Da, Sa * Dc]
static SkPMColor dstin_modeproc(SkPMColor src, SkPMColor dst)
{
    return SkAlphaMulQ(dst, SkAlpha255To256(SkGetPackedA32(src)));
}

//  kSrcOut_Mode,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
static SkPMColor srcout_modeproc(SkPMColor src, SkPMColor dst)
{
    return SkAlphaMulQ(src, SkAlpha255To256(255 - SkGetPackedA32(dst)));
}

//  kDstOut_Mode,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
static SkPMColor dstout_modeproc(SkPMColor src, SkPMColor dst)
{
    return SkAlphaMulQ(dst, SkAlpha255To256(255 - SkGetPackedA32(src)));
}

//  kSrcATop_Mode,  //!< [Da, Sc * Da + (1 - Sa) * Dc]
static SkPMColor srcatop_modeproc(SkPMColor src, SkPMColor dst)
{
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned src_scale = SkAlpha255To256(255 - sa);
    unsigned dst_scale = SkAlpha255To256(da);

    return SkPackARGB32(da,
                        (dst_scale * SkGetPackedR32(src) + src_scale * SkGetPackedR32(dst)) >> 8,
                        (dst_scale * SkGetPackedG32(src) + src_scale * SkGetPackedG32(dst)) >> 8,
                        (dst_scale * SkGetPackedB32(src) + src_scale * SkGetPackedB32(dst)) >> 8);
}

//  kDstATop_Mode,  //!< [Sa, Sa * Dc + Sc * (1 - Da)]
static SkPMColor dstatop_modeproc(SkPMColor src, SkPMColor dst)
{
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned src_scale = SkAlpha255To256(sa);
    unsigned dst_scale = SkAlpha255To256(255 - da);

    return SkPackARGB32(sa,
                        (dst_scale * SkGetPackedR32(src) + src_scale * SkGetPackedR32(dst)) >> 8,
                        (dst_scale * SkGetPackedG32(src) + src_scale * SkGetPackedG32(dst)) >> 8,
                        (dst_scale * SkGetPackedB32(src) + src_scale * SkGetPackedB32(dst)) >> 8);
}

//  kXor_Mode,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + (1 - Sa) * Dc]
static SkPMColor xor_modeproc(SkPMColor src, SkPMColor dst)
{
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned src_scale = SkAlpha255To256(255 - sa);
    unsigned dst_scale = SkAlpha255To256(255 - da);

    return SkPackARGB32(sa + da - (SkAlphaMulAlpha(sa, da) << 1),
                        (dst_scale * SkGetPackedR32(src) + src_scale * SkGetPackedR32(dst)) >> 8,
                        (dst_scale * SkGetPackedG32(src) + src_scale * SkGetPackedG32(dst)) >> 8,
                        (dst_scale * SkGetPackedB32(src) + src_scale * SkGetPackedB32(dst)) >> 8);
}


//        kDarken_Mode,   [Sa + Da - SaáDa, Scá(1 - Da) + Dcá(1 - Sa) + min(Sc, Dc)]

static inline unsigned darken_p(unsigned src, unsigned dst, unsigned src_mul, unsigned dst_mul)
{
    return (dst_mul * src + src_mul * dst >> 8) + SkMin32(src, dst);
}

static SkPMColor darken_modeproc(SkPMColor src, SkPMColor dst)
{
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned src_scale = SkAlpha255To256(255 - sa);
    unsigned dst_scale = SkAlpha255To256(255 - da);

    unsigned ra = sa + da - SkAlphaMulAlpha(sa, da);
    unsigned rr = darken_p(SkGetPackedR32(src), SkGetPackedR32(dst), src_scale, dst_scale);
    unsigned rg = darken_p(SkGetPackedG32(src), SkGetPackedG32(dst), src_scale, dst_scale);
    unsigned rb = darken_p(SkGetPackedB32(src), SkGetPackedB32(dst), src_scale, dst_scale);

    return SkPackARGB32(ra, SkFastMin32(rr, ra), SkFastMin32(rg, ra), SkFastMin32(rb, ra));
}

//        kLighten_Mode,  [Sa + Da - SaáDa, Scá(1 - Da) + Dcá(1 - Sa) + max(Sc, Dc)]
static inline unsigned lighten_p(unsigned src, unsigned dst, unsigned src_mul, unsigned dst_mul)
{
    return (dst_mul * src + src_mul * dst >> 8) + SkMax32(src, dst);
}

static SkPMColor lighten_modeproc(SkPMColor src, SkPMColor dst)
{
    unsigned sa = SkGetPackedA32(src);
    unsigned da = SkGetPackedA32(dst);
    unsigned src_scale = SkAlpha255To256(255 - sa);
    unsigned dst_scale = SkAlpha255To256(255 - da);
    
    unsigned ra = sa + da - SkAlphaMulAlpha(sa, da);
    unsigned rr = lighten_p(SkGetPackedR32(src), SkGetPackedR32(dst), src_scale, dst_scale);
    unsigned rg = lighten_p(SkGetPackedG32(src), SkGetPackedG32(dst), src_scale, dst_scale);
    unsigned rb = lighten_p(SkGetPackedB32(src), SkGetPackedB32(dst), src_scale, dst_scale);

    return SkPackARGB32(ra, SkFastMin32(rr, ra), SkFastMin32(rg, ra), SkFastMin32(rb, ra));
}

//////////////////////////////////////////////////////////////////////////////////

#if 0 // maybe do these later

#define SkPinToU8(value)    SkFastMin32(value, 0xFF)

//  kAdd_Mode,      //!< clamp [Sa + Da, Sc + Dc]
static SkPMColor add_modeproc(SkPMColor src, SkPMColor dst)
{
    return SkPackARGB32(SkPinToU8(SkGetPackedA32(src) + SkGetPackedA32(dst)),
                        SkPinToU8(SkGetPackedR32(src) + SkGetPackedR32(dst)),
                        SkPinToU8(SkGetPackedG32(src) + SkGetPackedG32(dst)),
                        SkPinToU8(SkGetPackedB32(src) + SkGetPackedB32(dst)));
}

static U8CPU do_mul(U8CPU src, U8CPU dst, unsigned src_scale, unsigned dst_scale)
{
    return (src * dst_scale + dst * (SkAlpha255To256(src) + src_scale)) >> 8;
}

//  kMul_Mode,      //!< clamp [Sa + Da - Sa * Da, Sc * Dc + Sc * (1 - Da) + (1 - Sa) * Dc]
static SkPMColor mul_modeproc(SkPMColor src, SkPMColor dst)
{
    unsigned sa = SkGetPackedA32(src);
    unsigned src_scale = SkAlpha255To256(255 - sa);

    unsigned da = SkGetPackedA32(dst);
    unsigned dst_scale = SkAlpha255To256(255 - da);

    return SkPackARGB32(sa + da - SkAlphaMul(SkAlpha255To256(sa), da),
                        do_mul(SkGetPackedR32(src), SkGetPackedR32(dst), src_scale, dst_scale),
                        do_mul(SkGetPackedG32(src), SkGetPackedG32(dst), src_scale, dst_scale),
                        do_mul(SkGetPackedB32(src), SkGetPackedB32(dst), src_scale, dst_scale));
}
#endif

//////////////////////////////////////////////////////////////////////////////////

class SkClearXfermode : public SkProcXfermode {
public:
    SkClearXfermode() : SkProcXfermode(clear_modeproc) {}

    virtual void xfer32(SkPMColor dst[], const SkPMColor[], int count, const SkAlpha aa[])
    {
        SkASSERT(dst && count >= 0);

        if (NULL == aa)
            memset(dst, 0, count << 2);
        else
        {
            for (int i = count - 1; i >= 0; --i)
            {
                unsigned a = aa[i];
                if (a == 0xFF)
                    dst[i] = 0;
                else if (a != 0)
                    dst[i] = SkAlphaMulQ(dst[i], SkAlpha255To256(255 - a));
            }
        }
    }
    virtual void xferA8(SkAlpha dst[], const SkPMColor[], int count, const SkAlpha aa[])
    {
        SkASSERT(dst && count >= 0);

        if (NULL == aa)
            memset(dst, 0, count);
        else
        {
            for (int i = count - 1; i >= 0; --i)
            {
                unsigned a = aa[i];
                if (a == 0xFF)
                    dst[i] = 0;
                else if (a != 0)
                    dst[i] = SkToU8(SkAlphaMul(dst[i], SkAlpha255To256(255 - a)));
            }
        }
    }
    
    virtual Factory getFactory() { return CreateProc; }
    // we have nothing to flatten(), so don't need to override it

private:
    SkClearXfermode(SkRBuffer& buffer) : SkProcXfermode(buffer) {}
    
    static SkFlattenable* CreateProc(SkRBuffer& buffer)
    {
        return SkNEW_ARGS(SkClearXfermode, (buffer));
    }
};

//////////////////////////////////////////////////////////////////////////////////

class SkSrcXfermode : public SkProcXfermode {
public:
    SkSrcXfermode() : SkProcXfermode(src_modeproc) {}

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count, const SkAlpha aa[])
    {
        SkASSERT(dst && src && count >= 0);

        if (NULL == aa)
            memcpy(dst, src, count << 2);
        else
        {
            for (int i = count - 1; i >= 0; --i)
            {
                unsigned a = aa[i];
                if (a == 0xFF)
                    dst[i] = src[i];
                else if (a != 0)
                    dst[i] = SkFourByteInterp(src[i], dst[i], a);
            }
        }
    }
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count, const SkAlpha aa[])
    {
        SkASSERT(dst && src && count >= 0);

        if (NULL == aa)
        {
            for (int i = count - 1; i >= 0; --i)
                dst[i] = SkToU8(SkGetPackedA32(src[i]));
        }
        else
        {
            for (int i = count - 1; i >= 0; --i)
            {
                unsigned a = aa[i];
                if (a != 0)
                {
                    unsigned srcA = SkGetPackedA32(src[i]);
                    if (a == 0xFF)
                        dst[i] = SkToU8(srcA);
                    else
                        dst[i] = SkToU8(SkAlphaBlend(srcA, dst[i], a));
                }
            }
        }
    }
    
    virtual Factory getFactory() { return CreateProc; }
    // we have nothing to flatten(), so don't need to override it

private:
    SkSrcXfermode(SkRBuffer& buffer) : SkProcXfermode(buffer) {}
    
    static SkFlattenable* CreateProc(SkRBuffer& buffer)
    {
        return SkNEW_ARGS(SkSrcXfermode, (buffer));
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPorterDuff.h"

static const SkXfermodeProc gPorterDuffModeProcs[] = {
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
    darken_modeproc,
    lighten_modeproc
};

SkXfermode* SkPorterDuff::CreateXfermode(SkPorterDuff::Mode mode)
{
    SkASSERT(SK_ARRAY_COUNT(gPorterDuffModeProcs) == SkPorterDuff::kModeCount);
    SkASSERT((unsigned)mode < SkPorterDuff::kModeCount);

    switch (mode) {
    case kClear_Mode:
        return SkNEW(SkClearXfermode);
    case kSrc_Mode:
        return SkNEW(SkSrcXfermode);
    case kSrcOver_Mode:
        return NULL;
    default:
        return SkNEW_ARGS(SkProcXfermode, (gPorterDuffModeProcs[mode]));
    }
}

#ifdef SK_DEBUG
static void unit_test()
{
    for (unsigned a = 0; a <= 255; a++) {
        for (unsigned c = 0; c <= a; c++) {
            SkPMColor pm = SkPackARGB32(a, c, c, c);
            for (unsigned aa = 0; aa <= 255; aa++) {
                for (unsigned cc = 0; cc <= aa; cc++) {
                    SkPMColor pm2 = SkPackARGB32(aa, cc, cc, cc);
                    
                    for (unsigned i = 0; i < SK_ARRAY_COUNT(gPorterDuffModeProcs); i++) {
                        gPorterDuffModeProcs[i](pm, pm2);
                    }
                }
            }
        }
    }            
}
#endif

SkXfermodeProc SkPorterDuff::GetXfermodeProc(Mode mode)
{
#ifdef SK_DEBUGx
    static bool gUnitTest;
    if (!gUnitTest) {
        gUnitTest = true;
        unit_test();
    }
#endif

    SkXfermodeProc  proc = NULL;

    if ((unsigned)mode < SkPorterDuff::kModeCount)
        proc = gPorterDuffModeProcs[mode];

    return proc;
}

