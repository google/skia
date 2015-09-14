/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageSource_DEFINED
#define SkImageSource_DEFINED

#include "SkImageFilter.h"

class SkImage;

class SK_API SkImageSource : public SkImageFilter {
public:
    static SkImageFilter* Create(const SkImage*);
    static SkImageFilter* Create(const SkImage*,
                                 const SkRect& srcRect,
                                 const SkRect& dstRect,
                                 SkFilterQuality);

    void computeFastBounds(const SkRect& src, SkRect* dst) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageSource)

protected:
    void flatten(SkWriteBuffer&) const override;

    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                       SkBitmap* result, SkIPoint* offset) const override;

private:
    explicit SkImageSource(const SkImage*);
    SkImageSource(const SkImage*,
                  const SkRect& srcRect,
                  const SkRect& dstRect,
                  SkFilterQuality);

    SkAutoTUnref<const SkImage> fImage;
    SkRect                      fSrcRect, fDstRect;
    SkFilterQuality             fFilterQuality;

    typedef SkImageFilter INHERITED;
};

#endif
