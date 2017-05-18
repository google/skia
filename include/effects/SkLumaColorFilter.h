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
class SK_API SkLumaColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make();

    void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const override;
    void filterSpan4f(const SkPM4f src[], int count, SkPM4f result[]) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLumaColorFilter)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SkLumaColorFilter();
    void onAppendStages(SkRasterPipeline*, SkColorSpace*, SkArenaAlloc*,
                        bool shaderIsOpaque) const override;

    typedef SkColorFilter INHERITED;
};

#endif
