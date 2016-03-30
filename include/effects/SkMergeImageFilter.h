/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMergeImageFilter_DEFINED
#define SkMergeImageFilter_DEFINED

#include "SkImageFilter.h"

#include "SkXfermode.h"

class SK_API SkMergeImageFilter : public SkImageFilter {
public:
    ~SkMergeImageFilter() override;

    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> first, sk_sp<SkImageFilter> second,
                                     SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode,
                                     const CropRect* cropRect = nullptr) {
        sk_sp<SkImageFilter> inputs[2] = { first, second };
        SkXfermode::Mode modes[2] = { mode, mode };
        return sk_sp<SkImageFilter>(new SkMergeImageFilter(inputs, 2, modes, cropRect));
    }

    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> filters[],
                                     int count,
                                     const SkXfermode::Mode modes[] = nullptr,
                                     const CropRect* cropRect = nullptr) {
        return sk_sp<SkImageFilter>(new SkMergeImageFilter(filters, count, modes, cropRect));
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

#ifdef SK_SUPPORT_LEGACY_IMAGEFILTER_PTR
    static SkImageFilter* Create(SkImageFilter* first, SkImageFilter* second,
                                 SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode,
                                 const CropRect* cropRect = nullptr) {
        return Make(sk_ref_sp<SkImageFilter>(first),
                    sk_ref_sp<SkImageFilter>(second),
                    mode, cropRect).release();
    }

    static SkImageFilter* Create(SkImageFilter* filters[], int count,
                                 const SkXfermode::Mode modes[] = nullptr,
                                 const CropRect* cropRect = nullptr) {
        SkAutoTDeleteArray<sk_sp<SkImageFilter>> temp(new sk_sp<SkImageFilter>[count]);
        for (int i = 0; i < count; ++i) {
            temp[i] = sk_ref_sp<SkImageFilter>(filters[i]);
        }
        return Make(temp.get(), count, modes, cropRect).release();
    }
#endif

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SkMergeImageFilter(sk_sp<SkImageFilter> filters[], int count, const SkXfermode::Mode modes[],
                       const CropRect* cropRect);

    uint8_t*    fModes; // SkXfermode::Mode

    // private storage, to avoid dynamically allocating storage for our copy
    // of the modes (unless the count is so large we can't fit).
    intptr_t    fStorage[16];

    void initAllocModes();
    void initModes(const SkXfermode::Mode []);

    typedef SkImageFilter INHERITED;
};

#endif
