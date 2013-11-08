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

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkNEONProcCoeffXfermode)

private:
    SkNEONProcCoeffXfermode(SkFlattenableReadBuffer& buffer);

    // void* is used to avoid pulling arm_neon.h in the core and having to build
    // it with -mfpu=neon.
    void* fProcSIMD;
    typedef SkProcCoeffXfermode INHERITED;
};

#endif //#ifdef SkXfermode_opts_arm_neon_DEFINED
