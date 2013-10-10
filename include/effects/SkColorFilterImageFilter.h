/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterImageFilter_DEFINED
#define SkColorFilterImageFilter_DEFINED

#include "SkImageFilter.h"

class SkColorFilter;

class SK_API SkColorFilterImageFilter : public SkImageFilter {
public:
    static SkColorFilterImageFilter* Create(SkColorFilter* cf,
                                            SkImageFilter* input = NULL,
                                            const CropRect* cropRect = NULL);
    virtual ~SkColorFilterImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorFilterImageFilter)

protected:
    SkColorFilterImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;

    virtual bool asColorFilter(SkColorFilter**) const SK_OVERRIDE;

private:
    SkColorFilterImageFilter(SkColorFilter* cf,
                             SkImageFilter* input,
                             const CropRect* cropRect = NULL);
    SkColorFilter*  fColorFilter;

    typedef SkImageFilter INHERITED;
};

#endif
