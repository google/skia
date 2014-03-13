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

class SkProcCoeffXfermode : public SkProcXfermode {
public:
    static SkProcCoeffXfermode* Create(const ProcCoeff& rec, Mode mode) {
        return SkNEW_ARGS(SkProcCoeffXfermode, (rec, mode));
    }

    virtual bool asMode(Mode* mode) const SK_OVERRIDE;

    virtual bool asCoeff(Coeff* sc, Coeff* dc) const SK_OVERRIDE;

#if SK_SUPPORT_GPU
    virtual bool asNewEffect(GrEffectRef** effect,
                             GrTexture* background) const SK_OVERRIDE;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcCoeffXfermode)

protected:
    SkProcCoeffXfermode(const ProcCoeff& rec, Mode mode)
            : INHERITED(rec.fProc) {
        fMode = mode;
        // these may be valid, or may be CANNOT_USE_COEFF
        fSrcCoeff = rec.fSC;
        fDstCoeff = rec.fDC;
    }

    SkProcCoeffXfermode(SkReadBuffer& buffer);

    virtual void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE;

    Mode getMode() const {
        return fMode;
    }

private:
    Mode    fMode;
    Coeff   fSrcCoeff, fDstCoeff;

    typedef SkProcXfermode INHERITED;
};

#endif // #ifndef SkXfermode_proccoeff_DEFINED
