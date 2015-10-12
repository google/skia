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
    static SkImageFilter* Create(SkColorFilter* cf, SkImageFilter* input = NULL,
                                 const CropRect* cropRect = NULL);

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorFilterImageFilter)

protected:
    void flatten(SkWriteBuffer&) const override;
    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&, SkBitmap* result,
                       SkIPoint* loc) const override;
    bool onIsColorFilterNode(SkColorFilter**) const override;
    bool affectsTransparentBlack() const override;

private:
    SkColorFilterImageFilter(SkColorFilter* cf,
                             SkImageFilter* input,
                             const CropRect* cropRect);
    virtual ~SkColorFilterImageFilter();

    SkColorFilter*  fColorFilter;

    typedef SkImageFilter INHERITED;
};

#endif
