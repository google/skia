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
    static SkImageFilter* Create(SkImage*);
    static SkImageFilter* Create(SkImage*,
                                 const SkRect& srcRect,
                                 const SkRect& dstRect,
                                 SkFilterQuality);

    SkRect computeFastBounds(const SkRect& src) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageSource)

protected:
    void flatten(SkWriteBuffer&) const override;

    SkSpecialImage* onFilterImage(SkSpecialImage* source, const Context&,
                                  SkIPoint* offset) const override;

private:
    explicit SkImageSource(SkImage*);
    SkImageSource(SkImage*,
                  const SkRect& srcRect,
                  const SkRect& dstRect,
                  SkFilterQuality);

    sk_sp<SkImage>   fImage;
    SkRect           fSrcRect, fDstRect;
    SkFilterQuality  fFilterQuality;

    typedef SkImageFilter INHERITED;
};

#endif
