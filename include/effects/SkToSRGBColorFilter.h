/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkToSRGBColorFilter_DEFINED
#define SkToSRGBColorFilter_DEFINED

#include "SkColorFilter.h"
#include "SkRefCnt.h"

class SkColorSpace;
class SkRasterPipeline;

/**
 *  Color filter that converts from supplied color space to sRGB (both gamut and transfer function).
 */
class SK_API SkToSRGBColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make(sk_sp<SkColorSpace> srcColorSpace);

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrContext*, const GrColorSpaceInfo&) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkToSRGBColorFilter)

private:
    void flatten(SkWriteBuffer&) const override;
    SkToSRGBColorFilter(sk_sp<SkColorSpace>);
    void onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;

    sk_sp<SkColorSpace> fSrcColorSpace;

    typedef SkColorFilter INHERITED;
};

#endif
