/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkComposeImageFilter_DEFINED
#define SkComposeImageFilter_DEFINED

#include "SkImageFilter.h"

class SK_API SkComposeImageFilter : public SkImageFilter {
public:
    static SkImageFilter* Create(SkImageFilter* outer, SkImageFilter* inner) {
        if (!outer) {
            return SkSafeRef(inner);
        }
        if (!inner) {
            return SkRef(outer);
        }
        SkImageFilter* inputs[2] = { outer, inner };
        return new SkComposeImageFilter(inputs);
    }
    SkRect computeFastBounds(const SkRect& src) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeImageFilter)

protected:
    explicit SkComposeImageFilter(SkImageFilter* inputs[2]) : INHERITED(2, inputs) {
        SkASSERT(inputs[0]);
        SkASSERT(inputs[1]);
    }
    SkSpecialImage* onFilterImage(SkSpecialImage* source, const Context&,
                                  SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect&, const SkMatrix&, MapDirection) const override;

private:
    typedef SkImageFilter INHERITED;
};

#endif
