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
    /** Create a tile image filter
        @param srcRect  Defines the pixels to tile
        @param dstRect  Defines the pixels where tiles are drawn
        @param input    Input from which the subregion defined by srcRect will be tiled
    */
    static SkTileImageFilter* Create(const SkRect& srcRect, const SkRect& dstRect,
                                     SkImageFilter* input) {
        return SkNEW_ARGS(SkTileImageFilter, (srcRect, dstRect, input));
    }

    virtual bool onFilterImage(Proxy* proxy, const SkBitmap& src, const Context& ctx,
                               SkBitmap* dst, SkIPoint* offset) const SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect& src, const SkMatrix&,
                                SkIRect* dst) const SK_OVERRIDE;

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkTileImageFilter)

protected:
    SkTileImageFilter(const SkRect& srcRect, const SkRect& dstRect, SkImageFilter* input)
        : INHERITED(1, &input), fSrcRect(srcRect), fDstRect(dstRect) {}
    explicit SkTileImageFilter(SkReadBuffer& buffer);

    virtual void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE;

private:
    SkRect fSrcRect;
    SkRect fDstRect;
};

#endif
