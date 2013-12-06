/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapSource_DEFINED
#define SkBitmapSource_DEFINED

#include "SkImageFilter.h"
#include "SkBitmap.h"

class SK_API SkBitmapSource : public SkImageFilter {
public:
    explicit SkBitmapSource(const SkBitmap& bitmap);

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBitmapSource)

protected:
    explicit SkBitmapSource(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* offset) SK_OVERRIDE;

private:
    SkBitmap fBitmap;
    typedef SkImageFilter INHERITED;
};

#endif
