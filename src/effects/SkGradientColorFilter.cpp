/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGradientColorFilter.h"
#include "SkColorPriv.h"
#include <algorithm>

#include "GrFragmentProcessor.h"

class SkGradColorFilter : public SkColorFilter {
public:
    SkGradColorFilter(float r, float g, float b, const SkColor grad[2]) {
        fCoeff[0] = r;
        fCoeff[1] = g;
        fCoeff[2] = b;
        fGrad[0] = grad[0];
        fGrad[1] = grad[1];
    }

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override {
        for (int i = 0; i < count; ++i) {
            SkColor c = src[i];
            float t = fCoeff[0] * SkGetPackedR32(c) +
                      fCoeff[1] * SkGetPackedG32(c) +
                      fCoeff[2] * SkGetPackedB32(c);
            t /= 255.f;
            t = std::min(std::max(t, 0.f), 1.f);
            dst[i] = SkFastFourByteInterp256(fGrad[1], fGrad[0], (int)(t * 256));
        }
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkGradColorFilter)

protected:
    void flatten(SkWriteBuffer&) const override {}

private:
//    bool onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*, bool shaderIsOpaque) const override;

    float fCoeff[3];    // red, green, blue
    SkColor fGrad[2];

    typedef SkColorFilter INHERITED;
};

sk_sp<SkFlattenable> SkGradColorFilter::CreateProc(SkReadBuffer&) {
    return nullptr;
}

#ifndef SK_IGNORE_TO_STRING
void SkGradColorFilter::toString(SkString* str) const {
    str->append("SkLumaColorFilter ");
}
#endif

sk_sp<SkColorFilter> SkGradientColorFilter::Make(float rCoeff, float gCoeff, float bCoeff,
                                                 const SkColor colors[], const SkScalar pos[],
                                                 int count) {
    return sk_sp<SkColorFilter>(new SkGradColorFilter(rCoeff, gCoeff, bCoeff, colors));
}
