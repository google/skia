#ifndef SkXfermode_proccoeff_DEFINED
#define SkXfermode_proccoeff_DEFINED

#include "SkXfermode.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

struct ProcCoeff {
    SkXfermodeProc      fProc;
    SkXfermode::Coeff   fSC;
    SkXfermode::Coeff   fDC;
};

#define CANNOT_USE_COEFF    SkXfermode::Coeff(-1)

class SK_API SkProcCoeffXfermode : public SkXfermode {
public:
    SkProcCoeffXfermode(const ProcCoeff& rec, Mode mode) {
        fMode = mode;
        fProc = rec.fProc;
        // these may be valid, or may be CANNOT_USE_COEFF
        fSrcCoeff = rec.fSC;
        fDstCoeff = rec.fDC;
    }

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const override;
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const override;
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const override;

    bool asMode(Mode* mode) const override;

    bool supportsCoverageAsAlpha() const override;

    bool isOpaque(SkXfermode::SrcColorOpacity opacityType) const override;

#if SK_SUPPORT_GPU
    virtual bool asFragmentProcessor(GrFragmentProcessor**,
                                     GrTexture* background) const override;

    virtual bool asXPFactory(GrXPFactory**) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcCoeffXfermode)

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    Mode getMode() const { return fMode; }

    SkXfermodeProc getProc() const { return fProc; }

private:
    SkXfermodeProc  fProc;
    Mode            fMode;
    Coeff           fSrcCoeff, fDstCoeff;

    friend class SkXfermode;

    typedef SkXfermode INHERITED;
};

#endif // #ifndef SkXfermode_proccoeff_DEFINED
