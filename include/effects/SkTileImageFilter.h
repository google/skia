/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileImageFilter_DEFINED
#define SkTileImageFilter_DEFINED

#include "SkImageFilter.h"

class SK_API SkTileImageFilter : public SkImageFilter {
    typedef SkImageFilter INHERITED;

public:
    /** Tile image filter constructor
        @param srcRect  Defines the pixels to tile
        @param dstRect  Defines the pixels where tiles are drawn
        @param input    Input from which the subregion defined by srcRect will be tiled
    */
    SkTileImageFilter(const SkRect& srcRect, const SkRect& dstRect, SkImageFilter* input)
        : INHERITED(input), fSrcRect(srcRect), fDstRect(dstRect) {}

    virtual bool onFilterImage(Proxy* proxy, const SkBitmap& src, const SkMatrix& ctm,
                               SkBitmap* dst, SkIPoint* offset) SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTileImageFilter)

protected:
    explicit SkTileImageFilter(SkFlattenableReadBuffer& buffer);

    virtual void flatten(SkFlattenableWriteBuffer& buffer) const SK_OVERRIDE;

private:
    SkRect fSrcRect;
    SkRect fDstRect;
};

#endif
