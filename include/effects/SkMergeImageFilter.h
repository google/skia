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
    virtual ~SkMergeImageFilter();

    static SkMergeImageFilter* Create(SkImageFilter* first, SkImageFilter* second,
                                      SkXfermode::Mode mode = SkXfermode::kSrcOver_Mode,
                                      const CropRect* cropRect = NULL,
                                      uint32_t uniqueID = 0) {
        SkImageFilter* inputs[2] = { first, second };
        SkXfermode::Mode modes[2] = { mode, mode };
        return SkNEW_ARGS(SkMergeImageFilter, (inputs, 2, modes, cropRect, uniqueID));
    }
    static SkMergeImageFilter* Create(SkImageFilter* filters[], int count,
                                      const SkXfermode::Mode modes[] = NULL,
                                      const CropRect* cropRect = NULL,
                                      uint32_t uniqueID = 0) {
        return SkNEW_ARGS(SkMergeImageFilter, (filters, count, modes, cropRect, uniqueID));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

protected:
    SkMergeImageFilter(SkImageFilter* filters[], int count,
                       const SkXfermode::Mode modes[],
                       const CropRect* cropRect,
                       uint32_t uniqueID);
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    explicit SkMergeImageFilter(SkReadBuffer& buffer);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;

private:
    uint8_t*            fModes; // SkXfermode::Mode

    // private storage, to avoid dynamically allocating storage for our copy
    // of the modes (unless the count is so large we can't fit).
    intptr_t    fStorage[16];

    void initAllocModes();
    void initModes(const SkXfermode::Mode []);

    typedef SkImageFilter INHERITED;
};

#endif
