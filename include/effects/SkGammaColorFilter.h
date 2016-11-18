/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGammaColorFilter_DEFINED
#define SkGammaColorFilter_DEFINED

#include "SkColorFilter.h"
#include "SkRefCnt.h"

// This colorfilter can be used to perform pixel-by-pixel conversion between linear and 
// power-law color spaces. A gamma of 2.2 is interpreted to mean convert from sRGB to linear
// while a gamma of 1/2.2 is interpreted to mean convert from linear to sRGB. Any other 
// values are just directly applied (i.e., out = in^gamma)
// 
// More complicated color space mapping (i.e., ICC profiles) should be handled via the
// SkColorSpace object.
class SK_API SkGammaColorFilter : public SkColorFilter {
public:
    static sk_sp<SkColorFilter> Make(SkScalar gamma);

    void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const override;

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> asFragmentProcessor(GrContext*, SkColorSpace*) const override;
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLumaColorFilter)

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    SkGammaColorFilter(SkScalar gamma);

    SkScalar fGamma;
    typedef SkColorFilter INHERITED;
};

#endif
