/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArithmeticModePriv.h"
#include "SkReadBuffer.h"

// This class only exists to unflatten instances that were serialized into old pictures as part of
// SkXfermodeImageFilter before the advent of SkBlendMode. Those image filters will now be
// transformed to SkArithmeticImageFilter which does not use this class in its implementation.
class SkArithmeticMode_scalar : public SkXfermode {
public:
    SkArithmeticMode_scalar(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4,
                            bool enforcePMColor) {
        fK[0] = k1;
        fK[1] = k2;
        fK[2] = k3;
        fK[3] = k4;
        fEnforcePMColor = enforcePMColor;
    }

    void xfer32(SkPMColor[], const SkPMColor[], int count, const SkAlpha[]) const override {
        SkFAIL("This should never be called.");
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkArithmeticMode_scalar)

    // This is used to extract the arithmetic params into an SkArithmeticImageFilter. Afterwards,
    // this object is destroyed and arithemtic blending is implemented directly in the image filter.
    bool isArithmetic(SkArithmeticParams* params) const override {
        if (params) {
            memcpy(params->fK, fK, 4 * sizeof(float));
            params->fEnforcePMColor = fEnforcePMColor;
        }
        return true;
    }

private:
    void flatten(SkWriteBuffer& buffer) const override { SkFAIL("This shouild never be called."); }

    SkScalar fK[4];
    bool fEnforcePMColor;

    friend class SkArithmeticMode;

    typedef SkXfermode INHERITED;
};

sk_sp<SkFlattenable> SkArithmeticMode_scalar::CreateProc(SkReadBuffer& buffer) {
    const SkScalar k1 = buffer.readScalar();
    const SkScalar k2 = buffer.readScalar();
    const SkScalar k3 = buffer.readScalar();
    const SkScalar k4 = buffer.readScalar();
    const bool enforcePMColor = buffer.readBool();
    return SkArithmeticMode::Make(k1, k2, k3, k4, enforcePMColor);
}

#ifndef SK_IGNORE_TO_STRING
void SkArithmeticMode_scalar::toString(SkString* str) const {
    SkFAIL("This should never be called.");
}
#endif

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkXfermode> SkArithmeticMode::Make(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4,
                                         bool enforcePMColor) {
    if (SkScalarNearlyZero(k1) && SkScalarNearlyEqual(k2, SK_Scalar1) &&
        SkScalarNearlyZero(k3) && SkScalarNearlyZero(k4)) {
        return SkXfermode::Make(SkXfermode::kSrc_Mode);
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) &&
               SkScalarNearlyEqual(k3, SK_Scalar1) && SkScalarNearlyZero(k4)) {
        return SkXfermode::Make(SkXfermode::kDst_Mode);
    }
    return sk_make_sp<SkArithmeticMode_scalar>(k1, k2, k3, k4, enforcePMColor);
}

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkArithmeticMode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArithmeticMode_scalar)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
