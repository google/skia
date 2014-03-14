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
                                      const CropRect* cropRect = NULL) {
        return SkNEW_ARGS(SkMergeImageFilter, (first, second, mode, cropRect));
    }
    static SkMergeImageFilter* Create(SkImageFilter* filters[], int count,
                                      const SkXfermode::Mode modes[] = NULL,
                                      const CropRect* cropRect = NULL) {
        return SkNEW_ARGS(SkMergeImageFilter, (filters, count, modes, cropRect));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

protected:
    SkMergeImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    SkMergeImageFilter(SkImageFilter* first, SkImageFilter* second,
                       SkXfermode::Mode = SkXfermode::kSrcOver_Mode,
                       const CropRect* cropRect = NULL);
    SkMergeImageFilter(SkImageFilter* filters[], int count,
                       const SkXfermode::Mode modes[] = NULL,
                       const CropRect* cropRect = NULL);
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
