/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorFilter.h"
#include "include/core/SkFlattenable.h"

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
class SK_API SkOverdrawColorFilter : public SkColorFilter {
public:
    static constexpr int kNumColors = 6;

    static sk_sp<SkColorFilter> MakeWithSkColors(const SkColor colors[kNumColors]) {
        return sk_sp<SkColorFilter>(new SkOverdrawColorFilter(colors));
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(GrRecordingContext*,
                                                             const GrColorInfo&) const override;
#endif

    static void RegisterFlattenables();

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    SK_FLATTENABLE_HOOKS(SkOverdrawColorFilter)

    SkOverdrawColorFilter(const SkColor colors[kNumColors]) {
        memcpy(fColors, colors, kNumColors * sizeof(SkColor));
    }

    bool onAppendStages(const SkStageRec&, bool) const override;
    skvm::Color onProgram(skvm::Builder*, skvm::Color, SkColorSpace*, skvm::Uniforms*,
                          SkArenaAlloc*) const override;

    SkColor fColors[kNumColors];

    typedef SkColorFilter INHERITED;
};

#endif // SkOverdrawColorFilter_DEFINED
