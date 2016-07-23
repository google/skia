/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArithmeticMode.h"
#include "SkColorPriv.h"
#include "SkNx.h"
#include "SkReadBuffer.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
#include "SkWriteBuffer.h"
#if SK_SUPPORT_GPU
#include "SkArithmeticMode_gpu.h"
#endif

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

    void xfer32(SkPMColor[], const SkPMColor[], int count, const SkAlpha[]) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkArithmeticMode_scalar)

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> makeFragmentProcessorForImageFilter(
                                                sk_sp<GrFragmentProcessor> dst) const override;
    sk_sp<GrXPFactory> asXPFactory() const override;
#endif

private:
    void flatten(SkWriteBuffer& buffer) const override {
        buffer.writeScalar(fK[0]);
        buffer.writeScalar(fK[1]);
        buffer.writeScalar(fK[2]);
        buffer.writeScalar(fK[3]);
        buffer.writeBool(fEnforcePMColor);
    }

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

void SkArithmeticMode_scalar::xfer32(SkPMColor dst[], const SkPMColor src[],
                                 int count, const SkAlpha aaCoverage[]) const {
    const Sk4f k1 = fK[0] * (1/255.0f),
               k2 = fK[1],
               k3 = fK[2],
               k4 = fK[3] * 255.0f + 0.5f;

    auto pin = [](float min, const Sk4f& val, float max) {
        return Sk4f::Max(min, Sk4f::Min(val, max));
    };

    for (int i = 0; i < count; i++) {
        if (aaCoverage && aaCoverage[i] == 0) {
            continue;
        }

        Sk4f s = SkNx_cast<float>(Sk4b::Load(src+i)),
             d = SkNx_cast<float>(Sk4b::Load(dst+i)),
             r = pin(0, k1*s*d + k2*s + k3*d + k4, 255);

        if (fEnforcePMColor) {
            Sk4f a = SkNx_shuffle<3,3,3,3>(r);
            r = Sk4f::Min(a, r);
        }

        if (aaCoverage && aaCoverage[i] != 255) {
            Sk4f c = aaCoverage[i] * (1/255.0f);
            r = d + (r-d)*c;
        }

        SkNx_cast<uint8_t>(r).store(dst+i);
    }
}

#ifndef SK_IGNORE_TO_STRING
void SkArithmeticMode_scalar::toString(SkString* str) const {
    str->append("SkArithmeticMode_scalar: ");
    for (int i = 0; i < 4; ++i) {
        str->appendScalar(fK[i]);
        str->append(" ");
    }
    str->appendS32(fEnforcePMColor ? 1 : 0);
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


//////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
sk_sp<GrFragmentProcessor> SkArithmeticMode_scalar::makeFragmentProcessorForImageFilter(
                                                            sk_sp<GrFragmentProcessor> dst) const {
    return GrArithmeticFP::Make(SkScalarToFloat(fK[0]),
                                SkScalarToFloat(fK[1]),
                                SkScalarToFloat(fK[2]),
                                SkScalarToFloat(fK[3]),
                                fEnforcePMColor,
                                std::move(dst));
}

sk_sp<GrXPFactory> SkArithmeticMode_scalar::asXPFactory() const {
    return GrArithmeticXPFactory::Make(SkScalarToFloat(fK[0]),
                                       SkScalarToFloat(fK[1]),
                                       SkScalarToFloat(fK[2]),
                                       SkScalarToFloat(fK[3]),
                                       fEnforcePMColor);
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkArithmeticMode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArithmeticMode_scalar)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
