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

    static SkImageFilter* Create(SkImageFilter* first, SkImageFilter* second,
                                 SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode,
                                 const CropRect* cropRect = NULL) {
        SkImageFilter* inputs[2] = { first, second };
        SkXfermode::Mode modes[2] = { mode, mode };
        return new SkMergeImageFilter(inputs, 2, modes, cropRect);
    }

    static SkImageFilter* Create(SkImageFilter* filters[], int count,
                                 const SkXfermode::Mode modes[] = NULL,
                                const CropRect* cropRect = NULL) {
        return new SkMergeImageFilter(filters, count, modes, cropRect);
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SkMergeImageFilter(SkImageFilter* filters[], int count, const SkXfermode::Mode modes[],
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
