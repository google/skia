/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkFlattenable.h"

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

    static sk_sp<SkOverdrawColorFilter> Make(const SkPMColor colors[kNumColors]) {
        return sk_sp<SkOverdrawColorFilter>(new SkOverdrawColorFilter(colors));
    }

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
#endif

    static void RegisterFlattenables();

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    SK_FLATTENABLE_HOOKS(SkOverdrawColorFilter)

    SkOverdrawColorFilter(const SkPMColor colors[kNumColors]) {
        memcpy(fColors, colors, kNumColors * sizeof(SkPMColor));
    }

    void onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*, bool) const override;

    SkPMColor fColors[kNumColors];

    typedef SkColorFilter INHERITED;
};

#endif // SkOverdrawColorFilter_DEFINED
