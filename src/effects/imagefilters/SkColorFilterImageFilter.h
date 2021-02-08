/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterImageFilter_DEFINED
#define SkColorFilterImageFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "src/core/SkImageFilter_Base.h"

class SkColorFilterImageFilter final : public SkImageFilter_Base {
public:
    SkColorFilterImageFilter(sk_sp<SkColorFilter> cf, sk_sp<SkImageFilter> input,
                             const SkRect* cropRect)
            : INHERITED(&input, 1, cropRect)
            , fColorFilter(std::move(cf)) {}

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(const Context&, SkIPoint* offset) const override;
    bool onIsColorFilterNode(SkColorFilter**) const override;
    bool onCanHandleComplexCTM() const override { return true; }
    bool affectsTransparentBlack() const override;

private:
    SK_FLATTENABLE_HOOKS(SkColorFilterImageFilter)

    static void RegisterFlattenables();

    sk_sp<SkColorFilter> fColorFilter;

    using INHERITED = SkImageFilter_Base;
};

#endif
