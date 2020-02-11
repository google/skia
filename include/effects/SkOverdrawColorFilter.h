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

    // For historical reasons, this version of Make() assumes the array is RGBA-premul
    static sk_sp<SkColorFilter> Make(const uint32_t colors[kNumColors]);

    static sk_sp<SkColorFilter> MakeWithSkColors(const SkColor colors[kNumColors]);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(GrRecordingContext*,
                                                             const GrColorInfo&) const override;
#endif

    static void RegisterFlattenables();

protected:
    void flatten(SkWriteBuffer& buffer) const override;

private:
    SK_FLATTENABLE_HOOKS(SkOverdrawColorFilter)

    SkOverdrawColorFilter(const SkPMColor colors[kNumColors]);

    bool onAppendStages(const SkStageRec&, bool) const override;

    SkPMColor fColors[kNumColors];

    typedef SkColorFilter INHERITED;
};

#endif // SkOverdrawColorFilter_DEFINED
