/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBBHFactory_DEFINED
#define SkBBHFactory_DEFINED

#include "SkSize.h"
#include "SkRect.h"

class SkBBoxHierarchy;

class SK_API SkBBHFactory {
public:
    /**
     *  Allocate a new SkBBoxHierarchy. Return NULL on failure.
     */
    virtual SkBBoxHierarchy* operator()(const SkRect& bounds) const = 0;
    virtual ~SkBBHFactory() {};
};

class SK_API SkRTreeFactory : public SkBBHFactory {
public:
    virtual SkBBoxHierarchy* operator()(const SkRect& bounds) const SK_OVERRIDE;
private:
    typedef SkBBHFactory INHERITED;
};

// TODO(mtklein): delete entirely when Chromium's clean for TileGridInfo
class SK_API SkTileGridFactory {
public:
    struct TileGridInfo {
        /** Tile placement interval */
        SkISize  fTileInterval;

        /** Pixel coverage overlap between adjacent tiles */
        SkISize  fMargin;

        /** Offset added to device-space bounding box positions to convert
          * them to tile-grid space. This can be used to adjust the "phase"
          * of the tile grid to match probable query rectangles that will be
          * used to search into the tile grid. As long as the offset is smaller
          * or equal to the margin, there is no need to extend the domain of
          * the tile grid to prevent data loss.
          */
        SkIPoint fOffset;
    };
};

#endif
