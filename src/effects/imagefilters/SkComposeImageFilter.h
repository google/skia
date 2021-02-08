/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkComposeImageFilter_DEFINED
#define SkComposeImageFilter_DEFINED

#include "src/core/SkImageFilter_Base.h"

class SkComposeImageFilter final : public SkImageFilter_Base {
public:
    explicit SkComposeImageFilter(sk_sp<SkImageFilter> inputs[2])
            : INHERITED(inputs, 2, nullptr) {
        SkASSERT(inputs[0].get());
        SkASSERT(inputs[1].get());
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    SkIRect onFilterBounds(const SkIRect&, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;
    bool onCanHandleComplexCTM() const override { return true; }

private:
    SK_FLATTENABLE_HOOKS(SkComposeImageFilter)

    static void RegisterFlattenables();

    using INHERITED = SkImageFilter_Base;
};

#endif
