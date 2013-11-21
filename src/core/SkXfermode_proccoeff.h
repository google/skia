#ifndef SkXfermode_proccoeff_DEFINED
#define SkXfermode_proccoeff_DEFINED

#include "SkXfermode.h"
#include "SkFlattenableBuffers.h"

struct ProcCoeff {
    SkXfermodeProc      fProc;
    SkXfermode::Coeff   fSC;
    SkXfermode::Coeff   fDC;
};

#define CANNOT_USE_COEFF    SkXfermode::Coeff(-1)

class SkProcCoeffXfermode : public SkProcXfermode {
public:
    SkProcCoeffXfermode(const ProcCoeff& rec, Mode mode)
            : INHERITED(rec.fProc) {
        fMode = mode;
        // these may be valid, or may be CANNOT_USE_COEFF
        fSrcCoeff = rec.fSC;
        fDstCoeff = rec.fDC;
    }

    virtual bool asMode(Mode* mode) const SK_OVERRIDE;

    virtual bool asCoeff(Coeff* sc, Coeff* dc) const SK_OVERRIDE;

#if SK_SUPPORT_GPU
    virtual bool asNewEffect(GrEffectRef** effect,
                             GrTexture* background) const SK_OVERRIDE;
#endif

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcCoeffXfermode)

protected:
    SkProcCoeffXfermode(SkFlattenableReadBuffer& buffer);

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE;

    Mode getMode() const {
        return fMode;
    }

private:
    Mode    fMode;
    Coeff   fSrcCoeff, fDstCoeff;

    typedef SkProcXfermode INHERITED;
};

#endif // #ifndef SkXfermode_proccoeff_DEFINED
