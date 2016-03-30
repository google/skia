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
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> outer, sk_sp<SkImageFilter> inner) {
        if (!outer) {
            return inner;
        }
        if (!inner) {
            return outer;
        }
        sk_sp<SkImageFilter> inputs[2] = { std::move(outer), std::move(inner) };
        return sk_sp<SkImageFilter>(new SkComposeImageFilter(inputs));
    }
    SkRect computeFastBounds(const SkRect& src) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(SkImageFilter* outer, SkImageFilter* inner) {
        return Make(sk_ref_sp<SkImageFilter>(outer),
                    sk_ref_sp<SkImageFilter>(inner)).release();
    }
#endif

protected:
    explicit SkComposeImageFilter(sk_sp<SkImageFilter> inputs[2]) : INHERITED(inputs, 2, nullptr) {
        SkASSERT(inputs[0].get());
        SkASSERT(inputs[1].get());
    }
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect&, const SkMatrix&, MapDirection) const override;

private:
    typedef SkImageFilter INHERITED;
};

#endif
