/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterImageFilter_DEFINED
#define SkColorFilterImageFilter_DEFINED

#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"

class SK_API SkColorFilterImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkColorFilter> cf,
                                     sk_sp<SkImageFilter> input,
                                     const CropRect* cropRect = nullptr);

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    bool onIsColorFilterNode(SkColorFilter**) const override;
    bool onCanHandleComplexCTM() const override { return true; }
    bool affectsTransparentBlack() const override;

private:
    SK_FLATTENABLE_HOOKS(SkColorFilterImageFilter)

    SkColorFilterImageFilter(sk_sp<SkColorFilter> cf,
                             sk_sp<SkImageFilter> input,
                             const CropRect* cropRect);

    sk_sp<SkColorFilter> fColorFilter;

    typedef SkImageFilter INHERITED;
};

#endif
