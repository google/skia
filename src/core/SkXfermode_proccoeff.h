/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermode_proccoeff_DEFINED
#define SkXfermode_proccoeff_DEFINED

#include "SkXfermodePriv.h"

#define CANNOT_USE_COEFF    SkXfermode::Coeff(-1)

class SK_API SkProcCoeffXfermode : public SkXfermode {
public:
    SkProcCoeffXfermode(SkXfermodeProc proc, SkBlendMode mode) {
        fMode = mode;
        fProc = proc;
    }

    void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                const SkAlpha aa[]) const override;

    bool asMode(SkBlendMode* mode) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> makeFragmentProcessorForImageFilter(
                                                        sk_sp<GrFragmentProcessor>) const override;
    const GrXPFactory* asXPFactory() const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkProcCoeffXfermode)

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    SkBlendMode getMode() const { return fMode; }

    SkXfermodeProc getProc() const { return fProc; }

private:
    SkXfermodeProc  fProc;
    SkBlendMode     fMode;

    friend class SkXfermode;

    typedef SkXfermode INHERITED;
};

#endif // #ifndef SkXfermode_proccoeff_DEFINED
