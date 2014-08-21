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

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeImageFilter)

protected:
    explicit SkComposeImageFilter(SkImageFilter* inputs[2]) : INHERITED(2, inputs) {
        SkASSERT(inputs[0]);
        SkASSERT(inputs[1]);
    }
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    explicit SkComposeImageFilter(SkReadBuffer& buffer);
#endif

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) const SK_OVERRIDE;

private:
    typedef SkImageFilter INHERITED;
};

#endif
