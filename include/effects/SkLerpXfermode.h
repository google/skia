/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLerpXfermode_DEFINED
#define SkLerpXfermode_DEFINED

#include "SkXfermode.h"

class SK_API SkLerpXfermode : public SkXfermode {
public:
    /**
     *  result = scale * src + (1 - scale) * dst
     *
     *  When scale == 1, this is the same as kSrc_Mode
     *  When scale == 0, this is the same as kDst_Mode
     */
    static SkXfermode* Create(SkScalar scale);

    // overrides from SkXfermode
    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const override;
    virtual void xfer16(uint16_t dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const override;
    virtual void xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLerpXfermode)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SkLerpXfermode(unsigned scale256);

    unsigned fScale256;  // 0..256

    typedef SkXfermode INHERITED;
};

#endif
