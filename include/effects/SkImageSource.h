/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageSource_DEFINED
#define SkImageSource_DEFINED

#include "SkImage.h"
#include "SkImageFilter.h"

class SK_API SkImageSource : public SkImageFilter {
public:
    static sk_sp<SkImageFilter> Make(sk_sp<SkImage> image);
    static sk_sp<SkImageFilter> Make(sk_sp<SkImage> image,
                                     const SkRect& srcRect,
                                     const SkRect& dstRect,
                                     SkFilterQuality filterQuality);

    SkRect computeFastBounds(const SkRect& src) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkImageSource)

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

private:
    explicit SkImageSource(sk_sp<SkImage>);
    SkImageSource(sk_sp<SkImage>,
                  const SkRect& srcRect,
                  const SkRect& dstRect,
                  SkFilterQuality);

    sk_sp<SkImage>   fImage;
    SkRect           fSrcRect, fDstRect;
    SkFilterQuality  fFilterQuality;

    typedef SkImageFilter INHERITED;
};

#endif
