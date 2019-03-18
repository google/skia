/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLumaColorFilter_DEFINED
#define SkLumaColorFilter_DEFINED

#include "SkColorFilter.h"
#include "SkRefCnt.h"

class SkRasterPipeline;

/**
 *  SkLumaColorFilter multiplies the luma of its input into the alpha channel,
 *  and sets the red, green, and blue channels to zero.
 *
 *    SkLumaColorFilter(r,g,b,a) = {0,0,0, a * luma(r,g,b)}
 *
 *  This is similar to a luminanceToAlpha feColorMatrix,
 *  but note how this filter folds in the previous alpha,
 *  something an feColorMatrix cannot do.
 *
 *    feColorMatrix(luminanceToAlpha; r,g,b,a) = {0,0,0, luma(r,g,b)}
 *
 *  (Despite its name, an feColorMatrix using luminanceToAlpha does
 *  actually compute luma, a dot-product of gamma-encoded color channels,
 *  not luminance, a dot-product of linear color channels.  So at least
 *  SkLumaColorFilter and feColorMatrix+luminanceToAlpha agree there.)
 */

 #include "SkFlattenable.h"

class SK_API SkLumaColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make();

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorSpaceInfo&) const override;
#endif

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SK_FLATTENABLE_HOOKS(SkLumaColorFilter)

    SkLumaColorFilter();
    void onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    typedef SkColorFilter INHERITED;
};

#endif
