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
    static SkBitmapSource* Create(const SkBitmap& bitmap) {
        return SkNEW_ARGS(SkBitmapSource, (bitmap));
    }
    static SkBitmapSource* Create(const SkBitmap& bitmap,
                                  const SkRect& srcRect, const SkRect& dstRect,
                                  SkFilterQuality filterQuality = kHigh_SkFilterQuality) {
        return SkNEW_ARGS(SkBitmapSource, (bitmap, srcRect, dstRect, filterQuality));
    }
    void computeFastBounds(const SkRect& src, SkRect* dst) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBitmapSource)

protected:
    explicit SkBitmapSource(const SkBitmap& bitmap);
    SkBitmapSource(const SkBitmap& bitmap,
                   const SkRect& srcRect, const SkRect& dstRect,
                   SkFilterQuality filterQuality);
    void flatten(SkWriteBuffer&) const override;

    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;

private:
    SkBitmap fBitmap;
    SkRect   fSrcRect, fDstRect;
    SkFilterQuality fFilterQuality;

    typedef SkImageFilter INHERITED;
};

#endif
