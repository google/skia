/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLumaXfermode_DEFINED
#define SkLumaXfermode_DEFINED

#include "SkXfermode.h"

/**
 *  Luminance mask transfer, as defined in
 *  http://www.w3.org/TR/SVG/masking.html#Masking
 *  http://www.w3.org/TR/css-masking/#MaskValues
 *
 *  The luminance-to-alpha function is applied before performing a standard
 *  SrcIn/DstIn/SrcOver xfer:
 *
 *    luma(C) = (0.2125 * C.r + 0.7154 * C.g + 0.0721 * C.b) * C.a
 *
 *  (where C is un-premultiplied)
 */
class SK_API SkLumaMaskXfermode : public SkXfermode {
public:
    /** Return an SkLumaMaskXfermode object for the specified submode.
     *
     *  Only kSrcIn_Mode, kDstIn_Mode kSrcOver_Mode are supported - for everything else,
     *  the factory returns NULL.
     */
    static SkXfermode* Create(SkXfermode::Mode);

    virtual SkPMColor xferColor(SkPMColor, SkPMColor) const SK_OVERRIDE;
    virtual void xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                        const SkAlpha aa[]) const SK_OVERRIDE;

    SK_DEVELOPER_TO_STRING()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLumaMaskXfermode)
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

#if SK_SUPPORT_GPU
    virtual bool asNewEffectOrCoeff(GrContext*, GrEffectRef**, Coeff*, Coeff*,
                                    GrTexture*) const SK_OVERRIDE;
#endif

protected:
    SkLumaMaskXfermode(SkFlattenableReadBuffer&);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    SkLumaMaskXfermode(SkXfermode::Mode);

private:
    const SkXfermode::Mode fMode;

    typedef SkXfermode INHERITED;

    virtual SkPMColor lumaProc(const SkPMColor a, const SkPMColor b) const;
};

#endif
