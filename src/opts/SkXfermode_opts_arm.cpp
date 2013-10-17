#include "SkXfermode.h"
#include "SkXfermode_proccoeff.h"
#include "SkColorPriv.h"
#include "SkUtilsArm.h"

#if !SK_ARM_NEON_IS_NONE

#include <arm_neon.h>

////////////////////////////////////////////////////////////////////////////////

typedef uint8x8x4_t (*SkXfermodeProcSIMD)(uint8x8x4_t src, uint8x8x4_t dst);

class SkNEONProcCoeffXfermode : public SkProcCoeffXfermode {
public:
    SkNEONProcCoeffXfermode(const ProcCoeff& rec, SkXfermode::Mode mode,
                            SkXfermodeProcSIMD procSIMD)
            : INHERITED(rec, mode), fProcSIMD(procSIMD) {}

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkNEONProcCoeffXfermode)

private:
    SkNEONProcCoeffXfermode(SkFlattenableReadBuffer& buffer)
        : INHERITED(buffer) {

        fProcSIMD = NULL;
        if (!buffer.isCrossProcess()) {
            fProcSIMD = (SkXfermodeProcSIMD)buffer.readFunctionPtr();
        }
    }

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE;

    SkXfermodeProcSIMD fProcSIMD;
    typedef SkProcCoeffXfermode INHERITED;
};


void SkNEONProcCoeffXfermode::xfer32(SkPMColor dst[], const SkPMColor src[],
                                     int count, const SkAlpha aa[]) const {
    SkASSERT(dst && src && count >= 0);

    SkXfermodeProc proc = this->getProc();
    SkXfermodeProcSIMD procSIMD = fProcSIMD;

    if (NULL == aa) {
        // Unrolled NEON code
        while (count >= 8) {
            uint8x8x4_t vsrc, vdst, vres;

            asm volatile (
                "vld4.u8    %h[vsrc], [%[src]]!  \t\n"
                "vld4.u8    %h[vdst], [%[dst]]   \t\n"
                : [vsrc] "=w" (vsrc), [vdst] "=w" (vdst)
                : [src] "r" (src), [dst] "r" (dst)
                :
            );

            vres = procSIMD(vsrc, vdst);

            vst4_u8((uint8_t*)dst, vres);

            count -= 8;
            dst += 8;
        }
        // Leftovers
        for (int i = 0; i < count; i++) {
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

#ifdef SK_DEVELOPER
void SkNEONProcCoeffXfermode::toString(SkString* str) const {
    this->INHERITED::toString(str);
}
#endif

void SkNEONProcCoeffXfermode::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    if (!buffer.isCrossProcess()) {
        buffer.writeFunctionPtr((void*)fProcSIMD);
    }
}

////////////////////////////////////////////////////////////////////////////////

SkXfermodeProcSIMD gNEONXfermodeProcs[] = {
    [SkXfermode::kClear_Mode]   = NULL,
    [SkXfermode::kSrc_Mode]     = NULL,
    [SkXfermode::kDst_Mode]     = NULL,
    [SkXfermode::kSrcOver_Mode] = NULL,
    [SkXfermode::kDstOver_Mode] = NULL,
    [SkXfermode::kSrcIn_Mode]   = NULL,
    [SkXfermode::kDstIn_Mode]   = NULL,
    [SkXfermode::kSrcOut_Mode]  = NULL,
    [SkXfermode::kDstOut_Mode]  = NULL,
    [SkXfermode::kSrcATop_Mode] = NULL,
    [SkXfermode::kDstATop_Mode] = NULL,
    [SkXfermode::kXor_Mode]     = NULL,
    [SkXfermode::kPlus_Mode]    = NULL,
    [SkXfermode::kModulate_Mode]= NULL,
    [SkXfermode::kScreen_Mode]  = NULL,

    [SkXfermode::kOverlay_Mode]    = NULL,
    [SkXfermode::kDarken_Mode]     = NULL,
    [SkXfermode::kLighten_Mode]    = NULL,
    [SkXfermode::kColorDodge_Mode] = NULL,
    [SkXfermode::kColorBurn_Mode]  = NULL,
    [SkXfermode::kHardLight_Mode]  = NULL,
    [SkXfermode::kSoftLight_Mode]  = NULL,
    [SkXfermode::kDifference_Mode] = NULL,
    [SkXfermode::kExclusion_Mode]  = NULL,
    [SkXfermode::kMultiply_Mode]   = NULL,

    [SkXfermode::kHue_Mode]        = NULL,
    [SkXfermode::kSaturation_Mode] = NULL,
    [SkXfermode::kColor_Mode]      = NULL,
    [SkXfermode::kLuminosity_Mode] = NULL,
};

SK_COMPILE_ASSERT(
    SK_ARRAY_COUNT(gNEONXfermodeProcs) == SkXfermode::kLastMode + 1,
    mode_count_arm
);

#endif

SkProcCoeffXfermode* SkPlatformXfermodeFactory(const ProcCoeff& rec,
                                               SkXfermode::Mode mode) {
#if !SK_ARM_NEON_IS_NONE
    #if SK_ARM_NEON_IS_DYNAMIC
    if ((sk_cpu_arm_has_neon()) && (gNEONXfermodeProcs[mode] != NULL)) {
    #elif SK_ARM_NEON_IS_ALWAYS
    if (gNEONXfermodeProcs[mode] != NULL) {
    #endif
        return SkNEW_ARGS(SkNEONProcCoeffXfermode,
                          (rec, mode, gNEONXfermodeProcs[mode]));
    }
#endif
    return NULL;
}
