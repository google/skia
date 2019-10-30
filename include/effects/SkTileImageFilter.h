/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTileImageFilter_DEFINED
#define SkTileImageFilter_DEFINED

#include "include/core/SkImageFilter.h"

// DEPRECATED: Use include/effects/SkImageFilters::Tile
class SK_API SkTileImageFilter {
public:
    /** Create a tile image filter
        @param src  Defines the pixels to tile
        @param dst  Defines the pixels where tiles are drawn
        @param input    Input from which the subregion defined by srcRect will be tiled
    */
    static sk_sp<SkImageFilter> Make(const SkRect& src,
                                     const SkRect& dst,
                                     sk_sp<SkImageFilter> input);

    static void RegisterFlattenables();

private:
    SkTileImageFilter() = delete;
};

#endif
