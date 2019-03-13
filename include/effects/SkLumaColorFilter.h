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
 *  Luminance-to-alpha color filter, as defined in
 *  http://www.w3.org/TR/SVG/masking.html#Masking
 *  http://www.w3.org/TR/css-masking/#MaskValues
 *
 *  The resulting color is black with transparency equal to the
 *  luminance value modulated by alpha:
 *
 *    C' = [ Lum * a, 0, 0, 0 ]
 *
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
