#include "SkColorPriv.h"
#include "SkColor_opts_SSE2.h"
#include "SkMathPriv.h"
#include "SkXfermode.h"
#include "SkXfermode_opts_SSE2.h"
#include "SkXfermode_proccoeff.h"

////////////////////////////////////////////////////////////////////////////////
// 4 pixels SSE2 version functions
////////////////////////////////////////////////////////////////////////////////

static inline __m128i SkDiv255Round_SSE2(const __m128i& a) {
    __m128i prod = _mm_add_epi32(a, _mm_set1_epi32(128)); // prod += 128;
    prod = _mm_add_epi32(prod, _mm_srli_epi32(prod, 8));  // prod + (prod >> 8)
    prod = _mm_srli_epi32(prod, 8);                       // >> 8

    return prod;
}

static inline __m128i clamp_div255round_SSE2(const __m128i& prod) {
    // test if > 0
    __m128i cmp1 = _mm_cmpgt_epi32(prod, _mm_setzero_si128());
    // test if < 255*255
    __m128i cmp2 = _mm_cmplt_epi32(prod, _mm_set1_epi32(255*255));

    __m128i ret = _mm_setzero_si128();

    // if value >= 255*255, value = 255
    ret = _mm_andnot_si128(cmp2,  _mm_set1_epi32(255));

    __m128i div = SkDiv255Round_SSE2(prod);

    // test if > 0 && < 255*255
    __m128i cmp = _mm_and_si128(cmp1, cmp2);

    ret = _mm_or_si128(_mm_and_si128(cmp, div), _mm_andnot_si128(cmp, ret));

    return ret;
}

static inline __m128i srcover_byte_SSE2(const __m128i& a, const __m128i& b) {
    // a + b - SkAlphaMulAlpha(a, b);
    return _mm_sub_epi32(_mm_add_epi32(a, b), SkAlphaMulAlpha_SSE2(a, b));

}

static inline __m128i blendfunc_multiply_byte_SSE2(const __m128i& sc, const __m128i& dc,
                                                   const __m128i& sa, const __m128i& da) {
    // sc * (255 - da)
    __m128i ret1 = _mm_sub_epi32(_mm_set1_epi32(255), da);
    ret1 = _mm_mullo_epi16(sc, ret1);

    // dc * (255 - sa)
    __m128i ret2 = _mm_sub_epi32(_mm_set1_epi32(255), sa);
    ret2 = _mm_mullo_epi16(dc, ret2);

    // sc * dc
    __m128i ret3 = _mm_mullo_epi16(sc, dc);

    __m128i ret = _mm_add_epi32(ret1, ret2);
    ret = _mm_add_epi32(ret, ret3);

    return clamp_div255round_SSE2(ret);
}

static __m128i multiply_modeproc_SSE2(const __m128i& src, const __m128i& dst) {
    __m128i sa = SkGetPackedA32_SSE2(src);
    __m128i da = SkGetPackedA32_SSE2(dst);
    __m128i a = srcover_byte_SSE2(sa, da);

    __m128i sr = SkGetPackedR32_SSE2(src);
    __m128i dr = SkGetPackedR32_SSE2(dst);
    __m128i r = blendfunc_multiply_byte_SSE2(sr, dr, sa, da);

    __m128i sg = SkGetPackedG32_SSE2(src);
    __m128i dg = SkGetPackedG32_SSE2(dst);
    __m128i g = blendfunc_multiply_byte_SSE2(sg, dg, sa, da);


    __m128i sb = SkGetPackedB32_SSE2(src);
    __m128i db = SkGetPackedB32_SSE2(dst);
    __m128i b = blendfunc_multiply_byte_SSE2(sb, db, sa, da);

    return SkPackARGB32_SSE2(a, r, g, b);
}

////////////////////////////////////////////////////////////////////////////////

extern SkXfermodeProcSIMD gSSE2XfermodeProcs[];

SkSSE2ProcCoeffXfermode::SkSSE2ProcCoeffXfermode(SkReadBuffer& buffer)
    : INHERITED(buffer) {
    fProcSIMD = gSSE2XfermodeProcs[this->getMode()];
}

void SkSSE2ProcCoeffXfermode::xfer32(SkPMColor dst[], const SkPMColor src[],
                                     int count, const SkAlpha aa[]) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = this->getProc();
    SkXfermodeProcSIMD procSIMD = fProcSIMD;
    SkASSERT(procSIMD != NULL);

    if (NULL == aa) {
        if (count >= 4) {
            while (((size_t)dst & 0x0F) != 0) {
                *dst = proc(*src, *dst);
                dst++;
                src++;
                count--;
            }

            const __m128i* s = reinterpret_cast<const __m128i*>(src);
            __m128i* d = reinterpret_cast<__m128i*>(dst);

            while (count >= 4) {
                __m128i src_pixel = _mm_loadu_si128(s++);
                __m128i dst_pixel = _mm_load_si128(d);

                dst_pixel = procSIMD(src_pixel, dst_pixel);
                _mm_store_si128(d++, dst_pixel);
                count -= 4;
            }

            src = reinterpret_cast<const SkPMColor*>(s);
            dst = reinterpret_cast<SkPMColor*>(d);
        }

        for (int i = count - 1; i >= 0; --i) {
            *dst = proc(*src, *dst);
            dst++;
            src++;
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

void SkSSE2ProcCoeffXfermode::xfer16(uint16_t dst[], const SkPMColor src[],
                                     int count, const SkAlpha aa[]) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = this->getProc();
    SkXfermodeProcSIMD procSIMD = fProcSIMD;
    SkASSERT(procSIMD != NULL);

    if (NULL == aa) {
        if (count >= 8) {
            while (((size_t)dst & 0x0F) != 0) {
                SkPMColor dstC = SkPixel16ToPixel32(*dst);
                *dst = SkPixel32ToPixel16_ToU16(proc(*src, dstC));
                dst++;
                src++;
                count--;
            }

            const __m128i* s = reinterpret_cast<const __m128i*>(src);
            __m128i* d = reinterpret_cast<__m128i*>(dst);

            while (count >= 8) {
                __m128i src_pixel1 = _mm_loadu_si128(s++);
                __m128i src_pixel2 = _mm_loadu_si128(s++);
                __m128i dst_pixel = _mm_load_si128(d);

                __m128i dst_pixel1 = _mm_unpacklo_epi16(dst_pixel, _mm_setzero_si128());
                __m128i dst_pixel2 = _mm_unpackhi_epi16(dst_pixel, _mm_setzero_si128());

                __m128i dstC1 = SkPixel16ToPixel32_SSE2(dst_pixel1);
                __m128i dstC2 = SkPixel16ToPixel32_SSE2(dst_pixel2);

                dst_pixel1 = procSIMD(src_pixel1, dstC1);
                dst_pixel2 = procSIMD(src_pixel2, dstC2);
                dst_pixel = SkPixel32ToPixel16_ToU16_SSE2(dst_pixel1, dst_pixel2);

                _mm_store_si128(d++, dst_pixel);
                count -= 8;
            }

            src = reinterpret_cast<const SkPMColor*>(s);
            dst = reinterpret_cast<uint16_t*>(d);
        }

        for (int i = count - 1; i >= 0; --i) {
            SkPMColor dstC = SkPixel16ToPixel32(*dst);
            *dst = SkPixel32ToPixel16_ToU16(proc(*src, dstC));
            dst++;
            src++;
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

#ifndef SK_IGNORE_TO_STRING
void SkSSE2ProcCoeffXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

////////////////////////////////////////////////////////////////////////////////

// 4 pixels modeprocs with SSE2
SkXfermodeProcSIMD gSSE2XfermodeProcs[] = {
    NULL, // kClear_Mode
    NULL, // kSrc_Mode
    NULL, // kDst_Mode
    NULL, // kSrcOver_Mode
    NULL, // kDstOver_Mode
    NULL, // kSrcIn_Mode
    NULL, // kDstIn_Mode
    NULL, // kSrcOut_Mode
    NULL, // kDstOut_Mode
    NULL, // kSrcATop_Mode
    NULL, // kDstATop_Mode
    NULL, // kXor_Mode
    NULL, // kPlus_Mode
    NULL, // kModulate_Mode
    NULL, // kScreen_Mode

    NULL, // kOverlay_Mode
    NULL, // kDarken_Mode
    NULL, // kLighten_Mode
    NULL, // kColorDodge_Mode
    NULL, // kColorBurn_Mode
    NULL, // kHardLight_Mode
    NULL, // kSoftLight_Mode
    NULL, // kDifference_Mode
    NULL, // kExclusion_Mode
    multiply_modeproc_SSE2,

    NULL, // kHue_Mode
    NULL, // kSaturation_Mode
    NULL, // kColor_Mode
    NULL, // kLuminosity_Mode
};

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_SSE2(const ProcCoeff& rec,
                                                         SkXfermode::Mode mode) {
    SkXfermodeProcSIMD procSIMD = gSSE2XfermodeProcs[mode];

    if (procSIMD != NULL) {
        return SkNEW_ARGS(SkSSE2ProcCoeffXfermode, (rec, mode, procSIMD));
    }
    return NULL;
}
