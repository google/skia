/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkComposeImageFilter_DEFINED
#define SkComposeImageFilter_DEFINED

#include "SkFlattenable.h"
#include "SkImageFilter.h"

class SK_API SkComposeImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> outer, sk_sp<SkImageFilter> inner);

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    explicit SkComposeImageFilter(sk_sp<SkImageFilter> inputs[2]) : INHERITED(inputs, 2, nullptr) {
        SkASSERT(inputs[0].get());
        SkASSERT(inputs[1].get());
    }
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;
    bool onCanHandleComplexCTM() const override { return true; }

private:
    SK_FLATTENABLE_HOOKS(SkComposeImageFilter)

    typedef SkImageFilter INHERITED;
};

#endif
