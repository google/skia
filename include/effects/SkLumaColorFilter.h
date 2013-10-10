/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLumaColorFilter_DEFINED
#define SkLumaColorFilter_DEFINED

#include "SkColorFilter.h"

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
    static SkColorFilter* Create();

    virtual void filterSpan(const SkPMColor src[], int count, SkPMColor[]) const SK_OVERRIDE;

#if SK_SUPPORT_GPU
    virtual GrEffectRef* asNewEffect(GrContext*) const SK_OVERRIDE;
#endif

    SkDEVCODE(virtual void toString(SkString* str) const SK_OVERRIDE;)
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkLumaColorFilter)

protected:
    SkLumaColorFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

private:
    SkLumaColorFilter();

    typedef SkColorFilter INHERITED;
};

#endif
