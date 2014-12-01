#ifndef SkXfermode_opts_arm_neon_DEFINED
#define SkXfermode_opts_arm_neon_DEFINED

#include "SkXfermode_proccoeff.h"

class SkNEONProcCoeffXfermode : public SkProcCoeffXfermode {
public:
    SkNEONProcCoeffXfermode(const ProcCoeff& rec, SkXfermode::Mode mode,
                            void* procSIMD)
            : INHERITED(rec, mode), fProcSIMD(procSIMD) {}

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;
    virtual void xfer16(uint16_t* SK_RESTRICT dst, const SkPMColor* SK_RESTRICT src,
                        int count, const SkAlpha* SK_RESTRICT aa) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()

private:
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkNEONProcCoeffXfermode(SkReadBuffer& buffer);
#endif

    // void* is used to avoid pulling arm_neon.h in the core and having to build
    // it with -mfpu=neon.
    void* fProcSIMD;
    typedef SkProcCoeffXfermode INHERITED;
};

extern SkPMColor srcatop_modeproc_neon(SkPMColor src, SkPMColor dst);
extern SkPMColor dstatop_modeproc_neon(SkPMColor src, SkPMColor dst);
extern SkPMColor xor_modeproc_neon(SkPMColor src, SkPMColor dst);
extern SkPMColor plus_modeproc_neon(SkPMColor src, SkPMColor dst);
extern SkPMColor modulate_modeproc_neon(SkPMColor src, SkPMColor dst);

#endif //#ifdef SkXfermode_opts_arm_neon_DEFINED
