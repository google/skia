/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilterImageFilter_DEFINED
#define SkColorFilterImageFilter_DEFINED

#include "SkSingleInputImageFilter.h"

class SkColorFilter;

class SkColorFilterImageFilter : public SkSingleInputImageFilter {
public:
    SkColorFilterImageFilter(SkColorFilter* cf, SkImageFilter* input = NULL);
    virtual ~SkColorFilterImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkColorFilterImageFilter)

protected:
    SkColorFilterImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;

private:
    SkColorFilter*  fColorFilter;

    typedef SkSingleInputImageFilter INHERITED;
};

#endif
