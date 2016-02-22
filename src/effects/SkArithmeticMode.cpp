/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkArithmeticMode.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"
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
    const GrFragmentProcessor* getFragmentProcessorForImageFilter(
                                                const GrFragmentProcessor* dst) const override;
    GrXPFactory* asXPFactory() const override;
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

SkFlattenable* SkArithmeticMode_scalar::CreateProc(SkReadBuffer& buffer) {
    const SkScalar k1 = buffer.readScalar();
    const SkScalar k2 = buffer.readScalar();
    const SkScalar k3 = buffer.readScalar();
    const SkScalar k4 = buffer.readScalar();
    const bool enforcePMColor = buffer.readBool();
    return SkArithmeticMode::Create(k1, k2, k3, k4, enforcePMColor);
}

static int pinToByte(int value) {
    if (value < 0) {
        value = 0;
    } else if (value > 255) {
        value = 255;
    }
    return value;
}

static int arith(SkScalar k1, SkScalar k2, SkScalar k3, SkScalar k4,
                 int src, int dst) {
    SkScalar result = SkScalarMul(k1, src * dst) +
                      SkScalarMul(k2, src) +
                      SkScalarMul(k3, dst) +
                      k4;
    int res = SkScalarRoundToInt(result);
    return pinToByte(res);
}

static int blend(int src, int dst, int scale) {
    return dst + ((src - dst) * scale >> 8);
}

void SkArithmeticMode_scalar::xfer32(SkPMColor dst[], const SkPMColor src[],
                                 int count, const SkAlpha aaCoverage[]) const {
    SkScalar k1 = fK[0] / 255;
    SkScalar k2 = fK[1];
    SkScalar k3 = fK[2];
    SkScalar k4 = fK[3] * 255;

    for (int i = 0; i < count; ++i) {
        if ((nullptr == aaCoverage) || aaCoverage[i]) {
            SkPMColor sc = src[i];
            SkPMColor dc = dst[i];

            int a, r, g, b;

            a = arith(k1, k2, k3, k4, SkGetPackedA32(sc), SkGetPackedA32(dc));
            r = arith(k1, k2, k3, k4, SkGetPackedR32(sc), SkGetPackedR32(dc));
            g = arith(k1, k2, k3, k4, SkGetPackedG32(sc), SkGetPackedG32(dc));
            b = arith(k1, k2, k3, k4, SkGetPackedB32(sc), SkGetPackedB32(dc));
            if (fEnforcePMColor) {
                r = SkMin32(r, a);
                g = SkMin32(g, a);
                b = SkMin32(b, a);
            }

            // apply antialias coverage if necessary
            if (aaCoverage && 0xFF != aaCoverage[i]) {
                int scale = aaCoverage[i] + (aaCoverage[i] >> 7);
                a = blend(a, SkGetPackedA32(sc), scale);
                r = blend(r, SkGetPackedR32(sc), scale);
                g = blend(g, SkGetPackedG32(sc), scale);
                b = blend(b, SkGetPackedB32(sc), scale);
            }

            dst[i] = fEnforcePMColor ? SkPackARGB32(a, r, g, b) : SkPackARGB32NoCheck(a, r, g, b);
        }
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

SkXfermode* SkArithmeticMode::Create(SkScalar k1, SkScalar k2,
                                     SkScalar k3, SkScalar k4,
                                     bool enforcePMColor) {
    if (SkScalarNearlyZero(k1) && SkScalarNearlyEqual(k2, SK_Scalar1) &&
        SkScalarNearlyZero(k3) && SkScalarNearlyZero(k4)) {
        return SkXfermode::Create(SkXfermode::kSrc_Mode);
    } else if (SkScalarNearlyZero(k1) && SkScalarNearlyZero(k2) &&
               SkScalarNearlyEqual(k3, SK_Scalar1) && SkScalarNearlyZero(k4)) {
        return SkXfermode::Create(SkXfermode::kDst_Mode);
    }
    
    return new SkArithmeticMode_scalar(k1, k2, k3, k4, enforcePMColor);
}


//////////////////////////////////////////////////////////////////////////////

#if SK_SUPPORT_GPU
const GrFragmentProcessor* SkArithmeticMode_scalar::getFragmentProcessorForImageFilter(
                                                            const GrFragmentProcessor* dst) const {
    return GrArithmeticFP::Create(SkScalarToFloat(fK[0]),
                                  SkScalarToFloat(fK[1]),
                                  SkScalarToFloat(fK[2]),
                                  SkScalarToFloat(fK[3]),
                                  fEnforcePMColor,
                                  dst);
}

GrXPFactory* SkArithmeticMode_scalar::asXPFactory() const {
    return GrArithmeticXPFactory::Create(SkScalarToFloat(fK[0]),
                                         SkScalarToFloat(fK[1]),
                                         SkScalarToFloat(fK[2]),
                                         SkScalarToFloat(fK[3]),
                                         fEnforcePMColor);
}

#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkArithmeticMode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkArithmeticMode_scalar)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
