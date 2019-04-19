/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMergeImageFilter_DEFINED
#define SkMergeImageFilter_DEFINED

#include "SkFlattenable.h"
#include "SkImageFilter.h"

class SK_API SkMergeImageFilter : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter>* const filters, int count,
                                     const CropRect* cropRect = nullptr);

    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> first, sk_sp<SkImageFilter> second,
                                     const CropRect* cropRect = nullptr) {
        sk_sp<SkImageFilter> array[] = {
            std::move(first),
            std::move(second),
        };
        return Make(array, 2, cropRect);
    }

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    bool onCanHandleComplexCTM() const override { return true; }

private:
    SK_FLATTENABLE_HOOKS(SkMergeImageFilter)

    SkMergeImageFilter(sk_sp<SkImageFilter>* const filters, int count, const CropRect* cropRect);

    typedef SkImageFilter INHERITED;
};

#endif
