/*
 * Copyright 2007 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPixelXorXfermode_DEFINED
#define SkPixelXorXfermode_DEFINED

#include "SkXfermode.h"

/** SkPixelXorXfermode implements a simple pixel xor (op ^ src ^ dst).
    This transformation does not follow premultiplied conventions, therefore
    this proc *always* returns an opaque color (alpha == 255). Thus it is
    not really usefull for operating on blended colors.
*/
class SK_API SkPixelXorXfermode : public SkXfermode {
public:
    static SkXfermode* Create(SkColor opColor) {
        return new SkPixelXorXfermode(opColor);
    }

#if SK_SUPPORT_GPU
    const GrFragmentProcessor* getFragmentProcessorForImageFilter(
                                                    const GrFragmentProcessor* dst) const override;
    GrXPFactory* asXPFactory() const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkPixelXorXfermode)

protected:
    void flatten(SkWriteBuffer&) const override;
    SkPMColor xferColor(SkPMColor src, SkPMColor dst) const override;

private:
    explicit SkPixelXorXfermode(SkColor opColor) {
        fOpColor = SkPreMultiplyColor(opColor | 0xFF000000);
    }

    SkPMColor fOpColor;

    typedef SkXfermode INHERITED;
};

#endif
