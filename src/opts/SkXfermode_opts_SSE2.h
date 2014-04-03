#ifndef SkXfermode_opts_SSE2_DEFINED
#define SkXfermode_opts_SSE2_DEFINED

#include "SkXfermode_proccoeff.h"

typedef __m128i (*SkXfermodeProcSIMD)(const __m128i& src, const __m128i& dst);

class SkSSE2ProcCoeffXfermode : public SkProcCoeffXfermode {
public:
    SkSSE2ProcCoeffXfermode(const ProcCoeff& rec, SkXfermode::Mode mode,
                            SkXfermodeProcSIMD procSIMD)
        : INHERITED(rec, mode), fProcSIMD(procSIMD) {}

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;
    virtual void xfer16(uint16_t dst[], const SkPMColor src[],
                        int count, const SkAlpha aa[]) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSSE2ProcCoeffXfermode)

private:
    SkSSE2ProcCoeffXfermode(SkReadBuffer& buffer);

    SkXfermodeProcSIMD fProcSIMD;
    typedef SkProcCoeffXfermode INHERITED;
};

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_SSE2(const ProcCoeff& rec,
                                                         SkXfermode::Mode mode);

#endif // SkXfermode_opts_SSE2_DEFINED
