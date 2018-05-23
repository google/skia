/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageSource_DEFINED
#define SkImageSource_DEFINED

#include "SkFlattenable.h"
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

    Factory getFactory() const override { return CreateProc; }

protected:
    void flatten(SkWriteBuffer&) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;

    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    explicit SkImageSource(sk_sp<SkImage>);
    SkImageSource(sk_sp<SkImage>,
                  const SkRect& srcRect,
                  const SkRect& dstRect,
                  SkFilterQuality);
    static sk_sp<SkFlattenable> CreateProc(SkReadBuffer&);
    friend class SkFlattenable::PrivateInitializer;

    sk_sp<SkImage>   fImage;
    SkRect           fSrcRect, fDstRect;
    SkFilterQuality  fFilterQuality;

    typedef SkImageFilter INHERITED;
};

#endif
