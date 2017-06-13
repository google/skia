/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermode_proccoeff_DEFINED
#define SkXfermode_proccoeff_DEFINED

#include "SkXfermodePriv.h"

class SkProcCoeffXfermode : public SkXfermode {
public:
    SkProcCoeffXfermode(SkBlendMode mode) {
        fMode = mode;
    }

    void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                const SkAlpha aa[]) const override;

protected:
    SkBlendMode getMode() const { return fMode; }

private:
    SkBlendMode fMode;

    friend class SkXfermode;

    typedef SkXfermode INHERITED;
};

#endif // #ifndef SkXfermode_proccoeff_DEFINED
