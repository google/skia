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
                                     SkBlendMode, const CropRect* cropRect = nullptr);
    static sk_sp<SkImageFilter> MakeN(sk_sp<SkImageFilter>[], int count, const SkBlendMode[],
                                     const CropRect* cropRect = nullptr);

#ifdef SK_SUPPORT_LEGACY_XFERMODE_PARAM
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> first, sk_sp<SkImageFilter> second,
                                     SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode,
                                     const CropRect* cropRect = nullptr) {
        return Make(first, second, (SkBlendMode)mode, cropRect);
    }
    static sk_sp<SkImageFilter> Make(sk_sp<SkImageFilter> filters[],
                                     int count,
                                     const SkXfermode::Mode modes[] = nullptr,
                                     const CropRect* cropRect = nullptr) {
        static_assert(sizeof(SkXfermode::Mode) == sizeof(SkBlendMode), "size mismatch");
        return MakeN(filters, count, (const SkBlendMode*)modes, cropRect);
    }
#endif

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    bool onCanHandleComplexCTM() const override { return true; }

private:
    SkMergeImageFilter(sk_sp<SkImageFilter> filters[], int count, const SkBlendMode modes[],
                       const CropRect* cropRect);

    uint8_t*    fModes; // SkXfermode::Mode

    // private storage, to avoid dynamically allocating storage for our copy
    // of the modes (unless the count is so large we can't fit).
    intptr_t    fStorage[16];

    void initAllocModes();
    void initModes(const SkBlendMode[]);

    typedef SkImageFilter INHERITED;
};

#endif
