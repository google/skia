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
    static SkBitmapSource* Create(const SkBitmap& bitmap, const SkRect& srcRect,
                                  const SkRect& dstRect) {
        return SkNEW_ARGS(SkBitmapSource, (bitmap, srcRect, dstRect));
    }
    virtual void computeFastBounds(const SkRect& src, SkRect* dst) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBitmapSource)

protected:
    explicit SkBitmapSource(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix& ctm, SkIRect* dst) const SK_OVERRIDE;

#ifdef SK_SUPPORT_LEGACY_PUBLICEFFECTCONSTRUCTORS
public:
#endif
    explicit SkBitmapSource(const SkBitmap& bitmap);
    SkBitmapSource(const SkBitmap& bitmap, const SkRect& srcRect, const SkRect& dstRect);

private:
    SkBitmap fBitmap;
    SkRect   fSrcRect, fDstRect;
    typedef SkImageFilter INHERITED;
};

#endif
