/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileImageFilter_DEFINED
#define SkTileImageFilter_DEFINED

#include "SkFlattenable.h"
#include "SkImageFilter.h"

class SK_API SkTileImageFilter : public SkImageFilter {
public:
    /** Create a tile image filter
        @param src  Defines the pixels to tile
        @param dst  Defines the pixels where tiles are drawn
        @param input    Input from which the subregion defined by srcRect will be tiled
    */
    static sk_sp<SkImageFilter> Make(const SkRect& src,
                                     const SkRect& dst,
                                     sk_sp<SkImageFilter> input);

    SkIRect onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                           MapDirection, const SkIRect* inputRect) const override;
    SkIRect onFilterNodeBounds(const SkIRect&, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;
    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;

private:
    SK_FLATTENABLE_HOOKS(SkTileImageFilter)

    SkTileImageFilter(const SkRect& srcRect, const SkRect& dstRect, sk_sp<SkImageFilter> input)
        : INHERITED(&input, 1, nullptr), fSrcRect(srcRect), fDstRect(dstRect) {}

    SkRect fSrcRect;
    SkRect fDstRect;

    typedef SkImageFilter INHERITED;
};

#endif
