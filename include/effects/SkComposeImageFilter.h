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
    virtual ~SkComposeImageFilter();

    static SkImageFilter* Create(SkImageFilter* outer, SkImageFilter* inner) {
        if (NULL == outer) {
            return SkSafeRef(inner);
        }
        if (NULL == inner) {
            return SkRef(outer);
        }
        SkImageFilter* inputs[2] = { outer, inner };
        return SkNEW_ARGS(SkComposeImageFilter, (inputs));
    }
    void computeFastBounds(const SkRect& src, SkRect* dst) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeImageFilter)

protected:
    explicit SkComposeImageFilter(SkImageFilter* inputs[2]) : INHERITED(2, inputs) {
        SkASSERT(inputs[0]);
        SkASSERT(inputs[1]);
    }
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const override;
    bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) const override;

private:
    typedef SkImageFilter INHERITED;
};

#endif
