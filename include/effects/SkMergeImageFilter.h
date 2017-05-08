/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMergeImageFilter_DEFINED
#define SkMergeImageFilter_DEFINED

#include "SkBlendMode.h"
#include "SkImageFilter.h"

class SK_API SkMergeImageFilter : public SkImageFilter {
public:
    ~SkMergeImageFilter() override;

    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> first, sk_sp<SkImageFilter> second,
                                     SkBlendMode, const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeN(sk_sp<SkImageFilter>[], int count, const SkBlendMode[],
                                     const CropRect* cropRect = nullptr);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;
    bool onCanHandleComplexCTM() const override { return true; }

private:
    SkMergeImageFilter(sk_sp<SkImageFilter> filters[], int count, const SkBlendMode modes[],
                       const CropRect* cropRect);

    uint8_t*    fModes; // SkBlendMode

    // private storage, to avoid dynamically allocating storage for our copy
    // of the modes (unless the count is so large we can't fit).
    intptr_t    fStorage[16];

    void initAllocModes();
    void initModes(const SkBlendMode[]);

    typedef SkImageFilter INHERITED;
};

#endif
