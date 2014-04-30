/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermode_opts_SSE2_DEFINED
#define SkXfermode_opts_SSE2_DEFINED

#include "SkTypes.h"
#include "SkXfermode_proccoeff.h"

class SK_API SkSSE2ProcCoeffXfermode : public SkProcCoeffXfermode {
public:
    SkSSE2ProcCoeffXfermode(const ProcCoeff& rec, SkXfermode::Mode mode,
                            void* procSIMD)
        : INHERITED(rec, mode), fProcSIMD(procSIMD) {}

    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;
    virtual void xfer16(uint16_t dst[], const SkPMColor src[],
                        int count, const SkAlpha aa[]) const SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkSSE2ProcCoeffXfermode)

private:
    SkSSE2ProcCoeffXfermode(SkReadBuffer& buffer);

    void* fProcSIMD;
    typedef SkProcCoeffXfermode INHERITED;
};

SkProcCoeffXfermode* SkPlatformXfermodeFactory_impl_SSE2(const ProcCoeff& rec,
                                                         SkXfermode::Mode mode);

#endif // SkXfermode_opts_SSE2_DEFINED
