/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "../../src/core/SkReadBuffer.h"

#ifndef SkOverdrawColorFilter_DEFINED
#define SkOverdrawColorFilter_DEFINED

/**
 *  Uses the value in the src alpha channel to set the dst pixel.
 *  0             -> fColors[0]
 *  1             -> fColors[1]
 *  ...
 *  5 (or larger) -> fColors[5]
 *
 */
class SkOverdrawColorFilter : public SkColorFilter {
public:
    static constexpr int kNumColors = 6;

    static sk_sp<SkOverdrawColorFilter> Make(const SkPMColor colors[kNumColors]) {
        return sk_sp<SkOverdrawColorFilter>(new SkOverdrawColorFilter(colors));
    }

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    void filterSpan(const SkPMColor src[], int count, SkPMColor dst[]) const override;
    void toString(SkString* str) const override;

    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer& buffer);
    Factory getFactory() const override { return CreateProc; }
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    SkOverdrawColorFilter(const SkPMColor colors[kNumColors]) {
        memcpy(fColors, colors, kNumColors * sizeof(SkPMColor));
    }

    SkPMColor fColors[kNumColors];

    typedef SkColorFilter INHERITED;
};

#endif // SkOverdrawColorFilter_DEFINED
