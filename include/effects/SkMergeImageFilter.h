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
    SkMergeImageFilter(SkImageFilter* first, SkImageFilter* second,
                       SkXfermode::Mode = SkXfermode::kSrcOver_Mode);
    SkMergeImageFilter(SkImageFilter* filters[], int count,
                       const SkXfermode::Mode modes[] = NULL);
    virtual ~SkMergeImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

protected:
    SkMergeImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) SK_OVERRIDE;

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
